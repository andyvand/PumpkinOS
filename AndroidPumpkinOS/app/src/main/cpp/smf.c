#include <stdlib.h>
#include <string.h>

#include "sys.h"
#include "debug.h"
#include "smf.h"

#define SMF_DEFAULT_TEMPO 500000  // microseconds per quarter note (120 bpm)

typedef struct {
  uint32_t tick;
  uint32_t seq;      // parse order, keeps the sort stable
  uint8_t status;    // 0x80-0xEF channel message, 0xF0 sysex, 0xFF meta
  uint8_t d1, d2;    // channel message data bytes
  uint8_t type;      // meta type
  uint32_t offset;   // offset of sysex/meta payload in smf->data
  uint32_t len;      // payload length
} smf_event_t;

struct smf_t {
  uint8_t *data;
  uint32_t size;
  uint16_t format, ntracks, division;
  smf_event_t *events;
  uint32_t nevents, capacity;
};

static uint32_t get32(const uint8_t *p) {
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

static uint16_t get16(const uint8_t *p) {
  return (uint16_t)((p[0] << 8) | p[1]);
}

static int read_vlq(const uint8_t *p, uint32_t end, uint32_t *pos, uint32_t *value) {
  uint32_t v = 0;
  int i;

  for (i = 0; i < 4 && *pos < end; i++) {
    uint8_t b = p[(*pos)++];
    v = (v << 7) | (b & 0x7F);
    if (!(b & 0x80)) {
      *value = v;
      return 0;
    }
  }

  return -1;
}

static int add_event(smf_t *smf, smf_event_t *ev) {
  smf_event_t *events;
  uint32_t capacity;

  if (smf->nevents == smf->capacity) {
    capacity = smf->capacity ? smf->capacity * 2 : 256;
    if ((events = sys_realloc(smf->events, capacity * sizeof(smf_event_t))) == NULL) {
      return -1;
    }
    smf->events = events;
    smf->capacity = capacity;
  }

  ev->seq = smf->nevents;
  smf->events[smf->nevents++] = *ev;

  return 0;
}

static int channel_msg_len(uint8_t status) {
  switch (status & 0xF0) {
    case 0xC0:
    case 0xD0:
      return 2;
    case 0x80:
    case 0x90:
    case 0xA0:
    case 0xB0:
    case 0xE0:
      return 3;
  }
  return 0;
}

static int parse_track(smf_t *smf, uint32_t start, uint32_t end) {
  smf_event_t ev;
  uint32_t pos, delta, tick, len;
  uint8_t status, running, b;

  pos = start;
  tick = 0;
  running = 0;

  while (pos < end) {
    if (read_vlq(smf->data, end, &pos, &delta) != 0) return -1;
    tick += delta;
    if (pos >= end) return -1;

    b = smf->data[pos];
    if (b & 0x80) {
      status = b;
      pos++;
    } else {
      // running status: reuse the previous channel status byte
      if (running == 0) return -1;
      status = running;
    }

    sys_memset(&ev, 0, sizeof(ev));
    ev.tick = tick;
    ev.status = status;

    if (status == 0xFF) {
      // meta event
      if (pos >= end) return -1;
      ev.type = smf->data[pos++];
      if (read_vlq(smf->data, end, &pos, &len) != 0) return -1;
      if (pos + len > end) return -1;
      ev.offset = pos;
      ev.len = len;
      pos += len;
      if (ev.type == 0x2F) {
        // end of track
        break;
      }
      if (add_event(smf, &ev) != 0) return -1;
      running = 0;

    } else if (status == 0xF0 || status == 0xF7) {
      // sysex (0xF0) or escape (0xF7); the payload of 0xF0 normally ends in 0xF7
      if (read_vlq(smf->data, end, &pos, &len) != 0) return -1;
      if (pos + len > end) return -1;
      ev.offset = pos;
      ev.len = len;
      pos += len;
      if (status == 0xF0) {
        if (add_event(smf, &ev) != 0) return -1;
      }
      running = 0;

    } else if (status >= 0x80 && status <= 0xEF) {
      int n = channel_msg_len(status);
      if (pos + (n - 1) > end) return -1;
      ev.d1 = smf->data[pos++] & 0x7F;
      if (n == 3) ev.d2 = smf->data[pos++] & 0x7F;
      if (add_event(smf, &ev) != 0) return -1;
      running = status;

    } else {
      // system common messages are not expected inside a SMF; skip their data
      switch (status) {
        case 0xF1: case 0xF3: pos += 1; break;
        case 0xF2: pos += 2; break;
        default: break;
      }
      running = 0;
    }
  }

  return 0;
}

static int compare_events(const void *a, const void *b) {
  const smf_event_t *ea = (const smf_event_t *)a;
  const smf_event_t *eb = (const smf_event_t *)b;

  if (ea->tick != eb->tick) return ea->tick < eb->tick ? -1 : 1;
  if (ea->seq != eb->seq) return ea->seq < eb->seq ? -1 : 1;
  return 0;
}

smf_t *smf_load(const uint8_t *buf, uint32_t len) {
  smf_t *smf;
  uint32_t pos, id, clen, i;

  if (buf == NULL || len < 14) {
    debug(DEBUG_ERROR, "SMF", "buffer too small (%u)", len);
    return NULL;
  }

  if ((smf = sys_calloc(1, sizeof(smf_t))) == NULL) {
    return NULL;
  }

  if ((smf->data = sys_calloc(1, len)) == NULL) {
    sys_free(smf);
    return NULL;
  }
  sys_memcpy(smf->data, buf, len);
  smf->size = len;

  pos = 0;
  id = get32(&smf->data[pos]); pos += 4;
  clen = get32(&smf->data[pos]); pos += 4;

  if (id != 0x4D546864 /* MThd */ || clen < 6 || pos + clen > len) {
    debug(DEBUG_ERROR, "SMF", "invalid header id 0x%08X length %u", id, clen);
    smf_free(smf);
    return NULL;
  }

  smf->format = get16(&smf->data[pos]);
  smf->ntracks = get16(&smf->data[pos + 2]);
  smf->division = get16(&smf->data[pos + 4]);
  pos += clen;

  if (smf->format > 1) {
    debug(DEBUG_ERROR, "SMF", "unsupported format %u", smf->format);
  }

  for (i = 0; i < smf->ntracks && pos + 8 <= len; i++) {
    id = get32(&smf->data[pos]); pos += 4;
    clen = get32(&smf->data[pos]); pos += 4;
    if (pos + clen > len) {
      debug(DEBUG_ERROR, "SMF", "track %u overflows file (%u + %u > %u)", i, pos, clen, len);
      clen = len - pos;
    }
    if (id == 0x4D54726B /* MTrk */) {
      if (parse_track(smf, pos, pos + clen) != 0) {
        debug(DEBUG_ERROR, "SMF", "track %u is malformed; using events parsed so far", i);
      }
    } else {
      debug(DEBUG_INFO, "SMF", "skipping chunk 0x%08X", id);
    }
    pos += clen;
  }

  if (smf->nevents > 1) {
    qsort(smf->events, smf->nevents, sizeof(smf_event_t), compare_events);
  }

  debug(DEBUG_INFO, "SMF", "format %u, %u track(s), division 0x%04X, %u event(s)", smf->format, smf->ntracks, smf->division, smf->nevents);

  return smf;
}

void smf_free(smf_t *smf) {
  if (smf) {
    if (smf->events) sys_free(smf->events);
    if (smf->data) sys_free(smf->data);
    sys_free(smf);
  }
}

uint32_t smf_num_events(smf_t *smf) {
  return smf ? smf->nevents : 0;
}

// microseconds per tick for the current tempo
static double us_per_tick(smf_t *smf, uint32_t tempo) {
  if (smf->division & 0x8000) {
    // SMPTE: frames per second (negative, two's complement) and ticks per frame
    int fps = -((int8_t)(smf->division >> 8));
    int tpf = smf->division & 0xFF;
    if (fps <= 0 || tpf <= 0) return 1000.0;
    return 1000000.0 / ((double)fps * (double)tpf);
  }
  if (smf->division == 0) return 1000.0;
  return (double)tempo / (double)smf->division;
}

static uint32_t tempo_of(smf_t *smf, smf_event_t *ev) {
  uint8_t *p = &smf->data[ev->offset];
  return ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | p[2];
}

uint64_t smf_duration(smf_t *smf) {
  double pos_us = 0.0, upt;
  uint32_t i, tempo, last_tick = 0;
  smf_event_t *ev;

  if (smf == NULL) return 0;

  tempo = SMF_DEFAULT_TEMPO;
  upt = us_per_tick(smf, tempo);

  for (i = 0; i < smf->nevents; i++) {
    ev = &smf->events[i];
    pos_us += (double)(ev->tick - last_tick) * upt;
    last_tick = ev->tick;
    if (ev->status == 0xFF && ev->type == 0x51 && ev->len == 3) {
      tempo = tempo_of(smf, ev);
      upt = us_per_tick(smf, tempo);
    }
  }

  return (uint64_t)pos_us;
}

int smf_play(smf_t *smf, smf_event_f event, smf_wait_f wait, void *data) {
  double pos_us = 0.0, sent_us = 0.0, upt;
  uint32_t i, tempo, last_tick = 0, delay;
  uint8_t msg[3];
  uint8_t *sysex;
  smf_event_t *ev;
  int n, r;

  if (smf == NULL || event == NULL || wait == NULL) return -1;

  tempo = SMF_DEFAULT_TEMPO;
  upt = us_per_tick(smf, tempo);

  for (i = 0; i < smf->nevents; i++) {
    ev = &smf->events[i];
    pos_us += (double)(ev->tick - last_tick) * upt;
    last_tick = ev->tick;

    // let the accumulated time elapse (fractional microseconds are carried over)
    if (pos_us - sent_us >= 1.0) {
      delay = (uint32_t)(pos_us - sent_us);
      sent_us += delay;
      if ((r = wait(delay, data)) != 0) return 1;
    }

    if (ev->status == 0xFF) {
      if (ev->type == 0x51 && ev->len == 3) {
        tempo = tempo_of(smf, ev);
        upt = us_per_tick(smf, tempo);
        debug(DEBUG_TRACE, "SMF", "tempo %u us/quarter", tempo);
      }
      continue;
    }

    if (ev->status == 0xF0) {
      // rebuild a complete SysEx message: F0 + payload (payload usually ends with F7)
      if ((sysex = sys_calloc(1, ev->len + 2)) == NULL) return -1;
      sysex[0] = 0xF0;
      sys_memcpy(&sysex[1], &smf->data[ev->offset], ev->len);
      n = ev->len + 1;
      if (n == 1 || sysex[n - 1] != 0xF7) {
        sysex[n++] = 0xF7;
      }
      r = event(sysex, n, data);
      sys_free(sysex);
      if (r != 0) return 1;
      continue;
    }

    n = channel_msg_len(ev->status);
    if (n == 0) continue;
    msg[0] = ev->status;
    msg[1] = ev->d1;
    msg[2] = ev->d2;
    if ((r = event(msg, n, data)) != 0) return 1;
  }

  return 0;
}

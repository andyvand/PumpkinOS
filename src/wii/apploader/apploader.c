// A minimal Wii disc apploader for the PumpkinOS disc image (see
// ../GNUMakefile, target "wbfs", and mkdisc.py).
//
// The console (or a USB loader, or Dolphin) reads sys/apploader.img from
// partition offset 0x2440: a 32 byte header (date, entry point, size,
// trailer size) and this code, loaded at 0x81200000. It calls the entry
// point, which hands back three functions:
//
//   init(report)          once, with a printf-like report function
//   main(&dst, &len, &off) repeatedly: each call that returns non-zero asks
//                         for len bytes at partition offset off (a word
//                         offset, i.e. bytes >> 2, on the Wii) to be read
//                         to dst before the next call
//   close()               once main returned 0; returns the entry point of
//                         the program
//
// This one reads the boot.bin fields at 0x420 (the DOL offset), the DOL
// header, then every text and data section of sys/main.dol in ascending
// address order (reads are rounded up to 32 bytes, so a section's tail may
// spill over the start of the next one, which is read afterwards), clears
// the BSS and makes the instruction cache see the new code. The FST is not
// loaded: the program (libogc) does not use it.
//
// Built freestanding with devkitPPC (no libogc): see apploader.ld.

typedef unsigned int u32;
typedef unsigned char u8;

typedef void (*report_f)(const char *fmt, ...);
typedef void (*init_f)(report_f report);
typedef int (*main_f)(void **dst, u32 *len, u32 *off);
typedef void *(*close_f)(void);

#define NSECTIONS 18  // 7 text + 11 data
#define NTEXT     7

typedef struct {
  u32 offset[NSECTIONS];
  u32 address[NSECTIONS];
  u32 size[NSECTIONS];
  u32 bss_address;
  u32 bss_size;
  u32 entry;
  u32 pad[7];
} dol_header_t;

// read targets must be 32 byte aligned (DMA)
static u32 boot_info[8] __attribute__((aligned(32)));
static dol_header_t dol __attribute__((aligned(32)));

static report_f report;
static int state;
static u32 dol_offset;  // word offset of main.dol in the partition
static int order[NSECTIONS];
static int nsections, current;

static void nop_report(const char *fmt, ...) {
}

static void sort_sections(void) {
  int i, j;

  nsections = 0;
  for (i = 0; i < NSECTIONS; i++) {
    if (dol.size[i] == 0 || dol.address[i] == 0) continue;
    // insertion sort on the load address
    for (j = nsections; j > 0 && dol.address[order[j-1]] > dol.address[i]; j--) {
      order[j] = order[j-1];
    }
    order[j] = i;
    nsections++;
  }
}

static void apl_init(report_f r) {
  report = r ? r : nop_report;
  state = 0;
  current = 0;
  report("PumpkinOS apploader\n");
}

static int apl_main(void **dst, u32 *len, u32 *off) {
  int i;

  switch (state) {
    case 0:
      // boot.bin 0x420: DOL offset, FST offset, FST size, FST max size
      *dst = boot_info;
      *len = sizeof(boot_info);
      *off = 0x420 >> 2;
      state = 1;
      return 1;
    case 1:
      dol_offset = boot_info[0];
      *dst = &dol;
      *len = sizeof(dol);
      *off = dol_offset;
      state = 2;
      return 1;
    case 2:
      sort_sections();
      current = 0;
      state = 3;
      // fall through
    case 3:
      if (current >= nsections) {
        state = 4;
        return 0;
      }
      i = order[current++];
      *dst = (void *)dol.address[i];
      *len = (dol.size[i] + 31) & ~31;
      *off = dol_offset + (dol.offset[i] >> 2);
      return 1;
  }

  return 0;
}

static void flush_range(u32 start, u32 size, int icache) {
  u32 a, end = start + size;

  for (a = start & ~31; a < end; a += 32) {
    __asm__ volatile ("dcbf 0,%0" : : "r"(a) : "memory");
  }
  __asm__ volatile ("sync" : : : "memory");
  if (icache) {
    for (a = start & ~31; a < end; a += 32) {
      __asm__ volatile ("icbi 0,%0" : : "r"(a) : "memory");
    }
    __asm__ volatile ("sync; isync" : : : "memory");
  }
}

static void *apl_close(void) {
  u8 *p;
  u32 n;
  int i;

  // the BSS (also clears whatever a rounded up read spilled into it)
  p = (u8 *)dol.bss_address;
  for (n = 0; n < dol.bss_size; n++) p[n] = 0;
  if (dol.bss_size) flush_range(dol.bss_address, dol.bss_size, 0);

  for (i = 0; i < NSECTIONS; i++) {
    if (dol.size[i] == 0 || dol.address[i] == 0) continue;
    flush_range(dol.address[i], dol.size[i], i < NTEXT);
  }

  report("PumpkinOS apploader: entry %08x\n", dol.entry);

  return (void *)dol.entry;
}

// placed first (see apploader.ld): the header's entry point is 0x81200000
__attribute__((section(".text.entry")))
void apploader_entry(init_f *init, main_f *main, close_f *close) {
  *init = apl_init;
  *main = apl_main;
  *close = apl_close;
}

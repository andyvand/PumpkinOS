// Add-on content (DLC) access for the Nintendo Switch port.
//
// libnx has no wrapper for the aoc:u service (nn::aoc::IAddOnContentManager),
// so the four commands needed here are sent directly:
//
//   2 CountAddOnContent(pid)                     -> u32 count
//   3 ListAddOnContent(offset, count, pid)       -> u32 total + u32 indices[]
//   5 GetAddOnContentBaseId(pid)                 -> u64 base id
//   7 PrepareAddOnContent(index, pid)
//
// The data ID of an add-on content is the base ID plus its index; its RomFS
// is opened with fsp-srv OpenDataStorageByDataId and mounted as a romfs
// device (see switch_dlc_mount).

#include <string.h>
#include <assert.h>

// the libnx dispatch macros use C11 static_assert, the port builds as C99
#ifndef static_assert
#define static_assert _Static_assert
#endif

#include <switch/types.h>
#include <switch/result.h>
#include <switch/sf/service.h>
#include <switch/services/sm.h>
#include <switch/services/fs.h>
#include <switch/services/ncm_types.h>
#include <switch/runtime/devices/romfs_dev.h>

#include "aoc.h"

static Service aoc;
static u64 aoc_base;

int switch_dlc_init(void) {
  return R_SUCCEEDED(smGetService(&aoc, "aoc:u")) ? 0 : -1;
}

void switch_dlc_exit(void) {
  serviceClose(&aoc);
}

int switch_dlc_list(uint64_t app_id, uint64_t *ids, int max) {
  u64 pid = 0, base = 0;
  u32 count = 0, total = 0, indices[SWITCH_DLC_MAX];
  struct {
    u32 offset;
    u32 count;
    u64 pid;
  } in;
  int i, n;

  if (!serviceIsActive(&aoc)) return -1;
  if (R_FAILED(serviceDispatchInOut(&aoc, 2, pid, count, .in_send_pid = true))) return -1;
  if (count == 0) return 0;
  if (R_FAILED(serviceDispatchInOut(&aoc, 5, pid, base, .in_send_pid = true))) return -1;
  if (base != app_id + 0x1000) return -2;
  aoc_base = base;

  n = count < SWITCH_DLC_MAX ? (int)count : SWITCH_DLC_MAX;
  if (n > max) n = max;
  in.offset = 0;
  in.count = n;
  in.pid = 0;
  if (R_FAILED(serviceDispatchInOut(&aoc, 3, in, total,
        .in_send_pid = true,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
        .buffers = { { indices, n * sizeof(u32) } }))) {
    return -1;
  }
  if ((int)total < n) n = total;

  for (i = 0; i < n; i++) {
    ids[i] = base + indices[i];
  }

  return n;
}

int switch_dlc_mount(uint64_t id, const char *name) {
  static const NcmStorageId storages[] = {
    NcmStorageId_SdCard, NcmStorageId_BuiltInUser, NcmStorageId_GameCard, NcmStorageId_Any
  };
  struct {
    u32 index;
    u32 pad;
    u64 pid;
  } in;
  FsStorage storage;
  unsigned int i;

  // make the content available (as nn::aoc::PrepareAddOnContent does)
  in.index = (u32)(id - aoc_base);
  in.pad = 0;
  in.pid = 0;
  serviceDispatchIn(&aoc, 7, in, .in_send_pid = true);

  for (i = 0; i < sizeof(storages) / sizeof(storages[0]); i++) {
    if (R_SUCCEEDED(fsOpenDataStorageByDataId(&storage, id, storages[i]))) {
      if (R_SUCCEEDED(romfsMountFromStorage(storage, 0, name))) {
        return 0;
      }
      fsStorageClose(&storage);
    }
  }

  return -1;
}

void switch_dlc_unmount(const char *name) {
  romfsUnmount(name);
}

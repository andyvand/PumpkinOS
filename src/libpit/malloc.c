#include "sys.h"

#if defined(KERNEL)

#include "custom_malloc.h"

#define HEAP_SIZE 256 * 1024 * 1024
static uint8_t heap_buffer[HEAP_SIZE];
static HEAP_INFO_t heap;

void malloc_init(void) {
  heap.pHeap = heap_buffer;
  heap.heapSize = HEAP_SIZE;
  CustomMallocInit(&heap);
}

void *sys_malloc(sys_size_t size) {
  return CustomMalloc(&heap, size);
}

void sys_free(void *ptr) {
  CustomFree(&heap, ptr);
}

void *sys_calloc(sys_size_t nmemb, sys_size_t size) {
  void *p = sys_malloc(nmemb * size);
  sys_memset(p, 0, nmemb * size);
  return p;
}

void *sys_realloc(void *ptr, sys_size_t size) {
  uint32_t oldSize;
  void *p = NULL;

  if (ptr) {
    p = CustomMalloc(&heap, size);
    if (p) {
      oldSize = CustomBlockSize(&heap, ptr);
      sys_memcpy(p, ptr, size < oldSize ? size : oldSize);
    }
    CustomFree(&heap, ptr);
  }

  return p;
}

#else
#ifdef __MINGW32__
#define _abs64 absm
#endif

#ifdef ESP32
#include "esp_heap_caps.h"
#else
#include <stdlib.h>
#endif

void *sys_malloc(sys_size_t size) {
#ifdef ESP32
  return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
  return malloc(size);
#endif
}

void sys_free(void *ptr) {
#ifdef ESP32
  heap_caps_free(ptr);
#else
  free(ptr);
#endif
}

void *sys_calloc(sys_size_t nmemb, sys_size_t size) {
#ifdef ESP322
  return heap_caps_calloc(size, nmemb, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
  return calloc(nmemb, size);
#endif
}

void *sys_realloc(void *ptr, sys_size_t size) {
#ifdef ESP32
  return heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
  return realloc(ptr, size);
#endif
}
#endif

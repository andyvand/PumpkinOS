/*
 * ESP32-S3 ELF MMU mapping (IDF 6.x compatible)
 */

#include <sys/errno.h>
#include <string.h>
#include "esp_attr.h"
#include "esp_heap_caps.h"
#include "esp_mmu_map.h"
#include "esp_log.h"
#include "private/elf_platform.h"

#define TAG "ELF-S3-MMU"

/**
 * @brief Initialize MMU mapping for .text section (PSRAM → IBUS)
 */
int IRAM_ATTR esp_elf_arch_init_mmu(esp_elf_t *elf)
{
    if (!elf || !elf->ptext)
        return -EINVAL;

    esp_elf_sec_t *sec = &elf->sec[ELF_SEC_TEXT];
    size_t page_size = CONFIG_MMU_PAGE_SIZE;
    size_t map_size = (sec->size + page_size - 1) & ~(page_size - 1);

    ESP_LOGI(TAG, "map .text: %p (%u bytes)", elf->ptext, (unsigned)map_size);

    void *vaddr = NULL;

    esp_err_t err = esp_mmu_map(
        (esp_paddr_t)elf->ptext,                 // source pointer (PSRAM virtual)
        map_size,
        MMU_TARGET_PSRAM0,
        MMU_MEM_CAP_EXEC | MMU_MEM_CAP_READ,
        0,
        &vaddr
    );

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_mmu_map failed: %s", esp_err_to_name(err));
        return -EIO;
    }

    /* Store mapping info */
    elf->mapped_addr = (uint32_t)vaddr;
    elf->mapped_size = map_size;

    /*
     * Calculate relocation offset:
     * mapped_addr - original virtual address
     */
    elf->text_off = (uint32_t)((uintptr_t)vaddr - (uintptr_t)elf->ptext);

    ESP_LOGI(TAG,
             ".text mapped: %p -> %p (%u bytes, 0x%x offset)",
             elf->ptext,
             vaddr,
             (unsigned)map_size,
             (unsigned)elf->text_off);

    return 0;
}

/**
 * @brief Remove MMU mapping
 */
void IRAM_ATTR esp_elf_arch_deinit_mmu(esp_elf_t *elf)
{
    if (!elf)
        return;

    if (elf->mapped_addr) {
        esp_mmu_unmap((void *)elf->mapped_addr);
        elf->mapped_addr = 0;
        elf->mapped_size = 0;
        elf->text_off = 0;
    }
}

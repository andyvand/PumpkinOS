/*
 * SPDX-FileCopyrightText: 2023-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <reent.h>
#include <pthread.h>
#include <setjmp.h>
#include <getopt.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/select.h> /* Voor select */
#include <sys/stat.h>   /* Voor stat, fstat, mkdir */
#include <arpa/inet.h>
#include <ctype.h>
#include <esp_dlfcn.h>
#include <netdb.h>
#include <signal.h>
#include <dirent.h>     /* Voor opendir, readdir, closedir */

#include "lwip/sockets.h"
#include "lwip/netdb.h"

#include "rom/ets_sys.h"

#include "esp_elf.h"

#include "private/elf_pit_symbols.h"
#include "private/elf_pumpkin_symbols.h"

extern int __ltdf2(double a, double b);
extern unsigned int __fixunsdfsi(double a);
extern int __gtdf2(double a, double b);
extern float __modsi3 (float a, float b);
extern float __divsf3 (float a, float b);
extern float __addsf3 (float a, float b);
extern float __subsf3 (float a, float b);
extern float __mulsf3 (float a, float b);
extern float __divsf3 (float a, float b);
extern float __udivsi3 (float a, float b);
extern float __divsi3 (float a, float b);
extern double __moddi3 (double a, double b);
extern double __floatunsidf(unsigned int i);
extern double __divdf3(double a, double b);
extern double __adddf3 (double a, double b);
extern double __subdf3 (double a, double b);
extern double __muldf3 (double a, double b);
extern double __divdf3 (double a, double b);
extern double __udivdi3 (double a, double b);
extern double __divdi3 (double a, double b);
extern double __floatsidf (double a);
extern double __truncdfsf2 (double a);
extern double __extendsfdf2 (double a);
extern double __fixdfsi (double a);
extern double __floatundidf (double a);
extern double __floatdidf (double a);
extern double __umoddi3 (double a, double b);
extern int __ledf2 (double a, double b);
extern int __gedf2 (double a, double b);
extern int __unorddf2 (double a, double b);
extern int __nedf2 (double a, double b);
extern int __eqdf2 (double a, double b);
extern unsigned int __fixunssfdi (float a);

#if CONFIG_ELF_DYNAMIC_LOAD_SHARED_OBJECT
#include "private/esp_dlmod.h"
#endif
#include "private/elf_symbol.h"

static const struct esp_elfsym g_esp_libc_elfsyms[] = {

    /* string.h */

    ESP_ELFSYM_EXPORT(strerror),
    ESP_ELFSYM_EXPORT(memset),
    ESP_ELFSYM_EXPORT(memcpy),
    ESP_ELFSYM_EXPORT(strlen),
    ESP_ELFSYM_EXPORT(strtod),
    ESP_ELFSYM_EXPORT(strrchr),
    ESP_ELFSYM_EXPORT(strchr),
    ESP_ELFSYM_EXPORT(strcmp),
    ESP_ELFSYM_EXPORT(strtol),
    ESP_ELFSYM_EXPORT(strcspn),
    ESP_ELFSYM_EXPORT(strncat),

    /* stdio.h */

    ESP_ELFSYM_EXPORT(puts),
    ESP_ELFSYM_EXPORT(putchar),
    ESP_ELFSYM_EXPORT(fputc),
    ESP_ELFSYM_EXPORT(fputs),
    ESP_ELFSYM_EXPORT(printf),
    ESP_ELFSYM_EXPORT(vfprintf),
    ESP_ELFSYM_EXPORT(fprintf),
    ESP_ELFSYM_EXPORT(fwrite),

    /* unistd.h */

    ESP_ELFSYM_EXPORT(usleep),
    ESP_ELFSYM_EXPORT(sleep),
    ESP_ELFSYM_EXPORT(exit),
    ESP_ELFSYM_EXPORT(close),

    /* stdlib.h */

    ESP_ELFSYM_EXPORT(malloc),
    ESP_ELFSYM_EXPORT(calloc),
    ESP_ELFSYM_EXPORT(realloc),
    ESP_ELFSYM_EXPORT(free),

    /* time.h */

    ESP_ELFSYM_EXPORT(clock_gettime),
    ESP_ELFSYM_EXPORT(strftime),

    /* pthread.h */

    ESP_ELFSYM_EXPORT(pthread_create),
    ESP_ELFSYM_EXPORT(pthread_attr_init),
    ESP_ELFSYM_EXPORT(pthread_attr_setstacksize),
    ESP_ELFSYM_EXPORT(pthread_detach),
    ESP_ELFSYM_EXPORT(pthread_join),
    ESP_ELFSYM_EXPORT(pthread_exit),

    /* newlib */

#if !CONFIG_LIBC_PICOLIBC
    ESP_ELFSYM_EXPORT(__errno),
    ESP_ELFSYM_EXPORT(__getreent),

#if defined(__HAVE_LOCALE_INFO__)
    ESP_ELFSYM_EXPORT(__locale_ctype_ptr),
#else
    ESP_ELFSYM_EXPORT(_ctype_),
#endif
#endif

    /* math */

    ESP_ELFSYM_EXPORT(__ltdf2),
    ESP_ELFSYM_EXPORT(__fixunsdfsi),
    ESP_ELFSYM_EXPORT(__gtdf2),
    ESP_ELFSYM_EXPORT(__floatunsidf),
    ESP_ELFSYM_EXPORT(__divdf3),

    /* getopt.h */

    ESP_ELFSYM_EXPORT(getopt_long),
    ESP_ELFSYM_EXPORT(optind),
    ESP_ELFSYM_EXPORT(opterr),
    ESP_ELFSYM_EXPORT(optarg),
    ESP_ELFSYM_EXPORT(optopt),

    /* setjmp.h */

    ESP_ELFSYM_EXPORT(longjmp),
    ESP_ELFSYM_EXPORT(setjmp),

    ESP_ELFSYM_END
};

/** @brief ESP-IDF public functions symbols look-up table */

static const struct esp_elfsym g_esp_espidf_elfsyms[] = {

    /* sys/socket.h */

    ESP_ELFSYM_EXPORT(lwip_bind),
    ESP_ELFSYM_EXPORT(lwip_setsockopt),
    ESP_ELFSYM_EXPORT(lwip_socket),
    ESP_ELFSYM_EXPORT(lwip_listen),
    ESP_ELFSYM_EXPORT(lwip_accept),
    ESP_ELFSYM_EXPORT(lwip_recv),
    ESP_ELFSYM_EXPORT(lwip_recvfrom),
    ESP_ELFSYM_EXPORT(lwip_send),
    ESP_ELFSYM_EXPORT(lwip_sendto),
    ESP_ELFSYM_EXPORT(lwip_connect),

    /* arpa/inet.h */

    ESP_ELFSYM_EXPORT(ipaddr_addr),
    ESP_ELFSYM_EXPORT(lwip_htons),
    ESP_ELFSYM_EXPORT(lwip_htonl),
    ESP_ELFSYM_EXPORT(ip4addr_ntoa),

    /* ROM functions */

    ESP_ELFSYM_EXPORT(ets_printf),

    ESP_ELFSYM_END
};

/**
 * @brief Find symbol address by name.
 *
 * @param sym_name - Symbol name
 *
 * @return Symbol address if success or 0 if failed.
 */
uintptr_t elf_find_sym(const char *sym_name)
{
    esp_elf_symbol_table_t *syms;

#ifdef CONFIG_ELF_LOADER_LIBC_SYMBOLS
    syms = g_esp_libc_elfsyms;
    while (syms->name) {
        if (!strcmp(syms->name, sym_name)) {
            return (uintptr_t)syms->sym;
        }

        syms++;
    }
#else
    syms = g_esp_libc_elfsyms;
    (void)syms;
#endif

#ifdef CONFIG_ELF_LOADER_ESPIDF_SYMBOLS
    syms = g_esp_espidf_elfsyms;
    while (syms->name) {
        if (!strcmp(syms->name, sym_name)) {
            return (uintptr_t)syms->sym;
        }

        syms++;
    }
#else
    syms = g_esp_espidf_elfsyms;
    (void)syms;
#endif

    syms = g_PitSyms_elfsyms;
    while (syms->name) {
        if (!strcmp(syms->name, sym_name)) {
            return (uintptr_t)syms->sym;
        }

        syms++;
    }

    syms = g_PumpkinSyms_elfsyms;
    while (syms->name) {
        if (!strcmp(syms->name, sym_name)) {
            return (uintptr_t)syms->sym;
        }

        syms++;
    }

#ifdef CONFIG_ELF_LOADER_CUSTOMER_SYMBOLS
    extern const struct esp_elfsym g_customer_elfsyms[];

    syms = g_customer_elfsyms;
    while (syms->name) {
        if (!strcmp(syms->name, sym_name)) {
            return (uintptr_t)syms->sym;
        }

        syms++;
    }

#endif

    uintptr_t sym_addr = esp_elf_find_symbol(sym_name);
    if (sym_addr) {
        return sym_addr;
    }

#if CONFIG_ELF_DYNAMIC_LOAD_SHARED_OBJECT
    return (uintptr_t)dlmod_getaddr(sym_name);
#else
    return 0;
#endif
}

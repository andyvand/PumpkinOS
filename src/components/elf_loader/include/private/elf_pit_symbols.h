/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>

#include "private/elf_symbol.h"

/* Extern declarations from ELF symbol table */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"
extern int script_add_boolean;
extern int script_add_function;
extern int script_add_iconst;
extern int script_add_sconst;
extern int script_call;
extern int script_call_args;
extern int script_create;
extern int script_create_function;
extern int script_create_function_data;
extern int script_create_object;
extern int script_destroy;
extern int script_engine_id;
extern int script_get_boolean;
extern int script_get_engine;
extern int script_get_function;
extern int script_get_integer;
extern int script_get_last_error;
extern int script_get_lstring;
extern int script_get_named_value;
extern int script_get_object;
extern int script_get_pointer;
extern int script_get_string;
extern int script_global_get;
extern int script_global_set;
extern int script_loadlib;
extern int script_object_get;
extern int script_object_set;
extern int script_opt_boolean;
extern int script_opt_integer;
extern int script_opt_string;
extern int script_push_boolean;
extern int script_push_integer;
extern int script_push_lstring;
extern int script_push_object;
extern int script_push_string;
extern int script_remove_ref;
extern int script_returned_value;
extern int script_run;
extern int script_set_pointer;
extern int pit_main;
extern int signal_install_handlers;
extern int sys_abs;
extern int sys_block_signals;
extern int sys_chdir;
extern int sys_close;
extern int sys_closedir;
extern int sys_country;
extern int sys_create;
extern int sys_daemonize;
extern int sys_errno;
extern int sys_exit;
extern int sys_fabs;
extern int sys_fabsl;
extern int sys_fdclr;
extern int sys_fdisset;
extern int sys_fdset;
extern int sys_fdzero;
extern int sys_fork_exec;
extern int sys_fstat;
extern int sys_get_clock;
extern int sys_get_clock_ts;
extern int sys_get_pid;
extern int sys_get_process_time;
extern int sys_get_thread_time;
extern int sys_get_tid;
extern int sys_getcwd;
extern int sys_getenv;
extern int sys_getpagesize;
extern int sys_install_handler;
extern int sys_isatty;
extern int sys_language;
extern int sys_lib_close;
extern int sys_lib_defsymbol;
extern int sys_lib_load;
extern int sys_longjmp;
extern int sys_mkdir;
extern int sys_mkstemp;
extern int sys_mkstempfile;
extern int sys_open;
extern int sys_opendir;
extern int sys_peek;
extern int sys_pipe;
extern int sys_qsort;
extern int sys_rand;
extern int sys_read;
extern int sys_read_timeout;
extern int sys_readdir;
extern int sys_rename;
extern int sys_rewinddir;
extern int sys_rmdir;
extern int sys_seek;
extern int sys_select;
extern int sys_select_fds;
extern int sys_serial_baud;
extern int sys_serial_open;
extern int sys_serial_word;
extern int sys_set_finish;
extern int sys_set_thread_name;
extern int sys_setenv;
extern int sys_setjmp;
extern int sys_setsockopt;
extern int sys_socket_accept;
extern int sys_socket_bind;
extern int sys_socket_bind_connect;
extern int sys_socket_binds;
extern int sys_socket_connect;
extern int sys_socket_fill_addr;
extern int sys_socket_ipv4;
extern int sys_socket_listen;
extern int sys_socket_open;
extern int sys_socket_open_connect;
extern int sys_socket_open_connect_timeout;
extern int sys_socket_recvfrom;
extern int sys_socket_sendto;
extern int sys_socket_shutdown;
extern int sys_srand;
extern int sys_sscanf;
extern int sys_stat;
extern int sys_statfs;
extern int sys_strerror;
extern int sys_strtod;
extern int sys_termsize;
extern int sys_time;
extern int sys_truncate;
extern int sys_tty_raw;
extern int sys_tty_restore;
extern int sys_unblock_signals;
extern int sys_unlink;
extern int sys_usleep;
extern int sys_vsnprintf;
extern int sys_vsprintf;
extern int sys_wait;
extern int sys_write;
extern int sys_acos;
extern int sys_asin;
extern int sys_atan;
extern int sys_atan2;
extern int sys_ceil;
extern int sys_cos;
extern int sys_cosh;
extern int sys_exp;
extern int sys_floor;
extern int sys_fmodl;
extern int sys_isinf;
extern int sys_isnan;
extern int sys_log;
extern int sys_log10;
extern int sys_modf;
extern int sys_pi;
extern int sys_pow;
extern int sys_signbit;
extern int sys_sin;
extern int sys_sinh;
extern int sys_sqrt;
extern int sys_tan;
extern int sys_tanh;
extern int sys_calloc;
extern int sys_free;
extern int sys_malloc;
extern int sys_realloc;
extern int sys_strtol;
extern int sys_strtoul;
extern int sys_isalnum;
extern int sys_isalpha;
extern int sys_isblank;
extern int sys_iscntrl;
extern int sys_isdigit;
extern int sys_isgraph;
extern int sys_islower;
extern int sys_isprint;
extern int sys_ispunct;
extern int sys_isspace;
extern int sys_isupper;
extern int sys_isxdigit;
extern int sys_tolower;
extern int sys_toupper;
extern int sys_bsearch;
extern int sys_snprintf;
extern int sys_sprintf;
extern int sys_atof;
extern int sys_atoi;
extern int sys_memchr;
extern int sys_memcmp;
extern int sys_memcpy;
extern int sys_memmove;
extern int sys_memrchr;
extern int sys_memset;
extern int sys_strcasecmp;
extern int sys_strcat;
extern int sys_strchr;
extern int sys_strchrnul;
extern int sys_strcmp;
extern int sys_strcpy;
extern int sys_strcspn;
extern int sys_strdup;
extern int sys_strlen;
extern int sys_strncasecmp;
extern int sys_strncat;
extern int sys_strncmp;
extern int sys_strncpy;
extern int sys_strndup;
extern int sys_strnlen;
extern int sys_strpbrk;
extern int sys_strrchr;
extern int sys_strspn;
extern int sys_strstr;
extern int ptr_check_tag;
extern int ptr_close;
extern int ptr_free_full;
extern int ptr_init;
extern int ptr_lock_full;
extern int ptr_new;
extern int ptr_new_c;
extern int ptr_signal_full;
extern int ptr_unlock_full;
extern int ptr_wait_full;
extern int ch;
extern int debug_aindent;
extern int debug_bytes_full;
extern int debug_bytes_offset_full;
extern int debug_close;
extern int debug_errno_full;
extern int debug_full;
extern int debug_getsyslevel;
extern int debug_indent;
extern int debug_init;
extern int debug_rawtty;
extern int debug_scope;
extern int debug_setsyslevel;
extern int debugva_full;
extern int str;
extern int libbuiltin_init;
extern int script_create_builtins;
extern int list_add;
extern int list_append;
extern int list_array;
extern int list_element;
extern int list_empty;
extern int list_new;
extern int list_next;
extern int list_remove;
extern int sock_client;
extern int sock_dgram_close;
extern int sock_dgram_server;
extern int sock_sendto;
extern int sock_stream_close;
extern int sock_stream_server;
extern int sock_write;
extern int io_accept;
extern int io_bind;
extern int io_connect;
extern int io_dgram_close;
extern int io_dgram_server;
extern int io_fill_addr;
extern int io_sendto_handle;
extern int io_simple_client;
extern int io_simple_server;
extern int io_stream_bound_client;
extern int io_stream_client;
extern int io_stream_close;
extern int io_stream_server;
extern int io_write_handle;
extern int load_fd;
extern int load_file;
extern int getext;
extern int get1;
extern int get2b;
extern int get2l;
extern int get4b;
extern int get4l;
extern int getID;
extern int put1;
extern int put2b;
extern int put2l;
extern int put4b;
extern int put4l;
extern int putID;
extern int time2ts;
extern int ts2time;
extern int bgra_rgb;
extern int bgra_rgba;
extern int desaturate_i420;
extern int desaturate_yuyv;
extern int gray_rgb;
extern int gray_rgba;
extern int gray_yuyv;
extern int i420_gray;
extern int i420_yuyv;
extern int old_yuyv_i420;
extern int rgb_gray;
extern int rgb_rgba;
extern int rgb_yuyv;
extern int rgba_gray;
extern int rgba_rgb;
extern int rgba_yuyv;
extern int uyvy_yuyv;
extern int yuyv_gray;
extern int yuyv_i420;
extern int yuyv_rgb;
extern int yuyv_rgb565;
extern int yuyv_rgba;
extern int graphic_curve;
extern int graphic_ellipse;
extern int graphic_line;
extern int graphic_polygon;
extern int graphic_printchar;
extern int graphic_rectangle;
extern int graphic_vfont_draw;
extern int graphic_vfont_finish;
extern int graphic_vfont_init;
extern int graphic_vfont_mapchar;
extern int graphic_vfont_size;
extern int vfont_finish;
extern int vfont_init;
extern int stbi__unpremultiply_on_load_thread;
extern int stbi_convert_iphone_png_to_rgb;
extern int stbi_convert_iphone_png_to_rgb_thread;
extern int stbi_failure_reason;
extern int stbi_flip_vertically_on_write;
extern int stbi_hdr_to_ldr_gamma;
extern int stbi_hdr_to_ldr_scale;
extern int stbi_image_free;
extern int stbi_info_from_callbacks;
extern int stbi_info_from_memory;
extern int stbi_is_16_bit_from_callbacks;
extern int stbi_is_16_bit_from_memory;
extern int stbi_is_hdr_from_callbacks;
extern int stbi_is_hdr_from_memory;
extern int stbi_load_16_from_callbacks;
extern int stbi_load_16_from_memory;
extern int stbi_load_from_callbacks;
extern int stbi_load_from_memory;
extern int stbi_load_gif_from_memory;
extern int stbi_set_flip_vertically_on_load;
extern int stbi_set_flip_vertically_on_load_thread;
extern int stbi_set_unpremultiply_on_load;
extern int stbi_write_bmp_to_func;
extern int stbi_write_force_png_filter;
extern int stbi_write_jpg_to_func;
extern int stbi_write_png_compression_level;
extern int stbi_write_png_to_func;
extern int stbi_write_png_to_mem;
extern int stbi_write_tga_to_func;
extern int stbi_write_tga_with_rle;
extern int stbi_zlib_compress;
extern int stbi_zlib_decode_buffer;
extern int stbi_zlib_decode_malloc;
extern int stbi_zlib_decode_malloc_guesssize;
extern int stbi_zlib_decode_malloc_guesssize_headerflag;
extern int stbi_zlib_decode_noheader_buffer;
extern int stbi_zlib_decode_noheader_malloc;
extern int surface_buffer;
extern int surface_color_rgb;
extern int surface_copy;
extern int surface_create;
extern int surface_curve;
extern int surface_destroy;
extern int surface_dither;
extern int surface_draw;
extern int surface_draw_alpha;
extern int surface_ellipse;
extern int surface_event;
extern int surface_font_char_width;
extern int surface_font_chars_width;
extern int surface_font_height;
extern int surface_font_width;
extern int surface_line;
extern int surface_load;
extern int surface_load_mem;
extern int surface_palette;
extern int surface_polygon;
extern int surface_print;
extern int surface_printvf;
extern int surface_rectangle;
extern int surface_rgb_color;
extern int surface_rotate;
extern int surface_save;
extern int surface_save_mem;
extern int surface_scale;
extern int surface_settitle;
extern int surface_sizevf;
extern int surface_update;
extern int sys_gmtime;
extern int sys_timegm;
extern int sys_timeofday;
extern int daysinmonth;
extern int timeutc;
extern int utctime;
extern int audio_encoding_name;
extern int media_frame_put;
extern int media_loop;
extern int media_play;
extern int media_stop;
extern int node_call;
extern int node_connect;
extern int node_create;
extern int node_destroy;
extern int node_destroy_chain;
extern int node_disconnect;
extern int node_option;
extern int node_show;
extern int node_sync;
extern int pcm_name;
extern int video_encoding_name;
extern int xcalloc_debug;
extern int xfree_debug;
extern int xmalloc_debug;
extern int xmemcpy_debug;
extern int xmemset_debug;
extern int xrealloc_debug;
extern int xstrdup_debug;
extern int little_endian;
extern int sys_be16toh;
extern int sys_be32toh;
extern int sys_htobe16;
extern int sys_htobe32;
extern int sys_htole16;
extern int sys_htole32;
extern int sys_le16toh;
extern int sys_le32toh;
extern int gps_client;
extern int vfs_abspath;
extern int vfs_chdir;
extern int vfs_checktype;
extern int vfs_close;
extern int vfs_close_session;
extern int vfs_closedir;
extern int vfs_cwd;
extern int vfs_finish;
extern int vfs_fstat;
extern int vfs_getc;
extern int vfs_getmount;
extern int vfs_gets;
extern int vfs_init;
extern int vfs_loadlib;
extern int vfs_map;
extern int vfs_mkdir;
extern int vfs_open;
extern int vfs_open_session;
extern int vfs_open_special;
extern int vfs_opendir;
extern int vfs_peek;
extern int vfs_printf;
extern int vfs_putc;
extern int vfs_read;
extern int vfs_readdir;
extern int vfs_refresh;
extern int vfs_rename;
extern int vfs_rewind;
extern int vfs_rewinddir;
extern int vfs_seek;
extern int vfs_stat;
extern int vfs_statfs;
extern int vfs_truncate;
extern int vfs_type;
extern int vfs_unlink;
extern int vfs_write;
extern int vfs_local_mount;
extern int conn_close;
extern int conn_filter;
extern int conn_set;
extern int telnet_close;
extern int telnet_echo;
extern int telnet_filter;
extern int telnet_linemode;
extern int telnet_naws;
extern int telnet_term;
extern int telnet_client_close;
extern int telnet_client_filter;
extern int telnet_client_state;
extern int login_loop;
extern int pterm_callback;
extern int pterm_close;
extern int pterm_clreol;
extern int pterm_cls;
extern int pterm_crlf_mode;
extern int pterm_cursor;
extern int pterm_cursor_blink;
extern int pterm_cursor_enable;
extern int pterm_getbg;
extern int pterm_getchar;
extern int pterm_getcursor;
extern int pterm_getfg;
extern int pterm_getsize;
extern int pterm_getstate;
extern int pterm_getx;
extern int pterm_gety;
extern int pterm_home;
extern int pterm_init;
extern int pterm_putchar;
extern int pterm_send;
extern int pterm_setbg;
extern int pterm_setfg;
extern int pterm_setx;
extern int pterm_sety;
extern int bmp_decode;
extern int pit_findargs;
extern int a32;
extern int b24;
extern int b32;
extern int b565;
extern int g24;
extern int g32;
extern int g565;
extern int r24;
extern int r32;
extern int r565;
extern int rgb24;
extern int rgb2gray;
extern int rgb32;
extern int rgb565;
extern int rgba32;
extern int average_click;
extern int CustomBlockSize;
extern int CustomFree;
extern int CustomMalloc;
extern int CustomMallocInit;
extern int thread_begin;
extern int thread_begin2;
extern int thread_client_read;
extern int thread_client_read_timeout;
extern int thread_client_write;
extern int thread_close;
extern int thread_end;
extern int thread_get;
extern int thread_get_flags;
extern int thread_get_handle;
extern int thread_get_name;
extern int thread_get_status;
extern int thread_init;
extern int thread_key;
extern int thread_key_delete;
extern int thread_must_end;
extern int thread_needs_run;
extern int thread_ps;
extern int thread_reset_flags;
extern int thread_resume;
extern int thread_run;
extern int thread_server_peek;
extern int thread_server_read;
extern int thread_server_read_timeout;
extern int thread_server_read_timeout_from;
extern int thread_server_write;
extern int thread_set;
extern int thread_set_flags;
extern int thread_set_name;
extern int thread_set_status;
extern int thread_setmain;
extern int thread_setup;
extern int thread_unsetup;
extern int thread_wait_all;
extern int thread_yield;
extern int cond_broadcast;
extern int cond_create;
extern int cond_destroy;
extern int cond_signal;
extern int cond_timedwait;
extern int cond_wait;
extern int mutex_create;
extern int mutex_destroy;
extern int mutex_lock;
extern int mutex_lock_only;
extern int mutex_unlock;
extern int mutex_unlock_only;
extern int semaphore_create;
extern int semaphore_create_named;
extern int semaphore_destroy;
extern int semaphore_post;
extern int semaphore_remove_named;
extern int semaphore_timedwait;
extern int semaphore_wait;
extern int pit_http_abort;
extern int pit_http_delete;
extern int pit_http_get;
extern int pit_http_post;
extern int pit_http_put;
extern int httpd_close;
extern int httpd_create;
extern int httpd_file;
extern int httpd_file_stream;
extern int httpd_reply;
extern int httpd_set_header;
extern int httpd_string;
extern int template_compile;
extern int template_create;
extern int template_destroy;
extern int template_getscript;
extern int template_gettype;
extern int template_tag;
#pragma GCC diagnostic pop

/* Available ELF symbols table: g_PitSyms_elfsyms */

const struct esp_elfsym g_PitSyms_elfsyms[] = {
    ESP_ELFSYM_EXPORT(script_add_boolean),
    ESP_ELFSYM_EXPORT(script_add_function),
    ESP_ELFSYM_EXPORT(script_add_iconst),
    ESP_ELFSYM_EXPORT(script_add_sconst),
    ESP_ELFSYM_EXPORT(script_call),
    ESP_ELFSYM_EXPORT(script_call_args),
    ESP_ELFSYM_EXPORT(script_create),
    ESP_ELFSYM_EXPORT(script_create_function),
    ESP_ELFSYM_EXPORT(script_create_function_data),
    ESP_ELFSYM_EXPORT(script_create_object),
    ESP_ELFSYM_EXPORT(script_destroy),
    ESP_ELFSYM_EXPORT(script_engine_id),
    ESP_ELFSYM_EXPORT(script_get_boolean),
    ESP_ELFSYM_EXPORT(script_get_engine),
    ESP_ELFSYM_EXPORT(script_get_function),
    ESP_ELFSYM_EXPORT(script_get_integer),
    ESP_ELFSYM_EXPORT(script_get_last_error),
    ESP_ELFSYM_EXPORT(script_get_lstring),
    ESP_ELFSYM_EXPORT(script_get_named_value),
    ESP_ELFSYM_EXPORT(script_get_object),
    ESP_ELFSYM_EXPORT(script_get_pointer),
    ESP_ELFSYM_EXPORT(script_get_string),
    ESP_ELFSYM_EXPORT(script_global_get),
    ESP_ELFSYM_EXPORT(script_global_set),
    ESP_ELFSYM_EXPORT(script_loadlib),
    ESP_ELFSYM_EXPORT(script_object_get),
    ESP_ELFSYM_EXPORT(script_object_set),
    ESP_ELFSYM_EXPORT(script_opt_boolean),
    ESP_ELFSYM_EXPORT(script_opt_integer),
    ESP_ELFSYM_EXPORT(script_opt_string),
    ESP_ELFSYM_EXPORT(script_push_boolean),
    ESP_ELFSYM_EXPORT(script_push_integer),
    ESP_ELFSYM_EXPORT(script_push_lstring),
    ESP_ELFSYM_EXPORT(script_push_object),
    ESP_ELFSYM_EXPORT(script_push_string),
    ESP_ELFSYM_EXPORT(script_remove_ref),
    ESP_ELFSYM_EXPORT(script_returned_value),
    ESP_ELFSYM_EXPORT(script_run),
    ESP_ELFSYM_EXPORT(script_set_pointer),
    ESP_ELFSYM_EXPORT(pit_main),
    ESP_ELFSYM_EXPORT(signal_install_handlers),
    ESP_ELFSYM_EXPORT(sys_abs),
    ESP_ELFSYM_EXPORT(sys_block_signals),
    ESP_ELFSYM_EXPORT(sys_chdir),
    ESP_ELFSYM_EXPORT(sys_close),
    ESP_ELFSYM_EXPORT(sys_closedir),
    ESP_ELFSYM_EXPORT(sys_country),
    ESP_ELFSYM_EXPORT(sys_create),
    ESP_ELFSYM_EXPORT(sys_daemonize),
    ESP_ELFSYM_EXPORT(sys_errno),
    ESP_ELFSYM_EXPORT(sys_exit),
    ESP_ELFSYM_EXPORT(sys_fabs),
    ESP_ELFSYM_EXPORT(sys_fabsl),
    ESP_ELFSYM_EXPORT(sys_fdclr),
    ESP_ELFSYM_EXPORT(sys_fdisset),
    ESP_ELFSYM_EXPORT(sys_fdset),
    ESP_ELFSYM_EXPORT(sys_fdzero),
    ESP_ELFSYM_EXPORT(sys_fork_exec),
    ESP_ELFSYM_EXPORT(sys_fstat),
    ESP_ELFSYM_EXPORT(sys_get_clock),
    ESP_ELFSYM_EXPORT(sys_get_clock_ts),
    ESP_ELFSYM_EXPORT(sys_get_pid),
    ESP_ELFSYM_EXPORT(sys_get_process_time),
    ESP_ELFSYM_EXPORT(sys_get_thread_time),
    ESP_ELFSYM_EXPORT(sys_get_tid),
    ESP_ELFSYM_EXPORT(sys_getcwd),
    ESP_ELFSYM_EXPORT(sys_getenv),
    ESP_ELFSYM_EXPORT(sys_getpagesize),
    ESP_ELFSYM_EXPORT(sys_install_handler),
    ESP_ELFSYM_EXPORT(sys_isatty),
    ESP_ELFSYM_EXPORT(sys_language),
    ESP_ELFSYM_EXPORT(sys_lib_close),
    ESP_ELFSYM_EXPORT(sys_lib_defsymbol),
    ESP_ELFSYM_EXPORT(sys_lib_load),
    ESP_ELFSYM_EXPORT(sys_longjmp),
    ESP_ELFSYM_EXPORT(sys_mkdir),
    ESP_ELFSYM_EXPORT(sys_mkstemp),
    ESP_ELFSYM_EXPORT(sys_mkstempfile),
    ESP_ELFSYM_EXPORT(sys_open),
    ESP_ELFSYM_EXPORT(sys_opendir),
    ESP_ELFSYM_EXPORT(sys_peek),
    ESP_ELFSYM_EXPORT(sys_pipe),
    ESP_ELFSYM_EXPORT(sys_qsort),
    ESP_ELFSYM_EXPORT(sys_rand),
    ESP_ELFSYM_EXPORT(sys_read),
    ESP_ELFSYM_EXPORT(sys_read_timeout),
    ESP_ELFSYM_EXPORT(sys_readdir),
    ESP_ELFSYM_EXPORT(sys_rename),
    ESP_ELFSYM_EXPORT(sys_rewinddir),
    ESP_ELFSYM_EXPORT(sys_rmdir),
    ESP_ELFSYM_EXPORT(sys_seek),
    ESP_ELFSYM_EXPORT(sys_select),
    ESP_ELFSYM_EXPORT(sys_select_fds),
    ESP_ELFSYM_EXPORT(sys_serial_baud),
    ESP_ELFSYM_EXPORT(sys_serial_open),
    ESP_ELFSYM_EXPORT(sys_serial_word),
    ESP_ELFSYM_EXPORT(sys_set_finish),
    ESP_ELFSYM_EXPORT(sys_set_thread_name),
    ESP_ELFSYM_EXPORT(sys_setenv),
    ESP_ELFSYM_EXPORT(sys_setjmp),
    ESP_ELFSYM_EXPORT(sys_setsockopt),
    ESP_ELFSYM_EXPORT(sys_socket_accept),
    ESP_ELFSYM_EXPORT(sys_socket_bind),
    ESP_ELFSYM_EXPORT(sys_socket_bind_connect),
    ESP_ELFSYM_EXPORT(sys_socket_binds),
    ESP_ELFSYM_EXPORT(sys_socket_connect),
    ESP_ELFSYM_EXPORT(sys_socket_fill_addr),
    ESP_ELFSYM_EXPORT(sys_socket_ipv4),
    ESP_ELFSYM_EXPORT(sys_socket_listen),
    ESP_ELFSYM_EXPORT(sys_socket_open),
    ESP_ELFSYM_EXPORT(sys_socket_open_connect),
    ESP_ELFSYM_EXPORT(sys_socket_open_connect_timeout),
    ESP_ELFSYM_EXPORT(sys_socket_recvfrom),
    ESP_ELFSYM_EXPORT(sys_socket_sendto),
    ESP_ELFSYM_EXPORT(sys_socket_shutdown),
    ESP_ELFSYM_EXPORT(sys_srand),
    ESP_ELFSYM_EXPORT(sys_sscanf),
    ESP_ELFSYM_EXPORT(sys_stat),
    ESP_ELFSYM_EXPORT(sys_statfs),
    ESP_ELFSYM_EXPORT(sys_strerror),
    ESP_ELFSYM_EXPORT(sys_strtod),
    ESP_ELFSYM_EXPORT(sys_termsize),
    ESP_ELFSYM_EXPORT(sys_time),
    ESP_ELFSYM_EXPORT(sys_truncate),
    ESP_ELFSYM_EXPORT(sys_tty_raw),
    ESP_ELFSYM_EXPORT(sys_tty_restore),
    ESP_ELFSYM_EXPORT(sys_unblock_signals),
    ESP_ELFSYM_EXPORT(sys_unlink),
    ESP_ELFSYM_EXPORT(sys_usleep),
    ESP_ELFSYM_EXPORT(sys_vsnprintf),
    ESP_ELFSYM_EXPORT(sys_vsprintf),
    ESP_ELFSYM_EXPORT(sys_wait),
    ESP_ELFSYM_EXPORT(sys_write),
    ESP_ELFSYM_EXPORT(sys_acos),
    ESP_ELFSYM_EXPORT(sys_asin),
    ESP_ELFSYM_EXPORT(sys_atan),
    ESP_ELFSYM_EXPORT(sys_atan2),
    ESP_ELFSYM_EXPORT(sys_ceil),
    ESP_ELFSYM_EXPORT(sys_cos),
    ESP_ELFSYM_EXPORT(sys_cosh),
    ESP_ELFSYM_EXPORT(sys_exp),
    ESP_ELFSYM_EXPORT(sys_floor),
    ESP_ELFSYM_EXPORT(sys_fmodl),
    ESP_ELFSYM_EXPORT(sys_isinf),
    ESP_ELFSYM_EXPORT(sys_isnan),
    ESP_ELFSYM_EXPORT(sys_log),
    ESP_ELFSYM_EXPORT(sys_log10),
    ESP_ELFSYM_EXPORT(sys_modf),
    ESP_ELFSYM_EXPORT(sys_pi),
    ESP_ELFSYM_EXPORT(sys_pow),
    ESP_ELFSYM_EXPORT(sys_signbit),
    ESP_ELFSYM_EXPORT(sys_sin),
    ESP_ELFSYM_EXPORT(sys_sinh),
    ESP_ELFSYM_EXPORT(sys_sqrt),
    ESP_ELFSYM_EXPORT(sys_tan),
    ESP_ELFSYM_EXPORT(sys_tanh),
    ESP_ELFSYM_EXPORT(sys_calloc),
    ESP_ELFSYM_EXPORT(sys_free),
    ESP_ELFSYM_EXPORT(sys_malloc),
    ESP_ELFSYM_EXPORT(sys_realloc),
    ESP_ELFSYM_EXPORT(sys_strtol),
    ESP_ELFSYM_EXPORT(sys_strtoul),
    ESP_ELFSYM_EXPORT(sys_isalnum),
    ESP_ELFSYM_EXPORT(sys_isalpha),
    ESP_ELFSYM_EXPORT(sys_isblank),
    ESP_ELFSYM_EXPORT(sys_iscntrl),
    ESP_ELFSYM_EXPORT(sys_isdigit),
    ESP_ELFSYM_EXPORT(sys_isgraph),
    ESP_ELFSYM_EXPORT(sys_islower),
    ESP_ELFSYM_EXPORT(sys_isprint),
    ESP_ELFSYM_EXPORT(sys_ispunct),
    ESP_ELFSYM_EXPORT(sys_isspace),
    ESP_ELFSYM_EXPORT(sys_isupper),
    ESP_ELFSYM_EXPORT(sys_isxdigit),
    ESP_ELFSYM_EXPORT(sys_tolower),
    ESP_ELFSYM_EXPORT(sys_toupper),
    ESP_ELFSYM_EXPORT(sys_bsearch),
    ESP_ELFSYM_EXPORT(sys_snprintf),
    ESP_ELFSYM_EXPORT(sys_sprintf),
    ESP_ELFSYM_EXPORT(sys_atof),
    ESP_ELFSYM_EXPORT(sys_atoi),
    ESP_ELFSYM_EXPORT(sys_memchr),
    ESP_ELFSYM_EXPORT(sys_memcmp),
    ESP_ELFSYM_EXPORT(sys_memcpy),
    ESP_ELFSYM_EXPORT(sys_memmove),
    ESP_ELFSYM_EXPORT(sys_memrchr),
    ESP_ELFSYM_EXPORT(sys_memset),
    ESP_ELFSYM_EXPORT(sys_strcasecmp),
    ESP_ELFSYM_EXPORT(sys_strcat),
    ESP_ELFSYM_EXPORT(sys_strchr),
    ESP_ELFSYM_EXPORT(sys_strchrnul),
    ESP_ELFSYM_EXPORT(sys_strcmp),
    ESP_ELFSYM_EXPORT(sys_strcpy),
    ESP_ELFSYM_EXPORT(sys_strcspn),
    ESP_ELFSYM_EXPORT(sys_strdup),
    ESP_ELFSYM_EXPORT(sys_strlen),
    ESP_ELFSYM_EXPORT(sys_strncasecmp),
    ESP_ELFSYM_EXPORT(sys_strncat),
    ESP_ELFSYM_EXPORT(sys_strncmp),
    ESP_ELFSYM_EXPORT(sys_strncpy),
    ESP_ELFSYM_EXPORT(sys_strndup),
    ESP_ELFSYM_EXPORT(sys_strnlen),
    ESP_ELFSYM_EXPORT(sys_strpbrk),
    ESP_ELFSYM_EXPORT(sys_strrchr),
    ESP_ELFSYM_EXPORT(sys_strspn),
    ESP_ELFSYM_EXPORT(sys_strstr),
    ESP_ELFSYM_EXPORT(ptr_check_tag),
    ESP_ELFSYM_EXPORT(ptr_close),
    ESP_ELFSYM_EXPORT(ptr_free_full),
    ESP_ELFSYM_EXPORT(ptr_init),
    ESP_ELFSYM_EXPORT(ptr_lock_full),
    ESP_ELFSYM_EXPORT(ptr_new),
    ESP_ELFSYM_EXPORT(ptr_new_c),
    ESP_ELFSYM_EXPORT(ptr_signal_full),
    ESP_ELFSYM_EXPORT(ptr_unlock_full),
    ESP_ELFSYM_EXPORT(ptr_wait_full),
    ESP_ELFSYM_EXPORT(ch),
    ESP_ELFSYM_EXPORT(debug_aindent),
    ESP_ELFSYM_EXPORT(debug_bytes_full),
    ESP_ELFSYM_EXPORT(debug_bytes_offset_full),
    ESP_ELFSYM_EXPORT(debug_close),
    ESP_ELFSYM_EXPORT(debug_errno_full),
    ESP_ELFSYM_EXPORT(debug_full),
    ESP_ELFSYM_EXPORT(debug_getsyslevel),
    ESP_ELFSYM_EXPORT(debug_indent),
    ESP_ELFSYM_EXPORT(debug_init),
    ESP_ELFSYM_EXPORT(debug_rawtty),
    ESP_ELFSYM_EXPORT(debug_scope),
    ESP_ELFSYM_EXPORT(debug_setsyslevel),
    ESP_ELFSYM_EXPORT(debugva_full),
    ESP_ELFSYM_EXPORT(str),
    ESP_ELFSYM_EXPORT(libbuiltin_init),
    ESP_ELFSYM_EXPORT(script_create_builtins),
    ESP_ELFSYM_EXPORT(list_add),
    ESP_ELFSYM_EXPORT(list_append),
    ESP_ELFSYM_EXPORT(list_array),
    ESP_ELFSYM_EXPORT(list_element),
    ESP_ELFSYM_EXPORT(list_empty),
    ESP_ELFSYM_EXPORT(list_new),
    ESP_ELFSYM_EXPORT(list_next),
    ESP_ELFSYM_EXPORT(list_remove),
    ESP_ELFSYM_EXPORT(sock_client),
    ESP_ELFSYM_EXPORT(sock_dgram_close),
    ESP_ELFSYM_EXPORT(sock_dgram_server),
    ESP_ELFSYM_EXPORT(sock_sendto),
    ESP_ELFSYM_EXPORT(sock_stream_close),
    ESP_ELFSYM_EXPORT(sock_stream_server),
    ESP_ELFSYM_EXPORT(sock_write),
    ESP_ELFSYM_EXPORT(io_accept),
    ESP_ELFSYM_EXPORT(io_bind),
    ESP_ELFSYM_EXPORT(io_connect),
    ESP_ELFSYM_EXPORT(io_dgram_close),
    ESP_ELFSYM_EXPORT(io_dgram_server),
    ESP_ELFSYM_EXPORT(io_fill_addr),
    ESP_ELFSYM_EXPORT(io_sendto_handle),
    ESP_ELFSYM_EXPORT(io_simple_client),
    ESP_ELFSYM_EXPORT(io_simple_server),
    ESP_ELFSYM_EXPORT(io_stream_bound_client),
    ESP_ELFSYM_EXPORT(io_stream_client),
    ESP_ELFSYM_EXPORT(io_stream_close),
    ESP_ELFSYM_EXPORT(io_stream_server),
    ESP_ELFSYM_EXPORT(io_write_handle),
    ESP_ELFSYM_EXPORT(load_fd),
    ESP_ELFSYM_EXPORT(load_file),
    ESP_ELFSYM_EXPORT(getext),
    ESP_ELFSYM_EXPORT(get1),
    ESP_ELFSYM_EXPORT(get2b),
    ESP_ELFSYM_EXPORT(get2l),
    ESP_ELFSYM_EXPORT(get4b),
    ESP_ELFSYM_EXPORT(get4l),
    ESP_ELFSYM_EXPORT(getID),
    ESP_ELFSYM_EXPORT(put1),
    ESP_ELFSYM_EXPORT(put2b),
    ESP_ELFSYM_EXPORT(put2l),
    ESP_ELFSYM_EXPORT(put4b),
    ESP_ELFSYM_EXPORT(put4l),
    ESP_ELFSYM_EXPORT(putID),
    ESP_ELFSYM_EXPORT(time2ts),
    ESP_ELFSYM_EXPORT(ts2time),
    ESP_ELFSYM_EXPORT(bgra_rgb),
    ESP_ELFSYM_EXPORT(bgra_rgba),
    ESP_ELFSYM_EXPORT(desaturate_i420),
    ESP_ELFSYM_EXPORT(desaturate_yuyv),
    ESP_ELFSYM_EXPORT(gray_rgb),
    ESP_ELFSYM_EXPORT(gray_rgba),
    ESP_ELFSYM_EXPORT(gray_yuyv),
    ESP_ELFSYM_EXPORT(i420_gray),
    ESP_ELFSYM_EXPORT(i420_yuyv),
    ESP_ELFSYM_EXPORT(old_yuyv_i420),
    ESP_ELFSYM_EXPORT(rgb_gray),
    ESP_ELFSYM_EXPORT(rgb_rgba),
    ESP_ELFSYM_EXPORT(rgb_yuyv),
    ESP_ELFSYM_EXPORT(rgba_gray),
    ESP_ELFSYM_EXPORT(rgba_rgb),
    ESP_ELFSYM_EXPORT(rgba_yuyv),
    ESP_ELFSYM_EXPORT(uyvy_yuyv),
    ESP_ELFSYM_EXPORT(yuyv_gray),
    ESP_ELFSYM_EXPORT(yuyv_i420),
    ESP_ELFSYM_EXPORT(yuyv_rgb),
    ESP_ELFSYM_EXPORT(yuyv_rgb565),
    ESP_ELFSYM_EXPORT(yuyv_rgba),
    ESP_ELFSYM_EXPORT(graphic_curve),
    ESP_ELFSYM_EXPORT(graphic_ellipse),
    ESP_ELFSYM_EXPORT(graphic_line),
    ESP_ELFSYM_EXPORT(graphic_polygon),
    ESP_ELFSYM_EXPORT(graphic_printchar),
    ESP_ELFSYM_EXPORT(graphic_rectangle),
    ESP_ELFSYM_EXPORT(graphic_vfont_draw),
    ESP_ELFSYM_EXPORT(graphic_vfont_finish),
    ESP_ELFSYM_EXPORT(graphic_vfont_init),
    ESP_ELFSYM_EXPORT(graphic_vfont_mapchar),
    ESP_ELFSYM_EXPORT(graphic_vfont_size),
    ESP_ELFSYM_EXPORT(vfont_finish),
    ESP_ELFSYM_EXPORT(vfont_init),
    ESP_ELFSYM_EXPORT(stbi__unpremultiply_on_load_thread),
    ESP_ELFSYM_EXPORT(stbi_convert_iphone_png_to_rgb),
    ESP_ELFSYM_EXPORT(stbi_convert_iphone_png_to_rgb_thread),
    ESP_ELFSYM_EXPORT(stbi_failure_reason),
    ESP_ELFSYM_EXPORT(stbi_flip_vertically_on_write),
    ESP_ELFSYM_EXPORT(stbi_hdr_to_ldr_gamma),
    ESP_ELFSYM_EXPORT(stbi_hdr_to_ldr_scale),
    ESP_ELFSYM_EXPORT(stbi_image_free),
    ESP_ELFSYM_EXPORT(stbi_info_from_callbacks),
    ESP_ELFSYM_EXPORT(stbi_info_from_memory),
    ESP_ELFSYM_EXPORT(stbi_is_16_bit_from_callbacks),
    ESP_ELFSYM_EXPORT(stbi_is_16_bit_from_memory),
    ESP_ELFSYM_EXPORT(stbi_is_hdr_from_callbacks),
    ESP_ELFSYM_EXPORT(stbi_is_hdr_from_memory),
    ESP_ELFSYM_EXPORT(stbi_load_16_from_callbacks),
    ESP_ELFSYM_EXPORT(stbi_load_16_from_memory),
    ESP_ELFSYM_EXPORT(stbi_load_from_callbacks),
    ESP_ELFSYM_EXPORT(stbi_load_from_memory),
    ESP_ELFSYM_EXPORT(stbi_load_gif_from_memory),
    ESP_ELFSYM_EXPORT(stbi_set_flip_vertically_on_load),
    ESP_ELFSYM_EXPORT(stbi_set_flip_vertically_on_load_thread),
    ESP_ELFSYM_EXPORT(stbi_set_unpremultiply_on_load),
    ESP_ELFSYM_EXPORT(stbi_write_bmp_to_func),
    ESP_ELFSYM_EXPORT(stbi_write_force_png_filter),
    ESP_ELFSYM_EXPORT(stbi_write_jpg_to_func),
    ESP_ELFSYM_EXPORT(stbi_write_png_compression_level),
    ESP_ELFSYM_EXPORT(stbi_write_png_to_func),
    ESP_ELFSYM_EXPORT(stbi_write_png_to_mem),
    ESP_ELFSYM_EXPORT(stbi_write_tga_to_func),
    ESP_ELFSYM_EXPORT(stbi_write_tga_with_rle),
    ESP_ELFSYM_EXPORT(stbi_zlib_compress),
    ESP_ELFSYM_EXPORT(stbi_zlib_decode_buffer),
    ESP_ELFSYM_EXPORT(stbi_zlib_decode_malloc),
    ESP_ELFSYM_EXPORT(stbi_zlib_decode_malloc_guesssize),
    ESP_ELFSYM_EXPORT(stbi_zlib_decode_malloc_guesssize_headerflag),
    ESP_ELFSYM_EXPORT(stbi_zlib_decode_noheader_buffer),
    ESP_ELFSYM_EXPORT(stbi_zlib_decode_noheader_malloc),
    ESP_ELFSYM_EXPORT(surface_buffer),
    ESP_ELFSYM_EXPORT(surface_color_rgb),
    ESP_ELFSYM_EXPORT(surface_copy),
    ESP_ELFSYM_EXPORT(surface_create),
    ESP_ELFSYM_EXPORT(surface_curve),
    ESP_ELFSYM_EXPORT(surface_destroy),
    ESP_ELFSYM_EXPORT(surface_dither),
    ESP_ELFSYM_EXPORT(surface_draw),
    ESP_ELFSYM_EXPORT(surface_draw_alpha),
    ESP_ELFSYM_EXPORT(surface_ellipse),
    ESP_ELFSYM_EXPORT(surface_event),
    ESP_ELFSYM_EXPORT(surface_font_char_width),
    ESP_ELFSYM_EXPORT(surface_font_chars_width),
    ESP_ELFSYM_EXPORT(surface_font_height),
    ESP_ELFSYM_EXPORT(surface_font_width),
    ESP_ELFSYM_EXPORT(surface_line),
    ESP_ELFSYM_EXPORT(surface_load),
    ESP_ELFSYM_EXPORT(surface_load_mem),
    ESP_ELFSYM_EXPORT(surface_palette),
    ESP_ELFSYM_EXPORT(surface_polygon),
    ESP_ELFSYM_EXPORT(surface_print),
    ESP_ELFSYM_EXPORT(surface_printvf),
    ESP_ELFSYM_EXPORT(surface_rectangle),
    ESP_ELFSYM_EXPORT(surface_rgb_color),
    ESP_ELFSYM_EXPORT(surface_rotate),
    ESP_ELFSYM_EXPORT(surface_save),
    ESP_ELFSYM_EXPORT(surface_save_mem),
    ESP_ELFSYM_EXPORT(surface_scale),
    ESP_ELFSYM_EXPORT(surface_settitle),
    ESP_ELFSYM_EXPORT(surface_sizevf),
    ESP_ELFSYM_EXPORT(surface_update),
    ESP_ELFSYM_EXPORT(sys_gmtime),
    ESP_ELFSYM_EXPORT(sys_timegm),
    ESP_ELFSYM_EXPORT(sys_timeofday),
    ESP_ELFSYM_EXPORT(daysinmonth),
    ESP_ELFSYM_EXPORT(timeutc),
    ESP_ELFSYM_EXPORT(utctime),
    ESP_ELFSYM_EXPORT(audio_encoding_name),
    ESP_ELFSYM_EXPORT(media_frame_put),
    ESP_ELFSYM_EXPORT(media_loop),
    ESP_ELFSYM_EXPORT(media_play),
    ESP_ELFSYM_EXPORT(media_stop),
    ESP_ELFSYM_EXPORT(node_call),
    ESP_ELFSYM_EXPORT(node_connect),
    ESP_ELFSYM_EXPORT(node_create),
    ESP_ELFSYM_EXPORT(node_destroy),
    ESP_ELFSYM_EXPORT(node_destroy_chain),
    ESP_ELFSYM_EXPORT(node_disconnect),
    ESP_ELFSYM_EXPORT(node_option),
    ESP_ELFSYM_EXPORT(node_show),
    ESP_ELFSYM_EXPORT(node_sync),
    ESP_ELFSYM_EXPORT(pcm_name),
    ESP_ELFSYM_EXPORT(video_encoding_name),
    ESP_ELFSYM_EXPORT(xcalloc_debug),
    ESP_ELFSYM_EXPORT(xfree_debug),
    ESP_ELFSYM_EXPORT(xmalloc_debug),
    ESP_ELFSYM_EXPORT(xmemcpy_debug),
    ESP_ELFSYM_EXPORT(xmemset_debug),
    ESP_ELFSYM_EXPORT(xrealloc_debug),
    ESP_ELFSYM_EXPORT(xstrdup_debug),
    ESP_ELFSYM_EXPORT(little_endian),
    ESP_ELFSYM_EXPORT(sys_be16toh),
    ESP_ELFSYM_EXPORT(sys_be32toh),
    ESP_ELFSYM_EXPORT(sys_htobe16),
    ESP_ELFSYM_EXPORT(sys_htobe32),
    ESP_ELFSYM_EXPORT(sys_htole16),
    ESP_ELFSYM_EXPORT(sys_htole32),
    ESP_ELFSYM_EXPORT(sys_le16toh),
    ESP_ELFSYM_EXPORT(sys_le32toh),
    ESP_ELFSYM_EXPORT(gps_client),
    ESP_ELFSYM_EXPORT(vfs_abspath),
    ESP_ELFSYM_EXPORT(vfs_chdir),
    ESP_ELFSYM_EXPORT(vfs_checktype),
    ESP_ELFSYM_EXPORT(vfs_close),
    ESP_ELFSYM_EXPORT(vfs_close_session),
    ESP_ELFSYM_EXPORT(vfs_closedir),
    ESP_ELFSYM_EXPORT(vfs_cwd),
    ESP_ELFSYM_EXPORT(vfs_finish),
    ESP_ELFSYM_EXPORT(vfs_fstat),
    ESP_ELFSYM_EXPORT(vfs_getc),
    ESP_ELFSYM_EXPORT(vfs_getmount),
    ESP_ELFSYM_EXPORT(vfs_gets),
    ESP_ELFSYM_EXPORT(vfs_init),
    ESP_ELFSYM_EXPORT(vfs_loadlib),
    ESP_ELFSYM_EXPORT(vfs_map),
    ESP_ELFSYM_EXPORT(vfs_mkdir),
    ESP_ELFSYM_EXPORT(vfs_open),
    ESP_ELFSYM_EXPORT(vfs_open_session),
    ESP_ELFSYM_EXPORT(vfs_open_special),
    ESP_ELFSYM_EXPORT(vfs_opendir),
    ESP_ELFSYM_EXPORT(vfs_peek),
    ESP_ELFSYM_EXPORT(vfs_printf),
    ESP_ELFSYM_EXPORT(vfs_putc),
    ESP_ELFSYM_EXPORT(vfs_read),
    ESP_ELFSYM_EXPORT(vfs_readdir),
    ESP_ELFSYM_EXPORT(vfs_refresh),
    ESP_ELFSYM_EXPORT(vfs_rename),
    ESP_ELFSYM_EXPORT(vfs_rewind),
    ESP_ELFSYM_EXPORT(vfs_rewinddir),
    ESP_ELFSYM_EXPORT(vfs_seek),
    ESP_ELFSYM_EXPORT(vfs_stat),
    ESP_ELFSYM_EXPORT(vfs_statfs),
    ESP_ELFSYM_EXPORT(vfs_truncate),
    ESP_ELFSYM_EXPORT(vfs_type),
    ESP_ELFSYM_EXPORT(vfs_unlink),
    ESP_ELFSYM_EXPORT(vfs_write),
    ESP_ELFSYM_EXPORT(vfs_local_mount),
    ESP_ELFSYM_EXPORT(conn_close),
    ESP_ELFSYM_EXPORT(conn_filter),
    ESP_ELFSYM_EXPORT(conn_set),
    ESP_ELFSYM_EXPORT(telnet_close),
    ESP_ELFSYM_EXPORT(telnet_echo),
    ESP_ELFSYM_EXPORT(telnet_filter),
    ESP_ELFSYM_EXPORT(telnet_linemode),
    ESP_ELFSYM_EXPORT(telnet_naws),
    ESP_ELFSYM_EXPORT(telnet_term),
    ESP_ELFSYM_EXPORT(telnet_client_close),
    ESP_ELFSYM_EXPORT(telnet_client_filter),
    ESP_ELFSYM_EXPORT(telnet_client_state),
    ESP_ELFSYM_EXPORT(login_loop),
    ESP_ELFSYM_EXPORT(pterm_callback),
    ESP_ELFSYM_EXPORT(pterm_close),
    ESP_ELFSYM_EXPORT(pterm_clreol),
    ESP_ELFSYM_EXPORT(pterm_cls),
    ESP_ELFSYM_EXPORT(pterm_crlf_mode),
    ESP_ELFSYM_EXPORT(pterm_cursor),
    ESP_ELFSYM_EXPORT(pterm_cursor_blink),
    ESP_ELFSYM_EXPORT(pterm_cursor_enable),
    ESP_ELFSYM_EXPORT(pterm_getbg),
    ESP_ELFSYM_EXPORT(pterm_getchar),
    ESP_ELFSYM_EXPORT(pterm_getcursor),
    ESP_ELFSYM_EXPORT(pterm_getfg),
    ESP_ELFSYM_EXPORT(pterm_getsize),
    ESP_ELFSYM_EXPORT(pterm_getstate),
    ESP_ELFSYM_EXPORT(pterm_getx),
    ESP_ELFSYM_EXPORT(pterm_gety),
    ESP_ELFSYM_EXPORT(pterm_home),
    ESP_ELFSYM_EXPORT(pterm_init),
    ESP_ELFSYM_EXPORT(pterm_putchar),
    ESP_ELFSYM_EXPORT(pterm_send),
    ESP_ELFSYM_EXPORT(pterm_setbg),
    ESP_ELFSYM_EXPORT(pterm_setfg),
    ESP_ELFSYM_EXPORT(pterm_setx),
    ESP_ELFSYM_EXPORT(pterm_sety),
    ESP_ELFSYM_EXPORT(bmp_decode),
    ESP_ELFSYM_EXPORT(pit_findargs),
    ESP_ELFSYM_EXPORT(a32),
    ESP_ELFSYM_EXPORT(b24),
    ESP_ELFSYM_EXPORT(b32),
    ESP_ELFSYM_EXPORT(b565),
    ESP_ELFSYM_EXPORT(g24),
    ESP_ELFSYM_EXPORT(g32),
    ESP_ELFSYM_EXPORT(g565),
    ESP_ELFSYM_EXPORT(r24),
    ESP_ELFSYM_EXPORT(r32),
    ESP_ELFSYM_EXPORT(r565),
    ESP_ELFSYM_EXPORT(rgb24),
    ESP_ELFSYM_EXPORT(rgb2gray),
    ESP_ELFSYM_EXPORT(rgb32),
    ESP_ELFSYM_EXPORT(rgb565),
    ESP_ELFSYM_EXPORT(rgba32),
    ESP_ELFSYM_EXPORT(average_click),
    ESP_ELFSYM_EXPORT(CustomBlockSize),
    ESP_ELFSYM_EXPORT(CustomFree),
    ESP_ELFSYM_EXPORT(CustomMalloc),
    ESP_ELFSYM_EXPORT(CustomMallocInit),
    ESP_ELFSYM_EXPORT(thread_begin),
    ESP_ELFSYM_EXPORT(thread_begin2),
    ESP_ELFSYM_EXPORT(thread_client_read),
    ESP_ELFSYM_EXPORT(thread_client_read_timeout),
    ESP_ELFSYM_EXPORT(thread_client_write),
    ESP_ELFSYM_EXPORT(thread_close),
    ESP_ELFSYM_EXPORT(thread_end),
    ESP_ELFSYM_EXPORT(thread_get),
    ESP_ELFSYM_EXPORT(thread_get_flags),
    ESP_ELFSYM_EXPORT(thread_get_handle),
    ESP_ELFSYM_EXPORT(thread_get_name),
    ESP_ELFSYM_EXPORT(thread_get_status),
    ESP_ELFSYM_EXPORT(thread_init),
    ESP_ELFSYM_EXPORT(thread_key),
    ESP_ELFSYM_EXPORT(thread_key_delete),
    ESP_ELFSYM_EXPORT(thread_must_end),
    ESP_ELFSYM_EXPORT(thread_needs_run),
    ESP_ELFSYM_EXPORT(thread_ps),
    ESP_ELFSYM_EXPORT(thread_reset_flags),
    ESP_ELFSYM_EXPORT(thread_resume),
    ESP_ELFSYM_EXPORT(thread_run),
    ESP_ELFSYM_EXPORT(thread_server_peek),
    ESP_ELFSYM_EXPORT(thread_server_read),
    ESP_ELFSYM_EXPORT(thread_server_read_timeout),
    ESP_ELFSYM_EXPORT(thread_server_read_timeout_from),
    ESP_ELFSYM_EXPORT(thread_server_write),
    ESP_ELFSYM_EXPORT(thread_set),
    ESP_ELFSYM_EXPORT(thread_set_flags),
    ESP_ELFSYM_EXPORT(thread_set_name),
    ESP_ELFSYM_EXPORT(thread_set_status),
    ESP_ELFSYM_EXPORT(thread_setmain),
    ESP_ELFSYM_EXPORT(thread_setup),
    ESP_ELFSYM_EXPORT(thread_unsetup),
    ESP_ELFSYM_EXPORT(thread_wait_all),
    ESP_ELFSYM_EXPORT(thread_yield),
    ESP_ELFSYM_EXPORT(cond_broadcast),
    ESP_ELFSYM_EXPORT(cond_create),
    ESP_ELFSYM_EXPORT(cond_destroy),
    ESP_ELFSYM_EXPORT(cond_signal),
    ESP_ELFSYM_EXPORT(cond_timedwait),
    ESP_ELFSYM_EXPORT(cond_wait),
    ESP_ELFSYM_EXPORT(mutex_create),
    ESP_ELFSYM_EXPORT(mutex_destroy),
    ESP_ELFSYM_EXPORT(mutex_lock),
    ESP_ELFSYM_EXPORT(mutex_lock_only),
    ESP_ELFSYM_EXPORT(mutex_unlock),
    ESP_ELFSYM_EXPORT(mutex_unlock_only),
    ESP_ELFSYM_EXPORT(semaphore_create),
    ESP_ELFSYM_EXPORT(semaphore_create_named),
    ESP_ELFSYM_EXPORT(semaphore_destroy),
    ESP_ELFSYM_EXPORT(semaphore_post),
    ESP_ELFSYM_EXPORT(semaphore_remove_named),
    ESP_ELFSYM_EXPORT(semaphore_timedwait),
    ESP_ELFSYM_EXPORT(semaphore_wait),
    ESP_ELFSYM_EXPORT(pit_http_abort),
    ESP_ELFSYM_EXPORT(pit_http_delete),
    ESP_ELFSYM_EXPORT(pit_http_get),
    ESP_ELFSYM_EXPORT(pit_http_post),
    ESP_ELFSYM_EXPORT(pit_http_put),
    ESP_ELFSYM_EXPORT(httpd_close),
    ESP_ELFSYM_EXPORT(httpd_create),
    ESP_ELFSYM_EXPORT(httpd_file),
    ESP_ELFSYM_EXPORT(httpd_file_stream),
    ESP_ELFSYM_EXPORT(httpd_reply),
    ESP_ELFSYM_EXPORT(httpd_set_header),
    ESP_ELFSYM_EXPORT(httpd_string),
    ESP_ELFSYM_EXPORT(template_compile),
    ESP_ELFSYM_EXPORT(template_create),
    ESP_ELFSYM_EXPORT(template_destroy),
    ESP_ELFSYM_EXPORT(template_getscript),
    ESP_ELFSYM_EXPORT(template_gettype),
    ESP_ELFSYM_EXPORT(template_tag),
    ESP_ELFSYM_END
};

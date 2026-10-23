// Script engine stub for the MS-DOS build.
//
// There is no Lua on DOS: the boot "script" is replaced by a fixed set of
// parameters (overridable from the command line, see main.c) and the
// providers are kept in plain static variables.

#include "sys.h"
#include "script.h"
#include "pwindow.h"
#include "audio.h"
#include "gpio.h"
#include "pspi.h"
#include "secure.h"
#include "msdos.h"

static void *gpiop = NULL;
static void *spip = NULL;
static void *wp = NULL;
static void *ap = NULL;
static void *securep = NULL;

// defaults: a 640x480 VESA mode (available on every VESA BIOS) in single
// application mode with the dynamic input area: a 640x320 application area
// (double density) over a 160 pixel DIA strip (graffiti area + icon bar, see
// the DOS layout in libpumpkin/dia.c). The mouse acts as the pen, the
// keyboard Home key returns to the Launcher.
static int cfg_width = 640;
static int cfg_height = 480;
static int cfg_density = 144;
static int cfg_hdepth = 16;
static int cfg_depth = 16;
static int cfg_mode = 1;
static int cfg_dia = 1;
static int cfg_fullscreen = 1;
// application started instead of the Launcher (-app <name>), empty = Launcher
// (libos "app" parameter: does not change the launcher preference)
static char cfg_launcher[64];

int msdos_script_set(const char *name, int value) {
  if (!sys_strcmp(name, "width")) cfg_width = value;
  else if (!sys_strcmp(name, "height")) cfg_height = value;
  else if (!sys_strcmp(name, "density")) cfg_density = value;
  else if (!sys_strcmp(name, "hdepth")) cfg_hdepth = value;
  else if (!sys_strcmp(name, "depth")) cfg_depth = value;
  else if (!sys_strcmp(name, "mode")) cfg_mode = value;
  else if (!sys_strcmp(name, "dia")) cfg_dia = value;
  else if (!sys_strcmp(name, "fullscreen")) cfg_fullscreen = value;
  else return -1;

  return 0;
}

int msdos_script_set_launcher(const char *name) {
  sys_memset(cfg_launcher, 0, sizeof(cfg_launcher));
  sys_strncpy(cfg_launcher, name, sizeof(cfg_launcher)-1);
  return 0;
}

int script_set_pointer(int pe, char *name, void *p) {
  if (!sys_strcmp(name, GPIO_PROVIDER)) {
    gpiop = p;
  } else if (!sys_strcmp(name, SPI_PROVIDER)) {
    spip = p;
  } else if (!sys_strcmp(name, WINDOW_PROVIDER)) {
    wp = p;
  } else if (!sys_strcmp(name, AUDIO_PROVIDER)) {
    ap = p;
  } else if (!sys_strcmp(name, SECURE_PROVIDER)) {
    securep = p;
  }

  return 0;
}

void *script_get_pointer(int pe, char *name) {
  if (!sys_strcmp(name, GPIO_PROVIDER)) {
    return gpiop;
  }
  if (!sys_strcmp(name, SPI_PROVIDER)) {
    return spip;
  }
  if (!sys_strcmp(name, WINDOW_PROVIDER)) {
    return wp;
  }
  if (!sys_strcmp(name, AUDIO_PROVIDER)) {
    return ap;
  }
  if (!sys_strcmp(name, SECURE_PROVIDER)) {
    return securep;
  }

  return NULL;
}

int script_get_object(int pe, int index, script_ref_t *ref) {
  sys_memset(ref, 0, sizeof(script_ref_t));
  return 0;
}

int script_get_named_value(int pe, int index, int type, char *lib, char *func, char *param, int opt, script_arg_t *arg) {
  return 0;
}

script_ref_t script_loadlib(int pe, char *libname) {
  return (script_ref_t)0;
}

int script_object_set(int pe, script_ref_t obj, script_arg_t *key, script_arg_t *value) {
  return 0;
}

#define OBJ_INTEGER(k,v) \
    if (!sys_strcmp(key->value.s, k)) { \
      value->type = SCRIPT_ARG_INTEGER; \
      value->value.i = v; \
      return 0; \
    }

#define OBJ_BOOLEAN(k,v) \
    if (!sys_strcmp(key->value.s, k)) { \
      value->type = SCRIPT_ARG_BOOLEAN; \
      value->value.i = v; \
      return 0; \
    }

int script_object_get(int pe, script_ref_t obj, script_arg_t *key, script_arg_t *value) {
  if (key->type == SCRIPT_ARG_STRING) {
    if (!sys_strcmp(key->value.s, "app") && cfg_launcher[0]) {
      value->type = SCRIPT_ARG_LSTRING;
      value->value.l.s = cfg_launcher;
      value->value.l.n = sys_strlen(cfg_launcher);
      return 0;
    }
    OBJ_INTEGER("density",    cfg_density);
    OBJ_INTEGER("width",      cfg_width);
    OBJ_INTEGER("height",     cfg_height);
    OBJ_INTEGER("hdepth",     cfg_hdepth);
    OBJ_INTEGER("depth",      cfg_depth);
    OBJ_INTEGER("mode",       cfg_mode);
    OBJ_BOOLEAN("dia",        cfg_dia);
    OBJ_BOOLEAN("fullscreen", cfg_fullscreen);
  }

  return -1;
}

script_engine_t *script_load_engine(char *libname) {
  return NULL;
}

int script_init(script_engine_t *engine) {
  return 0;
}

int script_finish(script_engine_t *engine) {
  return 0;
}

void script_idle_loop(int pe) {
}

int script_get_real(int pe, int index, script_real_t *d) {
  return 0;
}

int script_push_real(int pe, script_real_t d) {
  return 0;
}

int script_push_value(int pe, script_arg_t *value) {
  return 0;
}

int script_set_idle(int pe, script_ref_t ref, uint32_t t) {
  return 0;
}

int script_set_cleanup(int pe, script_ref_t ref) {
  return 0;
}

int script_get_function(int pe, int index, script_ref_t *ref) {
  return 0;
}

int script_get_integer(int pe, int index, script_int_t *i) {
  return 0;
}

int script_get_string(int pe, int index, char **s) {
  return 0;
}

int script_get_lstring(int pe, int index, char **s, int *len){
  return 0;
}

int script_get_boolean(int pe, int index, int *b) {
  return 0;
}

int script_opt_integer(int pe, int index, script_int_t *i) {
  return 0;
}

int script_opt_boolean(int pe, int index, int *b) {
  return 0;
}

int script_opt_string(int pe, int index, char **s) {
  return 0;
}

int script_push_boolean(int pe, int b) {
  return 0;
}

int script_push_integer(int pe, script_int_t i) {
  return 0;
}

int script_push_string(int pe, char *s) {
  return 0;
}

int script_push_lstring(int pe, char *s, int len) {
  return 0;
}

int script_push_object(int pe, script_ref_t r) {
  return 0;
}

int script_add_function(int pe, script_ref_t obj, char *name, int (*f)(int pe)) {
  return 0;
}

int script_add_boolean(int pe, script_ref_t obj, char *name, int b) {
  return 0;
}

int script_add_iconst(int pe, script_ref_t obj, char *name, script_int_t value) {
  return 0;
}

int script_add_sconst(int pe, script_ref_t obj, char *name, char *value) {
  return 0;
}

uint32_t script_engine_id(script_engine_t *engine) {
  return 0;
}

script_engine_t *script_get_engine(int pe) {
  return NULL;
}

script_ref_t script_create_function(int pe, int (*f)(int pe)) {
  return 0;
}

script_ref_t script_create_function_data(int pe, int (*f)(int pe, void *data), void *data) {
  return 0;
}

script_ref_t script_create_object(int pe) {
  return 0;
}

int script_global_set(int pe, char *name, script_arg_t *value) {
  return 0;
}

int script_global_get(int pe, char *name, script_arg_t *value) {
  return 0;
}

int script_returned_value(script_arg_t *ret) {
  return 0;
}

int script_get_last_error(int pe, char *buf, int max) {
  return 0;
}

int script_create(script_engine_t *engine) {
  return 0;
}

int script_destroy(int ptr) {
  return 0;
}

int script_call_args(int pe, script_ref_t ref, script_arg_t *ret, int n, script_arg_t *args) {
  return 0;
}

int script_call(int pe, script_ref_t ref, script_arg_t *ret, char *types, ...) {
  return 0;
}

int script_run(int pe, char *filename, int argc, char *argv[], int str) {
  return 0;
}

int script_remove_ref(int pe, script_ref_t ref) {
  return 0;
}

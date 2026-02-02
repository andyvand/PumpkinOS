#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "pumpkin.h"
#include "logtrap.h"
#include "m68k.h"
#include "m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_flpemtrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];
  uint32_t u = 0;
  flp_float_t f;
  int32_t i = 0;
  uint32_t resP = 0;
  flp_double_t d;
  Int32 res = 0;
  flp_float_t a;
  flp_float_t b;
  ARG_DOUBLE_BEGIN(ab);
  ARG_DOUBLE_BEGIN(bb);

  switch (sel) {
    case sysFloatEm_fp_round: {
      // Int32 _fp_round(Int32)
      // Constants passed to and received from _fp_round:
      //   flpToNearest, flpTowardZero, flpUpward, or flpDownward
      uint32_t u = ARG32;
      debug(DEBUG_TRACE, "FlpEm", "_fp_round(0x%08X): 0x%08X", u, u);
      m68k_set_reg(M68K_REG_D0, flpToNearest); // XXX argument ignored
    }
    break;
    case sysFloatEm_fp_get_fpscr: {
      // Int32 _fp_get_fpscr(void)
      // Constants passed to _fp_set_fpscr and received from _fp_get_fpscr:
      //   flpInvalid, flpOverflow, flpUnderflow, flpDivByZero, or flpInexact
      debug(DEBUG_TRACE, "FlpEm", "_fp_get_fpscr(): 0x%08X", flpOverflow);
      m68k_set_reg(M68K_REG_D0, flpOverflow);
    }
    break;
    case sysFloatEm_fp_set_fpscr: {
      // void _fp_set_fpscr(Int32)
      uint32_t u = ARG32;
      debug(DEBUG_TRACE, "FlpEm", "_fp_set_fpscr(0x%08X)", u);
    }
    break;
    case sysFloatEm_f_utof: {
      // FlpFloat _f_utof(UInt32)
      u = ARG32;
      f.f = (float)u;
      debug(DEBUG_TRACE, "FlpEm", "_f_utof(%u): %f", u, f.f);
      m68k_set_reg(M68K_REG_D0, f.i);
    }
    break;
    case sysFloatEm_f_itof: {
      // FlpFloat _f_itof(Int32)
      i = ARG32;
      f.f = (float)i;
      debug(DEBUG_TRACE, "FlpEm", "_f_itof(%d): %f", i, f.f);
      m68k_set_reg(M68K_REG_D0, f.i);
    }
      break;
    case sysFloatEm_d_itod: {
      // FlpDouble _d_itod(Int32)
      resP = ARG32;
      i = ARG32;
      d.d = (double)i;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_d_itod(%d): %f", i, d.d);
    }
      break;
    case sysFloatEm_d_utod: {
      // FlpDouble _d_utod(UInt32)
      resP = ARG32;
      u = ARG32;
      d.d = (double)u;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_d_utod(%u): %f", u, d.d);
    }
      break;
    case sysFloatEm_d_dtof: {
	  ARG_DOUBLE(ab);
      f.f = (float)(ab.d);
      debug(DEBUG_TRACE, "FlpEm", "_f_dtof(%f): %f", ab.d, f.f);
      m68k_set_reg(M68K_REG_D0, f.i);
    }
      break;
    case sysFloatEm_f_ftou: {
      f.i = ARG32;
      u = (UInt32)f.f;
      debug(DEBUG_TRACE, "FlpEm", "_f_ftou(%f): %u", f.f, u);
      m68k_set_reg(M68K_REG_D0, u);
    }
      break;
    case sysFloatEm_f_ftoi: {
      f.i = ARG32;
      i = (Int32)f.f;
      debug(DEBUG_TRACE, "FlpEm", "_f_ftoi(%f): %d", f.f, i);
      m68k_set_reg(M68K_REG_D0, i);
    }
      break;
    case sysFloatEm_f_ftod: {
      // FlpDouble _f_ftod(FlpFloat)
      resP = ARG32;
      f.i = ARG32;
      d.d = f.f;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_f_ftod(%f): %f", f.f, d.d);
    }
      break;
    case sysFloatEm_d_dtou: {
      // UInt32 _d_dtou(FlpDouble)
      ARG_DOUBLE(ab);
      u = (UInt32)(ab.d);
      debug(DEBUG_TRACE, "FlpEm", "_d_dtou(%f): %u", ab.d, u);
      m68k_set_reg(M68K_REG_D0, u);
    }
      break;
    case sysFloatEm_d_dtoi: {
      // Int32 _d_dtoi(FlpDouble)
      ARG_DOUBLE(ab);
      i = (Int32)(ab.d);
      debug(DEBUG_TRACE, "FlpEm", "_d_dtoi(%f): %d", ab.d, i);
      m68k_set_reg(M68K_REG_D0, i);
    }
      break;
    case sysFloatEm_d_neg: {
      resP = ARG32;
      ARG_DOUBLE(ab);
      d.d = -ab.d;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_d_neg(%f): %f", ab.d, d.d);
    }
      break;
    case sysFloatEm_d_cmp: {
      // Int32 _d_cmp(FlpDouble, FlpDouble)
      ARG_DOUBLE(ab);
      ARG_DOUBLE(bb);
      if (ab.d == bb.d) res = 0;
      else if (bb.d < ab.d) res = -1;
      else res = 1;
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_cmp(%f, %f): %d", ab.d, bb.d, res);
    }
      break;
    case sysFloatEm_d_add: {
      // FlpDouble _d_add(FlpDouble, FlpDouble)
      resP = ARG32;
      ARG_DOUBLE(ab);
      ARG_DOUBLE(bb);
      //debug(DEBUG_TRACE, "FlpEm", "_d_add a=%016llx b=%016llx resP=%08x", ab.i, bb.i, resP);
      d.d = ab.d + bb.d;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_d_add(%f, %f): %f", ab.d, bb.d, d.d);
    }
      break;
    case sysFloatEm_d_mul: {
      // FlpDouble _d_mul(FlpDouble, FlpDouble)
      resP = ARG32;
      ARG_DOUBLE(ab);
      ARG_DOUBLE(bb);
      //debug(DEBUG_TRACE, "FlpEm", "_d_mul a=%016llx b=%016llx", ab.i, bb.i);
      d.d = ab.d * bb.d;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_d_mul(%f, %f): %f", ab.d, bb.d, d.d);
    }
      break;
    case sysFloatEm_d_sub: {
      // FlpDouble _d_sub(FlpDouble, FlpDouble)
      resP = ARG32;
      ARG_DOUBLE(ab);
      ARG_DOUBLE(bb);
      d.d = ab.d - bb.d;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_d_sub(%f, %f): %f", ab.d, bb.d, d.d);
    }
      break;
    case sysFloatEm_d_div: {
      // FlpDouble _d_div(FlpDouble, FlpDouble)
      resP = ARG32;
      ARG_DOUBLE(ab);
      ARG_DOUBLE(bb);
      d.d = ab.d / bb.d;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_d_div(%f, %f): %f", ab.d, bb.d, d.d);
    }
      break;
    case sysFloatEm_d_feq: {
      // Int32 _d_feq(FlpDouble, FlpDouble)
      ARG_DOUBLE(ab);
      ARG_DOUBLE(bb);
      res = (ab.d == bb.d);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_feq(%f, %f): %d", ab.d, bb.d, res);
    }
      break;
    case sysFloatEm_d_fne: {
      // Int32 _d_fne(FlpDouble, FlpDouble)
      ARG_DOUBLE(ab);
      ARG_DOUBLE(bb);
      res = (ab.d != bb.d);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_fne(%f, %f): %d", ab.d, bb.d, res);
    }
      break;
    case sysFloatEm_d_flt: {
      // Int32 _d_flt(FlpDouble, FlpDouble)
      ARG_DOUBLE(ab);
      ARG_DOUBLE(bb);
      res = (ab.d < bb.d);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_flt(%f, %f): %d", ab.d, bb.d, res);
    }
      break;
    case sysFloatEm_d_fle: {
      // Int32 _d_fle(FlpDouble, FlpDouble)
      ARG_DOUBLE(ab);
      ARG_DOUBLE(bb);
      res = (ab.d <= bb.d);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_fle(%f, %f): %d", ab.d, bb.d, res);
    }
      break;
    case sysFloatEm_d_fgt: {
      // Int32 _d_fgt(FlpDouble, FlpDouble)
      ARG_DOUBLE(ab);
      ARG_DOUBLE(bb);
      res = (ab.d > bb.d);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_fgt(%f, %f): %d", ab.d, bb.d, res);
    }
      break;
    case sysFloatEm_d_fge: {
      // Int32 _d_fge(FlpDouble, FlpDouble)
      ARG_DOUBLE(ab);
      ARG_DOUBLE(bb);
      res = (ab.d >= bb.d);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_fge(%f, %f): %d", ab.d, bb.d, res);
    }
      break;
    case sysFloatEm_f_feq: {
      a.i = ARG32;
      b.i = ARG32;
      res = (a.f == b.f);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_f_feq(%f, %f): %d", a.f, b.f, res);
    }
      break;
    case sysFloatEm_f_fne: {
      a.i = ARG32;
      b.i = ARG32;
      res = (a.f != b.f);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_fne(%f, %f): %d", a.f, b.f, res);
    }
      break;
    case sysFloatEm_f_add: {
      a.i = ARG32;
      b.i = ARG32;
      f.f = a.f + b.f;
      debug(DEBUG_TRACE, "FlpEm", "_f_add(%f, %f): %f", (double)a.f, (double)b.f, (double)f.f);
      m68k_set_reg(M68K_REG_D0, f.i);
    }
      break;
    case sysFloatEm_f_sub: {
      a.i = ARG32;
      b.i = ARG32;
      f.f = a.f - b.f;
      debug(DEBUG_TRACE, "FlpEm", "_f_sub(%f, %f): %f", a.f, b.f, f.f);
      m68k_set_reg(M68K_REG_D0, f.i);
    }
      break;
    case sysFloatEm_f_mul: {
      a.i = ARG32;
      b.i = ARG32;
      f.f = a.f * b.f;
      debug(DEBUG_TRACE, "FlpEm", "_f_mul(%f, %f): %f", a.f, b.f, f.f);
      m68k_set_reg(M68K_REG_D0, f.i);
    }
      break;
    case sysFloatEm_f_div: {
      a.i = ARG32;
      b.i = ARG32;
      f.f = a.f / b.f;
      debug(DEBUG_TRACE, "FlpEm", "_f_div(%f, %f): %f", a.f, b.f, f.f);
      m68k_set_reg(M68K_REG_D0, f.i);
    }
      break;
    case sysFloatEm_f_flt: {
      // Int32 _f_flt(FlpFloat, FlpFloat)
      a.i = ARG32;
      b.i = ARG32;
      res = (a.f < b.f);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_flt(%f, %f): %d", a.f, b.f, res);
    }
      break;
    case sysFloatEm_f_fle: {
      // Int32 _f_fle(FlpFloat, FlpFloat)
      a.i = ARG32;
      b.i = ARG32;
      res = (a.f <= b.f);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_fle(%f, %f): %d", a.f, b.f, res);
    }
      break;
    case sysFloatEm_f_fgt: {
      // Int32 _f_fgt(FlpFloat, FlpFloat)
      a.i = ARG32;
      b.i = ARG32;
      res = (a.f > b.f);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_fgt(%f, %f): %d", a.f, b.f, res);
    }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "FlpEmDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}

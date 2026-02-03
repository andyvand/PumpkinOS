    case 0: { // void Pterm0(void)
        Pterm0();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pterm0()");
      }
      break;

    case 1: { // int32_t Cconin(void)
        res = 0;
        res = Cconin();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cconin(): %d", res);
      }
      break;

    case 2: { // int32_t Cconout(c)
        c = ARG16;
        res = 0;
        res = Cconout(c);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cconout(%d): %d", c, res);
      }
      break;

    case 3: { // int32_t Cauxin(void)
        res = 0;
        res = Cauxin();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cauxin(): %d", res);
      }
      break;

    case 4: { // int32_t Cauxout(c)
        c = ARG16;
        res = 0;
        res = Cauxout(c);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cauxout(%d): %d", c, res);
      }
      break;

    case 5: { // int32_t Cprnout(c)
        c = ARG16;
        res = 0;
        res = Cprnout(c);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cprnout(%d): %d", c, res);
      }
      break;

    case 6: { // int32_t Crawio(int16_t w)
        w = ARG16;
        res = 0;
        res = Crawio(w);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Crawio(%d): %d", w, res);
      }
      break;

    case 7: { // int32_t Crawcin(void)
        res = 0;
        res = Crawcin();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Crawcin(): %d", res);
      }
      break;

    case 8: { // int32_t Cnecin(void)
        res = 0;
        res = Cnecin();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cnecin(): %d", res);
      }
      break;

    case 9: { // int32_t Cconws(uint8_t *buf)
        valid = 0;
        abuf = ARG32;
        buf = (uint8_t *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Cconws(buf);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cconws(0x%08X): %d", abuf, res);
      }
      break;

    case 10: { // int32_t Cconrs(void)
        res = 0;
        res = Cconrs();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cconrs(): %d", res);
      }
      break;

    case 11: { // int32_t Cconis(void)
        res = 0;
        res = Cconis();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cconis(): %d", res);
      }
      break;

    case 14: { // int32_t Dsetdrv(drv)
        drv = ARG16;
        res = 0;
        res = Dsetdrv(drv);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsetdrv(%d): %d", drv, res);
      }
      break;

    case 16: { // cconos(void)
        res = 0;
        res = Cconos();
        m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cconos(): %d", res);
      }
      break;

    case 17: { // cprnos(void)
        res = 0;
        res = Cprnos();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cprnos(): %d", res);
      }
      break;

    case 18: { // cauxis(void)
        res = 0;
        res = Cauxis();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cauxis(): %d", res);
      }
      break;

    case 19: { // cauxos(void)
        res = 0;
        res = Cauxos();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cauxos(): %d", res);
      }
      break;

    case 20: { // int32_t Maddalt(start, size)
        valid = 0;
        astart = ARG32;
        start = (void *)(memory + astart);
		size = ARG32;
		res = 0;
		valid |= (uint8_t *)start >= data->memory && (uint8_t *)start < data->memory + data->memorySize;
        if (valid) {
          res = Maddalt(start, size);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Maddalt(0x%08X, %d): %d", astart, size, res);
      }
      break;

    case 21: { // int32_t Srealloc(int32_t len)
        len = ARG32;
        res = 0;
        res = Srealloc(len);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Srealloc(%d): %d", len, res);
      }
      break;

    case 25: { // int16_t Dgetdrv(void)
        res = 0;
        res = Dgetdrv();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dgetdrv(): %d", res);
      }
      break;

    case 26: { // void Fsetdta(DTA *buf)
        abuf = ARG32;
        write_long(data->basePageStart + 0x0020, abuf);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsetdta(0x%08X)", abuf);
      }
      break;

    case 32: { // int32_t Super(void *stack)
        // switches between user-mode and supervisor-mode, or interrogates the current mode
        // stack: if 1, the current mode will be interrogated
        //        if 0, switches between user-mode and supervisor-mode
        //        other value: sets the new address for the supervisor stack
        // return value:
        //   if the state is to be obtained, value 0 means user-mode and value -1 means supervisor-mode
        //   else returns the address of the old supervisor stack, provided mode is supervisor-mode

        astack = ARG32;
        res = 0;
        if (astack == 1) {
          res = data->supervisor ? -1 : 0;
        } else if (astack == 0) {
          data->supervisor = !data->supervisor;
          debug(DEBUG_TRACE, "TOS", "%s supervisor mode", data->supervisor ? "entering" : "leaving");
        } 
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Super(0x%08X): %d", astack, res);
      }
      break;

    case 42: { // uint16_t Tgetdate(void)
        res = 0;
        res = Tgetdate();
		m68k_set_reg(M68K_REG_D0, (uint16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tgetdate(): %d", res);
      }
      break;

    case 43: { // int16_t Tsetdate(uint16_t date)
        date = ARG16;
        res = 0;
		res = (uint16_t)Tsetdate(date);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tsetdate(%d): %d", date, res);
      }
      break;

    case 44: { // uint32_t Tgettime(void)
        res = 0;
        res = Tgettime();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tgettime(): %d", res);
      }
      break;

    case 45: { // int16_t Tsettime(uint16_t time)
        time = ARG16;
        res = 0;
		res = (uint16_t)Tsettime((int16_t)time);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tsettime(%d): %d", time, res);
      }
      break;

    case 47: { // DTA *Fgetdta(void)
        res = read_long(data->basePageStart + 0x0020);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fgetdta(): 0x%08X", res);
      }
      break;

    case 48: { // uint16_t Sversion(void)
        res = 0;
		res = (uint16_t)Sversion();
		m68k_set_reg(M68K_REG_D0, (uint16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Sversion(): %d", res);
      }
      break;

    case 49: { // void Ptermres(int32_t keepcnt, int16_t retcode)
        keepcnt = ARG32;
		retcode = ARG16;
        Ptermres(keepcnt, retcode);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ptermres(%d, %d)", keepcnt, retcode);
      }
      break;

    case 54: { // int16_t Dfree(DISKINFO *buf, driveno)
        valid = 0;
        abuf = ARG32;
        bufd = (DISKINFO *)(memory + abuf);
        valid |= (uint8_t *)bufd >= data->memory && (uint8_t *)bufd < data->memory + data->memorySize;
        driveno = ARG16;
        res = 0;
        if (valid) {
          res = (uint16_t)Dfree(bufd, driveno);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dfree(0x%08X, %d): %d", abuf, driveno, res);
      }
      break;

    case 57: { // int32_t Dcreate(path)
        valid = 0;
        apath = ARG32;
        path = (char *)(memory + apath);
        valid |= (uint8_t *)path >= data->memory && (uint8_t *)path < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Dcreate(path);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dcreate(0x%08X): %d", apath, res);
      }
      break;

    case 58: { // int32_t Ddelete(path)
        valid = 0;
        apath = ARG32;
        path = (char *)(memory + apath);
        valid |= (uint8_t *)path >= data->memory && (uint8_t *)path < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Ddelete(path);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ddelete(0x%08X): %d", apath, res);
      }
      break;

    case 59: { // int16_t Dsetpath(path)
        valid = 0;
        apath = ARG32;
        path = (char *)(memory + apath);
        valid |= (uint8_t *)path >= data->memory && (uint8_t *)path < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = (uint16_t)Dsetpath(path);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsetpath(0x%08X): %d", apath, res);
      }
      break;

    case 60: { // int16_t Fcreate(fname, attr)
        valid = 0;
        afname = ARG32;
        fname = (char *)(memory + afname);
        valid |= (uint8_t *)fname >= data->memory && (uint8_t *)fname < data->memory + data->memorySize;
        attr = ARG16;
        res = 0;
        if (valid) {
		  res = (uint16_t)Fcreate(fname, attr);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fcreate(0x%08X, %d): %d", afname, attr, res);
      }
      break;

    case 61: { // int32_t Fopen(fname, mode)
        valid = 0;
        afname = ARG32;
        fname = (char *)(memory + afname);
        valid |= (uint8_t *)fname >= data->memory && (uint8_t *)fname < data->memory + data->memorySize;
        mode = ARG16;
        res = -1;
        if (valid) {
		  res = Fopen(fname, (int16_t)mode);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fopen(0x%08X, %d): %d", afname, mode, res);
      }
      break;

    case 62: { // int16_t Fclose(handle)
        handle = ARG16;
        res = 0;
		res = (uint16_t)Fclose(handle);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fclose(%d): %d", handle, res);
      }
      break;

    case 63: { // int32_t Fread(handle, count, buf)
        valid = 0;
        handle = ARG16;
        count = ARG32;
        abuf = ARG32;
        buf = (void *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        res = -1;
        if (valid) {
          res = Fread(handle, count, buf);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fread(%d, %d, 0x%08X): %d", handle, count, abuf, res);
      }
      break;

    case 64: { // int32_t Fwrite(handle, count, buf)
        valid = 0;
        handle = ARG16;
        count = ARG32;
        abuf = ARG32;
        buf = (void *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        res = -1;
        if (valid) {
          res = Fwrite(handle, count, buf);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fwrite(%d, %d, 0x%08X): %d", handle, count, abuf, res);
      }
      break;

    case 65: { // fdelete(fname)
        valid = 0;
        afname = ARG32;
        fname = (char *)(memory + afname);
        valid |= (uint8_t *)fname >= data->memory && (uint8_t *)fname < data->memory + data->memorySize;
        res = 0;
        if (valid) {
		  res = (uint16_t)Fdelete(fname);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fdelete(0x%08X): %d", afname, res);
      }
      break;

    case 66: { // int32_t Fseek(offset, handle, seekmode)
        offset = ARG32;
        handle = ARG16;
        seekmode = ARG16;
        res = 0;
        res = Fseek(offset, handle, seekmode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fseek(%d, %d, %d): %d", offset, handle, seekmode, res);
      }
      break;

    case 67: { // int16_t Fattrib(filename, wflag, attrib)
        valid = 0;
        afilename = ARG32;
        filename = (char *)(memory + afilename);
        valid |= (uint8_t *)filename >= data->memory && (uint8_t *)filename < data->memory + data->memorySize;
        wflag = ARG16;
        attrib = ARG16;
        res = 0;
        if (valid) {
		  res = (uint16_t)Fattrib(filename, wflag, attrib);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fattrib(0x%08X, %d, %d): %d", afilename, wflag, attrib, res);
      }
      break;

    case 68: { // void *Mxalloc(amount, mode)
        amount = ARG32;
        mode = ARG16;
        resv = NULL;
		resv = Mxalloc(amount, (int16_t)mode);
        ares = resv ? (uint32_t)((uint8_t *)resv - memory) : 0;
        m68k_set_reg(M68K_REG_D0, ares);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Mxalloc(%d, %d): 0x%08X", amount, mode, ares);
      }
      break;

    case 69: { // fdup(handle)
        handle = ARG16;
        res = 0;
		res = (uint16_t)Fdup(handle);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fdup(%d): %d", handle, res);
      }
      break;

    case 70: { // int16_t Fforce(stdh, nonstdh)
        stdh = ARG16;
        nonstdh = ARG16;
        res = 0;
		res = (uint16_t)Fforce(stdh, nonstdh);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fforce(%d, %d): %d", stdh, nonstdh, res);
      }
      break;

    case 71: { // int16_t Dgetpath(path, driveno)
        valid = 0;
        apath = ARG32;
        path = (char *)(memory + apath);
        valid |= (uint8_t *)path >= data->memory && (uint8_t *)path < data->memory + data->memorySize;
        driveno = ARG16;
        res = 0;
        if (valid) {
		  res = (uint16_t)Dgetpath(path, driveno);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dgetpath(0x%08X, %d): %d", apath, driveno, res);
      }
      break;

    case 73: { // int32_t Mfree(block)
        valid = 0;
        ablock = ARG32;
        block = (void *)(memory + ablock);
        valid |= (uint8_t *)block >= data->memory && (uint8_t *)block < data->memory + data->memorySize;
        res = -1;
        if (valid) {
          res = Mfree(block);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Mfree(0x%08X): %d", ablock, res);
      }
      break;

    case 74: { // int32_t Mshrink(block, int32_t newsiz)
        valid = 0;
        ARG16; // remove unused 16-bits word from stack
        ablock = ARG32;
        block = (void *)(memory + ablock);
        valid |= (uint8_t *)block >= data->memory && (uint8_t *)block < data->memory + data->memorySize;
        newsiz = ARG32;
        res = 0;
        if (valid) {
          res = Mshrink(block, newsiz);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Mshrink(0x%08X, %d): %d", ablock, newsiz, res);
      }
      break;

    case 76: { // void Pterm(uint16_t retcode)
        retcode = (int16_t)ARG16;
        Pterm((uint16_t)retcode);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pterm(%d)", retcode);
      }
      break;

    case 78: { // int32_t Fsfirst(filename, attr)
        valid = 0;
        afilename = ARG32;
        filename = (char *)(memory + afilename);
        valid |= (uint8_t *)filename >= data->memory && (uint8_t *)filename < data->memory + data->memorySize;
        attr = ARG16;
        res = 0;
        if (valid) {
          res = Fsfirst(filename, attr);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsfirst(0x%08X, %d): %d", afilename, attr, res);
      }
      break;

    case 79: { // int16_t Fsnext(void)
        res = 0;
		res = (uint16_t)Fsnext();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsnext(): %d", res);
      }
      break;

    case 86: { // int32_t Frename(oldname, newname)
        valid = 0;
        aoldname = ARG32;
        oldname = (char *)(memory + aoldname);
        valid |= (uint8_t *)oldname >= data->memory && (uint8_t *)oldname < data->memory + data->memorySize;
        anewname = ARG32;
        newname = (char *)(memory + anewname);
        valid |= (uint8_t *)newname >= data->memory && (uint8_t *)newname < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Frename(oldname, newname);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Frename(0x%08X, 0x%08X): %d", aoldname, anewname, res);
      }
      break;

    case 87: { // void Fdatime(timeptr, handle, wflag)
        valid = 0;
        atimeptr = ARG32;
        timeptr = (DOSTIME *)(memory + atimeptr);
        valid |= (uint8_t *)timeptr >= data->memory && (uint8_t *)timeptr < data->memory + data->memorySize;
        handle = ARG16;
        wflag = ARG16;
        if (valid) {
          Fdatime(timeptr, handle, wflag);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fdatime(0x%08X, %d, %d)", atimeptr, handle, wflag);
      }
      break;

    case 92: { // int32_t Flock(handle, mode, int32_t start, length)
        handle = ARG16;
        mode = ARG16;
        astart = ARG32;
        length = ARG32;
        res = 0;
		res = Flock(handle, (int16_t)mode, astart, length);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Flock(%d, %d, %d, %d): %d", handle, mode, astart, length, res);
      }
      break;

    case 96: { // int32_t Nversion(void)
        res = 0;
        res = Nversion();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Nversion(): %d", res);
      }
      break;

    case 98: { // int32_t Frlock(handle, int32_t start, length)
        handle = ARG16;
        astart = ARG32;
        length = ARG32;
        res = 0;
		res = Frlock(handle, (int32_t)astart, length);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Frlock(%d, %d, %d): %d", handle, astart, length, res);
      }
      break;

    case 99: { // int32_t Frunlock(handle, int32_t start)
        handle = ARG16;
        astart = ARG32;
        res = 0;
		res = Frunlock(handle, (int32_t)astart);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Frunlock(%d, %d): %d", handle, astart, res);
      }
      break;

    case 100: { // int32_t Flock2(handle, length)
        handle = ARG16;
        length = ARG32;
        res = 0;
        res = Flock2(handle, length);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Flock2(%d, %d): %d", handle, length, res);
      }
      break;

    case 101: { // int32_t Funlock(handle)
        handle = ARG16;
        res = 0;
        res = Funlock(handle);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Funlock(%d): %d", handle, res);
      }
      break;

    case 102: { // int32_t Fflush(handle)
        handle = ARG16;
        res = 0;
        res = Fflush(handle);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fflush(%d): %d", handle, res);
      }
      break;

    case 255: { // void Syield(void)
        Syield();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Syield()");
      }
      break;

    case 256: { // int32_t Fpipe(usrh)
        valid = 0;
        ausrh = ARG32;
        usrh = (int16_t *)(memory + ausrh);
        valid |= (uint8_t *)usrh >= data->memory && (uint8_t *)usrh < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Fpipe(usrh);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fpipe(0x%08X): %d", ausrh, res);
      }
      break;

    case 257: { // int32_t Ffchown(fd, uid, gid)
        fd = ARG16;
        uid = ARG16;
        gid = ARG16;
        res = 0;
        res = Ffchown(fd, uid, gid);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ffchown(%d, %d, %d): %d", fd, uid, gid, res);
      }
      break;

    case 258: { // int32_t Ffchmod(fd, mode)
        fd = ARG16;
        mode = ARG16;
        res = 0;
		res = Ffchmod(fd, (int16_t)mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ffchmod(%d, %d): %d", fd, mode, res);
      }
      break;

    case 259: { // int16_t Fsync(handle)
        handle = ARG16;
        res = 0;
		res = (uint16_t)Fsync(handle);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsync(%d): %d", handle, res);
      }
      break;

    case 261: { // int32_t Finstat(fh)
        fh = ARG16;
        res = 0;
        res = Finstat(fh);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Finstat(%d): %d", fh, res);
      }
      break;

    case 262: { // int32_t Foutstat(fh)
        fh = ARG16;
        res = 0;
        res = Foutstat(fh);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Foutstat(%d): %d", fh, res);
      }
      break;

    case 263: { // int32_t Fgetchar(fh, mode)
        fh = ARG16;
        mode = ARG16;
        res = 0;
		res = Fgetchar(fh, (int16_t)mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fgetchar(%d, %d): %d", fh, mode, res);
      }
      break;

    case 264: { // int32_t Fputchar(fh, ch, mode)
        fh = ARG16;
        ch = ARG32;
        mode = ARG16;
        res = 0;
		res = Fputchar(fh, ch, (int16_t)mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fputchar(%d, %d, %d): %d", fh, ch, mode, res);
      }
      break;

    case 265: { // int32_t Pwait(void)
        res = 0;
        res = Pwait();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pwait(): %d", res);
      }
      break;

    case 266: { // int16_t Pnice(delta)
        delta = ARG16;
        res = 0;
		res = (uint16_t)Pnice(delta);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pnice(%d): %d", delta, res);
      }
      break;

    case 267: { // int16_t Pgetpid(void)
        res = 0;
		res = (uint16_t)Pgetpid();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetpid(): %d", res);
      }
      break;

    case 268: { // int16_t Pgetppid(void)
        res = 0;
		res = (uint16_t)Pgetppid();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetppid(): %d", res);
      }
      break;

    case 269: { // int16_t Pgetpgrp(void)
        res = 0;
		res = (uint16_t)Pgetpgrp();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetpgrp(): %d", res);
      }
      break;

    case 270: { // int16_t Psetpgrp(pid, newgrp)
        pid = ARG16;
        newgrp = ARG16;
        res = 0;
		res = (uint16_t)Psetpgrp(pid, newgrp);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetpgrp(%d, %d): %d", pid, newgrp, res);
      }
      break;

    case 271: { // int16_t Pgetuid(void)
        res = 0;
		res = (uint16_t)Pgetuid();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetuid(): %d", res);
      }
      break;

    case 272: { // int16_t Psetuid(id)
        id = ARG16;
        res = 0;
		res = (uint16_t)Psetuid((int16_t)id);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetuid(%d): %d", id, res);
      }
      break;

    case 273: { // int32_t Pkill(pid, sig)
        pid = ARG16;
        sig = ARG16;
        res = 0;
        res = Pkill(pid, sig);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pkill(%d, %d): %d", pid, sig, res);
      }
      break;

    case 274: { // int32_t Psignal(sig, handler)
        sig = ARG16;
        handler = ARG32;
        res = 0;
        res = Psignal(sig, handler);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psignal(%d, %d): %d", sig, handler, res);
      }
      break;

    case 275: { // int16_t Pvfork(void)
        res = 0;
		res = (uint16_t)Pvfork();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pvfork(): %d", res);
      }
      break;

    case 276: { // int16_t Pgetgid(void)
        res = 0;
		res = (uint16_t)Pgetgid();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetgid(): %d", res);
      }
      break;

    case 277: { // int16_t Psetgid(id)
        id = ARG16;
        res = 0;
		res = (uint16_t)Psetgid((int16_t)id);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetgid(%d): %d", id, res);
      }
      break;

    case 278: { // int32_t Psigblock(mask)
        mask = ARG32;
        res = 0;
        res = Psigblock(mask);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psigblock(%d): %d", mask, res);
      }
      break;

    case 279: { // int32_t Psigsetmask(mask)
        mask = ARG32;
        res = 0;
        res = Psigsetmask(mask);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psigsetmask(%d): %d", mask, res);
      }
      break;

    case 280: { // int32_t Pusrval(int32_t val)
        val = ARG32;
        res = 0;
        res = Pusrval(val);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pusrval(%d): %d", val, res);
      }
      break;

    case 281: { // int16_t Pdomain(dom)
        dom = ARG16;
        res = 0;
		res = (uint16_t)Pdomain(dom);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pdomain(%d): %d", dom, res);
      }
      break;

    case 282: { // void Psigreturn(void)
        Psigreturn();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psigreturn()");
      }
      break;

    case 283: { // int16_t Pfork(void)
        res = 0;
		res = (uint16_t)Pfork();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pfork(): %d", res);
      }
      break;

    case 284: { // int32_t Pwait3(flag, rusage)
        valid = 0;
        flag = ARG16;
        arusage = ARG32;
        rusage = (int32_t *)(memory + arusage);
        valid |= (uint8_t *)rusage >= data->memory && (uint8_t *)rusage < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Pwait3(flag, rusage);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pwait3(%d, 0x%08X): %d", flag, arusage, res);
      }
      break;

    case 285: { // int32_t Fselect(timeout, rfds, wfds)
        valid = 0;
        timeout = ARG16;
        arfds = ARG32;
        rfds = (int32_t *)(memory + arfds);
        valid |= (uint8_t *)rfds >= data->memory && (uint8_t *)rfds < data->memory + data->memorySize;
        awfds = ARG32;
        wfds = (int32_t *)(memory + awfds);
        valid |= (uint8_t *)wfds >= data->memory && (uint8_t *)wfds < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Fselect(timeout, rfds, wfds);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fselect(%d, 0x%08X, 0x%08X): %d", timeout, arfds, awfds, res);
      }
      break;

    case 286: { // int32_t Prusage(r)
        valid = 0;
        ar = ARG32;
        rp = (int32_t *)(memory + ar);
        valid |= (uint8_t *)rp >= data->memory && (uint8_t *)rp < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Prusage(rp);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Prusage(0x%08X): %d", ar, res);
      }
      break;

    case 287: { // int32_t Psetlimit(lim, value)
        lim = ARG16;
        value = ARG32;
        res = 0;
        res = Psetlimit(lim, value);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetlimit(%d, %d): %d", lim, value, res);
      }
      break;

    case 288: { // int32_t Talarm(time)
        time = ARG32;
        res = 0;
        res = Talarm(time);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Talarm(%d): %d", time, res);
      }
      break;

    case 289: { // void Pause(void)
        Pause();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pause()");
      }
      break;

    case 290: { // int32_t Sysconf(int16_t n)
        int16_t n = ARG16;
        res = 0;
        res = Sysconf(n);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Sysconf(%d): %d", n, res);
      }
      break;

    case 291: { // int32_t Psigpending(void)
        res = 0;
        res = Psigpending();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psigpending(): %d", res);
      }
      break;

    case 292: { // int32_t Dpathconf(name, mode)
        valid = 0;
        aname = ARG32;
        name = (char *)(memory + aname);
        valid |= (uint8_t *)name >= data->memory && (uint8_t *)name < data->memory + data->memorySize;
        mode = ARG16;
        res = 0;
        if (valid) {
			res = Dpathconf((uint8_t *)name, (int16_t)mode);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dpathconf(0x%08X, %d): %d", aname, mode, res);
      }
      break;

    case 293: { // int32_t Pmsg(mode, mbox, msg)
        valid = 0;
        mode = ARG16;
        mbox = ARG32;
        amsg = ARG32;
        msg = (void *)(memory + amsg);
        valid |= (uint8_t *)msg >= data->memory && (uint8_t *)msg < data->memory + data->memorySize;
        res = 0;
        if (valid) {
			res = Pmsg((int16_t)mode, mbox, msg);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pmsg(%d, %d, 0x%08X): %d", mode, mbox, amsg, res);
      }
      break;

    case 294: { // int32_t Fmidipipe(pid, int16_t in, int16_t out)
        pid = ARG16;
        in = ARG16;
        out = ARG16;
        res = 0;
        res = Fmidipipe(pid, in, out);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fmidipipe(%d, %d, %d): %d", pid, in, out, res);
      }
      break;

    case 295: { // int32_t Prenice(pid, delta)
        pid = ARG16;
        delta = ARG16;
        res = 0;
        res = Prenice(pid, delta);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Prenice(%d, %d): %d", pid, delta, res);
      }
      break;

    case 296: { // int32_t Dopendir(name, flag)
        valid = 0;
        aname = ARG32;
        name = (char *)(memory + aname);
        valid |= (uint8_t *)name >= data->memory && (uint8_t *)name < data->memory + data->memorySize;
        flag = ARG16;
        res = 0;
        if (valid) {
          res = Dopendir(name, flag);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dopendir(0x%08X, %d): %d", aname, flag, res);
      }
      break;

    case 297: { // int32_t Dreaddir(int16_t len, dirhandle, buf)
        valid = 0;
        len = ARG16;
        dirhandle = ARG32;
        abuf = ARG32;
        buf = (uint8_t *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        res = 0;
        if (valid) {
		  res = Dreaddir((int16_t)len, dirhandle, (char *)buf);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dreaddir(%d, %d, 0x%08X): %d", len, dirhandle, abuf, res);
      }
      break;

    case 298: { // int32_t Drewinddir(handlel)
        handlel = ARG32;
        res = 0;
        res = Drewinddir(handlel);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Drewinddir(%d): %d", handlel, res);
      }
      break;

    case 299: { // int32_t Dclosedir(dirhandle)
        dirhandle = ARG32;
        res = 0;
        res = Dclosedir(dirhandle);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dclosedir(%d): %d", dirhandle, res);
      }
      break;

    case 300: { // int32_t Fxattr(flag, name, xattr)
        valid = 0;
        flag = ARG16;
        aname = ARG32;
        name = (char *)(memory + aname);
        valid |= (uint8_t *)name >= data->memory && (uint8_t *)name < data->memory + data->memorySize;
        axattr = ARG32;
        xattr = (XATTR *)(memory + axattr);
        valid |= (uint8_t *)xattr >= data->memory && (uint8_t *)xattr < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Fxattr(flag, name, xattr);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fxattr(%d, 0x%08X, 0x%08X): %d", flag, aname, axattr, res);
      }
      break;

    case 301: { // int32_t Flink(oldname, newname)
        valid = 0;
        aoldname = ARG32;
        oldname = (char *)(memory + aoldname);
        valid |= (uint8_t *)oldname >= data->memory && (uint8_t *)oldname < data->memory + data->memorySize;
        anewname = ARG32;
        newname = (char *)(memory + anewname);
        valid |= (uint8_t *)newname >= data->memory && (uint8_t *)newname < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Flink(oldname, newname);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Flink(0x%08X, 0x%08X): %d", aoldname, anewname, res);
      }
      break;

    case 302: { // int32_t Fsymlink(oldname, newname)
        valid = 0;
        aoldname = ARG32;
        oldname = (char *)(memory + aoldname);
        valid |= (uint8_t *)oldname >= data->memory && (uint8_t *)oldname < data->memory + data->memorySize;
        anewname = ARG32;
        newname = (char *)(memory + anewname);
        valid |= (uint8_t *)newname >= data->memory && (uint8_t *)newname < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Fsymlink(oldname, newname);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsymlink(0x%08X, 0x%08X): %d", aoldname, anewname, res);
      }
      break;

    case 303: { // int32_t Freadlink(bufsiz, buf, name)
        valid = 0;
        bufsiz = ARG16;
        abuf = ARG32;
        buf = (uint8_t *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        aname = ARG32;
        name = (char *)(memory + aname);
        valid |= (uint8_t *)name >= data->memory && (uint8_t *)name < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Freadlink(bufsiz, (char *)buf, name);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Freadlink(%d, 0x%08X, 0x%08X): %d", bufsiz, abuf, aname, res);
      }
      break;

    case 304: { // int32_t Dcntl(cmd, name, arg)
        valid = 0;
        cmd = ARG16;
        aname = ARG32;
        name = (char *)(memory + aname);
        valid |= (uint8_t *)name >= data->memory && (uint8_t *)name < data->memory + data->memorySize;
        arg = ARG32;
        res = 0;
        if (valid) {
          res = Dcntl(cmd, name, arg);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dcntl(%d, 0x%08X, %d): %d", cmd, aname, arg, res);
      }
      break;

    case 305: { // int32_t Fchown(name, uid, gid)
        valid = 0;
        aname = ARG32;
        name = (char *)(memory + aname);
        valid |= (uint8_t *)name >= data->memory && (uint8_t *)name < data->memory + data->memorySize;
        uid = ARG16;
        gid = ARG16;
        res = 0;
        if (valid) {
          res = Fchown(name, uid, gid);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fchown(0x%08X, %d, %d): %d", aname, uid, gid, res);
      }
      break;

    case 306: { // int32_t Fchmod(name, mode)
        valid = 0;
        aname = ARG32;
        name = (char *)(memory + aname);
        valid |= (uint8_t *)name >= data->memory && (uint8_t *)name < data->memory + data->memorySize;
        mode = ARG16;
        res = 0;
        if (valid) {
			res = Fchmod(name, (int16_t)mode);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fchmod(0x%08X, %d): %d", aname, mode, res);
      }
      break;

    case 307: { // int16_t Pumask(mode)
        mode = ARG16;
        res = 0;
		res = (uint16_t)Pumask((int16_t)mode);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pumask(%d): %d", mode, res);
      }
      break;

    case 308: { // int32_t Psemaphore(mode, id, timeout)
        mode = ARG16;
        id = ARG32;
        timeout = ARG32;
        res = 0;
		res = Psemaphore((int16_t)mode, id, timeout);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psemaphore(%d, %d, %d): %d", mode, id, timeout, res);
      }
      break;

    case 309: { // int32_t Dlock(mode, drv)
        mode = ARG16;
        drv = ARG16;
        res = 0;
		res = Dlock((int16_t)mode, drv);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dlock(%d, %d): %d", mode, drv, res);
      }
      break;

    case 310: { // void Psigpause(mask)
        mask = ARG32;
        Psigpause(mask);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psigpause(%d)", mask);
      }
      break;

    case 311: { // int32_t Psigaction(sig, struct sigaction *act, struct sigaction *oact)
        valid = 0;
        sig = ARG16;
        aact = ARG32;
        act = (struct g_sigaction *)(memory + aact);
        valid |= (uint8_t *)act >= data->memory && (uint8_t *)act < data->memory + data->memorySize;
        aoact = ARG32;
        oact = (struct g_sigaction *)(memory + aoact);
        valid |= (uint8_t *)oact >= data->memory && (uint8_t *)oact < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Psigaction(sig, act, oact);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psigaction(%d, 0x%08X, 0x%08X): %d", sig, aact, aoact, res);
      }
      break;

    case 312: { // int32_t Pgeteuid(void)
        res = 0;
        res = Pgeteuid();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgeteuid(): %d", res);
      }
      break;

    case 313: { // int32_t Pgetegid(void)
        res = 0;
        res = Pgetegid();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetegid(): %d", res);
      }
      break;

    case 314: { // int32_t Pwaitpid(pid, flag, rusage)
        valid = 0;
        pid = ARG16;
        flag = ARG16;
        arusage = ARG32;
        rusage = (int32_t *)(memory + arusage);
        valid |= (uint8_t *)rusage >= data->memory && (uint8_t *)rusage < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Pwaitpid(pid, flag, rusage);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pwaitpid(%d, %d, 0x%08X): %d", pid, flag, arusage, res);
      }
      break;

    case 315: { // int32_t Dgetcwd(path, drv, int16_t size)
        valid = 0;
        apath = ARG32;
        path = (char *)(memory + apath);
        valid |= (uint8_t *)path >= data->memory && (uint8_t *)path < data->memory + data->memorySize;
        drv = ARG16;
        size = ARG16;
        res = 0;
        if (valid) {
		  res = Dgetcwd(path, drv, (int16_t)size);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dgetcwd(0x%08X, %d, %d): %d", apath, drv, size, res);
      }
      break;

    case 316: { // void Salert(msg)
        valid = 0;
        amsg = ARG32;
        msg = (uint8_t *)(memory + amsg);
        valid |= (uint8_t *)msg >= data->memory && (uint8_t *)msg < data->memory + data->memorySize;
        if (valid) {
          Salert(msg);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Salert(0x%08X)", amsg);
      }
      break;

    case 317: { // int32_t Tmalarm(time)
        time = ARG32;
        res = 0;
        res = Tmalarm(time);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tmalarm(%d): %d", time, res);
      }
      break;

    case 318: { // int32_t Psigintr(vec, sig)
        vec = ARG16;
        sig = ARG16;
        res = 0;
        res = Psigintr(vec, sig);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psigintr(%d, %d): %d", vec, sig, res);
      }
      break;

    case 319: { // int32_t Suptime(uptime, loadaverage)
        valid = 0;
        auptime = ARG32;
        uptime = (int32_t *)(memory + auptime);
        valid |= (uint8_t *)uptime >= data->memory && (uint8_t *)uptime < data->memory + data->memorySize;
        aloadaverage = ARG32;
        loadaverage = (int32_t *)(memory + aloadaverage);
        valid |= (uint8_t *)loadaverage >= data->memory && (uint8_t *)loadaverage < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Suptime(uptime, loadaverage);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Suptime(0x%08X, 0x%08X): %d", auptime, aloadaverage, res);
      }
      break;

    case 320: { // int16_t Ptrace(request, pid, addr, data)
        valid = 0;
        request = ARG16;
        pid = ARG16;
        aaddr = ARG32;
        addr = (void *)(memory + aaddr);
        valid |= (uint8_t *)addr >= data->memory && (uint8_t *)addr < data->memory + data->memorySize;
        datai = ARG32;
        res = 0;
        if (valid) {
		  res = (uint16_t)Ptrace(request, pid, addr, datai);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ptrace(%d, %d, 0x%08X, %d): %d", request, pid, aaddr, datai, res);
      }
      break;

    case 321: { // int32_t Mvalidate(pid, start, size, flags)
        valid = 0;
        pid = ARG16;
        astart = ARG32;
        start = (void *)(memory + astart);
        valid |= (uint8_t *)start >= data->memory && (uint8_t *)start < data->memory + data->memorySize;
        size = ARG32;
        aflags = ARG32;
        flags = (int32_t *)(memory + aflags);
        valid |= (uint8_t *)flags >= data->memory && (uint8_t *)flags < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Mvalidate(pid, start, size, flags);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Mvalidate(%d, 0x%08X, %d, 0x%08X): %d", pid, astart, size, aflags, res);
      }
      break;

    case 322: { // int32_t Dxreaddir(ln, dirh, buf, xattr, xr)
        valid = 0;
        ln = ARG16;
        dirh = ARG32;
        abuf = ARG32;
        buf = (uint8_t *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        axattr = ARG32;
        xattr = (XATTR *)(memory + axattr);
        valid |= (uint8_t *)xattr >= data->memory && (uint8_t *)xattr < data->memory + data->memorySize;
        axr = ARG32;
        xr = (int32_t *)(memory + axr);
        valid |= (uint8_t *)xr >= data->memory && (uint8_t *)xr < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Dxreaddir(ln, dirh, (char *)buf, xattr, xr);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dxreaddir(%d, %d, 0x%08X, 0x%08X, 0x%08X): %d", ln, dirh, abuf, axattr, axr, res);
      }
      break;

    case 323: { // int32_t Pseteuid(euid)
        euid = ARG16;
        res = 0;
        res = Pseteuid(euid);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pseteuid(%d): %d", euid, res);
      }
      break;

    case 324: { // int32_t Psetegid(egid)
        egid = ARG16;
        res = 0;
        res = Psetegid(egid);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetegid(%d): %d", egid, res);
      }
      break;

    case 325: { // int16_t Pgetauid(void)
        res = 0;
		res = (uint16_t)Pgetauid();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetauid(): %d", res);
      }
      break;

    case 326: { // int16_t Psetauid(id)
        id = ARG16;
        res = 0;
		res = (uint16_t)Psetauid((int16_t)id);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetauid(%d): %d", id, res);
      }
      break;

    case 327: { // int32_t Pgetgroups(int16_t len, gidset)
        valid = 0;
        len = ARG16;
        agidset = ARG32;
        gidset = (int16_t *)(memory + agidset);
        valid |= (uint8_t *)gidset >= data->memory && (uint8_t *)gidset < data->memory + data->memorySize;
        res = 0;
        if (valid) {
		  res = Pgetgroups((int16_t)len, gidset);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetgroups(%d, 0x%08X): %d", len, agidset, res);
      }
      break;

    case 328: { // int32_t Psetgroups(int16_t len, gidset)
        valid = 0;
        len = ARG16;
        agidset = ARG32;
        gidset = (int16_t *)(memory + agidset);
        valid |= (uint8_t *)gidset >= data->memory && (uint8_t *)gidset < data->memory + data->memorySize;
        res = 0;
        if (valid) {
		  res = Psetgroups((int16_t)len, gidset);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetgroups(%d, 0x%08X): %d", len, agidset, res);
      }
      break;

    case 329: { // int32_t Tsetitimer(which, interval, valuep, ointerval, ovalue)
        valid = 0;
        which = ARG16;
        ainterval = ARG32;
        interval = (int32_t *)(memory + ainterval);
        valid |= (uint8_t *)interval >= data->memory && (uint8_t *)interval < data->memory + data->memorySize;
        avalue = ARG32;
        valuep = (int32_t *)(memory + avalue);
        valid |= (uint8_t *)valuep >= data->memory && (uint8_t *)valuep < data->memory + data->memorySize;
        aointerval = ARG32;
        ointerval = (int32_t *)(memory + aointerval);
        valid |= (uint8_t *)ointerval >= data->memory && (uint8_t *)ointerval < data->memory + data->memorySize;
        aovalue = ARG32;
        ovalue = (int32_t *)(memory + aovalue);
        valid |= (uint8_t *)ovalue >= data->memory && (uint8_t *)ovalue < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Tsetitimer(which, interval, valuep, ointerval, ovalue);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tsetitimer(%d, 0x%08X, 0x%08X, 0x%08X, 0x%08X): %d", which, ainterval, avalue, aointerval, aovalue, res);
      }
      break;

    case 330: { // int32_t Dchroot(path)
        valid = 0;
        apath = ARG32;
        path = (char *)(memory + apath);
        valid |= (uint8_t *)path >= data->memory && (uint8_t *)path < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Dchroot(path);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dchroot(0x%08X): %d", apath, res);
      }
      break;

    case 331: { // int32_t Fstat64(flag, name, stat)
        valid = 0;
        flag = ARG16;
        aname = ARG32;
        name = (char *)(memory + aname);
        valid |= (uint8_t *)name >= data->memory && (uint8_t *)name < data->memory + data->memorySize;
        astat = ARG32;
        stat = (STAT *)(memory + astat);
        valid |= (uint8_t *)stat >= data->memory && (uint8_t *)stat < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Fstat64(flag, name, stat);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fstat64(%d, 0x%08X, 0x%08X): %d", flag, aname, astat, res);
      }
      break;

    case 332: { // int32_t Fseek64(hioffset, lowoffset, handle, seekmode, newpos)
        valid = 0;
        hioffset = ARG32;
        lowoffset = ARG32;
        handle = ARG16;
        seekmode = ARG16;
        anewpos = ARG32;
        newpos = (int64_t *)(memory + anewpos);
        valid |= (uint8_t *)newpos >= data->memory && (uint8_t *)newpos < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Fseek64(hioffset, lowoffset, handle, seekmode, newpos);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fseek64(%d, %d, %d, %d, 0x%08X): %d", hioffset, lowoffset, handle, seekmode, anewpos, res);
      }
      break;

    case 333: { // int32_t Dsetkey(hidev, lowdev, key, cipher)
        valid = 0;
        hidev = ARG32;
        lowdev = ARG32;
        akey = ARG32;
        key = (char *)(memory + akey);
        valid |= (uint8_t *)key >= data->memory && (uint8_t *)key < data->memory + data->memorySize;
        cipher = ARG16;
        res = 0;
        if (valid) {
          res = Dsetkey(hidev, lowdev, key, cipher);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsetkey(%d, %d, 0x%08X, %d): %d", hidev, lowdev, akey, cipher, res);
      }
      break;

    case 334: { // int32_t Psetreuid(ruid, euid)
        ruid = ARG16;
        euid = ARG16;
        res = 0;
        res = Psetreuid(ruid, euid);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetreuid(%d, %d): %d", ruid, euid, res);
      }
      break;

    case 335: { // int32_t Psetregid(rgid, egid)
        rgid = ARG16;
        egid = ARG16;
        res = 0;
        res = Psetregid(rgid, egid);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetregid(%d, %d): %d", rgid, egid, res);
      }
      break;

    case 336: { // void Sync(void)
        Sync();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Sync()");
      }
      break;

    case 337: { // int32_t Shutdown(int32_t mode)
        int32_t mode = ARG32;
        res = 0;
        res = Shutdown(mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Shutdown(%d): %d", mode, res);
      }
      break;

    case 338: { // int32_t Dreadlabel(path, label, int16_t length)
        valid = 0;
        apath = ARG32;
        path = (char *)(memory + apath);
        valid |= (uint8_t *)path >= data->memory && (uint8_t *)path < data->memory + data->memorySize;
        alabel = ARG32;
        label = (char *)(memory + alabel);
        valid |= (uint8_t *)label >= data->memory && (uint8_t *)label < data->memory + data->memorySize;
        length = ARG16;
        res = 0;
        if (valid) {
		  res = Dreadlabel(path, label, (int16_t)length);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dreadlabel(0x%08X, 0x%08X, %d): %d", apath, alabel, length, res);
      }
      break;

    case 339: { // int32_t Dwritelabel(path, label)
        valid = 0;
        apath = ARG32;
        path = (char *)(memory + apath);
        valid |= (uint8_t *)path >= data->memory && (uint8_t *)path < data->memory + data->memorySize;
        alabel = ARG32;
        label = (char *)(memory + alabel);
        valid |= (uint8_t *)label >= data->memory && (uint8_t *)label < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Dwritelabel(path, label);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dwritelabel(0x%08X, 0x%08X): %d", apath, alabel, res);
      }
      break;

    case 340: { // int32_t Ssystem(mode, arg1, arg2)
        mode = ARG16;
        arg1 = ARG32;
        arg2 = ARG32;
        res = 0;
        res = Ssystem((int16_t)mode, arg1, arg2);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ssystem(%d, %d, %d): %d", mode, arg1, arg2, res);
      }
      break;

    case 341: { // int32_t Tgettimeofday(struct timeval *tv, tzp)
        valid = 0;
        atv = ARG32;
        tv = (struct g_timeval *)(memory + atv);
        valid |= (uint8_t *)tv >= data->memory && (uint8_t *)tv < data->memory + data->memorySize;
        atzp = ARG32;
        tzp = (timezone *)(memory + atzp);
        valid |= (uint8_t *)tzp >= data->memory && (uint8_t *)tzp < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Tgettimeofday(tv, tzp);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tgettimeofday(0x%08X, 0x%08X): %d", atv, atzp, res);
      }
      break;

    case 342: { // int32_t Tsettimeofday(struct timeval *tv, tzp)
        valid = 0;
        atv = ARG32;
        tv = (struct g_timeval *)(memory + atv);
        valid |= (uint8_t *)tv >= data->memory && (uint8_t *)tv < data->memory + data->memorySize;
        atzp = ARG32;
        tzp = (timezone *)(memory + atzp);
        valid |= (uint8_t *)tzp >= data->memory && (uint8_t *)tzp < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Tsettimeofday(tv, tzp);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tsettimeofday(0x%08X, 0x%08X): %d", atv, atzp, res);
      }
      break;

    case 343: { // int Tadjtime(struct timeval *delta, struct timeval *olddelta)
        valid = 0;
        adelta = ARG32;
        deltat = (struct g_timeval *)(memory + adelta);
        valid |= (uint8_t *)deltat >= data->memory && (uint8_t *)deltat < data->memory + data->memorySize;
        aolddelta = ARG32;
        olddelta = (struct g_timeval *)(memory + aolddelta);
        valid |= (uint8_t *)olddelta >= data->memory && (uint8_t *)olddelta < data->memory + data->memorySize;
        resint = 0;
        if (valid) {
          resint = Tadjtime(deltat, olddelta);
        }
        m68k_set_reg(M68K_REG_D0, resint);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tadjtime(0x%08X, 0x%08X): %d", adelta, aolddelta, resint);
      }
      break;

    case 344: { // int32_t Pgetpriority(which, who)
        which = ARG16;
        who = ARG16;
        res = 0;
        res = Pgetpriority(which, who);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetpriority(%d, %d): %d", which, who, res);
      }
      break;

    case 345: { // int32_t Psetpriority(which, who, pri)
        which = ARG16;
        who = ARG16;
        pri = ARG16;
        res = 0;
        res = Psetpriority(which, who, pri);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetpriority(%d, %d, %d): %d", which, who, pri, res);
      }
      break;

    case 346: { // int32_t Fpoll(POLLFD *fds, nfds, timeout)
        valid = 0;
        afds = ARG32;
        fdsp = (POLLFD *)(memory + afds);
        valid |= (uint8_t *)fdsp >= data->memory && (uint8_t *)fdsp < data->memory + data->memorySize;
        nfds = ARG32;
        timeout = ARG32;
        res = 0;
        if (valid) {
          res = Fpoll(fdsp, nfds, timeout);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fpoll(0x%08X, %d, %d): %d", afds, nfds, timeout, res);
      }
      break;

    case 347: { // int32_t Fwritev(handle, struct iovec *iov, niov)
        valid = 0;
        handle = ARG16;
        aiov = ARG32;
        iov = (struct g_iovec *)(memory + aiov);
        valid |= (uint8_t *)iov >= data->memory && (uint8_t *)iov < data->memory + data->memorySize;
        niov = ARG32;
        res = 0;
        if (valid) {
          res = Fwritev(handle, iov, niov);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fwritev(%d, 0x%08X, %d): %d", handle, aiov, niov, res);
      }
      break;

    case 348: { // int32_t Freadv(handle, struct iovec *iov, niov)
        valid = 0;
        handle = ARG16;
        aiov = ARG32;
        iov = (struct g_iovec *)(memory + aiov);
        valid |= (uint8_t *)iov >= data->memory && (uint8_t *)iov < data->memory + data->memorySize;
        niov = ARG32;
        res = 0;
        if (valid) {
          res = Freadv(handle, iov, niov);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Freadv(%d, 0x%08X, %d): %d", handle, aiov, niov, res);
      }
      break;

    case 349: { // int32_t Ffstat64(fd, stat)
        valid = 0;
        fd = ARG16;
        astat = ARG32;
        stat = (STAT *)(memory + astat);
        valid |= (uint8_t *)stat >= data->memory && (uint8_t *)stat < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Ffstat64(fd, stat);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ffstat64(%d, 0x%08X): %d", fd, astat, res);
      }
      break;

    case 350: { // int32_t Psysctl(int32_t *name, namelen, old, oldlenp, new, newlen)
        valid = 0;
        aname = ARG32;
        namei = (int32_t *)(memory + aname);
        valid |= (uint8_t *)namei >= data->memory && (uint8_t *)namei < data->memory + data->memorySize;
        namelen = ARG32;
        aold = ARG32;
        old = (void *)(memory + aold);
        valid |= (uint8_t *)old >= data->memory && (uint8_t *)old < data->memory + data->memorySize;
        aoldlenp = ARG32;
        oldlenp = (uint32_t *)(memory + aoldlenp);
        valid |= (uint8_t *)oldlenp >= data->memory && (uint8_t *)oldlenp < data->memory + data->memorySize;
        anew = ARG32;
        new = (void *)(memory + anew);
        valid |= (uint8_t *)new >= data->memory && (uint8_t *)new < data->memory + data->memorySize;
        newlen = ARG32;
        res = 0;
        if (valid) {
          res = Psysctl(namei, namelen, old, oldlenp, new, newlen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psysctl(0x%08X, %d, 0x%08X, 0x%08X, 0x%08X, %d): %d", aname, namelen, aold, aoldlenp, anew, newlen, res);
      }
      break;

    case 352: { // int32_t Fsocket(domain, type, protocol)
        domain = ARG32;
        type = ARG32;
        protocol = ARG32;
        res = 0;
        res = Fsocket(domain, type, protocol);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsocket(%d, %d, %d): %d", domain, type, protocol, res);
      }
      break;

    case 353: { // int32_t Fsocketpair(domain, type, protocol, fds)
        valid = 0;
        domain = ARG32;
        type = ARG32;
        protocol = ARG32;
        afds = ARG32;
        fds = (int16_t *)(memory + afds);
        valid |= (uint8_t *)fds >= data->memory && (uint8_t *)fds < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Fsocketpair(domain, type, protocol, fds);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsocketpair(%d, %d, %d, 0x%08X): %d", domain, type, protocol, afds, res);
      }
      break;

    case 354: { // int32_t Faccept(fd, struct sockaddr *name, anamelen)
        valid = 0;
        fd = ARG16;
        aname = ARG32;
        names = (struct g_sockaddr *)(memory + aname);
        valid |= (uint8_t *)names >= data->memory && (uint8_t *)names < data->memory + data->memorySize;
        aanamelen = ARG32;
        anamelen = (uint32_t *)(memory + aanamelen);
        valid |= (uint8_t *)anamelen >= data->memory && (uint8_t *)anamelen < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Faccept(fd, names, anamelen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Faccept(%d, 0x%08X, 0x%08X): %d", fd, aname, aanamelen, res);
      }
      break;

    case 355: { // int32_t Fconnect(fd, struct sockaddr *name, anamelen)
        valid = 0;
        fd = ARG16;
        aname = ARG32;
        names = (struct g_sockaddr *)(memory + aname);
        valid |= (uint8_t *)names >= data->memory && (uint8_t *)names < data->memory + data->memorySize;
        aanamelen = ARG32;
        res = 0;
        if (valid) {
          res = Fconnect(fd, names, aanamelen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fconnect(%d, 0x%08X, %d): %d", fd, aname, aanamelen, res);
      }
      break;

    case 356: { // int32_t Fbind(fd, struct sockaddr *name, anamelen)
        valid = 0;
        fd = ARG16;
        aname = ARG32;
        names = (struct g_sockaddr *)(memory + aname);
        valid |= (uint8_t *)names >= data->memory && (uint8_t *)names < data->memory + data->memorySize;
        aanamelen = ARG32;
        res = 0;
        if (valid) {
          res = Fbind(fd, names, aanamelen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fbind(%d, 0x%08X, %d): %d", fd, aname, aanamelen, res);
      }
      break;

    case 357: { // int32_t Flisten(fd, backlog)
        fd = ARG16;
        backlog = ARG32;
        res = 0;
        res = Flisten(fd, backlog);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Flisten(%d, %d): %d", fd, backlog, res);
      }
      break;

    case 358: { // int32_t Frecvmsg(fd, struct msghdr *msg, int32_t flags)
        valid = 0;
        fd = ARG16;
        amsg = ARG32;
        msghdr = (struct g_msghdr *)(memory + amsg);
        valid |= (uint8_t *)msghdr >= data->memory && (uint8_t *)msghdr < data->memory + data->memorySize;
        aflags = ARG32;
        res = 0;
        if (valid) {
          res = Frecvmsg(fd, msghdr, aflags);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Frecvmsg(%d, 0x%08X, %d): %d", fd, amsg, aflags, res);
      }
      break;

    case 359: { // int32_t Fsendmsg(fd, struct msghdr *msg, int32_t flags)
        valid = 0;
        fd = ARG16;
        amsg = ARG32;
        msghdr = (struct g_msghdr *)(memory + amsg);
        valid |= (uint8_t *)msghdr >= data->memory && (uint8_t *)msghdr < data->memory + data->memorySize;
        aflags = ARG32;
        res = 0;
        if (valid) {
          res = Fsendmsg(fd, msghdr, (int32_t)aflags);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsendmsg(%d, 0x%08X, %d): %d", fd, amsg, aflags, res);
      }
      break;

    case 360: { // int32_t Frecvfrom(fd, buf, buflen, int32_t flags, struct sockaddr *to, addrlen)
        valid = 0;
        fd = ARG16;
        abuf = ARG32;
        buf = (void *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        buflen = ARG32;
        aflags = ARG32;
        ato = ARG32;
        to = (struct g_sockaddr *)(memory + ato);
        valid |= (uint8_t *)to >= data->memory && (uint8_t *)to < data->memory + data->memorySize;
        aaddrlen = ARG32;
        addrlen = (uint32_t *)(memory + aaddrlen);
        valid |= (uint8_t *)addrlen >= data->memory && (uint8_t *)addrlen < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Frecvfrom(fd, buf, buflen, aflags, to, addrlen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Frecvfrom(%d, 0x%08X, %d, %d, 0x%08X, 0x%08X): %d", fd, abuf, buflen, aflags, ato, aaddrlen, res);
      }
      break;

    case 361: { // int32_t Fsendto(fd, buf, buflen, int32_t flags, struct sockaddr *to, addrlen)
        valid = 0;
        fd = ARG16;
        abuf = ARG32;
        buf = (void *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        buflen = ARG32;
        aflags = ARG32;
        ato = ARG32;
        to = (struct g_sockaddr *)(memory + ato);
        valid |= (uint8_t *)to >= data->memory && (uint8_t *)to < data->memory + data->memorySize;
        aaddrlen = ARG32;
        res = 0;
        if (valid) {
          res = Fsendto(fd, buf, buflen, aflags, to, aaddrlen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsendto(%d, 0x%08X, %d, %d, 0x%08X, %d): %d", fd, abuf, buflen, aflags, ato, aaddrlen, res);
      }
      break;

    case 362: { // int32_t Fsetsockopt(fd, level, int32_t name, void *val, valsize)
        valid = 0;
        fd = ARG16;
        level = ARG32;
        aname = ARG32;
        aval = ARG32;
        valv = (void *)(memory + aval);
        valid |= (uint8_t *)valv >= data->memory && (uint8_t *)valv < data->memory + data->memorySize;
        valsize = ARG32;
        res = 0;
        if (valid) {
          res = Fsetsockopt(fd, level, aname, valv, valsize);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsetsockopt(%d, %d, %d, 0x%08X, %d): %d", fd, level, aname, aval, valsize, res);
      }
      break;

    case 363: { // int32_t Fgetsockopt(fd, level, int32_t name, void *val, uint32_t *valsize)
        valid = 0;
        fd = ARG16;
        level = ARG32;
        aname = ARG32;
        aval = ARG32;
        valv = (void *)(memory + aval);
        valid |= (uint8_t *)valv >= data->memory && (uint8_t *)valv < data->memory + data->memorySize;
        avalsize = ARG32;
        valsizep = (uint32_t *)(memory + avalsize);
        valid |= (uint8_t *)valsizep >= data->memory && (uint8_t *)valsizep < data->memory + data->memorySize;
        res = 0;
        if (valid) {
		  res = Fgetsockopt(fd, level, (int32_t)aname, valv, valsizep);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fgetsockopt(%d, %d, %d, 0x%08X, 0x%08X): %d", fd, level, aname, aval, avalsize, res);
      }
      break;

    case 364: { // int32_t Fgetpeername(fd, struct sockaddr *asa, alen)
        valid = 0;
        fd = ARG16;
        aasa = ARG32;
        asa = (struct g_sockaddr *)(memory + aasa);
        valid |= (uint8_t *)asa >= data->memory && (uint8_t *)asa < data->memory + data->memorySize;
        aalen = ARG32;
        alen = (uint32_t *)(memory + aalen);
        valid |= (uint8_t *)alen >= data->memory && (uint8_t *)alen < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Fgetpeername(fd, asa, alen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fgetpeername(%d, 0x%08X, 0x%08X): %d", fd, aasa, aalen, res);
      }
      break;

    case 365: { // int32_t Fgetsockname(fd, struct sockaddr *asa, alen)
        valid = 0;
        fd = ARG16;
        aasa = ARG32;
        asa = (struct g_sockaddr *)(memory + aasa);
        valid |= (uint8_t *)asa >= data->memory && (uint8_t *)asa < data->memory + data->memorySize;
        aalen = ARG32;
        alen = (uint32_t *)(memory + aalen);
        valid |= (uint8_t *)alen >= data->memory && (uint8_t *)alen < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Fgetsockname(fd, asa, alen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fgetsockname(%d, 0x%08X, 0x%08X): %d", fd, aasa, aalen, res);
      }
      break;

    case 366: { // int32_t Fshutdown(fd, how)
        fd = ARG16;
        how = ARG32;
        res = 0;
        res = Fshutdown(fd, how);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fshutdown(%d, %d): %d", fd, how, res);
      }
      break;

    case 381: { // int32_t Maccess(start, size, mode)
        valid = 0;
        astart = ARG32;
        start = (void *)(memory + astart);
        valid |= (uint8_t *)start >= data->memory && (uint8_t *)start < data->memory + data->memorySize;
        size = ARG32;
        mode = ARG16;
        res = 0;
        if (valid) {
          res = Maccess(start, size, (int16_t)mode);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Maccess(0x%08X, %d, %d): %d", astart, size, mode, res);
      }
      break;

    case 384: { // int32_t Fchown16(name, uid, gid, flag)
        valid = 0;
        aname = ARG32;
        name = (char *)(memory + aname);
        valid |= (uint8_t *)name >= data->memory && (uint8_t *)name < data->memory + data->memorySize;
        uid = ARG16;
        gid = ARG16;
        flag = ARG16;
        res = 0;
        if (valid) {
          res = Fchown16(name, uid, gid, flag);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fchown16(0x%08X, %d, %d, %d): %d", aname, uid, gid, flag, res);
      }
      break;

    case 385: { // int32_t Fchdir(handle)
        handle = ARG16;
        res = 0;
        res = Fchdir(handle);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fchdir(%d): %d", handle, res);
      }
      break;

    case 386: { // int32_t Ffdopendir(fd)
        fd = ARG16;
        res = 0;
        res = Ffdopendir(fd);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ffdopendir(%d): %d", fd, res);
      }
      break;

    case 387: { // fdirfd(handlel)
        handlel = ARG32;
        res = 0;
		res = (uint16_t)Fdirfd(handlel);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fdirfd(%d): %d", handlel, res);
      }
      break;

    case 1296: { // int32_t Dxopendir(name, flag)
        valid = 0;
        aname = ARG32;
        name = (char *)(memory + aname);
        valid |= (uint8_t *)name >= data->memory && (uint8_t *)name < data->memory + data->memorySize;
        flag = ARG16;
        res = 0;
        if (valid) {
          res = Dxopendir(name, flag);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dxopendir(0x%08X, %d): %d", aname, flag, res);
      }
      break;


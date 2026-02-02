    case 0: { // void Initmouse(type, par, mousevec)
        valid = 0;
        type = ARG16;
        apar = ARG32;
        par = (MOUSE *)(memory + apar);
        valid |= (uint8_t *)par >= data->memory && (uint8_t *)par < data->memory + data->memorySize;
        amousevec = ARG32;
        mousevec = (void *)(memory + amousevec);
        valid |= (uint8_t *)mousevec >= data->memory && (uint8_t *)mousevec < data->memory + data->memorySize;
        if (valid) {
          Initmouse(type, par, mousevec);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Initmouse(%d, 0x%08X, 0x%08X)", type, apar, amousevec);
      }
      break;

    case 1: { // void *Ssbrk(count)
        count = ARG16;
        resv = NULL;
        resv = Ssbrk(count);
		ares = resv ? (uint32_t)((uint8_t *)resv - memory) : 0;
        m68k_set_reg(M68K_REG_D0, ares);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ssbrk(%d): 0x%08X", count, ares);
      }
      break;

    case 2: { // void *Physbase(void)
        res = data->physbase;
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Physbase(): 0x%08X", res);
      }
      break;

    case 3: { // void *Logbase(void)
        res = data->logbase;
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Logbase(): 0x%08X", res);
      }
      break;

    case 4: { // int16_t Getrez(void)
        // returns the current resolution of the screen:
        // 0  320x200 (4 planes)
        // 1  640x200 (2 planes)
        // 2  640x400 (one plane)

        res = data->screen_res;
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Getrez(): %d", res);
      }
      break;

    case 5: { // void Setscreen(void *laddr, void *paddr, rezz)
        // alter the screen resolution and screen memory addresses
        // laddr: address of the logical screen memory
        // paddr: address of the physical screen memory
        // rezz: same as in Getrez
        // a value of -1 means that the corresponding address or resolution will not be altered

        laddr = ARG32;
        paddr = ARG32;
        rezz = ARG16;

        if (laddr != 0xFFFFFFFF) {
          data->logbase = laddr;
        }

        if (paddr != 0xFFFFFFFF) {
          data->physbase = paddr;
        }

        if (rezz != -1) {
          switch (rezz) {
            case 0:
            case 1:
              data->screen_res = rezz;
              data->font = tos8x8Font;
              FntSetFont(data->font);
              break;
            case 2:
              data->screen_res = rezz;
              data->font = tos8x16Font;
              FntSetFont(data->font);
              break;
            default:
              debug(DEBUG_ERROR, "TOS", "GEMDOS Setscreen invalid resolution %d", rezz);
              break;
          }
        }

        debug(DEBUG_TRACE, "TOS", "GEMDOS Setscreen(0x%08X, 0x%08X, %d)", laddr, paddr, rezz);
      }
      break;

    case 6: { // void Setpalette(void *pallptr)
        // loads the ST color lookup table with a new palette
        pallptr = ARG32;
        if (pallptr) {
          for (colornum = 0; colornum < 16; colornum++) {
            color = m68k_read_memory_16(pallptr);
            rgb = tos_convert_color(color);
            data->tos_color[colornum] = color;
            data->pumpkin_color[colornum] = rgb;
            debug(DEBUG_TRACE, "TOS", "GEMDOS Setpalette %u color 0x%04X rgb565 0x%04X", colornum, color, rgb);
            pallptr += 2;
          }
          data->screen_updated = 1;
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Setpalette(0x%08X)", pallptr);
      }
      break;

    case 7: { // int16_t Setcolor(colorNum, color)
        // returns the value of a colour register or sets this to a new value
        // colornum: number of the colour register (0..15)
        // color: new colour value (-1 = don't alter)
        // returns the old value of the colour register
        // each of the sixteen color registers is bitmapped into a WORD as follows:
        // xxxx xRRR xGGG xBBB
        // xxxx x321 x321 x321

        colornum = ARG16;
        color = ARG16;

        if (colornum < 16) {
          old = data->tos_color[colornum];
          if (color != 0xffff) {
            rgb = tos_convert_color(color);
            data->tos_color[colornum] = color;
            data->pumpkin_color[colornum] = rgb;
            data->screen_updated = 1;
            debug(DEBUG_TRACE, "TOS", "GEMDOS Setcolor %u color 0x%04X rgb565 0x%04X", colornum, color, rgb);
          }
        }
        m68k_set_reg(M68K_REG_D0, old);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Setcolor(%u, %u): %u", colornum, color, old);
      }
      break;

    case 8: { // int16_t Floprd(buf, filler, devno, sectno, trackno, sideno, count)
        valid = 0;
        abuf = ARG32;
        buf = (void *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        filler = ARG32;
        devno = ARG16;
        sectno = ARG16;
        trackno = ARG16;
        sideno = ARG16;
        count = ARG16;
        if (valid) {
          res = Floprd(buf, filler, devno, sectno, trackno, sideno, count);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Floprd(0x%08X, %d, %d, %d, %d, %d, %d): %d", abuf, filler, devno, sectno, trackno, sideno, count, res);
      }
      break;

    case 9: { // int16_t Flopwr(buf, filler, devno, sectno, trackno, sideno, count)
        valid = 0;
        abuf = ARG32;
        buf = (void *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        filler = ARG32;
        devno = ARG16;
        sectno = ARG16;
        trackno = ARG16;
        sideno = ARG16;
        count = ARG16;
        res = 0;
        if (valid) {
          res = Flopwr(buf, filler, devno, sectno, trackno, sideno, count);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Flopwr(0x%08X, %d, %d, %d, %d, %d, %d): %d", abuf, filler, devno, sectno, trackno, sideno, count, res);
      }
      break;

    case 10: { // int16_t Flopfmt(void)
        res = 0;
        res = Flopfmt();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Flopfmt(): %d", res);
      }
      break;

    case 11: { // void Dbmsg(rsrvd, msg_num, msg_arg)
        rsrvd = ARG16;
        msg_num = ARG16;
        msg_arg = ARG32;
        Dbmsg(rsrvd, msg_num, msg_arg);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dbmsg(%d, %d, %d)", rsrvd, msg_num, msg_arg);
      }
      break;

    case 12: { // void Midiws(cnt, ptr)
        valid = 0;
        cnt = ARG16;
        aptr = ARG32;
        ptr = (void *)(memory + aptr);
        valid |= (uint8_t *)ptr >= data->memory && (uint8_t *)ptr < data->memory + data->memorySize;
        if (valid) {
          Midiws(cnt, ptr);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Midiws(%d, 0x%08X)", cnt, aptr);
      }
      break;

    case 13: { // void Mfpint(number, vector)
        valid = 0;
        number = ARG16;
        avector = ARG32;
        vector = (void *)(memory + avector);
        valid |= (uint8_t *)vector >= data->memory && (uint8_t *)vector < data->memory + data->memorySize;
        if (valid) {
          Mfpint(number, vector);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Mfpint(%d, 0x%08X)", number, avector);
      }
      break;

    case 14: { // IOREC *Iorec(dev)
        dev = ARG16;
        resi = NULL;
        resi = Iorec(dev);
		ares = resi ? (uint32_t)((uint8_t *)resi - memory) : 0;
        m68k_set_reg(M68K_REG_D0, ares);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Iorec(%d): 0x%08X", dev, ares);
      }
      break;

    case 15: { // int32_t Rsconf(baud, ctr, ucr, rsr, tsr, scr)
        baud = ARG16;
        ctr = ARG16;
        ucr = ARG16;
        rsr = ARG16;
        tsr = ARG16;
        scr = ARG16;
        res = 0;
		res = (uint32_t)Rsconf(baud, ctr, ucr, rsr, tsr, scr);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
		debug(DEBUG_TRACE, "TOS", "GEMDOS Rsconf(%d, %d, %d, %d, %d, %d): %d", baud, ctr, ucr, rsr, tsr, scr, (int32_t)res);
      }
      break;

    case 16: { // KEYTAB *Keytbl(void *unshift, void *shift, void *capslock)
        valid = 0;
        aunshift = ARG32;
        unshift = (void *)(memory + aunshift);
        valid |= (uint8_t *)unshift >= data->memory && (uint8_t *)unshift < data->memory + data->memorySize;
        ashift = ARG32;
        shift = (void *)(memory + ashift);
        valid |= (uint8_t *)shift >= data->memory && (uint8_t *)shift < data->memory + data->memorySize;
        acapslock = ARG32;
        capslock = (void *)(memory + acapslock);
        valid |= (uint8_t *)capslock >= data->memory && (uint8_t *)capslock < data->memory + data->memorySize;
        resk = NULL;
        if (valid) {
          resk = Keytbl(unshift, shift, capslock);
        }
		ares = resk ? (uint32_t)((uint8_t *)resk - memory) : 0;
        m68k_set_reg(M68K_REG_D0, ares);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Keytbl(0x%08X, 0x%08X, 0x%08X): 0x%08X", aunshift, ashift, acapslock, ares);
      }
      break;

    case 17: { // int32_t Random(void)
        res = 0;
		res = (uint32_t)Random();
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
		debug(DEBUG_TRACE, "TOS", "GEMDOS Random(): %d", (int32_t)res);
      }
      break;

    case 18: { // void Protobt(buf, serialno, disktype, execflag)
        valid = 0;
        abuf = ARG32;
        buf = (void *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        serialno = ARG32;
        disktype = ARG16;
        execflag = ARG16;
        if (valid) {
          Protobt(buf, serialno, disktype, execflag);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Protobt(0x%08X, %d, %d, %d)", abuf, serialno, disktype, execflag);
      }
      break;

    case 19: { // int16_t Flopver(buf, filler, devno, sectno, trackno, sideno, count)
        valid = 0;
        abuf = ARG32;
        buf = (void *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        filler = ARG32;
        devno = ARG16;
        sectno = ARG16;
        trackno = ARG16;
        sideno = ARG16;
        count = ARG16;
        res = 0;
        if (valid) {
          res = Flopver(buf, filler, devno, sectno, trackno, sideno, count);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Flopver(0x%08X, %d, %d, %d, %d, %d, %d): %d", abuf, filler, devno, sectno, trackno, sideno, count, res);
      }
      break;

    case 20: { // void Scrdmp(void)
        Scrdmp();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Scrdmp()");
      }
      break;

    case 21: { // int16_t Cursconf(int16_t func, int16_t rate)
        func = ARG16;
        rate = ARG16;
        res = 0;
        res = Cursconf(func, (int16_t)rate);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cursconf(%d, %d): %d", func, rate, res);
      }
      break;

    case 22: { // void Settime(uint32_t time)
        time = ARG32;
        Settime(time);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Settime(%d)", time);
      }
      break;

    case 23: { // uint32_t Gettime(void)
        res = 0;
        res = Gettime();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Gettime(): %d", res);
      }
      break;

    case 24: { // void Bioskeys(void)
        Bioskeys();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Bioskeys()");
      }
      break;

    case 25: { // void Ikbdws(count, int8_t *ptr)
        valid = 0;
        count = ARG16;
        aptr = ARG32;
        ptr8 = (int8_t *)(memory + aptr);
        valid |= (uint8_t *)ptr8 >= data->memory && (uint8_t *)ptr8 < data->memory + data->memorySize;
        if (valid) {
          Ikbdws(count, ptr8);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ikbdws(%d, 0x%08X)", count, aptr);
      }
      break;

    case 26: { // void Jdisint(number)
        number = ARG16;
        Jdisint(number);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Jdisint(%d)", number);
      }
      break;

    case 27: { // void Jenabint(number)
        number = ARG16;
        Jenabint(number);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Jenabint(%d)", number);
      }
      break;

    case 28: { // int8_t Giaccess(dat, regno)
        dat = ARG16;
        regno = ARG16;
        res = 0;
        res = Giaccess(dat, regno);
		m68k_set_reg(M68K_REG_D0, (int8_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Giaccess(%d, %d): %d", dat, regno, res);
      }
      break;

    case 29: { // void Offgibit(bitno)
        bitno = ARG16;
        Offgibit(bitno);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Offgibit(%d)", bitno);
      }
      break;

    case 30: { // void Ongibit(void)
        Ongibit();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ongibit()");
      }
      break;

    case 31: { // void Xbtimer(timer, control, dat, vector)
        valid = 0;
        timer = ARG16;
        control = ARG16;
        dat = ARG16;
        avector = ARG32;
        vector = (void *)(memory + avector);
        valid |= (uint8_t *)vector >= data->memory && (uint8_t *)vector < data->memory + data->memorySize;
        if (valid) {
          Xbtimer(timer, control, dat, vector);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Xbtimer(%d, %d, %d, 0x%08X)", timer, control, dat, avector);
      }
      break;

    case 32: { // void *Dosound(int8_t *buf)
        valid = 0;
        abuf = ARG32;
        buf8 = (int8_t *)(memory + abuf);
        valid |= (uint8_t *)buf8 >= data->memory && (uint8_t *)buf8 < data->memory + data->memorySize;
        resv = NULL;
        if (valid) {
          resv = Dosound(buf8);
        }
		ares = resv ? (uint32_t)((uint8_t *)resv - memory) : 0;
        m68k_set_reg(M68K_REG_D0, ares);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dosound(0x%08X): 0x%08X", abuf, ares);
      }
      break;

    case 33: { // int16_t Setprt(config)
        config = ARG16;
        res = 0;
        res = Setprt(config);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Setprt(%d): %d", config, res);
      }
      break;

    case 34: { // KBDVBASE *Kbdvbase(void)
        // returns a pointer to a KBDVBASE structure contaning a jump table to system vector handlers

        // typedef struct {
        //    void   (*kb_midivec)();   /* MIDI interrupt vector    */
        //    void   (*kb_vkbderr)();   /* Keyboard error vector    */
        //    void   (*kb_vmiderr)();   /* MIDI error vector        */
        //    void   (*kb_statvec)();   /* Keyboard status          */
        //    void   (*kb_mousevec)();  /* Keyboard mouse status    */
        //    void   (*kb_clockvec)();  /* Keyboard clock           */
        //    void   (*kb_joyvec)();    /* Keyboard joystick status */
        //    void   (*kb_midisys)();   /* System Midi vector       */
        //    void   (*kb_kbdsys)();    /* Keyboard vector          */
        //    int8_t drvstat;           /* Keyboard driver status   */
        // } KBDVBASE;
        // The element drvstat contains a non-zero value when the IKBD is in the process of sending a packet

        res = data->kbdvbase;
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Kbdvbase(): 0x%08X", res);
      }
      break;

    case 35: { // int16_t Kbrate(initial, repeat)
        initial = ARG16;
        repeat = ARG16;
        res = 0;
        res = Kbrate(initial, repeat);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Kbrate(%d, %d): %d", initial, repeat, res);
      }
      break;

    case 36: { // int16_t Prtblk(par)
        valid = 0;
        apar = ARG32;
        par2 = (PBDEF *)(memory + apar);
        valid |= (uint8_t *)par2 >= data->memory && (uint8_t *)par2 < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Prtblk(par2);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Prtblk(0x%08X): %d", apar, res);
      }
      break;

    case 37: { // void Vsync(void)
        Vsync();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Vsync()");
      }
      break;

    case 38: { // int32_t Supexec(void *func)
        func32 = ARG32;
        debug(DEBUG_TRACE, "TOS", "GEMDOS Supexec(0x%08X)", func32);
        r = func;
      }
      break;

    case 39: { // void Puntaes(void)
        Puntaes();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Puntaes()");
      }
      break;

    case 41: { // int16_t Floprate(devno, newrate)
        devno = ARG16;
        newrate = ARG16;
        res = 0;
        res = Floprate(devno, newrate);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Floprate(%d, %d): %d", devno, newrate, res);
      }
      break;

    case 42: { // int16_t DMAread(sector, count, buffer, devno)
        valid = 0;
        sector = ARG32;
        count = ARG16;
        abuffer = ARG32;
        buffer = (void *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        devno = ARG16;
        res = 0;
        if (valid) {
          res = DMAread(sector, count, buffer, devno);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS DMAread(%d, %d, 0x%08X, %d): %d", sector, count, abuffer, devno, res);
      }
      break;

    case 43: { // int16_t DMAwrite(sector, count, buffer, devno)
        valid = 0;
        sector = ARG32;
        count = ARG16;
        abuffer = ARG32;
        buffer = (void *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        devno = ARG16;
        res = 0;
        if (valid) {
          res = DMAwrite(sector, count, buffer, devno);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS DMAwrite(%d, %d, 0x%08X, %d): %d", sector, count, abuffer, devno, res);
      }
      break;

    case 44: { // int32_t Bconmap(devno)
        devno = ARG16;
        res = 0;
        res = Bconmap(devno);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Bconmap(%d): %d", devno, res);
      }
      break;

    case 46: { // nVMaccess(op, start, count, buffer)
        valid = 0;
        op = ARG16;
        start = ARG16;
        count = ARG16;
        abuffer = ARG32;
        buffer = (int8_t *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = NVMaccess(op, start, count, buffer);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS NVMaccess(%d, %d, %d, 0x%08X): %d", op, start, count, abuffer, res);
      }
      break;

    case 48: { // void Metainit(META_INFO_1 *buffer)
        valid = 0;
        abuffer = ARG32;
        bufferm = (META_INFO_1 *)(memory + abuffer);
        valid |= (uint8_t *)bufferm >= data->memory && (uint8_t *)bufferm < data->memory + data->memorySize;
        if (valid) {
          Metainit(bufferm);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metainit(0x%08X)", abuffer);
      }
      break;

    case 49: { // int32_t Metaopen(drive, META_DRVINFO *buffer)
        valid = 0;
        drive = ARG16;
        abuffer = ARG32;
        bufferd = (META_DRVINFO *)(memory + abuffer);
        valid |= (uint8_t *)bufferd >= data->memory && (uint8_t *)bufferd < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Metaopen(drive, bufferd);
        }
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metaopen(%d, 0x%08X): %d", drive, abuffer, res);
      }
      break;

    case 50: { // int32_t Metaclose(drive)
        drive = ARG16;
        res = 0;
        res = Metaclose(drive);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metaclose(%d): %d", drive, res);
      }
      break;

    case 51: { // int32_t Metaread(drive, buffer, blockno, count)
        valid = 0;
        drive = ARG16;
        abuffer = ARG32;
        buffer = (void *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        blockno = ARG32;
        count = ARG16;
        res = 0;
        if (valid) {
          res = Metaread(drive, buffer, blockno, count);
        }
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metaread(%d, 0x%08X, %d, %d): %d", drive, abuffer, blockno, count, res);
      }
      break;

    case 52: { // int32_t Metawrite(drive, buffer, blockno, count)
        valid = 0;
        drive = ARG16;
        abuffer = ARG32;
        buffer = (void *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        blockno = ARG32;
        count = ARG16;
        res = 0;
        if (valid) {
          res = Metawrite(drive, buffer, blockno, count);
        }
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metawrite(%d, 0x%08X, %d, %d): %d", drive, abuffer, blockno, count, res);
      }
      break;

    case 53: { // int32_t Metaseek(drive, blockno)
        drive = ARG16;
        blockno = ARG32;
        res = 0;
        res = Metaseek(drive, blockno);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metaseek(%d, %d): %d", drive, blockno, res);
      }
      break;

    case 54: { // int32_t Metastatus(drive, buffer)
        valid = 0;
        drive = ARG16;
        abuffer = ARG32;
        buffer = (void *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Metastatus(drive, buffer);
        }
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metastatus(%d, 0x%08X): %d", drive, abuffer, res);
      }
      break;

    case 55: { // int32_t Metaioctl(drive, magic, opcode, buffer)
        valid = 0;
        drive = ARG16;
        magic = ARG32;
        opcode = ARG16;
        abuffer = ARG32;
        buffer = (void *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Metaioctl(drive, magic, opcode, buffer);
        }
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metaioctl(%d, %d, %d, 0x%08X): %d", drive, magic, opcode, abuffer, res);
      }
      break;

    case 59: { // int32_t Metastartaudio(drive, flag, bytearray)
        valid = 0;
        drive = ARG16;
        flag = ARG16;
        abytearray = ARG32;
        bytearray = (uint8_t *)(memory + abytearray);
        valid |= (uint8_t *)bytearray >= data->memory && (uint8_t *)bytearray < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Metastartaudio(drive, flag, bytearray);
        }
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metastartaudio(%d, %d, 0x%08X): %d", drive, flag, abytearray, res);
      }
      break;

    case 60: { // int32_t Metastopaudio(drive)
        drive = ARG16;
        res = 0;
        res = Metastopaudio(drive);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metastopaudio(%d): %d", drive, res);
      }
      break;

    case 61: { // int32_t Metasetsongtime(drive, repeat, starttime, endtime)
        drive = ARG16;
        repeat = ARG16;
        starttime = ARG32;
        endtime = ARG32;
        res = 0;
        res = Metasetsongtime(drive, repeat, starttime, endtime);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metasetsongtime(%d, %d, %d, %d): %d", drive, repeat, starttime, endtime, res);
      }
      break;

    case 62: { // int32_t Metagettoc(drive, flag, CD_TOC_ENTRY *buffer)
        valid = 0;
        drive = ARG16;
        flag = ARG16;
        abuffer = ARG32;
        bufferc = (CD_TOC_ENTRY *)(memory + abuffer);
        valid |= (uint8_t *)bufferc >= data->memory && (uint8_t *)bufferc < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Metagettoc(drive, flag, bufferc);
        }
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metagettoc(%d, %d, 0x%08X): %d", drive, flag, abuffer, res);
      }
      break;

    case 63: { // int32_t Metadiscinfo(drive, p)
        valid = 0;
        drive = ARG16;
        ap = ARG32;
        p = (CD_DISC_INFO *)(memory + ap);
        valid |= (uint8_t *)p >= data->memory && (uint8_t *)p < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Metadiscinfo(drive, p);
        }
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metadiscinfo(%d, 0x%08X): %d", drive, ap, res);
      }
      break;

    case 64: { // int16_t Blitmode(mode)
        mode = ARG16;
        res = 0;
        res = Blitmode(mode);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Blitmode(%d): %d", mode, res);
      }
      break;

    case 80: { // int16_t EsetShift(shftMode)
        shftMode = ARG16;
        res = 0;
        res = EsetShift(shftMode);
        m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS EsetShift(%d): %d", shftMode, res);
      }
      break;

    case 81: { // int16_t EgetShift(void)
        res = 0;
        res = EgetShift();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS EgetShift(): %d", res);
      }
      break;

    case 82: { // int16_t EsetBank(bankNum)
        bankNum = ARG16;
        res = 0;
        res = EsetBank(bankNum);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS EsetBank(%d): %d", bankNum, res);
      }
      break;

    case 83: { // int16_t EsetColor(colorNum, color)
        colorNum = ARG16;
        color = ARG16;
        res = 0;
        res = EsetColor(colorNum, color);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS EsetColor(%d, %d): %d", colorNum, color, res);
      }
      break;

    case 84: { // void EsetPalette(colorNum, count, palettePtr)
        valid = 0;
        colorNum = ARG16;
        count = ARG16;
        apalettePtr = ARG32;
        palettePtr = (int16_t *)(memory + apalettePtr);
        valid |= (uint8_t *)palettePtr >= data->memory && (uint8_t *)palettePtr < data->memory + data->memorySize;
        if (valid) {
          EsetPalette(colorNum, count, palettePtr);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS EsetPalette(%d, %d, 0x%08X)", colorNum, count, apalettePtr);
      }
      break;

    case 85: { // void EgetPalette(colorNum, count, palettePtr)
        valid = 0;
        colorNum = ARG16;
        count = ARG16;
        apalettePtr = ARG32;
        palettePtr = (int16_t *)(memory + apalettePtr);
        valid |= (uint8_t *)palettePtr >= data->memory && (uint8_t *)palettePtr < data->memory + data->memorySize;
        if (valid) {
          EgetPalette(colorNum, count, palettePtr);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS EgetPalette(%d, %d, 0x%08X)", colorNum, count, apalettePtr);
      }
      break;

    case 86: { // int16_t EsetGray(sw)
        sw = ARG16;
        res = 0;
        res = EsetGray(sw);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS EsetGray(%d): %d", sw, res);
      }
      break;

    case 87: { // int16_t EsetSmear(sw)
        sw = ARG16;
        res = 0;
        res = EsetSmear(sw);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS EsetSmear(%d): %d", sw, res);
      }
      break;

    case 88: { // int16_t VsetMode(mode)
        mode = ARG16;
        res = 0;
        res = VsetMode(mode);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS VsetMode(%d): %d", mode, res);
      }
      break;

    case 89: { // int16_t mon_type(void)
        res = 0;
        res = mon_type();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS mon_type(): %d", res);
      }
      break;

    case 90: { // void VsetSync(flag)
        flag = ARG16;
        VsetSync(flag);
        debug(DEBUG_TRACE, "TOS", "GEMDOS VsetSync(%d)", flag);
      }
      break;

    case 91: { // int32_t VgetSize(mode)
        mode = ARG16;
        res = 0;
        res = VgetSize(mode);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS VgetSize(%d): %d", mode, res);
      }
      break;

    case 93: { // void VsetRGB(index, count, array)
        valid = 0;
        index = ARG16;
        count = ARG16;
        aarray = ARG32;
        array = (int32_t *)(memory + aarray);
        valid |= (uint8_t *)array >= data->memory && (uint8_t *)array < data->memory + data->memorySize;
        if (valid) {
          VsetRGB(index, count, array);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS VsetRGB(%d, %d, 0x%08X)", index, count, aarray);
      }
      break;

    case 94: { // void VgetRGB(index, count, array)
        valid = 0;
        index = ARG16;
        count = ARG16;
        aarray = ARG32;
        array = (int32_t *)(memory + aarray);
        valid |= (uint8_t *)array >= data->memory && (uint8_t *)array < data->memory + data->memorySize;
        if (valid) {
          VgetRGB(index, count, array);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS VgetRGB(%d, %d, 0x%08X)", index, count, aarray);
      }
      break;

    case 95: { // int16_t ValidMode(mode)
        mode = ARG16;
        res = 0;
        res = ValidMode(mode);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS ValidMode(%d): %d", mode, res);
      }
      break;

    case 96: { // void Dsp_DoBlock(data_in, size_in, data_out, size_out)
        valid = 0;
        adata_in = ARG32;
        data_in = (int8_t *)(memory + adata_in);
        valid |= (uint8_t *)data_in >= data->memory && (uint8_t *)data_in < data->memory + data->memorySize;
        size_in = ARG32;
        adata_out = ARG32;
        data_out = (int8_t *)(memory + adata_out);
        valid |= (uint8_t *)data_out >= data->memory && (uint8_t *)data_out < data->memory + data->memorySize;
        size_out = ARG32;
        if (valid) {
          Dsp_DoBlock(data_in, size_in, data_out, size_out);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_DoBlock(0x%08X, %d, 0x%08X, %d)", adata_in, size_in, adata_out, size_out);
      }
      break;

    case 97: { // void Dsp_BlkHandShake(data_in, size_in, data_out, size_out)
        valid = 0;
        adata_in = ARG32;
        data_in = (int8_t *)(memory + adata_in);
        valid |= (uint8_t *)data_in >= data->memory && (uint8_t *)data_in < data->memory + data->memorySize;
        size_in = ARG32;
        adata_out = ARG32;
        data_out = (int8_t *)(memory + adata_out);
        valid |= (uint8_t *)data_out >= data->memory && (uint8_t *)data_out < data->memory + data->memorySize;
        size_out = ARG32;
        if (valid) {
          Dsp_BlkHandShake(data_in, size_in, data_out, size_out);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_BlkHandShake(0x%08X, %d, 0x%08X, %d)", adata_in, size_in, adata_out, size_out);
      }
      break;

    case 98: { // void Dsp_BlkUnpacked(data_in, size_in, int32_t *data_out, size_out)
        valid = 0;
        adata_in = ARG32;
        data_in32 = (int32_t *)(memory + adata_in);
        valid |= (uint8_t *)data_in32 >= data->memory && (uint8_t *)data_in32 < data->memory + data->memorySize;
        size_in = ARG32;
        adata_out = ARG32;
        data_out32 = (int32_t *)(memory + adata_out);
        valid |= (uint8_t *)data_out32 >= data->memory && (uint8_t *)data_out32 < data->memory + data->memorySize;
        size_out = ARG32;
        if (valid) {
          Dsp_BlkUnpacked(data_in32, size_in, data_out32, size_out);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_BlkUnpacked(0x%08X, %d, 0x%08X, %d)", adata_in, size_in, adata_out, size_out);
      }
      break;

    case 99: { // void Dsp_InStream(data_in, block_size, num_blocks, blocks_done)
        valid = 0;
        adata_in = ARG32;
        data_in = (int8_t *)(memory + adata_in);
        valid |= (uint8_t *)data_in >= data->memory && (uint8_t *)data_in < data->memory + data->memorySize;
        block_size = ARG32;
        num_blocks = ARG32;
        ablocks_done = ARG32;
        blocks_done = (int32_t *)(memory + ablocks_done);
        valid |= (uint8_t *)blocks_done >= data->memory && (uint8_t *)blocks_done < data->memory + data->memorySize;
        if (valid) {
          Dsp_InStream(data_in, block_size, num_blocks, blocks_done);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_InStream(0x%08X, %d, %d, 0x%08X)", adata_in, block_size, num_blocks, ablocks_done);
      }
      break;

    case 100: { // void Dsp_OutStream(data_out, block_size, num_blocks, blocks_done)
        valid = 0;
        adata_out = ARG32;
        data_out = (int8_t *)(memory + adata_out);
        valid |= (uint8_t *)data_out >= data->memory && (uint8_t *)data_out < data->memory + data->memorySize;
        block_size = ARG32;
        num_blocks = ARG32;
        ablocks_done = ARG32;
        blocks_done = (int32_t *)(memory + ablocks_done);
        valid |= (uint8_t *)blocks_done >= data->memory && (uint8_t *)blocks_done < data->memory + data->memorySize;
        if (valid) {
          Dsp_OutStream(data_out, block_size, num_blocks, blocks_done);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_OutStream(0x%08X, %d, %d, 0x%08X)", adata_out, block_size, num_blocks, ablocks_done);
      }
      break;

    case 101: { // void Dsp_IOStream(data_in, data_out, block_insize, block_outsize, num_blocks, blocks_done)
        valid = 0;
        adata_in = ARG32;
        data_in = (int8_t *)(memory + adata_in);
        valid |= (uint8_t *)data_in >= data->memory && (uint8_t *)data_in < data->memory + data->memorySize;
        adata_out = ARG32;
        data_out = (int8_t *)(memory + adata_out);
        valid |= (uint8_t *)data_out >= data->memory && (uint8_t *)data_out < data->memory + data->memorySize;
        block_insize = ARG32;
        block_outsize = ARG32;
        num_blocks = ARG32;
        ablocks_done = ARG32;
        blocks_done = (int32_t *)(memory + ablocks_done);
        valid |= (uint8_t *)blocks_done >= data->memory && (uint8_t *)blocks_done < data->memory + data->memorySize;
        if (valid) {
          Dsp_IOStream(data_in, data_out, block_insize, block_outsize, num_blocks, blocks_done);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_IOStream(0x%08X, 0x%08X, %d, %d, %d, 0x%08X)", adata_in, adata_out, block_insize, block_outsize, num_blocks, ablocks_done);
      }
      break;

    case 102: { // void Dsp_RemoveInterrupts(int16_t mask)
        int16_t mask = ARG16;
        Dsp_RemoveInterrupts(mask);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_RemoveInterrupts(%d)", mask);
      }
      break;

    case 103: { // int16_t Dsp_GetWordSize(void)
        res = 0;
        res = Dsp_GetWordSize();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_GetWordSize(): %d", res);
      }
      break;

    case 104: { // int16_t Dsp_Lock(void)
        int16_t res = 0;
        res = Dsp_Lock();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Lock(): %d", res);
      }
      break;

    case 105: { // void Dsp_Unlock(void)
        Dsp_Unlock();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Unlock()");
      }
      break;

    case 106: { // void Dsp_Available(xavailable, yavailable)
        valid = 0;
        axavailable = ARG32;
        xavailable = (int32_t *)(memory + axavailable);
        valid |= (uint8_t *)xavailable >= data->memory && (uint8_t *)xavailable < data->memory + data->memorySize;
        ayavailable = ARG32;
        yavailable = (int32_t *)(memory + ayavailable);
        valid |= (uint8_t *)yavailable >= data->memory && (uint8_t *)yavailable < data->memory + data->memorySize;
        if (valid) {
          Dsp_Available(xavailable, yavailable);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Available(0x%08X, 0x%08X)", axavailable, ayavailable);
      }
      break;

    case 107: { // int16_t Dsp_Reserve(xreserve, yreserve)
        xreserve = ARG32;
        yreserve = ARG32;
        res = 0;
        res = Dsp_Reserve(xreserve, yreserve);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Reserve(%d, %d): %d", xreserve, yreserve, res);
      }
      break;

    case 108: { // int16_t Dsp_LoadProg(file, ability, buffer)
        valid = 0;
        afile = ARG32;
        file = (int8_t *)(memory + afile);
        valid |= (uint8_t *)file >= data->memory && (uint8_t *)file < data->memory + data->memorySize;
        ability = ARG16;
        abuffer = ARG32;
        buffer = (int8_t *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Dsp_LoadProg(file, ability, buffer);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_LoadProg(0x%08X, %d, 0x%08X): %d", afile, ability, abuffer, res);
      }
      break;

    case 109: { // void Dsp_ExecProg(codeptr, codesize, ability)
        valid = 0;
        acodeptr = ARG32;
        codeptr = (int8_t *)(memory + acodeptr);
        valid |= (uint8_t *)codeptr >= data->memory && (uint8_t *)codeptr < data->memory + data->memorySize;
        codesize = ARG32;
        ability = ARG16;
        if (valid) {
          Dsp_ExecProg(codeptr, codesize, ability);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_ExecProg(0x%08X, %d, %d)", acodeptr, codesize, ability);
      }
      break;

    case 110: { // void Dsp_ExecBoot(codeptr, codesize, ability)
        valid = 0;
        acodeptr = ARG32;
        codeptr = (int8_t *)(memory + acodeptr);
        valid |= (uint8_t *)codeptr >= data->memory && (uint8_t *)codeptr < data->memory + data->memorySize;
        codesize = ARG32;
        ability = ARG16;
        if (valid) {
          Dsp_ExecBoot(codeptr, codesize, ability);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_ExecBoot(0x%08X, %d, %d)", acodeptr, codesize, ability);
      }
      break;

    case 111: { // int32_t Dsp_LodToBinary(file, codeptr)
        valid = 0;
        afile = ARG32;
        file = (int8_t *)(memory + afile);
        valid |= (uint8_t *)file >= data->memory && (uint8_t *)file < data->memory + data->memorySize;
        acodeptr = ARG32;
        codeptr = (int8_t *)(memory + acodeptr);
        valid |= (uint8_t *)codeptr >= data->memory && (uint8_t *)codeptr < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Dsp_LodToBinary(file, codeptr);
        }
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_LodToBinary(0x%08X, 0x%08X): %d", afile, acodeptr, res);
      }
      break;

    case 112: { // void Dsp_TriggerHC(vector)
        avector = ARG16;
        Dsp_TriggerHC((int16_t)avector);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_TriggerHC(%d)", avector);
      }
      break;

    case 113: { // int16_t Dsp_RequestUniqueAbility(void)
        res = 0;
        res = Dsp_RequestUniqueAbility();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_RequestUniqueAbility(): %d", res);
      }
      break;

    case 114: { // int16_t Dsp_GetProgAbility(void)
        res = 0;
        res = Dsp_GetProgAbility();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_GetProgAbility(): %d", res);
      }
      break;

    case 115: { // void Dsp_FlushSubroutines(void)
        Dsp_FlushSubroutines();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_FlushSubroutines()");
      }
      break;

    case 116: { // int16_t Dsp_LoadSubroutine(codeptr, codesize, ability)
        valid = 0;
        acodeptr = ARG32;
        codeptr = (int8_t *)(memory + acodeptr);
        valid |= (uint8_t *)codeptr >= data->memory && (uint8_t *)codeptr < data->memory + data->memorySize;
        codesize = ARG32;
        ability = ARG16;
        res = 0;
        if (valid) {
          res = Dsp_LoadSubroutine(codeptr, codesize, ability);
        }
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_LoadSubroutine(0x%08X, %d, %d): %d", acodeptr, codesize, ability, res);
      }
      break;

    case 117: { // int16_t Dsp_InqSubrAbility(ability)
        ability = ARG16;
        res = 0;
        res = Dsp_InqSubrAbility(ability);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_InqSubrAbility(%d): %d", ability, res);
      }
      break;

    case 118: { // int16_t Dsp_RunSubroutine(handle)
        handle = ARG16;
        res = 0;
        res = Dsp_RunSubroutine(handle);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_RunSubroutine(%d): %d", handle, res);
      }
      break;

    case 119: { // int16_t Dsp_Hf0(flag)
        flag = ARG16;
        res = 0;
        res = Dsp_Hf0(flag);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Hf0(%d): %d", flag, res);
      }
      break;

    case 120: { // int16_t Dsp_Hf1(flag)
        flag = ARG16;
        res = 0;
        res = Dsp_Hf1(flag);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Hf1(%d): %d", flag, res);
      }
      break;

    case 121: { // int16_t Dsp_Hf2(void)
        int16_t res = 0;
        res = Dsp_Hf2();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Hf2(): %d", res);
      }
      break;

    case 122: { // int16_t Dsp_Hf3(void)
        res = 0;
        res = Dsp_Hf3();
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Hf3(): %d", res);
      }
      break;

    case 123: { // void Dsp_BlkWords(data_in, size_in, data_out, size_out)
        valid = 0;
        adata_in = ARG32;
        data_in = (void *)(memory + adata_in);
        valid |= (uint8_t *)data_in >= data->memory && (uint8_t *)data_in < data->memory + data->memorySize;
        size_in = ARG32;
        adata_out = ARG32;
        data_out = (void *)(memory + adata_out);
        valid |= (uint8_t *)data_out >= data->memory && (uint8_t *)data_out < data->memory + data->memorySize;
        size_out = ARG32;
        if (valid) {
          Dsp_BlkWords(data_in, size_in, data_out, size_out);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_BlkWords(0x%08X, %d, 0x%08X, %d)", adata_in, size_in, adata_out, size_out);
      }
      break;

    case 124: { // void Dsp_BlkBytes(data_in, size_in, data_out, size_out)
        valid = 0;
        adata_in = ARG32;
        data_in = (void *)(memory + adata_in);
        valid |= (uint8_t *)data_in >= data->memory && (uint8_t *)data_in < data->memory + data->memorySize;
        size_in = ARG32;
        adata_out = ARG32;
        data_out = (void *)(memory + adata_out);
        valid |= (uint8_t *)data_out >= data->memory && (uint8_t *)data_out < data->memory + data->memorySize;
        size_out = ARG32;
        if (valid) {
          Dsp_BlkBytes(data_in, size_in, data_out, size_out);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_BlkBytes(0x%08X, %d, 0x%08X, %d)", adata_in, size_in, adata_out, size_out);
      }
      break;

    case 125: { // int8_t Dsp_HStat(void)
        res = 0;
        res = Dsp_HStat();
		m68k_set_reg(M68K_REG_D0, (int8_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_HStat(): %d", res);
      }
      break;

    case 126: { // void Dsp_SetVectors(receiver, transmitter)
        valid = 0;
        areceiver = ARG32;
        receiver = (void *)(memory + areceiver);
        valid |= (uint8_t *)receiver >= data->memory && (uint8_t *)receiver < data->memory + data->memorySize;
        atransmitter = ARG32;
        transmitter = (void *)(memory + atransmitter);
        valid |= (uint8_t *)transmitter >= data->memory && (uint8_t *)transmitter < data->memory + data->memorySize;
        if (valid) {
          Dsp_SetVectors(receiver, transmitter);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_SetVectors(0x%08X, 0x%08X)", areceiver, atransmitter);
      }
      break;

    case 127: { // void Dsp_MultBlocks(numsend, numreceive, sendblocks, receiveblocks)
        valid = 0;
        numsend = ARG32;
        numreceive = ARG32;
        asendblocks = ARG32;
        sendblocks = (DSPBLOCK *)(memory + asendblocks);
        valid |= (uint8_t *)sendblocks >= data->memory && (uint8_t *)sendblocks < data->memory + data->memorySize;
        areceiveblocks = ARG32;
        receiveblocks = (DSPBLOCK *)(memory + areceiveblocks);
        valid |= (uint8_t *)receiveblocks >= data->memory && (uint8_t *)receiveblocks < data->memory + data->memorySize;
        if (valid) {
          Dsp_MultBlocks(numsend, numreceive, sendblocks, receiveblocks);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_MultBlocks(%d, %d, 0x%08X, 0x%08X)", numsend, numreceive, asendblocks, areceiveblocks);
      }
      break;

    case 128: { // int32_t locksnd(void)
        res = 0;
        res = locksnd();
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS locksnd(): %d", res);
      }
      break;

    case 129: { // int32_t unlocksnd(void)
        res = 0;
        res = unlocksnd();
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS unlocksnd(): %d", res);
      }
      break;

    case 130: { // int32_t soundcmd(mode, dat)
        mode = ARG16;
        dat = ARG16;
        res = 0;
        res = soundcmd(mode, dat);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS soundcmd(%d, %d): %d", mode, dat, res);
      }
      break;

    case 131: { // int32_t setbuffer(reg, begaddr, endaddr)
        valid = 0;
        reg = ARG16;
        abegaddr = ARG32;
        begaddr = (void *)(memory + abegaddr);
        valid |= (uint8_t *)begaddr >= data->memory && (uint8_t *)begaddr < data->memory + data->memorySize;
        aendaddr = ARG32;
        endaddr = (void *)(memory + aendaddr);
        valid |= (uint8_t *)endaddr >= data->memory && (uint8_t *)endaddr < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = setbuffer(reg, begaddr, endaddr);
        }
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS setbuffer(%d, 0x%08X, 0x%08X): %d", reg, abegaddr, aendaddr, res);
      }
      break;

    case 132: { // int32_t setmode(mode)
        mode = ARG16;
        res = 0;
        res = setmode(mode);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS setmode(%d): %d", mode, res);
      }
      break;

    case 133: { // int32_t settracks(playtracks, rectracks)
        playtracks = ARG16;
        rectracks = ARG16;
        res = 0;
        res = settracks(playtracks, rectracks);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS settracks(%d, %d): %d", playtracks, rectracks, res);
      }
      break;

    case 134: { // int32_t setmontracks(int16_t montrack)
        int16_t montrack = ARG16;
        res = 0;
        res = setmontracks(montrack);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS setmontracks(%d): %d", montrack, res);
      }
      break;

    case 135: { // int32_t setinterrupt(src_inter, cause)
        src_inter = ARG16;
        cause = ARG16;
        res = 0;
        res = setinterrupt(src_inter, cause);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS setinterrupt(%d, %d): %d", src_inter, cause, res);
      }
      break;

    case 136: { // int32_t buffoper(mode)
        mode = ARG16;
        res = 0;
        res = buffoper(mode);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS buffoper(%d): %d", mode, res);
      }
      break;

    case 137: { // int32_t dsptristate(dspxmit, dsprec)
        dspxmit = ARG16;
        dsprec = ARG16;
        res = 0;
        res = dsptristate(dspxmit, dsprec);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS dsptristate(%d, %d): %d", dspxmit, dsprec, res);
      }
      break;

    case 138: { // int32_t gpio(mode, dat)
        mode = ARG16;
        dat = ARG16;
        res = 0;
        res = gpio(mode, dat);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS gpio(%d, %d): %d", mode, dat, res);
      }
      break;

    case 139: { // int32_t devconnect(src, dst, srcclk, prescale, protocol)
        src = ARG16;
        dst = ARG16;
        srcclk = ARG16;
        prescale = ARG16;
        protocol = ARG16;
        res = 0;
        res = devconnect(src, dst, srcclk, prescale, protocol);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS devconnect(%d, %d, %d, %d, %d): %d", src, dst, srcclk, prescale, protocol, res);
      }
      break;

    case 140: { // int16_t sndstatus(reset)
        reset = ARG16;
        res = 0;
        res = sndstatus(reset);
		m68k_set_reg(M68K_REG_D0, (int16_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS sndstatus(%d): %d", reset, res);
      }
      break;

    case 141: { // int32_t buffptr(int32_t *ptr)
        valid = 0;
        aptr = ARG32;
        ptr32 = (int32_t *)(memory + aptr);
        valid |= (uint8_t *)ptr32 >= data->memory && (uint8_t *)ptr32 < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = buffptr(ptr32);
        }
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS buffptr(0x%08X): %d", aptr, res);
      }
      break;

    case 150: { // void VsetMask(ormask, andmask, overlay)
        ormask = ARG32;
        andmask = ARG32;
        overlay = ARG16;
        VsetMask(ormask, andmask, overlay);
        debug(DEBUG_TRACE, "TOS", "GEMDOS VsetMask(%d, %d, %d)", ormask, andmask, overlay);
      }
      break;

    case 160: { // int32_t CacheCtrl(opCode, Param)
        opCode = ARG16;
        param = ARG16;
        res = 0;
        res = CacheCtrl(opCode, param);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS CacheCtrl(%d, %d): %d", opCode, param, res);
      }
      break;

    case 161: { // int32_t WdgCtrl(opCode)
        opCode = ARG16;
        res = 0;
        res = WdgCtrl(opCode);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS WdgCtrl(%d): %d", opCode, res);
      }
      break;

    case 162: { // int32_t Xbios(command, device, int32_t param)
        command = ARG16;
        device = ARG16;
        param32 = ARG32;
        res = 0;
        res = Xbios(command, device, param32);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Xbios(%d, %d, %d): %d", command, device, param32, res);
      }
      break;

    case 165: { // int32_t WavePlay(flags, rate, sptr, slen)
        flags = ARG16;
        rate = ARG32;
        sptr = ARG32;
        slen = ARG32;
        res = 0;
        res = WavePlay(flags, rate, sptr, slen);
		m68k_set_reg(M68K_REG_D0, (int32_t)res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS WavePlay(%d, %d, %d, %d): %d", flags, rate, sptr, slen, res);
      }
      break;


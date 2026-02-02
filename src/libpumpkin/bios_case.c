    case 0: { // void Getmpb(ptr)
        valid = 0;
        aptr = ARG32;
        ptr = (MPB *)(memory + aptr);
        valid |= (uint8_t *)ptr >= data->memory && (uint8_t *)ptr < data->memory + data->memorySize;
        if (valid) {
          Getmpb(ptr);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Getmpb(0x%08X)", aptr);
      }
      break;

    case 1: { // int16_t Bconstat(dev)
        dev = ARG16;
        res = 0;
        res = (int16_t)Bconstat(dev);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Bconstat(%d): %d", dev, res);
      }
      break;

    case 2: { // int32_t Bconin(dev)
        dev = ARG16;
        res = 0;
        res = Bconin(dev);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Bconin(%d): %d", dev, res);
      }
      break;

    case 3: { // void Bconout(dev, int16_t c)a
        dev = ARG16;
        c = ARG16;
        Bconout(dev, c);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Bconout(%d, %d)", dev, c);
      }
      break;

    case 4: { // int32_t Rwabs(rwflag, buff, cnt, recnr, dev, lrecno)
        valid = 0;
        rwflag = ARG16;
        abuff = ARG32;
        buff = (void *)(memory + abuff);
        valid |= (uint8_t *)buff >= data->memory && (uint8_t *)buff < data->memory + data->memorySize;
        cnt = ARG16;
        recnr = ARG16;
        dev = ARG16;
        lrecno = ARG32;
        res = 0;
        if (valid) {
          res = Rwabs(rwflag, buff, cnt, recnr, dev, lrecno);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Rwabs(%d, 0x%08X, %d, %d, %d, %d): %d", rwflag, abuff, cnt, recnr, dev, lrecno, res);
      }
      break;

    case 5: { // int32_t Setexc(int16_t number, vec)
        valid = 0;
        number = ARG16;
        avec = ARG32;
        vecv = (void *)(memory + avec);
        valid |= (uint8_t *)vecv >= data->memory && (uint8_t *)vecv < data->memory + data->memorySize;
        res = 0;
        if (valid) {
          res = Setexc((int16_t)number, vecv);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Setexc(%d, 0x%08X): %d", number, avec, res);
      }
      break;

    case 6: { // int32_t Tickcal(void)
        res = 0;
        res = Tickcal();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tickcal(): %d", res);
      }
      break;

    case 7: { // BPB *Getbpb(dev)
        dev = ARG16;
        resb = NULL;
        resb = Getbpb(dev);
        ares = res ? (uint32_t)((uint8_t *)resb - memory) : 0;
        m68k_set_reg(M68K_REG_D0, ares);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Getbpb(%d): 0x%08X", dev, ares);
      }
      break;

    case 8: { // int32_t Bcostat(dev)
        dev = ARG16;
        res = 0;
        res = Bcostat(dev);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Bcostat(%d): %d", dev, res);
      }
      break;

    case 9: { // int32_t Mediach(dev)
        dev = ARG16;
        res = 0;
        res = Mediach(dev);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Mediach(%d): %d", dev, res);
      }
      break;

    case 10: { // int32_t Drvmap(void)
        res = 0;
        res = Drvmap();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Drvmap(): %d", res);
      }
      break;

    case 11: { // int32_t Kbshift(int16_t mode)
        mode = ARG16;
        res = 0;
		res = Kbshift((int16_t)mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Kbshift(%d): %d", mode, res);
      }
      break;


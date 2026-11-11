PumpkinOS add-on content (DLC) sample.

This file was copied from the PumpkinOS DLC into the card's Programs
directory (sdmc:/switch/pumpkinos/vfs/app_card/PALM/Programs). Build your
own DLC from a folder laid out like sdmc:/switch/pumpkinos/:

  vfs/app_install/*.prc            application resources to deploy
  vfs/app_card/PALM/Programs/...   card files (e.g. GnuBoy ROMs)
  soundfont.sf2, cert.pem          MIDI soundfont, https CA bundle

with: make OSNAME=switch -f GNUMakefile SWITCH_DLC_DIR=<folder>

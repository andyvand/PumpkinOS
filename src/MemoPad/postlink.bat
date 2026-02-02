@REM Add post-link commands below.
copy obj\%1\MemoPad.dll dlib0089.bin
..\..\tools\sh.exe -c "../../tools/prcbuild.exe -f MemoPad.prc -t appl -c memo -n \"Memo Pad\" dlib0089.bin resources/*"
copy MemoPad.prc %_FLATRELEASEDIR%\MemoPad.prc

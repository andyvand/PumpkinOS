@REM Add post-link commands below.
copy obj\%1\DateBook.dll dlib0089.bin
..\..\tools\sh.exe -c "../../tools/prcbuild.exe -f DateBook.prc -t appl -c date -n \"Date Book\" dlib0089.bin resources/*"
copy DateBook.prc %_FLATRELEASEDIR%\DateBook.prc

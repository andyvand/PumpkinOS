@REM Add post-link commands below.
copy obj\%1\Command.dll dlib0089.bin
..\..\tools\sh.exe -c "../../tools/prcbuild.exe -f Command.prc -t appl -c CmdP -n \"Command\" dlib0089.bin resources/*"
copy Command.prc %_FLATRELEASEDIR%\Command.prc

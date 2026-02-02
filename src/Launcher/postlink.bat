@REM Add post-link commands below.
copy obj\%1\Launcher.dll dlib0089.bin
..\..\tools\sh.exe -c "../../tools/prcbuild.exe -f Launcher.prc -t appl -c lnch -n \"Launcher\" dlib0089.bin resources/*"
copy Launcher.prc %_FLATRELEASEDIR%\Launcher.prc

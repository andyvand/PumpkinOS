@REM Add post-link commands below.
..\..\tools\sh.exe -c "../../tools/prcbuild.exe -f BOOT.prc -t boot -c BOOT -n \"BOOT\" resources/*"
copy BOOT.prc %_FLATRELEASEDIR%\BOOT.prc

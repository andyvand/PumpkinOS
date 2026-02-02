@REM Add post-link commands below.
copy obj\%1\Preferences.dll dlib0089.bin
..\..\tools\sh.exe -c "../../tools/prcbuild.exe -f Preferences.prc -t appl -c pref -n \"Preferences\" dlib0089.bin resources/*"
copy Preferences.prc %_FLATRELEASEDIR%\Preferences.prc

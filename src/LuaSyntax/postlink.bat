@REM Add post-link commands below.
copy obj\%1\LuaSyntax.dll dlib0089.bin
..\..\tools\sh.exe -c "../../tools/prcbuild.exe -f LuaSyntax.prc -t plgi -c LuaS -n \"LuaSyntax\" dlib0089.bin"
copy LuaSyntax.prc %_FLATRELEASEDIR%\LuaSyntax.prc

@REM Add post-link commands below.
copy obj\%1\vi.dll dlib0089.bin
..\..\tools\sh.exe -c "../../tools/prcbuild.exe -f vi.prc -t plgi -c viEd -n \"vi\" dlib0089.bin"
copy vi.prc %_FLATRELEASEDIR%\vi.prc

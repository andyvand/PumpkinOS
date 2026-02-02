@REM Add post-link commands below.
copy obj\%1\ToDoList.dll dlib0089.bin
..\..\tools\sh.exe -c "../../tools/prcbuild.exe -f ToDoList.prc -t appl -c todo -n \"To Do List\" dlib0089.bin resources/*"
copy ToDoList.prc %_FLATRELEASEDIR%\ToDoList.prc

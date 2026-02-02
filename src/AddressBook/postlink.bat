@REM Add post-link commands below.
copy obj\%1\AddressBook.dll dlib0089.bin
..\..\tools\sh.exe -c "../../tools/prcbuild.exe -f AddressBook.prc -t appl -c addr -n \"Address Book\" dlib0089.bin resources/*"
copy AddressBook.prc %_FLATRELEASEDIR%\AddressBook.prc

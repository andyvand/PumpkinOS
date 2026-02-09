cd ..
idf.py build
cd release_esp32
/Applications/ffmpeg -i Tile.png -f rawvideo -pix_fmt rgb565 tile.raw
/Users/andyvand/Documents/GitHub/odroid-go-firmware-20181001/tools/mkfw/mkfw GNUBoy tile.raw 0 16 1048576 app ../build/gnuboy.bin
rm GNUBoy.fw
mv firmware.fw GNUBoy.fw
/Users/andyvand/Documents/GitHub/odroid-go-firmware-20181001/tools/mkfw/mkfw GNUBoy tile.raw 0 16 1048576 app ../build/gnuboy.bin
rm GNUBoy-20181001.fw
mv firmware.fw GNUBoy-20181001.fw

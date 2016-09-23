@echo off

cd loader/cube
echo.
echo Building Cube DOL Loader
echo.
make clean
make

cd ../wii
echo.
echo Building Wii DOL Loader
echo.
make clean
make

cd ..
echo.
echo Building Cube Main Loader
echo.
make -f Makefile.cube clean
make -f Makefile.cube

echo.
echo Building Wii Main Loader
echo.
make -f Makefile.wii clean
make -f Makefile.wii

cd ..
echo.
echo Building Executable
echo.
echo cubexz
gcc -DHW_DOL=1 -Wall -static -O2 -s main.c -llzma -o cubexz
echo wiixz
gcc -DHW_RVL=1 -Wall -static -O2 -s main.c -llzma -o wiixz
echo.

pause

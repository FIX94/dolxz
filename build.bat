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
make -f Makefile.cube high=1 clean
make -f Makefile.cube high=1

echo.
echo Building Wii Main Loader
echo.
make -f Makefile.wii clean
make -f Makefile.wii
make -f Makefile.wii high=1 clean
make -f Makefile.wii high=1

cd ..
echo.
echo Building Executable
echo.
echo dolxz
gcc -Wall -static -O2 -s main.c -llzma -o dolxz
echo.

pause

#!/bin/bash

cd loader/cube
echo
echo Building Cube DOL Loader
echo
PATH=${PATH}:/opt/devkitpro/devkitPPC/bin make clean
PATH=${PATH}:/opt/devkitpro/devkitPPC/bin make

cd ../wii
echo
echo Building Wii DOL Loader
echo
PATH=${PATH}:/opt/devkitpro/devkitPPC/bin make clean
PATH=${PATH}:/opt/devkitpro/devkitPPC/bin make

cd ..
echo
echo Building Cube Main Loader
echo
PATH=${PATH}:/opt/devkitpro/devkitPPC/bin make -f Makefile.cube clean
PATH=${PATH}:/opt/devkitpro/devkitPPC/bin make -f Makefile.cube
PATH=${PATH}:/opt/devkitpro/devkitPPC/bin make -f Makefile.cube high=1 clean
PATH=${PATH}:/opt/devkitpro/devkitPPC/bin make -f Makefile.cube high=1

echo
echo Building Wii Main Loader
echo
PATH=${PATH}:/opt/devkitpro/devkitPPC/bin make -f Makefile.wii clean
PATH=${PATH}:/opt/devkitpro/devkitPPC/bin make -f Makefile.wii
PATH=${PATH}:/opt/devkitpro/devkitPPC/bin make -f Makefile.wii high=1 clean
PATH=${PATH}:/opt/devkitpro/devkitPPC/bin make -f Makefile.wii high=1

cd ..
echo
echo Building Executable
echo
echo dolxz
gcc -Wall -static -O2 -s main.c -llzma -o dolxz
echo

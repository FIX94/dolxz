ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

DEVKITPATH=$(shell echo "$(DEVKITPRO)" | sed -e 's/^\([a-zA-Z]\):/\/\1/')
export PATH	:=	$(DEVKITPATH)/tools/bin:$(DEVKITPATH)/devkitPPC/bin:$(PATH)

dolxz: loader/loader_cube.h loader/loader_cube_high.h loader/loader_wii.h loader/loader_wii_high.h
	gcc -Wall -static -O2 -s main.c -llzma -o dolxz
	chmod +x dolxz

loader/cube/dolloader.h:
	@$(MAKE) -C loader/cube

loader/wii/dolloader.h:
	@$(MAKE) -C loader/wii

loader/loader_cube.h: loader/cube/dolloader.h
	@$(MAKE) -C loader -f Makefile.cube

loader/loader_cube_high.h: loader/cube/dolloader.h
	@$(MAKE) -C loader -f Makefile.cube high=1

loader/loader_wii.h: loader/wii/dolloader.h
	@$(MAKE) -C loader -f Makefile.wii

loader/loader_wii_high.h: loader/wii/dolloader.h
	@$(MAKE) -C loader -f Makefile.wii high=1

clean:
	@$(MAKE) -C loader/cube clean
	@$(MAKE) -C loader/wii clean
	@$(MAKE) -C loader -f Makefile.cube clean
	@$(MAKE) -C loader -f Makefile.cube high=1 clean
	@$(MAKE) -C loader -f Makefile.wii clean
	@$(MAKE) -C loader -f Makefile.wii high=1 clean
	rm -f dolxz
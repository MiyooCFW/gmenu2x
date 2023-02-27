PLATFORM := retrofw

BUILDTIME := $(shell date +%s)

CHAINPREFIX := /opt/mipsel-RetroFW-linux-uclibc
CROSS_COMPILE := $(CHAINPREFIX)/usr/bin/mipsel-linux-
export CROSS_COMPILE

CC			:= $(CROSS_COMPILE)gcc
CXX			:= $(CROSS_COMPILE)g++
STRIP		:= $(CROSS_COMPILE)strip

SYSROOT     := $(shell $(CC) --print-sysroot)
SDL_CFLAGS  := $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
SDL_LIBS    := $(shell $(SYSROOT)/usr/bin/sdl-config --libs)

CFLAGS = -DPLATFORM=\"$(PLATFORM)\" -D__BUILDTIME__="$(BUILDTIME)" -DLOG_LEVEL=3
CFLAGS += -Os -ggdb -g3 $(SDL_CFLAGS)
CFLAGS += -mhard-float -mips32 -mno-mips16
CFLAGS += -std=c++11 -fdata-sections -ffunction-sections -fno-exceptions -fno-math-errno -fno-threadsafe-statics -Wno-narrowing
CFLAGS += -Isrc -Isrc/libopk
CFLAGS += -DTARGET_RETROFW -DHW_TVOUT -DHW_UDC -DHW_EXT_SD -DHW_SCALER -DOPK_SUPPORT -DIPK_SUPPORT
CFLAGS += -D_GLIBCXX_USE_CXX11_ABI=0

LDFLAGS = -Wl,-Bstatic -Lsrc/libopk -l:libopk.a
LDFLAGS += -Wl,-Bdynamic -lz $(SDL_LIBS) -lSDL_image -lSDL_ttf
LDFLAGS += -Wl,--as-needed -Wl,--gc-sections

OBJDIR = /tmp/gmenu2x/$(PLATFORM)
DISTDIR = dist/$(PLATFORM)
TARGET = $(DISTDIR)/gmenu2x

SOURCES := $(wildcard src/*.cpp)
OBJS := $(patsubst src/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

# File types rules
$(OBJDIR)/%.o: src/%.cpp src/%.h src/platform/$(PLATFORM).h
	$(CXX) $(CFLAGS) -o $@ -c $<

all: dir libopk shared

dir:
	@mkdir -p $(OBJDIR) $(DISTDIR)

libopk:
	make -C src/libopk clean
	make -C src/libopk

debug: $(OBJS)
	@echo "Linking gmenu2x-debug..."
	$(CXX) -o $(TARGET)-debug $(OBJS) $(LDFLAGS)

shared: debug
	$(STRIP) $(TARGET)-debug -o $(TARGET)

clean:
	make -C src/libopk clean
	rm -rf $(OBJDIR) *.gcda *.gcno $(TARGET) $(TARGET)-debug /tmp/.gmenu-ipk/ $(DISTDIR) dist/gmenunx-$(PLATFORM).zip dist/gmenunx-$(PLATFORM).ipk

ipk: dist
	rm -rf /tmp/.gmenu-ipk/; mkdir -p /tmp/.gmenu-ipk/
	sed "s/^Version:.*/Version: $$(date +%Y%m%d)/" assets/control > /tmp/.gmenu-ipk/control
	cp assets/conffiles /tmp/.gmenu-ipk/
	echo -e "#!/bin/sh\nsync; echo -e 'Installing gmenunx..'; mount -o remount,rw /; rm /var/lib/opkg/info/gmenunx.list; exit 0" > /tmp/.gmenu-ipk/preinst
	echo -e "#!/bin/sh\nsync; mount -o remount,ro /; echo -e 'Installation finished.\nRestarting gmenunx..'; sleep 1; killall gmenu2x; exit 0" > /tmp/.gmenu-ipk/postinst
	chmod +x /tmp/.gmenu-ipk/postinst /tmp/.gmenu-ipk/preinst
	tar --owner=0 --group=0 -czvf /tmp/.gmenu-ipk/control.tar.gz -C /tmp/.gmenu-ipk/ control conffiles postinst preinst
	tar --owner=0 --group=0 -czvf /tmp/.gmenu-ipk/data.tar.gz -C $(DISTDIR) about.txt COPYING gmenu2x gmenu2x.conf input.conf skins translations
	echo 2.0 > /tmp/.gmenu-ipk/debian-binary
	ar r dist/gmenunx-$(PLATFORM).ipk /tmp/.gmenu-ipk/control.tar.gz /tmp/.gmenu-ipk/data.tar.gz /tmp/.gmenu-ipk/debian-binary

dist: dir libopk shared
	mkdir -p $(DISTDIR)/skins/Default
	install -m644 -D about.txt $(DISTDIR)/about.txt
	install -m644 -D COPYING $(DISTDIR)/COPYING
	cp -RH assets/translations $(DISTDIR)
	cp -RH assets/skins/RetroFW/* $(DISTDIR)/skins/Default
	cp -RH assets/skins/Default/font.ttf $(DISTDIR)/skins/Default
	cp -RH assets/$(PLATFORM)/input.conf $(DISTDIR)
	echo "wallpaper=\"skins/Default/wallpapers/RetroFW.png\"" > $(DISTDIR)/gmenu2x.conf

zip: dist
	cd $(DISTDIR)/ && rm -f ../gmenunx-$(PLATFORM).zip && zip -r ../gmenunx-$(PLATFORM).zip skins translations COPYING gmenu2x input.conf gmenu2x.conf about.txt

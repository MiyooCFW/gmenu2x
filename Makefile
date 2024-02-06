PLATFORM := linux

BUILDTIME := "$(shell date +%F\ %H:%M)"
COMMIT_HASH := $(shell git rev-parse --short HEAD)
GMENU2X_HASH := -D__BUILDTIME__=$(BUILDTIME) -D__COMMIT_HASH__=$(COMMIT_HASH)

CC			:= gcc
CXX			:= g++
STRIP		:= strip

SYSROOT     := $(shell $(CC) --print-sysroot)
SDL_CFLAGS  := $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
SDL_LIBS    := $(shell $(SYSROOT)/usr/bin/sdl-config --libs)

CFLAGS = -DPLATFORM=\"$(PLATFORM)\" $(GMENU2X_HASH) -DLOG_LEVEL=4
CFLAGS += -O0 -ggdb -g -pg $(SDL_CFLAGS)
CFLAGS += -Wundef -Wno-deprecated -Wno-unknown-pragmas -Wno-format -Wno-narrowing
CFLAGS += -Isrc -Isrc/libopk
CFLAGS += -DTARGET_LINUX -DHW_TVOUT -DHW_UDC -DHW_EXT_SD -DHW_SCALER -DOPK_SUPPORT -DIPK_SUPPORT

LDFLAGS = -Wl,-Bstatic -Lsrc/libopk -l:libopk.a
LDFLAGS += -Wl,-Bdynamic -lz $(SDL_LIBS) -lSDL_image -lSDL_ttf

OBJDIR = /tmp/gmenu2x/$(PLATFORM)
DISTDIR = dist/$(PLATFORM)
TARGET = dist/$(PLATFORM)/gmenu2x

DEFSKIN = FontiGrid

SOURCES := $(wildcard src/*.cpp)
OBJS := $(patsubst src/%.cpp, $(OBJDIR)/src/%.o, $(SOURCES))

# File types rules
$(OBJDIR)/src/%.o: src/%.cpp src/%.h src/platform/linux.h
	$(CXX) $(CFLAGS) -o $@ -c $<

-include $(patsubst src/%.cpp, $(OBJDIR)/src/%.d, $(SOURCES))

all: dir libopk shared

dir:
	@mkdir -p $(OBJDIR)/src dist/$(PLATFORM)

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
	rm -rf $(OBJDIR) $(DISTDIR) *.gcda *.gcno $(TARGET) $(TARGET)-debug $(DISTDIR) dist/gmenu2x-$(PLATFORM).zip

dist: dir libopk shared
	install -m644 -D *about*.txt $(DISTDIR)/
	install -m644 -D README.md $(DISTDIR)/README.txt
	install -m644 -D COPYING $(DISTDIR)/LICENSE
	install -m644 -D CONTRIBUTORS.md $(DISTDIR)/CONTRIBUTORS
	install -m644 -D ChangeLog.md $(DISTDIR)/ChangeLog
	cp -RH assets/translations $(DISTDIR)
	cp -RH assets/skins $(DISTDIR)
	-find $(DISTDIR)/skins -type d -name "template" -exec rm -rf {} \;
ifneq ($(DEFSKIN), Default)
	cp -RH assets/skins/Default $(DISTDIR)/skins/Legacy
	cp -RH assets/skins/$(DEFSKIN)/* $(DISTDIR)/skins/Default
	rm -rf $(DISTDIR)/skins/$(DEFSKIN)
	sed -i '/wallpaper/d' $(DISTDIR)/skins/Default/skin.conf
endif
	cp -RH assets/$(PLATFORM)/input.conf $(DISTDIR)

zip: dist
	cd $(DISTDIR)/ && zip -r ../gmenu2x-$(PLATFORM).zip *

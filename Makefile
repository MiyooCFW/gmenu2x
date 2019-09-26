TARGET=miyoo

CHAINPREFIX := /opt/miyoo
CROSS_COMPILE := /opt/miyoo/bin/arm-linux-

BUILDTIME=$(shell date +'\"%Y-%m-%d %H:%M\"')

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE)strip

SYSROOT     := $(shell $(CC) --print-sysroot)
SDL_CFLAGS  := $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
SDL_LIBS    := $(shell $(SYSROOT)/usr/bin/sdl-config --libs)

CFLAGS = -ggdb -DTARGET_MIYOO -DTARGET=$(TARGET) -D__BUILDTIME__="$(BUILDTIME)" -DLOG_LEVEL=3 $(SDL_CFLAGS) -I$(CHAINPREFIX)/usr/include/ -I$(SYSROOT)/usr/include/  -I$(SYSROOT)/usr/include/SDL/
#CFLAGS += -std=c++11 -fdata-sections -ffunction-sections -fno-exceptions -fno-math-errno -fno-threadsafe-statics -Os

CXXFLAGS = $(CFLAGS)
LDFLAGS = $(SDL_LIBS) -lfreetype -lSDL_image -lSDL_mixer -lSDL_ttf -lSDL -lpthread
#LDFLAGS +=-Wl,--as-needed -Wl,--gc-sections -s

OBJDIR = objs/$(TARGET)
DISTDIR = dist/$(TARGET)/gmenu2x
APPNAME = $(OBJDIR)/gmenu2x

SOURCES := $(wildcard src/*.cpp)
OBJS := $(patsubst src/%.cpp, $(OBJDIR)/src/%.o, $(SOURCES))

$(OBJDIR)/src/%.o: src/%.cpp src/%.h
	$(CXX) $(CFLAGS) -o $@ -c $<

all: dir shared

dir:
	@if [ ! -d $(OBJDIR)/src ]; then mkdir -p $(OBJDIR)/src; fi

debug: $(OBJS)
	@echo "Linking gmenu2x-debug..."
	$(CXX) -o $(APPNAME)-debug $(LDFLAGS) $(OBJS)

shared: debug
	$(STRIP) $(APPNAME)-debug -o $(APPNAME)

clean:
	rm -rf $(OBJDIR) $(DISTDIR) *.gcda *.gcno $(APPNAME)

dist: dir shared
	install -m755 -D $(APPNAME) $(DISTDIR)/gmenu2x
	install -m644 assets/$(TARGET)/input.conf $(DISTDIR)
	# install -m755 -d $(DISTDIR)/sections/applications $(DISTDIR)/sections/emulators $(DISTDIR)/sections/games $(DISTDIR)/sections/settings
	install -m755 -d $(DISTDIR)/sections
	install -m644 -D README.md $(DISTDIR)/README.txt
	install -m644 -D COPYING $(DISTDIR)/COPYING
	install -m644 -D ChangeLog.md $(DISTDIR)/ChangeLog
	install -m644 -D about.txt $(DISTDIR)/about.txt
	cp -RH assets/skins assets/translations $(DISTDIR)
	cp -RH assets/$(TARGET)/BlackJeans.png $(DISTDIR)/skins/Default/wallpapers
	cp -RH assets/$(TARGET)/skin.conf $(DISTDIR)/skins/Default
	cp -RH assets/$(TARGET)/font.ttf $(DISTDIR)/skins/Default
	cp -RH assets/$(TARGET)/gmenu2x.conf $(DISTDIR)
	cp -RH assets/$(TARGET)/icons/* $(DISTDIR)/skins/Default/icons/
	cp -RH assets/$(TARGET)/skins/* $(DISTDIR)/skins/
	#cp -RH assets/$(TARGET)/emulators/* $(DISTDIR)/sections/emulators/
	#cp -RH assets/$(TARGET)/games/* $(DISTDIR)/sections/games/
	#cp -RH assets/$(TARGET)/applications/* $(DISTDIR)/sections/applications/
	cd $(DISTDIR)/.. && zip -FSr GMenuNX.zip gmenu2x

-include $(patsubst src/%.cpp, $(OBJDIR)/src/%.d, $(SOURCES))

$(OBJDIR)/src/%.d: src/%.cpp
	@if [ ! -d $(OBJDIR)/src ]; then mkdir -p $(OBJDIR)/src; fi
	$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


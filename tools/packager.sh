#!/bin/bash

# ABOUT: 
## GMenu2X packager tool to generate working release for your binaries working with this frontend (aimed at Miyoo currently)

# USAGE:  
## Put inside working directory:  
## - program's `<target_name>` binary
## - `./assets` dir with all necessary files which goes in same place where binary goes
## - `./opkg_assets` dir with custom IPK's control files (these are auto-generated if missing).
## - edit ENVironment VARiables and EXECution commands in `gm2x_packager.sh` script to perform desired outcome
## - run `./gm2x_packager.sh`

# NOTES: 
## Optionally put `Aliases.txt`, `<target_name>.man.txt` or <target_name>.lnk file in script current working directory

if test -f pkg.cfg; then
	source pkg.cfg
	echo "config file found, setting following variables:"
	grep -v '^#' pkg.cfg | head -n -1
else
	echo "no config file found, executing with predefined values from env or script"
	sleep 1
fi

# EXEC commands (set to 1 anyone for desired outcome):
if test -z $PACKAGE; then
	PACKAGE=0
elif test -z $ZIP; then
	ZIP=0
elif test -z $IPK; then
	IPK=0
elif test -z $CLEAN; then
	CLEAN=0
fi

# ENV VAR.
## Specific (mandatory to provide!)
if test -z $TARGET; then
	echo "No binary name provided, please set $TARGET in your env with correct execution program name"
	sleep 2
	exit
fi
if test -z $VERSION; then
	VERSION=$(date +%Y-%m-%d\ %H:%M) #replace with correct release version if exist
fi

## Generic common to all apps (better to not modify)
HOMEPATH="/mnt"
RELEASEDIR=package
ASSETSDIR=assets
OPKG_ASSETSDIR=opkg_assets
LINK=$TARGET.lnk #Modify if exec binary is different - place in CWD (warning: it may be removed with CLEAN=1)
ALIASES=aliases.txt #file with new names for selector e.g. old_title=new_title - place in CWD
MANUAL=$TARGET.man.txt #file with usage description of target app - place in CWD

## Link entries (better modify if no <target_name>.lnk file provided)
TITLE="${TARGET}"
DESCRI="${TARGET} app description"
SELDIR=""
DESTDIR=apps
SECTION=applications

## IPK control entries (if needed modify)
PRIORITY=optional
MAINTAINER=Unknown
CONFFILES="" # TODO (to preserve & not reinstall user configs)
ARCH=arm
CONTROL="Package: ${TARGET}\n\
Version: ${VERSION}\n\
Description: ${DESCRI}\n\
Section: ${SECTION}\n\
Priority: ${PRIORITY}\n\
Maintainer: ${MAINTAINER}\n\
Architecture: ${ARCH}"
#---------------------------------------------#
# CODE execution
if test $PACKAGE -ne 0 >/dev/null 2>&1 || test $ZIP -ne 0 >/dev/null 2>&1 || test $IPK -ne 0 >/dev/null 2>&1; then
	# Create ./package
	rm -rf $RELEASEDIR
	mkdir -p $RELEASEDIR
	mkdir -p $ASSETSDIR
	mkdir -p $OPKG_ASSETSDIR
	cp *$TARGET $RELEASEDIR/
	mkdir -p $RELEASEDIR/$DESTDIR/$TARGET
	mkdir -p $RELEASEDIR/gmenu2x/sections/$SECTION
	mv $RELEASEDIR/*$TARGET $RELEASEDIR/$DESTDIR/$TARGET/
	cp -r $ASSETSDIR/* $RELEASEDIR/$DESTDIR/$TARGET
	if !(test -e $LINK); then
		touch $LINK
		echo -e "title=${TITLE}\ndescription=${DESCRI}\nexec=" > $LINK
		sed -i "s/^exec=.*/exec=\/mnt\/${DESTDIR}\/${TARGET}\/${TARGET}/" $LINK
		test -n "$SELDIR" && echo "selectordir=${SELDIR}" >> $LINK
		if test -e $ALIASES; then
			echo "selectoraliases=\/mnt\/${DESTDIR}\/${TARGET}\/${ALIASES}" >> $LINK
		fi
	fi
	cp $LINK $RELEASEDIR/gmenu2x/sections/$SECTION
	cp $ALIASES $RELEASEDIR/$DESTDIR/$TARGET
	cp $MANUAL $RELEASEDIR/$DESTDIR/$TARGET
	
	# Create ./package/<target_version>.zip
	if test $ZIP -ne 0 >/dev/null 2>&1; then
		rm -rf $RELEASEDIR/*.ipk
		cd $RELEASEDIR && zip -rq $TARGET$VERSION.zip ./* && mv *.zip ..
		rm -rf $RELEASEDIR/*
		mv $TARGET*.zip $RELEASEDIR/
	fi
	
	# Create ./package/<target>.ipk
	if test $IPK -ne 0 >/dev/null 2>&1; then
		rm -rf $RELEASEDIR/*.zip
		mkdir -p .$HOMEPATH
		mv $RELEASEDIR/* .$HOMEPATH && mv .$HOMEPATH $RELEASEDIR
		mkdir -p $RELEASEDIR/data
		mv $RELEASEDIR$HOMEPATH $RELEASEDIR/data/
		if !(test -d $OPKG_ASSETSDIR/CONTROL); then
			mkdir -p $OPKG_ASSETSDIR/CONTROL
			echo -e "#!/bin/sh\nsync; echo 'Installing new ${TARGET}..'; rm /var/lib/opkg/info/${TARGET}.list; exit 0" > $OPKG_ASSETSDIR/CONTROL/preinst
			echo -e "#!/bin/sh\nsync; echo 'Installation finished.'; echo 'Restarting ${TARGET}..'; sleep 1; killall ${TARGET}; exit 0" > $OPKG_ASSETSDIR/CONTROL/postinst
			echo -e $CONTROL > $OPKG_ASSETSDIR/CONTROL/control
		fi
		chmod +x $OPKG_ASSETSDIR/CONTROL/postinst $OPKG_ASSETSDIR/CONTROL/preinst
		cp -r $OPKG_ASSETSDIR/CONTROL $RELEASEDIR
		sed "s/^Version:.*/Version: ${VERSION}/" $OPKG_ASSETSDIR/CONTROL/control > $RELEASEDIR/CONTROL/control
		echo 2.0 > $RELEASEDIR/debian-binary
		tar --owner=0 --group=0 -czvf $RELEASEDIR/data.tar.gz -C $RELEASEDIR/data/ . >/dev/null 2>&1
		tar --owner=0 --group=0 -czvf $RELEASEDIR/control.tar.gz -C $RELEASEDIR/CONTROL/ . >/dev/null 2>&1
		ar r $TARGET.ipk $RELEASEDIR/control.tar.gz $RELEASEDIR/data.tar.gz $RELEASEDIR/debian-binary
		rm -rf ${RELEASEDIR:?}/*
		mv $TARGET.ipk $RELEASEDIR/
	fi
elif test $CLEAN -ne 0 >/dev/null 2>&1; then
	rm -rf $RELEASEDIR
	rm -rf $OPKG_ASSETSDIR
	rm -f *.ipk
	rm -f *.zip
	rm -f $LINK
else
	echo "No instructions provided, please set \$PACKAGE/\$ZIP/\$IPK or \$CLEAN in env to 1 for correct output"
	sleep 1
fi
#---------------------------------------------#

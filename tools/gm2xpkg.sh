#!/bin/bash

VER=0.1
MIYOOCFW=2.0.0
# Help & About info
help_func() {
	echo -e "GMenu2X packager tool to generate working release for your binaries working with this frontend (aimed at MiyooCFW currently)\n\
	 Put inside working directory:\n\
	 \t- program's <target_name> binary\n\
	 \t- ./assets dir with all necessary files which goes in same place where binary goes\n\
	 \t- ./opkg_assets dir with custom IPK's control files (these are auto-generated if missing).\n\
	 Edit settings in pkg.cfg file\n\
	 Run program:\n\
	 \t$: ./gm2xpkg.sh <config_file>\n\
	 or install & run from usr space:\n\
	 \t$: install -m 755 gm2xpkg.sh /usr/bin/gm2xpkg\n\
	 \t$: gm2xpkg"
}

# OPTIONS
while :
do
	case $1 in
		-h | --help | -\?)
			help_func
			exit 0
			;;
		--version)
			echo -e "GM2X PACKAGER version ${VER} for MiyooCFW ${MIYOOCFW}"
			exit 0
			;;
		--) 
			shift
			break
			;;
		-*)
			echo "Unknown option: $1" >&2
			shift
			;;
		*)
			break
			;;
	esac
done

# NOTES: 
## Optionally put `Aliases.txt`, `<target_name>.man.txt` or <target_name>.lnk file in script current working directory

# CONFIG FILE
## Grabing predefined settings from configuration file
if test -f pkg.cfg; then
	source pkg.cfg
	echo "config file found, setting following variables:"
	grep -v -e '^#' -e '""' pkg.cfg
	if test "${VER}" != "${PKGVER}" ; then
		echo -e "GM2X PACKAGER version ${VER} doesn't match CONFIGURATION FILE version ${PKGVER}\n\n\tPlease update your pkg.cfg config"
		sleep 2
		exit
	fi
else
	echo "no config pkg.cfg file found, executing with predefined values from env or script"
	sleep 1
fi

# EXEC commands
PACKAGE=${PACKAGE:=0}
ZIP=${ZIP:=0}
IPK=${IPK:=0}
CLEAN=${CLEAN:=0}

# ENV VAR.
## Specific
if test -z $TARGET; then
	echo "No binary name provided, please set $TARGET in your env with correct execution program name"
	sleep 2
	exit
elif ! test -f "$TARGET"; then
	echo "No binary found matching name \"${TARGET}\", exiting..."
	sleep 2
	exit
fi
if test -z $VERSION; then
	VERSION=$(date +%Y-%m-%d\ %H:%M)
	echo "no release Version provided, setting it to curret time ${VERSION}"
fi

## Generic
HOMEPATH=${HOMEPATH:="/mnt"}
RELEASEDIR=${RELEASEDIR:=package}
ASSETSDIR=${ASSETSDIR:=assets}
OPKG_ASSETSDIR=${OPKG_ASSETSDIR:=opkg_assets}
LINK=${LINK:=$TARGET.lnk}
ALIASES=${ALIASES:=aliases.txt}
MANUAL=${MANUAL:=$TARGET.man.txt}

## Link entries
TITLE=${TITLE:="$TARGET"}
DESCRI=${DESCRI:="${TARGET} app"}
SELDIR=${SELDIR:=""}
if test -z $DESTDIR; then
	DESTDIR=apps
	echo "no destination directory provided, setting path to default /mnt/${DESTDIR}"
fi
if test -z $SECTION; then
	SECTION=applications
	echo "no gmenu2x section provided, setting default \"${SECTION}\" in use"
fi
if test -f "${TARGET}.lnk"; then
	# source ${TARGET}.lnk
	echo "gmenu2x link file found, setting following link entries:"
	grep -v '^#' ${TARGET}.lnk
else
	echo "no link file found, executing with predefined values:"
	echo -e "title=$TITLE\ndescription=$DESCRI\nselectordir=$SELDIR"
fi

## IPK control entries
PRIORITY=${PRIORITY:=optional}
MAINTAINER=${MAINTAINER:=Unknown}
CONFFILES=${CONFFILES:=""} # TODO (to preserve & not reinstall user configs)
ARCH=${ARCH:=arm}

### automate output of CONTROL file
CONTROL="Package: ${TARGET}\n\
Version: ${VERSION}\n\
Description: ${DESCRI}\n\
Section: ${SECTION}\n\
Priority: ${PRIORITY}\n\
Maintainer: ${MAINTAINER}\n\
Architecture: ${ARCH}"
#---------------------------------------------#
# CODE execution

echo -e "Using following configuration:\n\
PACKAGE=${PACKAGE}\nZIP=${ZIP}\nIPK=${IPK}\nCLEAN=${CLEAN}\n\
TARGET=${TARGET}\nVERSION=${VERSION}\n\
HOMEPATH=${HOMEPATH}\nRELEASEDIR=${RELEASEDIR}\nASSETSDIR=${ASSETSDIR}\nOPKG_ASSETSDIR=${OPKG_ASSETSDIR}\nLINK=${LINK}\nALIASES=${ALIASES}\nMANUAL=${MANUAL}\n\
TITLE=${TITLE}\nDESCRI=${DESCRI}\nSELDIR=${SELDIR}\nDESTDIR=${DESTDIR}\nSECTION=${SECTION}\n\
PRIORITY=${PRIORITY}\nMAINTAINER=${MAINTAINER}\nCONFFILES=${CONFFILES}\nARCH=${ARCH}"

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

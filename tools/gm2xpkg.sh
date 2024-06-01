#!/bin/bash

VER=0.3
MIYOOCFW_VER=2.0.0
# Help & About info
help_func() {
	echo -e "Usage: gm2xpkg [OPTION] [FILE]"
	echo -e "   or: gm2xpkg"
	echo -e "GMenu2X packager v${VER} tool to generate working release for your binary in CWD with configuration FILE.\n(aimed at MiyooCFW-${MIYOOCFW_VER} currently)"
	echo -e "With no FILE provided, use \"pkg.cfg\" in CWD.\n
	 Options:
	 \t -h, --help      print this help screen
	 \t -V, --version   print gm2xpkg version
	 \t -i, --ipk       generate IPK package
	 \t -z, --zip       generate ZIP archive
	 \t -p, --pkg       generate ./package
	 Instructions:
	 \t 1. Put inside CWD:
	 \t\t- ./<target_name> binary
	 \t\t- ./assets/ dir with all necessary files which goes in same place where binary goes
	 \t\t- ./opkg_assets/ dir with custom IPK's control files (these are auto-generated if missing).
	 \t 2. Edit settings in ./pkg.cfg file
	 \t 3. Run program:
	 \t\t$: ./gm2xpkg.sh
	 \t --or-- 
	 \t 3. Install & run program from usr space in CWD:
	 \t\t$: install -m 755 gm2xpkg.sh /usr/bin/gm2xpkg
	 \t\t$: gm2xpkg
	 Notes:
	 \t CWD  - Current Working Directory
	 \t FILE - configuration with formula from gh repo file: \"MiyooCFW/gmenu2x/tools/pkg.cfg\""
}

# ARGS
## Sanity test if there was any argument passed:
test $# -ne 0 &&\
 PKGCFG="${!#}" || PKGCFG="pkg.cfg" # last argument used of [FILE] or use default ./pkg.cfg placement

# OPTIONS
## TODO: use getopts

while :
do
	case $1 in
		-h | --help | -\?)
			help_func
			exit 0
			;;
		-V | --ver | --version)
			echo -e "GM2X PACKAGER version ${VER} for MiyooCFW ${MIYOOCFW}"
			shift
			;;
		-i | --ipk)
			IPK_OPT="1"
			echo -e "generating IPK package"
			shift
			;;
		-z | --zip)
			ZIP_OPT="1"
			echo -e "generating ZIP archive"
			shift
			;;
		-p | --pkg)
			PACKAGE_OPT="1"
			echo -e "generating ./package"
			shift
			;;
		-c | --clean)
			CLEAN_OPT="1"
			echo -e "cleaning all PACKAGES"
			shift
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
if test -f "${PKGCFG}"; then
	source "${PKGCFG}"
	echo "config file found in $(realpath ${PKGCFG}), setting following variables:"
	grep -v -e '^#' -e '""' "${PKGCFG}"
	if test "${VER}" != "${PKGVER}" ; then
		echo -e "GM2X PACKAGER version ${VER} doesn't match CONFIGURATION FILE version ${PKGVER}\n\n\tPlease update your ${PKGCFG} config file"
		sleep 2
		exit
	fi
else
	echo "no config pkg.cfg file found, executing with predefined values from env or script"
	sleep 1
fi

# OPTIONS
PACKAGE=${PACKAGE:=${PACKAGE_OPT}}
ZIP=${ZIP:=${ZIP_OPT}}
IPK=${IPK:=${IPK_OPT}}
CLEAN=${CLEAN:=${CLEAN_OPT}}

# EXEC commands
PACKAGE=${PACKAGE:=0}
ZIP=${ZIP:=0}
IPK=${IPK:=0}
CLEAN=${CLEAN:=0}

# ENV VAR.
## Specific
if test -z $TARGET; then
	echo "No binary name provided, please set \$TARGET in your env with correct execution program name"
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
if test -z $TARGET_DIR; then
	TARGET_DIR=${TARGET}
	echo "no target directory provided, setting path to default /${HOMEPATH}/${DESTDIR}/${TARGET_DIR}"
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
TITLE=${TITLE}\nDESCRI=${DESCRI}\nSELDIR=${SELDIR}\nDESTDIR=${DESTDIR}\nTARGET_DIR=${TARGET_DIR}\nSECTION=${SECTION}\n\
PRIORITY=${PRIORITY}\nMAINTAINER=${MAINTAINER}\nCONFFILES=${CONFFILES}\nARCH=${ARCH}"

if ! test -d $ASSETSDIR; then
	echo "No assets directory found matching name \"${ASSETSDIR}/\", exiting..."
	sleep 2
	exit
fi

if test $PACKAGE -ne 0 >/dev/null 2>&1 || test $ZIP -ne 0 >/dev/null 2>&1 || test $IPK -ne 0 >/dev/null 2>&1; then
	# Create ./package
	rm -rf $RELEASEDIR
	mkdir -p $RELEASEDIR
	# mkdir -p $ASSETSDIR
	mkdir -p $OPKG_ASSETSDIR
	cp *$TARGET $RELEASEDIR/
	mkdir -p $RELEASEDIR/$DESTDIR/$TARGET_DIR
	mkdir -p $RELEASEDIR/gmenu2x/sections/$SECTION
	mv $RELEASEDIR/*$TARGET $RELEASEDIR/$DESTDIR/$TARGET_DIR/
	cp -r $ASSETSDIR/* $RELEASEDIR/$DESTDIR/$TARGET_DIR
	if !(test -e $LINK); then
		touch $LINK
		echo -e "title=${TITLE}\ndescription=${DESCRI}\nexec=" > $LINK
		sed -i "s/^exec=.*/exec=\/mnt\/${DESTDIR}\/${TARGET_DIR}\/${TARGET}/" $LINK
		test -n "$SELDIR" && echo "selectordir=${SELDIR}" >> $LINK
		if test -e $ALIASES; then
			echo "selectoraliases=\/mnt\/${DESTDIR}\/${TARGET_DIR}\/${ALIASES}" >> $LINK
		fi
	fi
	cp $LINK $RELEASEDIR/gmenu2x/sections/$SECTION
	cp $ALIASES $RELEASEDIR/$DESTDIR/$TARGET_DIR
	cp $MANUAL $RELEASEDIR/$DESTDIR/$TARGET_DIR/${TARGET}.man.txt
	test -d $RELEASEDIR/gmenu2x && test -d $RELEASEDIR/$DESTDIR/$TARGET_DIR\
	 && (test $PACKAGE -ne 0 && echo "Done packaging ./$RELEASEDIR/ data" || echo "Ready to use ./$RELEASEDIR/ data for deaper packaging")\
	 || echo "Upss smth went wrong and I couldn't locate auto-gen data in ./$RELEASEDIR/" 
	
	# Create ./package/<target_version>.zip
	if test $ZIP -ne 0 >/dev/null 2>&1; then
		# rm -rf $RELEASEDIR/*.ipk $RELEASEDIR/*.zip
		cd $RELEASEDIR && zip -rq $TARGET$VERSION.zip ./* && mv *.zip ..
		cd ..
		test -f "${TARGET}${VERSION}.zip"\
		 && echo "Done packaging ./${TARGET}${VERSION}.zip archive"\
		 || echo "Upss smth went wrong and I couldn't locate ${TARGET}${VERSION}.zip"
	fi
	
	# Create ./package/<target>.ipk
	if test $IPK -ne 0 >/dev/null 2>&1; then
		# rm -rf $RELEASEDIR/*.ipk $RELEASEDIR/*.zip
		mkdir -p .$HOMEPATH
		cp -r $RELEASEDIR/* .$HOMEPATH && mv .$HOMEPATH $RELEASEDIR/
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
		ar r $TARGET.ipk $RELEASEDIR/control.tar.gz $RELEASEDIR/data.tar.gz $RELEASEDIR/debian-binary\
		 && echo "Done creating ./${TARGET}.ipk package"\
		 && rm $RELEASEDIR/control.tar.gz $RELEASEDIR/data.tar.gz $RELEASEDIR/debian-binary && rm -r $RELEASEDIR/CONTROL/ $RELEASEDIR/data/
		# mv $TARGET.ipk $RELEASEDIR/
	fi
	if test $PACKAGE -eq 0 >/dev/null 2>&1; then rm -rf ${RELEASEDIR:?}/*; fi
elif test $CLEAN -ne 0 >/dev/null 2>&1; then
	rm -rf ${RELEASEDIR:?}
	rm -rf ${OPKG_ASSETSDIR:?}
	rm -f $TARGET.ipk
	rm -f $TARGET*.zip
	rm -f $LINK
else
	echo "No instructions provided, please set \$PACKAGE/\$ZIP/\$IPK or \$CLEAN in env to 1 for correct output"
	sleep 1
fi
#---------------------------------------------#

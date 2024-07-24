#!/bin/bash

VER=0.6
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
	 \t -c, --clean     remove ./package ./opkg_assets ./<target_name>.ipk ./<target_name>.zip ./<link_name>lnk
	 \t -g, --gencfg    generate standard config \"pkg.cfg\" file in CWD
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

pkg_config_func() {
	if test "x${UPDATE_PKGCFG}" == "xyes"; then
		source "${PKGCFG}"
	fi
	echo "# NOTES:
## All variable values should be enclosed within double quotes: \"<value>\"
## CONFIGURATION FILE for \`gm2xpkg\` script version:
PKGVER=\"${VER}\"

# EXEC commands (set to \"1\" anyone for desired outcome), you can instead use [OPTIONS] of \`gm2xpkg\`:
PACKAGE=\"${PACKAGE}\"
ZIP=\"${ZIP}\"
IPK=\"${IPK}\"
CLEAN=\"${CLEAN}\"

# DEBUG session (insert \"yes\" to perform)
DEBUG=\"${DEBUG}\"

# ENV VAR.
## Specific (mandatory to provide!)
TARGET=\"${TARGET}\"  # replace with binary name
VERSION=\"${VERSION}\"  # replace with correct release version if exist

## Generic - common to all apps (better to not modify)
HOMEPATH=\"${HOMEPATH}\"
RELEASEDIR=\"${RELEASEDIR}\"
ASSETSDIR=\"${ASSETSDIR}\"
OPKG_ASSETSDIR=\"${OPKG_ASSETSDIR}\"
LINK=\"${LINK}\" # full name of gm2x link, modify if exec binary name may be different from target name - place in CWD
ALIASES=\"${ALIASES}\" # full name (with ext) of *.txt file with new names for selector e.g. old_title=new_title - place in CWD
MANUAL=\"${MANUAL}\" # full name (with ext) of *.man.txt file with usage description of target app - place in CWD

## Link entries (better modify if no <target_name>.lnk file provided)
TITLE=\"${TITLE}\"
DESCRI=\"${DESCRI}\"
SELDIR=\"${SELDIR}\"
DESTDIR=\"${DESTDIR}\" # default=apps
SECTION=\"${SECTION}\" # default=applications

## Custom entries (if needed then modify)
TARGET_EXEC=\"${TARGET_EXEC}\" # the executable name that's being used by frontend when running an app, may be a script name or binary (default)
TARGET_DIR=\"${TARGET_DIR}\" # the install directory /\$HOMEPATH/\$DESTDIR/\$TARGET_DIR of executable binary if not provided the TARGET_DIR=\$TARGET
DOCS=($(for i in "${!DOCS[@]}"; do test "${i}" != "0" && SPACE=" "; echo -n "${SPACE}\"${DOCS[$i]}\""; done))\
 # array of extra text files e.g. =(\"LICENSE\" \"CHANGELOG\" \"CONTRIBUTORS\") which will be copied & converted to *.txt files for ease of use by frontend

## IPK control entries (if needed then modify)
PRIORITY=\"${PRIORITY}\"
MAINTAINER=\"${MAINTAINER}\"
CONFFILES=\"${CONFFILES}\"
ARCH=\"${ARCH}\" # default=arm - do not modify for ARM chips
# CONTROL= # automated output of *.ipk control config file
DEPENDS=\"${DEPENDS}\" # list of dependency packages e.g. =\"sdl, libpng\" or =\"sdl (>= 2.9.2), sdl_mixer (= ed76d39cda0735d26c14a3e4f4da996e420f6478)\" provide only for shared libs build, otherwise ignored
SOURCE=\"${SOURCE}\"
LICENSE=\"${LICENSE}\"\
" > "${PKGCFG}"
}

# DEBUG options
if test "x${DEBUG}" == "xyes"; then
	set -e
	# set -xuv
	trap 'echo "Error on line $LINENO"; sleep 1; exit' ERR
	trap 'echo "$LINENO: $BASH_COMMAND"' DEBUG
fi

# ARGS
## Sanity test if there was any argument passed:
test $# -ne 0 &&\
 PKGCFG="${!#}" || PKGCFG="pkg.cfg" # last argument used of [FILE] or use default ./pkg.cfg placement

case "${PKGCFG}" in
	-h|--help|-V|--ver|--version|-i|--ipk|-z|--zip|-p|--pkg|-c|--clean|-g|--gencfg)
		PKGCFG="pkg.cfg"
		;;
	*)
		;;
esac

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
			exit 0
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
		-g | --gencfg)
			echo "using ./${PKGCFG} name as config file"
			if ! test -e "${PKGCFG}"; then
				pkg_config_func
				echo -e "generating standard \"${PKGCFG}\" config file"
			else 
				echo -e "Detected present ${PKGCFG} file. Do you wish to overwrite existing configuration?"
				read -rp "[Y]es, [N]o, [U]pdate:" INPUT
				case "${INPUT}" in
					[Yy]*)
						echo "YES, overwriting existing ${PKGCFG} file"
						pkg_config_func
						;;
					[Nn]*)
						echo "NO, exiting..."
						;;
					[Uu]*)
						echo "Update, upgrading present ${PKGCFG} file with new config version - ${VER}"
						UPDATE_PKGCFG=yes
						pkg_config_func
						;;
					*)
						echo "Invalid choice, please try again"
						;;
				esac
			fi
			sleep 1
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
## Optionally put `Aliases.txt`, `<target_name>.man.txt` or <target_name>.lnk file in script's current working directory

# CONFIG FILE
## Grabing predefined settings from configuration file
if test -f "${PKGCFG}"; then
	source "${PKGCFG}"
	echo "config file found in $(realpath ${PKGCFG}), setting predefined variables..."
	if test "x${DEBUG}" == "xyes"; then
		echo "Following variables has been read from ${PKGCFG} file:"
		grep -v -e '^#' -e '""' "${PKGCFG}"
	fi
	if test "${VER}" != "${PKGVER}" ; then
		echo -e "GM2X PACKAGER version ${VER} doesn't match CONFIGURATION FILE version ${PKGVER}\n\n\tPlease update your ${PKGCFG} config file, use [--gencfg] option"
		sleep 2
		exit
	fi
else
	echo "no config \"${PKGCFG}\" file found, executing with predefined values from env or script"
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
	echo "No binary/script found matching name \"${TARGET}\", exiting..."
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
	echo "no destination directory provided, setting path to default ${HOMEPATH}/${DESTDIR}"
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

## Custom entries
if test -z $TARGET_DIR; then
	TARGET_DIR=${TARGET}
	echo "no target directory provided, setting target install path to default ${HOMEPATH}/${DESTDIR}/${TARGET_DIR}"
fi
if test ${#DOCS[@]} -eq 0 || test -z "${DOCS[*]}"; then
	DOCS=("")
	echo "INFO: Hmm... I suggest you add some documention via \$DOCS[] variable"
fi

## IPK control entries
PRIORITY=${PRIORITY:=optional}
MAINTAINER=${MAINTAINER:=Unknown}
CONFFILES=${CONFFILES:=""} # TODO (to preserve & not reinstall user configs)
ARCH=${ARCH:=arm}
DEPENDS=${DEPENDS:=""}
SOURCE=${SOURCE:="Unknown"}
LICENSE=${LICENSE:="Unknown"}

#---------------------------------------------#
# CODE execution

LIBS_LD="$(file ${TARGET} | sed -E 's/.* ([^ ]+) linked.*/\1/')"
if test "${LIBS_LD}" == "dynamically"; then
	LIBC=$(file ${TARGET} | sed -n 's/.*ld-\([a-zA-Z]*\).*/\1/p' | tr '[:upper:]' '[:lower:]')
	! test -z "${DEPENDS}" && DEPENDS="${LIBC}, ${DEPENDS}" || DEPENDS="${LIBC}"
	echo "Target binary \"${TARGET}\" is ${LIBS_LD} linked with ${LIBC} libc implementation"
	test "${LIBC}" == "uclibc" || test "${LIBC}" == "musl"\
	 || bash -c "echo "ERROR:\ The\ \"${LIBC}\"\ is\ invalid\ libs\ interpreter" && sleep 2 && exit 1"
elif test "${LIBS_LD}" == "statically"; then
	DEPENDS=""
	echo "Target binary \"${TARGET}\" is ${LIBS_LD} linked with no need for externall dependencies"
else
	echo "WARNING: Probably not a binary file (or linking problem), if it's a script pls provide correct interpreter as dependency"
	sleep 1
fi

CONTROL="Package: ${TARGET}\n\
Version: ${VERSION}\n\
Depends: ${DEPENDS}\n\
Source: ${SOURCE}\n\
License: ${LICENSE}\n\
Description: ${DESCRI}\n\
Section: ${SECTION}\n\
Priority: ${PRIORITY}\n\
Maintainer: ${MAINTAINER}\n\
Architecture: ${ARCH}"

echo -e "Using following configuration:
PACKAGE=${PACKAGE}\nZIP=${ZIP}\nIPK=${IPK}\nCLEAN=${CLEAN}\n
TARGET=${TARGET}\nVERSION=${VERSION}\n
HOMEPATH=${HOMEPATH}\nRELEASEDIR=${RELEASEDIR}\nASSETSDIR=${ASSETSDIR}\nOPKG_ASSETSDIR=${OPKG_ASSETSDIR}\nLINK=${LINK}\nALIASES=${ALIASES}\nMANUAL=${MANUAL}\n
TITLE=${TITLE}\nDESCRI=${DESCRI}\nSELDIR=${SELDIR}\nDESTDIR=${DESTDIR}\nSECTION=${SECTION}\n
TARGET_EXEC=${TARGET_EXEC}\nTARGET_DIR=${TARGET_DIR}\nDOCS=(${DOCS[*]})\n
PRIORITY=${PRIORITY}\nMAINTAINER=${MAINTAINER}\nCONFFILES=${CONFFILES}\nARCH=${ARCH}\nDEPENDS=${DEPENDS}\nSOURCE=${SOURCE}\nLICENSE=${LICENSE}
"

if test $PACKAGE -ne 0 >/dev/null 2>&1 || test $ZIP -ne 0 >/dev/null 2>&1 || test $IPK -ne 0 >/dev/null 2>&1; then
	TARGET_PATH=$RELEASEDIR/$DESTDIR/$TARGET_DIR
	# Create ./package
	rm -rf $RELEASEDIR
	mkdir -p $RELEASEDIR
	# mkdir -p $ASSETSDIR
	mkdir -p $OPKG_ASSETSDIR
	cp *$TARGET $RELEASEDIR/
	mkdir -p $TARGET_PATH
	mkdir -p $RELEASEDIR/gmenu2x/sections/$SECTION
	mv $RELEASEDIR/*$TARGET $TARGET_PATH/
	test -d $ASSETSDIR\
	 && cp -r $ASSETSDIR/* $TARGET_PATH\
	 || echo "WARNING: No assets directory found matching name \"${ASSETSDIR}/\""
	test -z "${TARGET_EXEC}"\
	 && TARGET_EXEC="${TARGET}"\
	 || echo "Custom executable \"${TARGET_EXEC}\" is being used by frontend's links launcher"
	if ! (test -e $LINK); then
		touch $LINK
		echo -e "title=${TITLE}\ndescription=${DESCRI}\nexec=" > $LINK
		sed -i "s/^exec=.*/exec=\/mnt\/${DESTDIR}\/${TARGET_DIR}\/${TARGET_EXEC}/" $LINK
		test -n "$SELDIR" && echo "selectordir=${SELDIR}" >> $LINK
		if test -e $ALIASES; then
			echo "selectoraliases=\/mnt\/${DESTDIR}\/${TARGET_DIR}\/${ALIASES}" >> $LINK
		fi
	else
		LINK_CUSTOM="yes"
	fi
	cp $LINK $RELEASEDIR/gmenu2x/sections/$SECTION
	if test -e $ALIASES; then
		cp $ALIASES $TARGET_PATH
	else
		echo "WARNING: Couldn't locate aliases in ${ALIASES} file"
	fi
	if test -e $MANUAL; then
		if file $MANUAL | grep -q "PNG image"; then
			MANUAL_EXT=".man.png"
		elif file $MANUAL | grep -q "ASCII text"; then
			MANUAL_EXT=".man.txt"
		else
			MANUAL_EXT=""
			echo "WARNING: Unsupported format of manual in ${MANUAL} file. Use PNG image or plain text file"
		fi
		! test -z "${MANUAL_EXT}"\
		 && cp $MANUAL $TARGET_PATH/${TARGET}${MANUAL_EXT}
	else
		echo "WARNING: Couldn't locate manual in ${MANUAL} file"
	fi
	! test -z "${DOCS[*]}"\
	 && for i in "${!DOCS[@]}"; do cp "${DOCS[$i]}" "${TARGET_PATH}/" && mv "${TARGET_PATH}"/"${DOCS[$i]##*/}" "${TARGET_PATH}"/"${DOCS[$i]##*/}.txt"; done\
	 || echo "WARNING: Upss smth went wrong and I couldn't read text ${DOCS[*]} files"
	test -d $RELEASEDIR/gmenu2x && test -d $TARGET_PATH\
	 && (test $PACKAGE -ne 0 && echo "Done packaging ./$RELEASEDIR/ data" || echo "Ready to use ./$RELEASEDIR/ data for deaper packaging")\
	 || echo "WARNING: Upss smth went wrong and I couldn't locate auto-gen data in ./$RELEASEDIR/"
	
	# Create ./package/<target_version>.zip
	if test $ZIP -ne 0 >/dev/null 2>&1; then
		# rm -rf $RELEASEDIR/*.ipk $RELEASEDIR/*.zip
		cd $RELEASEDIR && zip -rq ${TARGET}_${VERSION}.zip ./* && mv *.zip ..
		cd ..
		test -f "${TARGET}_${VERSION}.zip"\
		 && echo "Done packaging ./${TARGET}_${VERSION}.zip archive"\
		 || echo "WARNING: Upss smth went wrong and I couldn't locate ${TARGET}_${VERSION}.zip"
	fi
	
	# Create ./package/<target>.ipk
	if test $IPK -ne 0 >/dev/null 2>&1; then
		# rm -rf $RELEASEDIR/*.ipk $RELEASEDIR/*.zip
		mkdir -p .$HOMEPATH
		cp -r $RELEASEDIR/* .$HOMEPATH && mv .$HOMEPATH $RELEASEDIR/
		mkdir -p $RELEASEDIR/data
		mv $RELEASEDIR$HOMEPATH $RELEASEDIR/data/
		if ! { test -d ${OPKG_ASSETSDIR}/CONTROL && test -f ${OPKG_ASSETSDIR}/CONTROL/preinst && test -f ${OPKG_ASSETSDIR}/CONTROL/postinst; }; then
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
	rm -rf ${RELEASEDIR:?} && echo "Done CLEANING release dir ./${RELEASEDIR}" || echo "WARNING: Couldn't clean release dir ./${RELEASEDIR}"
	rm -rf ${OPKG_ASSETSDIR:?} && echo "Done CLEANING opkg assets dir ./${OPKG_ASSETSDIR}" || echo "WARNING: Couldn't clean opkg assets dir ./${OPKG_ASSETSDIR}"
	rm -f $TARGET.ipk && echo "Done CLEANING ./${TARGET}.ipk" || echo "WARNING: Couldn't clean ./${TARGET}.ipk"
	rm -f $TARGET*.zip && echo "Done CLEANING ./${TARGET}.zip" || echo "WARNING: Couldn't clean ./${TARGET}.zip"
	if ! test "x${LINK_CUSTOM}" == "xyes"; then
		rm -f $LINK && echo "Done CLEANING link ./${LINK}" || echo "WARNING: Couldn't clean ./${LINK}"
	fi
else
	echo "\nWARNING: No instructions provided, please use -i/-p/-z/-c option or set \$PACKAGE/\$ZIP/\$IPK/\$CLEAN in env to 1 for correct output\n\n"
	sleep 1
fi
#---------------------------------------------#

#!/bin/bash

VER=0.7
MIYOOCFW_VER=2.0.0
# Help & About info
help_func() {
	echo -e "Usage: gm2xpkg [OPTION] [FILE]"
	echo -e "   or: gm2xpkg"
	echo -e "GMenu2X packager v${VER} tool to generate working release for your binary in PWD with configuration FILE.\n(aimed at MiyooCFW-${MIYOOCFW_VER} currently)"
	echo -e "With no FILE provided, use \"pkg.cfg\" in PWD.\n
	 Options:
	 \t -h, --help      print this help screen
	 \t -v, --verbose   output more process info
	 \t -V, --version   print gm2xpkg version
	 \t -i, --ipk       generate IPK package
	 \t -z, --zip       generate ZIP archive
	 \t -p, --pkg       generate ./package
	 \t -c, --clean     remove ./package ./opkg_assets ./<target_name>.ipk ./<target_name>.zip ./<link_name>lnk
	 \t -g, --gencfg    generate standard config \"pkg.cfg\" file in PWD
	 Instructions:
	 \t 1. Put inside PWD:
	 \t\t- ./<target_name> binary
	 \t\t- ./assets/ dir with all necessary files which goes in same place where binary goes
	 \t\t- ./opkg_assets/ dir with custom IPK's control files (these are auto-generated if missing).
	 \t 2. Edit settings in ./pkg.cfg file
	 \t 3. Run program:
	 \t\t$: ./gm2xpkg.sh
	 \t --or-- 
	 \t 3. Install & run program from usr space in PWD:
	 \t\t$: install -m 755 gm2xpkg.sh /usr/bin/gm2xpkg
	 \t\t$: gm2xpkg
	 Notes:
	 \t PWD  - Present Working Directory PATH
	 \t FILE - path to configuration with formula from gh repo file: \"MiyooCFW/gmenu2x/tools/pkg.cfg\""
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

# VERBOSE mode (insert \"yes\" to have more info)
VERBOSE=\"${VERBOSE}\"

# ENV VAR.
## Specific (mandatory to provide!)
TARGET=\"${TARGET}\"  # [filepath], replace with target's working binary path (<path>/<file_name>)
VERSION=\"${VERSION}\"  # [string], replace with correct release version if exist of target binary

## Generic - common to all apps (better to not modify)
HOMEPATH=\"${HOMEPATH}\" # [dirpath], target device fullpath home directory for installation process
RELEASEDIR=\"${RELEASEDIR}\" # [dirpath], host package output directory, specified with [-p] option
ASSETSDIR=\"${ASSETSDIR}\" # [dirpath], host dir containg all the necessary assets for a target
OPKG_ASSETSDIR=\"${OPKG_ASSETSDIR}\" # [dirpath], host dir containg the ./CONTROL directory with [control, preinst, postinst] files, auto-generated if not provided (warning: it may be removed with CLEAN=1)
LINK=\"${LINK}\" # [filepath], host path to custom gm2x link, modify if you want to use your pre-edited *.lnk file (warning: it may be removed with CLEAN=1)
ALIASES=\"${ALIASES}\" # [filepath], host path to *.txt file holding new names for selector e.g. old_title=new_title
MANUAL=\"${MANUAL}\" # [filepath], host path to *.man.txt file holding usage description of target app

## Link entries (better modify if no <target_name>.lnk file provided)
### Primary
TITLE=\"${TITLE}\" # [string], program title
DESCRI=\"${DESCRI}\" # [string], short description
DESTDIR=\"${DESTDIR}\" # [string], (default=\"apps\") installation pathname in target device \$HOMEPATH directory - not a link entry
SECTION=\"${SECTION}\" # [string], (default=\"applications\") section in menu
### Additional
SELDIR=\"${SELDIR}\" # [dirpath], target device fullpath search directory (activates selector, don't append path with \"/\" to use AUTO selectorelement mode)
SELBROWSER=\"${SELBROWSER}\" # [bool], (default=\"true\") don't show directories in selector browser with \"false\" - aka \"Show Folders\" option
SELFILTER=\"${SELFILTER}\" # [string], activates FileFilter in selector e.g. =\".gba,.zip\"
SELSCREENS=\"${SELSCREENS}\" # [dirpath],  target fullpath Boxarts' directory in selector
ICON=\"${ICON}\" # [filepath], target fullpath to icon being used in menu (instead of default)
BACKDROP=\"${BACKDROP}\" # [filepath], target fullpath to backdrop being displayed under icon in menu (default=\"\" thus OFF)
PARAMS=\"${PARAMS}\" # [string], parameters (options; args) being passed to execution cmd
### HW Specific
CLOCK=\"${CLOCK}\" # [int], CPU frequency in MHz
LAYOUT=\"${LAYOUT}\" # [int], SDL Keyboard (face buttons) layout
TEFIX=\"${TEFIX}\" # [int], Tearing FIX method

## Custom entries (if needed then modify)
TARGET_DIR=\"${TARGET_DIR}\" # [dirpath], target device install directory \$HOMEPATH/\$DESTDIR/\$TARGET_DIR of executable binary (default TARGET_DIR=\$(basename \$TARGET))
TARGET_EXEC=\"${TARGET_EXEC}\" # [string], the executable <file_name> that's being used by frontend when running an app from \$TARGET_DIR, for e.g. may be a custom script (default TARGET_EXEC=\$(basename \$TARGET))
DOCS=($(for i in "${!DOCS[@]}"; do test "${i}" != "0" && SPACE=" "; echo -n "${SPACE}\"${DOCS[$i]}\""; done))\
 # [array] of filepaths to extra text files e.g. =(\"docs/LICENSE\" \"CHANGELOG\" \"CONTRIBUTORS\") which will be copied & converted to *.txt files for ease of use by frontend and placed in \$TARGET_DIR

## IPK control entries (if needed then modify)
PKG=\"${PKG}\" # default=\$TARGET - name of the opkg package
PRIORITY=\"${PRIORITY}\"
MAINTAINER=\"${MAINTAINER}\"
CONFFILES=\"${CONFFILES}\"
ARCH=\"${ARCH}\" # default=arm - do not modify for ARM chips
# CONTROL= # automated output of *.ipk control config file
DEPENDS=\"${DEPENDS}\" # list of dependency packages e.g. =\"sdl, libpng\" or =\"sdl (>= 2.9.2), sdl_mixer (= ed76d39cda0735d26c14a3e4f4da996e420f6478)\" provide only for shared libs build, otherwise ignored (run \"readelf -d \$TARGET | grep NEEDED\" to bisect)
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
	-h|--help|-V|--ver|--version|-v|--verbose|-i|--ipk|-z|--zip|-p|--pkg|-c|--clean|-g|--gencfg)
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
		-v | --verbose)
			VERBOSE_OPT="yes"
			VERBOSE="${VERBOSE_OPT}"
			echo -e "running in VERBOSE mode"
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
	echo "config file found in $(realpath ${PKGCFG}), setting predefined variables..."
	if test "x${VERBOSE}" == "xyes"; then
		echo "INFO: Following variables will be read from ${PKGCFG} file:"
		echo "<<<${PKGCFG##*/}>>>"
		grep -v -e '^#' -e '""' "${PKGCFG}"
		echo "<<<EOF>>>"
	fi
	source "${PKGCFG}"
	if test "${VER}" != "${PKGVER}" ; then
		echo -e "ERROR: GM2X PACKAGER version ${VER} doesn't match CONFIGURATION FILE version ${PKGVER}\n\n\tPlease update your ${PKGCFG} config file, use [--gencfg] option"
		sleep 2
		exit 1
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
VERBOSE=${VERBOSE:=${VERBOSE_OPT}}

# EXEC commands
PACKAGE=${PACKAGE:=0}
ZIP=${ZIP:=0}
IPK=${IPK:=0}
CLEAN=${CLEAN:=0}

# ENV VAR.
## Specific
TARGET=${TARGET:=""}
VERSION=${VERSION:=""}
if test -z $TARGET; then
	echo "ERROR: No binary PATH/<filename> provided, please set \$TARGET in your env with correct execution program name"
	sleep 2
	exit 1
elif ! test -f "$TARGET"; then
	echo "ERROR: No binary/script found matching \"${TARGET}\", exiting..."
	sleep 2
	exit 1
else
	TARGET_PATH_DIR="${TARGET%/*}"
	TARGET_PATH="${TARGET}"
	TARGET="${TARGET##*/}"
fi
if test -z $VERSION; then
	VERSION=$(date +%Y-%m-%d\_%H:%M)
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
DESTDIR=${DESTDIR:=""}
SECTION=${SECTION:=""}
if test -z $DESTDIR; then
	DESTDIR=apps
	echo "no destination directory provided, setting path to default ${HOMEPATH}/${DESTDIR}/"
else
	DESTDIR=${DESTDIR##*/}
fi
if test -z $SECTION; then
	SECTION=applications
	echo "no gmenu2x section provided, setting default \"${SECTION}\" in use"
fi

SELBROWSER=${SELBROWSER:="true"}
SELFILTER=${SELFILTER:=""}
SELSCREENS=${SELSCREENS:=""}
ICON=${ICON:=""}
BACKDROP=${BACKDROP:=""}
PARAMS=${PARAMS:=""}

if test -z $SELDIR || test "$SELBROWSER" == "true"; then
	SELBROWSER=""
elif ! test -z $SELDIR &&  test "$SELBROWSER" == "false"; then
	SELBROWSER=""
	echo "WARNING: you can't set selectorbrowser value for \"Show Folders\" while selectordirectory is empty"
fi

CLOCK=${CLOCK:=""}
LAYOUT=${LAYOUT:=""}
TEFIX=${TEFIX:=""}

if test -f "${LINK}"; then
	# source ${TARGET}.lnk
	echo "gmenu2x link file found in $(realpath ${LINK}), setting following link entries:"
	echo "<<<${LINK##*/}>>>"
	grep -v '^#' ${LINK}
	echo "<<<EOF>>>"
else
	echo -e "no link file found, executing with predefined values..."
	if test "x${VERBOSE}" == "xyes"; then
		echo -e "INFO: Following gmenu parameters has been set from predefined values:"
		echo -e "title=$TITLE\ndescription=$DESCRI\nicon=$ICON\nexec=<auto_generated>\nparams=$PARAMS"
		echo -e "title=$TITLE\ndescription=$DESCRI\nicon=$ICON\nexec=<auto_generated>\nparams=$PARAMS"
		echo -e "manual=<auto_generated>\nclock=$CLOCK\nlayout=$LAYOUT\ntefix=$TEFIX\nselectordir=$SELDIR"
		echo -e "selectorbrowser=$SELBROWSER\nselectorfilter=$SELFILTER\nselectorscreens=$SELSCREENS"
		echo -e "selectoraliases=<auto_generated>\nbackdrop=$BACKDROP\n"
	fi
fi

## Custom entries
if test -z $TARGET_DIR; then
	TARGET_DIR="$(basename ${TARGET})"
	echo "no target install directory provided, setting target install path to default ${HOMEPATH}/${DESTDIR}/${TARGET_DIR}/"
fi
if test -z "${TARGET_EXEC}"; then
	TARGET_EXEC="${TARGET##*/}"
else
	TARGET_EXEC="${TARGET_EXEC##*/}"
	echo "Custom executable \"${TARGET_EXEC}\" is being used by frontend's links launcher"
fi
if test ${#DOCS[@]} -eq 0 || test -z "${DOCS[*]}"; then
	DOCS=("")
	echo "WARNING: Hmm... I suggest you add some documention via \$DOCS[] variable"
fi

## IPK control entries
PKG=${PKG:=${TARGET}}
PRIORITY=${PRIORITY:=optional}
MAINTAINER=${MAINTAINER:=Unknown}
CONFFILES=${CONFFILES:=""} # TODO (to preserve & not reinstall user configs)
ARCH=${ARCH:=arm}
DEPENDS=${DEPENDS:=""}
SOURCE=${SOURCE:="Unknown"}
LICENSE=${LICENSE:="Unknown"}

CONTROL="Package: ${PKG}\n\
Version: ${VERSION}\n\
Depends: ${DEPENDS}\n\
Source: ${SOURCE}\n\
License: ${LICENSE}\n\
Description: ${DESCRI}\n\
Section: ${SECTION}\n\
Priority: ${PRIORITY}\n\
Maintainer: ${MAINTAINER}\n\
Architecture: ${ARCH}"

if ! { test -d ${OPKG_ASSETSDIR}/CONTROL && test -f ${OPKG_ASSETSDIR}/CONTROL/preinst && test -f ${OPKG_ASSETSDIR}/CONTROL/postinst && test -f ${OPKG_ASSETSDIR}/CONTROL/control; }; then
	echo -e "no opkg assets dir&files found, executing with predefined values..."
	if test "x${VERBOSE}" == "xyes"; then
		echo -e "INFO: Following opkg <<<control>>> instructions has been set from predefined values (plus basic <<<preinst|postinst>>>):"
		echo -e "${CONTROL}"
	fi
else
	echo "opkg assets dir&files found in \"$(realpath ${OPKG_ASSETSDIR})/CONTROL\", setting following CONTROL entries:"
	echo "<<<control>>>:"
	cat ${OPKG_ASSETSDIR}/CONTROL/control
	echo "<<<EOF>>>"
	echo "<<<preinst>>>:"
	cat ${OPKG_ASSETSDIR}/CONTROL/preinst
	echo "<<<EOF>>>"
	echo "<<<postinst>>>:"
	cat ${OPKG_ASSETSDIR}/CONTROL/postinst
	echo "<<<EOF>>>"
fi

#---------------------------------------------#
# CODE execution
echo -e "\tStarting gm2xpkg..."
LIBS_LD="$(file ${TARGET_PATH} | sed -E 's/.* ([^ ]+) linked.*/\1/')"
if test "${LIBS_LD}" == "dynamically"; then
	LIBC=$(file ${TARGET_PATH} | sed -n 's/.*ld-\([a-zA-Z]*\).*/\1/p' | tr '[:upper:]' '[:lower:]')
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

echo -e "Starting configuration..."
if test "x${VERBOSE}" == "xyes"; then
	echo -e "\nINFO: Using following configuration setup:"
	echo -e "PACKAGE=${PACKAGE}\nZIP=${ZIP}\nIPK=${IPK}\nCLEAN=${CLEAN}\n-"
	echo -e "TARGET=${TARGET}\nVERSION=${VERSION}\n-"
	echo -e "HOMEPATH=${HOMEPATH}\nRELEASEDIR=${RELEASEDIR}\nASSETSDIR=${ASSETSDIR}\nOPKG_ASSETSDIR=${OPKG_ASSETSDIR}\nLINK=${LINK}\nALIASES=${ALIASES}\nMANUAL=${MANUAL}\n-"
	echo -e "TITLE=${TITLE}\nDESCRI=${DESCRI}\nSELDIR=${SELDIR}\nDESTDIR=${DESTDIR}\nSECTION=${SECTION}\n-"
	echo -e "TARGET_EXEC=${TARGET_EXEC}\nTARGET_DIR=${TARGET_DIR}\nDOCS=(${DOCS[*]})\n-"
	echo -e "PRIORITY=${PRIORITY}\nMAINTAINER=${MAINTAINER}\nCONFFILES=${CONFFILES}\nARCH=${ARCH}\nDEPENDS=${DEPENDS}\nSOURCE=${SOURCE}\nLICENSE=${LICENSE}\n"
fi
if test $PACKAGE -eq 1 >/dev/null 2>&1 || test $ZIP -eq 1 >/dev/null 2>&1 || test $IPK -eq 1 >/dev/null 2>&1; then
	TARGET_INSTALL_DIR=$RELEASEDIR/$DESTDIR/$TARGET_DIR
	# Create ./package
	rm -rf $RELEASEDIR
	mkdir -p $RELEASEDIR
	# mkdir -p $ASSETSDIR
	mkdir -p $OPKG_ASSETSDIR
	cp $TARGET_PATH $RELEASEDIR/
	mkdir -p $TARGET_INSTALL_DIR
	mkdir -p $RELEASEDIR/gmenu2x/sections/$SECTION
	mv $RELEASEDIR/*$TARGET $TARGET_INSTALL_DIR/
	test -d $ASSETSDIR\
	 && cp -r $ASSETSDIR/* $TARGET_INSTALL_DIR\
	 || echo "WARNING: No assets directory found matching name \"${ASSETSDIR}/\""
	if ! (test -e $LINK); then
		touch $LINK
		echo -e "title=${TITLE}\ndescription=${DESCRI}\nexec=" > $LINK
		test -n "$ICON"   && echo "icon=${ICON}" >> $LINK
		sed -i "s/^exec=.*/exec=\\${HOMEPATH}\/${DESTDIR}\/${TARGET_DIR}\/${TARGET_EXEC}/" $LINK
		test -n "$PARAMS" && echo "params=${PARAMS}" >> $LINK
		test -n "$CLOCK"  && echo "clock=${CLOCK}" >> $LINK
		test -n "$LAYOUT" && echo "layout=${LAYOUT}" >> $LINK
		test -n "$LAYOUT" && echo "layout=${LAYOUT}" >> $LINK
		test -n "$TEFIX"  && echo "tefix=${TEFIX}" >> $LINK
		test -n "$SELDIR" && echo "selectordir=${SELDIR}" >> $LINK
		test -n "$SELBROWSER" && echo "selectorbrowser=${SELBROWSER}" >> $LINK
		test -n "$SELFILTER" && echo "selectorfilter=${SELFILTER}" >> $LINK
		test -n "$SELSCREENS" && echo "selectorscreens=${SELSCREENS}" >> $LINK
		if test -e $ALIASES; then
			echo "selectoraliases=${HOMEPATH}/${DESTDIR}/${TARGET_DIR}/${ALIASES}" >> $LINK
		fi
		test -n "$BACKDROP" && echo "backdrop=${BACKDROP}" >> $LINK
	else
		LINK_CUSTOM="yes"
	fi
	cp $LINK $RELEASEDIR/gmenu2x/sections/$SECTION/
	if test -e $ALIASES; then
		cp $ALIASES $TARGET_INSTALL_DIR
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
		 && cp $MANUAL $TARGET_INSTALL_DIR/${TARGET}${MANUAL_EXT}
	else
		echo "WARNING: Couldn't locate manual in ${MANUAL} file"
	fi
	! test -z "${DOCS[*]}"\
	 && for i in "${!DOCS[@]}"; do cp "${DOCS[$i]}" "${TARGET_INSTALL_DIR}/" && mv "${TARGET_INSTALL_DIR}"/"${DOCS[$i]##*/}" "${TARGET_INSTALL_DIR}"/"$(basename ${DOCS[$i]%.*}).txt"; done\
	 || echo "WARNING: Upss smth went wrong and I couldn't read text ${DOCS[*]} files"
	test -d $RELEASEDIR/gmenu2x && test -d $TARGET_INSTALL_DIR\
	 && (test $PACKAGE -eq 1 && echo "Done packaging ./$RELEASEDIR/ data" || echo "Ready to use ./$RELEASEDIR/ data for deaper packaging")\
	 || echo "WARNING: Upss smth went wrong and I couldn't locate auto-gen data in ./$RELEASEDIR/"
	
	# Create ./package/<target_version>.zip
	if test $ZIP -eq 1 >/dev/null 2>&1; then
		# rm -rf $RELEASEDIR/*.ipk $RELEASEDIR/*.zip
		cd $RELEASEDIR && zip -rq ${TARGET}_${VERSION}.zip ./* && mv *.zip ..\
		 && echo "Done packaging ./${TARGET}_${VERSION}.zip archive"\
		 || echo "WARNING: Upss smth went wrong and I couldn't create ${TARGET}_${VERSION}.zip"
		cd ..
	fi
	
	# Create ./package/<target>.ipk
	if test $IPK -eq 1 >/dev/null 2>&1; then
		# rm -rf $RELEASEDIR/*.ipk $RELEASEDIR/*.zip
		mkdir -p .$HOMEPATH
		cp -r $RELEASEDIR/* .$HOMEPATH && mv .$HOMEPATH $RELEASEDIR/
		mkdir -p $RELEASEDIR/data
		mv $RELEASEDIR$HOMEPATH $RELEASEDIR/data/
		if ! { test -d ${OPKG_ASSETSDIR}/CONTROL && test -f ${OPKG_ASSETSDIR}/CONTROL/preinst && test -f ${OPKG_ASSETSDIR}/CONTROL/postinst && test -f ${OPKG_ASSETSDIR}/CONTROL/control; }; then
			mkdir -p $OPKG_ASSETSDIR/CONTROL
			echo -e "#!/bin/sh\nsync; echo 'Installing new ${TARGET}..'; rm /var/lib/opkg/info/${TARGET}.list; exit 0" > $OPKG_ASSETSDIR/CONTROL/preinst
			echo -e "#!/bin/sh\nsync; echo 'Installation finished.'; echo 'Restarting ${TARGET}..'; sleep 1; killall ${TARGET}; exit 0" > $OPKG_ASSETSDIR/CONTROL/postinst
			echo -e $CONTROL > $OPKG_ASSETSDIR/CONTROL/control
		else
			OPKG_ASSETSDIR_CUSTOM="yes"
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
	if test $PACKAGE -eq 0 >/dev/null 2>&1; then rm -r ${RELEASEDIR:?}/ >/dev/null 2>&1 || echo "WARNING: Couldn't clean release dir ./${RELEASEDIR} after done packaging"; fi
fi
if test $CLEAN -eq 1 >/dev/null 2>&1; then
	echo -e "---"
	if test $PACKAGE -ne 1; then
		rm -r ${RELEASEDIR:?} >/dev/null 2>&1 && echo "Done CLEANING release dir ./${RELEASEDIR}" || echo "Not able or no need to clean release dir ./${RELEASEDIR}"
	fi
	if ! test "x${OPKG_ASSETSDIR_CUSTOM}" == "xyes"; then
		rm -r ${OPKG_ASSETSDIR:?} >/dev/null 2>&1 && echo "Done CLEANING opkg assets dir ./${OPKG_ASSETSDIR}" || echo "WARNING: Couldn't clean opkg assets dir ./${OPKG_ASSETSDIR}"
	fi
	if test $IPK -ne 1; then
		rm $TARGET.ipk >/dev/null 2>&1 && echo "Done CLEANING ./${TARGET}.ipk" || echo "Not able or no need to clean ./${TARGET}.ipk"
	fi
	if test $ZIP -ne 1; then
		rm ${TARGET}_${VERSION}.zip >/dev/null 2>&1 && echo "Done CLEANING ./${TARGET}_${VERSION}.zip" || echo "Not able or no need to clean ./${TARGET}_${VERSION}.zip"
	fi
	if ! test "x${LINK_CUSTOM}" == "xyes"; then
		rm $LINK >/dev/null 2>&1 && echo "Done CLEANING link ./${LINK}" || echo "WARNING: Couldn't clean link ./${LINK}"
	fi
fi
if test $PACKAGE -ne 1 >/dev/null 2>&1 && test $ZIP -ne 1 >/dev/null 2>&1 && test $IPK -ne 1 >/dev/null 2>&1 && test $CLEAN -ne 1 >/dev/null; then
	echo -e "\nWARNING: No instructions provided, please use -i/-p/-z/-c option or set \$PACKAGE/\$ZIP/\$IPK/\$CLEAN in env to 1 for correct output\n\n"
	sleep 1
else
	echo -e "\tExiting gm2xpkg..."
fi
#---------------------------------------------#

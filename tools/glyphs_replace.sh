#!/usr/bin/bash

ORIGINAL_FONT=$1
MODIFIED_FONT=$2
FINAL_FONT="$3"
GLYPHS=$4
GLYPHS_ARRAY=($4)

# Sanity checks
if (test "$MODIFIED_FONT" == "" || test "$ORIGINAL_FONT" == "" || test "$FINAL_FONT" == "" || test "$GLYPHS" == ""); then
	echo -e "\nUSAGE: \n\t<ENV_VAR> ./glyphs_replace.sh <original_font.ttf> <modified_font.ttf> <final_font_name>.ttf <\"glyph1 glyph2...\">\n"
	echo -e "\nENVIRONMENT \n\tVERBOSE\n\t\t Slow verbose build\n\tDEBUG\n\t\t Debugging session\n\tVALIDATE\n\t\t Validate glyphs in modified font\n\tGDB\n\t\t Run script with gdb\n"
	sleep 2
	exit
elif ! (test "$(file -b --mime-type ${MODIFIED_FONT})" == "font/sfnt" && test "$(file -b --mime-type ${ORIGINAL_FONT})" == "font/sfnt"); then
	echo -e "\nERROR: provide correct TTF font format\n"
	sleep 1
	exit
elif ! (test -f /usr/bin/fontforge); then
	echo -e "\nERROR: Missing dependencie! Please install \"fontforge\" app from your package manager!\n\n\n\
	On Debian distro run \"apt update && apt install fontforge\".\n\n"
	sleep 2
	exit
fi

if test "x${GDB}" != "x"; then
	GDB="gdb --batch -ex=run -ex=bt --args /usr/bin/"
	echo -e "\n----------\nprocessing with GDB backend\n----------\n"
fi

if (test "x${DEBUG}" == "x" && test "x${VERBOSE}" == "x" && test "x${VALIDATE}" == "x"); then
	${GDB}fontforge -quiet -lang=ff -c '\
Open("'$MODIFIED_FONT'"); \
i=1; \
while (i<$argc); \
	SelectMoreSingletons($argv[i]); \
	i = i+1; \
endloop; \
Copy(); \
Close(); \
Open("'$ORIGINAL_FONT'"); \
i=1; \
while (i<$argc); \
	SelectMoreSingletons($argv[i]); \
	i = i+1; \
endloop; \
Paste(); \
Generate("'$FINAL_FONT.ttf'"); \
Close()' $GLYPHS
elif test "x${VERBOSE}" != "x"; then
	${GDB}fontforge -lang=ff -c '\
i=1; \
while (i<$argc); \
	Print("---------- checking ", $argv[i], " ----------"); \
	Open("'$MODIFIED_FONT'"); \
		SelectSingletons($argv[i]); \
	Copy(); \
	Close(); \
	Open("'$ORIGINAL_FONT'"); \
		SelectSingletons($argv[i]); \
	Paste(); \
	Generate("'$FINAL_FONT.ttf'");
	Close(); \
	Print("---------- replaced ", $argv[i], " ----------"); \
	i = i+1; \
endloop' $GLYPHS
elif test "x${DEBUG}" != "x"; then
# Tested on ff version: 20201107, Based on sources from 2022-01-13 19:13 UTC-ML-D-GDK3.
## May not work on all machines/versions, will thus output Segfault. Moreover the ff syntax is quite fragile here "silently" using ff_post_notice of FF_SetUiInterface()
	for ((i=0; i<${#GLYPHS_ARRAY[@]}; i++))
	do
		echo -e "----------\ndebugging ${GLYPHS_ARRAY[$i]}\n----------"
		glyph=$(\
${GDB}fontforge -quiet -lang=ff -c '\
Open("'$MODIFIED_FONT'"); \
Select("'${GLYPHS_ARRAY[$i]}'"); \
g=GlyphInfo("Encoding"); \
Print(g); \
Close()'\
 | grep -o '[0-9]\+')
		glyph_uni=$(printf "0x%X" $glyph)

#test -f $FINAL_FONT.ttf && ORIGINAL_FONT="${FINAL_FONT}.ttf"
		${GDB}fontforge -quiet -lang=ff -c '\
Open("'$MODIFIED_FONT'"); \
	Select('$glyph_uni'); \
Copy(); \
Close(); \
Open("'$ORIGINAL_FONT'"); \
	Select('$glyph_uni'); \
Paste(); \
Generate("'$FINAL_FONT.ttf'"); \
Close()'
		echo -e "----------\nreplaced ${GLYPHS_ARRAY[$i]}\n----------"
	done
elif test "x${VALIDATE}" != "x"; then
# For reference https://fontforge.org/docs/scripting/python/fontforge.html#fontforge.glyph.validation_state
## Valstates are in decimal, reduce by 1 and convert to HEX to read error type
	echo -e "----------------------\nvalidating ${MODIFIED_FONT}\n----------------------"
	${GDB}fontforge -quiet -lang=ff -c '\
Open("'$MODIFIED_FONT'"); \
Validate(); \
i=1; \
while (i<$argc); \
	Select($argv[i]); \
	Print("ValidationState of ", $argv[i], "=", GlyphInfo("ValidationState"));
	i = i+1; \
endloop; \
Close()' $GLYPHS
		echo -e "----------\ndone processing\n----------"
fi

test "x${DEBUG}" != "x" && rm $FINAL_FONT.ttf && echo -e "\nRemoved test font: ${FINAL_FONT}.ttf\n"

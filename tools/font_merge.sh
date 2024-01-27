#!/usr/bin/bash
# ABOUT: 
## Font-merge tool to include missing glyphs in the main_font from a second_font, thus giving us the final_font.ttf

# USAGE:  
## <main_font>.ttf <second_font>.ttf <final_font_name>

# Supress stderr output
exec 3>&2
exec 2> /dev/null

MAIN_FONT=$1
SECOND_FONT=$2
FINAL_FONT="$3"

# Grab enumaration size of main_font to apply to rest of the fonts before/after merge
ENUM=$(fontforge -lang=ff -c 'Open($1); Print($em); Close()' "${MAIN_FONT}")

if (test "$MAIN_FONT" == "" || test "$SECOND_FONT" = ""); then
	echo -e "\nUSAGE: ./font_merge.sh <main_font.ttf> <second_font.ttf> <final_font_name>.ttf \n"
	exit
elif ! (test "$(file -b --mime-type ${MAIN_FONT})" = "font/sfnt" && test "$(file -b --mime-type ${SECOND_FONT})" = "font/sfnt"); then
	echo -e "\nERROR: provide correct TTF font format\n"
	exit
fi

echo -e "\nProcessing new font, pls wait a few seconds.."

# MAIN CODE:
## Merging script using fontfroge legacy scripting language
fontforge -lang=ff -c 'Open($1); SelectAll(); ScaleToEm(Strtol($3)); Generate("temp_1.ttf"); Close(); Open($2); SelectAll(); ScaleToEm(Strtol($3)); Generate("temp_2.ttf"); Close(); Open("temp_1.ttf"); MergeFonts("temp_2.ttf"); Generate($4); Close()' $MAIN_FONT $SECOND_FONT $ENUM $FINAL_FONT.ttf

echo -e "\nSuccessfully generated merged ${FINAL_FONT}.ttf\n"

rm temp_1.ttf temp_2.ttf

# Restore stderr output
exec 2>&3

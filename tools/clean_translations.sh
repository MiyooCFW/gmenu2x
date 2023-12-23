#!/bin/bash
# Compares translations with translate.txt and remove any redundant entries + adds missing ones
## Usage: run from main repo directory ./tools/clean_translation [Language_Name] or ALL or DICTIONARY (to sanitize translate.txt)

#Cleanup tr strings data file
DATA_sorted="$(sort -f translate.txt)"
if [ -z "$DATA_sorted" ]; then
	DATA_init="$(cat translate.txt)"
else
	DATA_init="$(echo "$DATA_sorted")"
fi
##Correct output of translation data file
echo "$DATA_init" | grep -v '^$' | uniq > translate.txt

languages="$(ls -1b assets/translations/)"
LANGUAGE="$1"

#Test if correct argument provided or echo usage
FOUND_LANGUAGE=false
for i in $languages; do
	if test "$i" = "$LANGUAGE"; then
		FOUND_LANGUAGE=true
		break
	fi
done

if ! ($FOUND_LANGUAGE || test "$LANGUAGE" = "ALL" || test "$LANGUAGE" = "DICTIONARY"); then
	echo -e "\nERROR: Wrong translation file name!\n\nProvide correct \"Language\" name or type ALL to check every one.\n\
	Type DICTIONARY to sanitize translate.txt list\n\n"
	sleep 2
	exit
elif test "$LANGUAGE" = "ALL"; then
	LANGUAGE="*"
elif test "$LANGUAGE" = "DICTIONARY"; then
	echo "Opening dictionary.txt..."
	sleep 1
fi


#Check available translations
TEMP2="$(sed 's/=.*//' translate.txt)"
TEMP3="$(sed 's/=.*//' dictionary.txt)"
for file in assets/translations/$LANGUAGE; do
	if [ -f "$file" ]; then
	##CODE
		TEMP="$(sed 's/=.*//' $file)"
		PATTERNS="$(grep "${TEMP}" translate.txt)"
		LANG_error="$(grep -v "${TEMP2}" <<< "${TEMP}" | sed '/^$/!s/$/=/g')"
		LANG_lost="$(comm -23 <(sort <<< "$TEMP2") <(sort <<< "$TEMP") | sed '/^$/!s/$/=/g')"
		LANG_udiff="$(grep "${PATTERNS}" $file)"
		LANG_missing="$(grep -v "${PATTERNS}" translate.txt)"
		###Correct output of translation asset file
		sort <<< "${LANG_udiff}"$'\n'"${LANG_missing}"$'\n'"${LANG_lost}" | grep -v '^$' | uniq > $file
	##LOGS
		LANG_diff="$(sed '/[^=]$/d' $file)"
		echo -e "\n\n    Language file ${file}:\n\n"
		echo -e "\n\n List of valid translations:\n\n"
		echo "$LANG_udiff"
		echo -e "\n\n List of wrong and removed translations:\n\n"
		echo "$LANG_error"
		echo -e "\n\n List of new and added translations:\n\n"
		echo "$LANG_lost"
		echo -e "\n\n List of missing translations:\n\n"
		echo "$LANG_diff"
	fi
done

if test "$LANGUAGE" == "DICTIONARY" ; then
	echo -e "\n\n Checking translate.txt:\n\n"
	sleep 1
	LANG_wrong="$(grep -v "${TEMP3}" <<< "${TEMP2}" | sed '/^$/!s/$/=/g')"
	echo -e "\n\n List of incorrect translations in translate.txt\n not present in dictionary.txt (excluding sections names) pls remove:\n\n"
	#Exclude sections names
	SECTIONS="$(cat sections.txt)"
	LANG_wrong_clean="$(grep -v "${SECTIONS}" <<< "${LANG_wrong}")"
	echo "$LANG_wrong_clean"
else
	echo "Finished."
	exit
fi

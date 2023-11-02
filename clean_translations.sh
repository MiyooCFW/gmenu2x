#!/bin/bash
#Compares translations with translate.txt and remove any redundant entries + adds missing ones

#Cleanup tr strings data file
DATA_sorted="$(sort -f translate.txt)"
if [ -z "$DATA_sorted" ]; then
	DATA_init="$(cat translate.txt)"
else
	DATA_init="$(echo "$DATA_sorted")"
fi
##Correct output of translation data file
echo "$DATA_init" | grep -v '^$' | uniq > translate.txt

#Check available translations
for file in assets/translations/*; do
	if [ -f "$file" ]; then
	##CODE
		TEMP="$(sed 's/=.*//' $file)"
		TEMP2="$(sed 's/=.*//' translate.txt)"
		PATTERNS="$(grep "${TEMP}" translate.txt)"
		LANG_error="$(grep -v "${TEMP2}" <<< "${TEMP}" | sed '/^$/!s/$/=/g')"
		LANG_lost="$(comm -23 <(sort <<< "$TEMP2") <(sort <<< "$TEMP") | sed '/^$/!s/$/=/g')"
		LANG_udiff="$(grep "${PATTERNS}" $file)"
		LANG_missing="$(grep -v "${PATTERNS}" translate.txt)"
		###Correct output of translation asset file
		sort <<< "${LANG_udiff}"$'\n'"${LANG_missing}"$'\n'"${LANG_lost}" | grep -v '^$' | uniq > $file
	##LOGS
		LANG_diff="$(sed '/[^=]$/d' $file)"
		echo -e "\n\n    Language ${file}:\n\n"
		echo -e "\n\n List of missing translations:\n\n"
		echo "$LANG_diff"
		echo -e "\n\n List of valid translations:\n\n"
		echo "$LANG_udiff"
		echo -e "\n\n List of wrong and removed translations:\n\n"
		echo "$LANG_error"
		echo -e "\n\n List of new and added translations:\n\n"
		echo "$LANG_lost"
	fi
done

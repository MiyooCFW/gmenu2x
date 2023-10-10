#!/bin/sh
#Compares translations with translate.txt and remove any redundant entries
for file in assets/translations/*
do
  if [ -f "$file" ]; then
	sed 's/=.*//' $file > temp
	grep -f temp translate.txt > patterns
	grep -f patterns $file > LANG_diff
	grep -vf patterns translate.txt > temp
	cp LANG_diff temp2
	cat temp > LANG_missing
	cat temp >> temp2
	sort -f temp2 > $file
	rm temp temp2 patterns
	echo -e "\n\n List of missing translations:\n\n"
	cat LANG_missing
	echo -e "\n\n List of valid translations:\n\n"
	cat LANG_diff
	rm LANG_missing LANG_diff
  fi
done
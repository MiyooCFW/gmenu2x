#!/bin/sh
#Compares translations with translate.txt and remove any redundant entries
for file in assets/translations/*
do
  if [ -f "$file" ]; then
	sed 's/=.*//' $file > temp
	sed 's/=.*//' translate.txt > temp2
	grep -f temp translate.txt > patterns
	grep -vf temp2 temp > LANG_error
	grep -f patterns $file > LANG_diff
	grep -vf patterns translate.txt > temp
	cp LANG_diff temp2
	cat temp > LANG_missing
	cat temp >> temp2
	sort -f temp2 > $file
	rm temp temp2 patterns
	echo -e "\n\n    Language ${file}:\n\n"
	echo -e "\n\n List of missing translations:\n\n"
	cat LANG_missing
	echo -e "\n\n List of valid translations:\n\n"
	cat LANG_diff
	echo -e "\n\n List of wrong and removed translations:\n\n"
	cat LANG_error
	rm LANG_missing LANG_diff LANG_error
  fi
done

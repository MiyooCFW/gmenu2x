#!/bin/sh
#Compares translations with translate.txt and remove any redundant entries + adds missing ones

#Cleanup tr strings data file
sorted_data=$(sort -f translate.txt)
if [ -z "$sorted_data" ]; then
  cat translate.txt > translate_data
else
  echo "$sorted_data" > translate_data
fi
cat translate_data | uniq > translate.txt
rm translate_data

#Check available translations
for file in assets/translations/*
do
  if [ -f "$file" ]; then
	sed 's/=.*//' $file > temp
	sed 's/=.*//' translate.txt > temp2
	grep -f temp translate.txt > patterns
	grep -vf temp2 temp > LANG_error
	comm -23 <(sort temp2) <(sort temp) > temp3
	sed -i 's/$/=/g' temp3
	grep -vf $file temp3 > LANG_lost
	grep -f patterns $file > LANG_diff
	grep -vf patterns translate.txt > temp
	cp LANG_diff temp2
	cat temp > LANG_missing
	cat temp >> temp2
	cat temp3 >> temp2
	sort -f temp2 | uniq > $file
	rm temp temp2 temp3 patterns
	echo -e "\n\n    Language ${file}:\n\n"
	echo -e "\n\n List of missing translations:\n\n"
	cat LANG_missing
	echo -e "\n\n List of valid translations:\n\n"
	cat LANG_diff
	echo -e "\n\n List of wrong and removed translations:\n\n"
	cat LANG_error
	echo -e "\n\n List of new and added translations:\n\n"
	cat LANG_lost
	rm LANG_missing LANG_diff LANG_error LANG_lost
  fi
done

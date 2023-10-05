#!/bin/sh
#1 argument is Translation e.g Polish (old)
#2 argument is translation.txt (generated from src)
sed 's/=.*//' $1 > temp
grep -f temp $2 > patterns
grep -f patterns $1 > LANG_diff
grep -vf patterns $2 > temp
cp LANG_diff temp2
cat temp > LANG_missing
cat temp >> temp2
sort temp2 > $1
rm temp temp2 patterns
echo -e "\n\n List of missing translations:\n\n"
cat LANG_missing
echo -e "\n\n List of valid translations:\n\n"
cat LANG_diff
rm LANG_missing LANG_diff

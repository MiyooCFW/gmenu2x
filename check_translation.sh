#!/bin/sh
# usage ./check_translation [Language_Name]
sed '/[^=]$/d' assets/translations/$1 > LANG_diff
echo -e "\n\n List of missing translations:\n\n"
cat LANG_diff
rm LANG_diff

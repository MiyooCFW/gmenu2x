#!/bin/bash
# usage ./check_translation [Language_Name]
LANG_diff="$(sed '/[^=]$/d' assets/translations/$1)"
echo -e "\n\n List of missing translations:\n\n"
echo "$LANG_diff"

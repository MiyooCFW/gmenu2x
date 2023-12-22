#!/bin/bash
# Strips c++ source and header files and generates translate.txt
## Usage: run from main repo directory ./tools/miyoo_translate.sh
## NOTES: this will remove sections strings from translate.txt!

# Generate #undef macros list
find . -name "*.cpp" -o -name "*.h" | while read F; do
	unifdef -s $F >> ./tools/undefines;
done
sed -i 's/^/#undef /' ./tools/undefines

# Generate #ifdef macros list
## TODO
## miyoo_defines (read from Makefile.miyoo & src/platform/miyoo.h)

# Remove unused platform headers
cp src/platform/miyoo.h /tmp/miyoo.h & rm src/platform/*
mv /tmp/miyoo.h src/platform/miyoo.h

# Remove comments
find . -name "*.cpp" -o -name "*.h" | while read file; do
	g++ -fpreprocessed -dD -E -P -w $file > /tmp/temp_file
	mv /tmp/temp_file $file
done

# Clean source code of unused preprocessors
find . -name "*.cpp" -o -name "*.h" | while read file; do
	unifdef -m -t -f ./tools/undefines -f ./tools/miyoo_defines $file; \
done

# Generate translate.txt
(grep -o -rn . -P -e "\ttr\["[^]]*"\]" ; \
grep -o -rn . -e '>tr\["[^]]*"\]\|\+tr\["[^]]*"\]\|\ tr\["[^]]*"\]\|,tr\["[^]]*"\]') | \
sed 's/.*\[\(.*\)\].*/\1/' | \
sed 's/\"\(.*\)\"/\1=/' | \
tr -d '\\' | \
sort | \
uniq > translate.txt

#Re-add sections' strings to translate.txt
cat sections.txt >> translate.txt
cat translate.txt | sort | uniq > translate.txt

# CLEAN
rm ./tools/undefines
git restore src/*

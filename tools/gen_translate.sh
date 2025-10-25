#!/bin/bash
# Strips c++ source and header files and generates translate.txt
## Usage: run from main repo directory ./tools/gen_translate.sh <name_defines.txt>
## WARNING: remember to commit your src changes BEFORE running this script, or they will be cleaned!!

defines="$1" # "defines" need to be provided, "undefines" are auto-generated
platform="$2" # provide platform to be used for translations

# Sanity checks
if  (test -z $defines || test -z $platform); then
	echo -e "\nUsage: run from main repo directory ./tools/gen_translate.sh <name_defines.txt> <platform>\n\n"
	sleep 1
	exit
elif test -z "$(cat "${defines}")"; then
	echo -e "\nERROR: Empty macros defines file!\n\n\n\
	Provide correct <name_defines>.txt with #ifdef preprocessors to include in generate src.\n\n"
	sleep 2
	exit
elif ! (test -f /usr/bin/unifdef); then
	echo -e "\nERROR: Missing dependencie! Please install \"unfidef\" app from your package manager!\n\n\n\
	On Debian distro run \"apt update && apt install unifdef\".\n\n"
	sleep 2
	exit
elif ! (test -z "$(git status -z --porcelain src/*)"); then
        echo -e "\n\nWARNING: Please commit/stage your src changes before continuing or they will be stashed!\n\n"
	while true; do
		read -p "Do you want to proceed anyway? [y/n]: " yn
		case $yn in
		[Yy]* ) echo "Yes. Continuing..."; break;;
		[Nn]* ) echo "No. Exiting..."; sleep 2; exit;;
		* ) echo "Please answer yes(Y) or no(N).";;
		esac
	done
	sleep 2
fi

echo -e "\nINFO: Running generate translate.txt script, pls wait a few seconds..."

# Generate #undef macros list
find . -name "*.cpp" -o -name "*.h" | while read F; do
	unifdef -s $F >> ./tools/undefines;
done
sed -i 's/^/#undef /' ./tools/undefines

# Generate #ifdef macros list
## TODO
## e.g. miyoo_defines (read from Makefile.miyoo & src/platform/miyoo.h)

# Remove unused platform headers
cp -r src/platform/* /tmp/ && rm -r src/platform/*

# Re-add specific platform header
mv /tmp/${platform}.h src/platform/

# Remove comments
find . -name "*.cpp" -o -name "*.h" | while read file; do
	g++ -fpreprocessed -dD -E -P -w $file > /tmp/temp_file
	mv /tmp/temp_file $file
done


# Clean source code of unused preprocessors
find . -name "*.cpp" -o -name "*.h" | while read file; do
	unifdef -m -t -f ./tools/undefines -f $defines $file; \
done

# Generate translate.txt
(grep -o -rn . -P -e "\ttr\["[^]]*"\]" ; \
grep -o -rn . -e '>tr\["[^]]*"\]\|(tr\["[^]]*"\]\|\+tr\["[^]]*"\]\|\ tr\["[^]]*"\]\|,tr\["[^]]*"\]') | \
sed 's/.*\[\(.*\)\].*/\1/' | \
sed 's/\"\(.*\)\"/\1=/' | \
sed ':a;N;$!ba;s/\\n/ /g' | \
tr -d '\\' | \
sort -f | \
uniq > translate.txt

#Re-add sections' strings to translate.txt
cat sections.txt >> translate.txt
cat translate.txt | sort -f > /tmp/translate.txt
uniq /tmp/translate.txt > translate.txt
rm /tmp/translate.txt

# CLEAN
rm ./tools/undefines
git restore src/*

# Generate dictionary
(grep -o -rn . -P -e "\ttr\["[^]]*"\]" ; grep -o -rn . -e '>tr\["[^]]*"\]\|(tr\["[^]]*"\]\|\+tr\["[^]]*"\]\|\ tr\["[^]]*"\]\|,tr\["[^]]*"\]') | sed 's/.*\[\(.*\)\].*/\1/' | sed 's/\"\(.*\)\"/\1=/' | sed ':a;N;$!ba;s/\\n/ /g' | tr -d '\\' | sort | uniq > dictionary.txt


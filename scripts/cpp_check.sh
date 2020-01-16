#!/bin/bash
#
# this must be the first non-commented line in this script. It ensures
# bash doesn't choke on \r on Windows
(set -o igncr) 2>/dev/null && set -o igncr; # this comment is needed

#
# This script does static code analysis using Cppcheck tool
# Copyright (c) 2019 Cypress Semiconductor.
#

# It performs Cppcheck code analysis with following inputs
# 1. CypressBootloader/sources - Code analysis is done on all the sources of CypressBootloader.
# 2. Additional source files to be analyzed are grabbed from config file that is provided as a first argument to the script.
# 3. Files to be ignored are grabbed from config file that is provided as a first argument to the script.
# 4. To ignore a file its name need to be added to the config file with word "ignore" as perfix
# 5. To add any additional files, apart the files from CypressBootloader/sources, those names need
#    to be added in a config file.
#    Example
#    A). add below entries in cpp_check.dat file
#        ignore cy_bootloader_hw.c
#        file1.c
#        file2.c
#        ignore cy_bootloader_services.c
#    B). invoke cpp_check shell script
#        cpp_check.sh cpp_check.dat
#
#    Above example performs Cppcheck analysis on CypressBootloader/sources, ignore cy_bootloader_hw.c, file1.c, file2.c and ignores cy_bootloader_services.c


config_file="$1"
#Retrieve list of files need to be ignored and additional files need to be checked from config file
while IFS= read -r line
do
    if [[ $line != ignore* ]] ; then
        CPP_CHECK_FILES="$CPP_CHECK_FILES $line"
    else
        ignore_file="${line#* }"
        CPP_CHECK_IGNORE_FILES="$CPP_CHECK_IGNORE_FILES "-i"$ignore_file"
    fi
done < "$config_file"

echo "Additional files:" "$CPP_CHECK_FILES"
echo "Ignoring files:" "$CPP_CHECK_IGNORE_FILES"

cppcheck --xml 2>cppcheck_report.xml CypressBootloader/source $CPP_CHECK_FILES $CPP_CHECK_IGNORE_FILES

echo
echo "Cppcheck report in xml format"
echo "-------------------------------------------"
cat cppcheck_report.xml

# Parse xml report and print number of errors
errors=($(grep -oP '(?<=error )[^<]' "cppcheck_report.xml"))

ERROR_COUNT=0

for i in ${!errors[*]}
do
  ERROR_COUNT=$(( $ERROR_COUNT + 1 ))
done

echo "Total Errors: $ERROR_COUNT"

if [[ $ERROR_COUNT -ne 0 ]]; then
	exit $ERROR_COUNT
fi

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
# 2. Additional source files to be analyzed are grabbed from cpp_check.dat file.
# 3. Files to be ignored are grabbed from cpp_check.dat file.
# 4. To ignore a file its name need to be added in cpp_check.dat file word "ignore" as perfix
# 5. To add any additional files, apart the files from CypressBootloader/sources, those names need
#    to be added in cpp_check.dat files.
#    Example to add entries in cpp_check.dat file
#    ignore cy_bootloader_hw.c
#    file1.c
#    file2.c
#    ignore cy_bootloader_services.c

config_file="scripts/cpp_check.dat"
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

cppcheck --xml 2>cppcheck_report.xml boot/cypress/CypressBootloader/source $CPP_CHECK_FILES $CPP_CHECK_IGNORE_FILES
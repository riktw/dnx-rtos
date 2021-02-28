#!/bin/bash

usage="Usage: $0 <system-map> "

# Check for the required path to the System.map file

sysmap=$1
if [ -z "$sysmap" ]; then
	echo "ERROR: Missing <system-map>"
	echo ""
	echo $usage
	exit 1
fi

# Verify the System.map and the partially linked file

if [ ! -r "$sysmap" ]; then
	echo "ERROR:  $sysmap does not exist"
	echo ""
	echo $usage
	exit 1
fi



# Now output the linker script that provides a value for all of the undefined symbols

varlist=`grep '\.text\.[a-Z]' $sysmap | awk '{print $1}' | awk 'BEGIN { FS = "." } ; { print $3 }'`

for var in $varlist; do
    map=`grep " ${var}$" ${sysmap}`
	if [ -n "$map" ]; then
      varaddr=`echo ${map} | cut -d' ' -f1`
      echo "${var} ${varaddr}"
	fi
done

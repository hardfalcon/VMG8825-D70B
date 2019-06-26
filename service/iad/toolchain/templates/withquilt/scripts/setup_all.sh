#!/bin/bash
# Script takes no arguments

export SOURCE

source ./scripts/project_defaults.inc

if [ -z "$SOURCE" ] 
then
	export SOURCE="$PWD/${DEFAULT_SOURCE}"
	echo "WARNING: \$SOURCE is not set... using defaults..($SOURCE)"
fi

if [ -d "$SOURCE" ]
then
	echo "ERROR: already got prepared working directory $SOURCE" >&2
	exit 1
fi

if [ -e series ]
then
	echo "ERROR: series file already generated (rm it and try again if wanted)." >&2
	exit 1
fi

# if SAS_CONFIG was given
if [ "$#" -gt 0 ]
then
	export FILES="00-series-generic"
	export MYARGS="$(echo $@ |tr ',' ' ')"
	for newfile in $MYARGS
	do
		export FILES="$FILES series-$newfile"
		if ! [ -e "series-$newfile" ]
		then
			echo "WARNING: series-$newfile does NOT EXIST" >&2
		fi
	done
	cat $FILES > series
	echo $@ >current_sas_config
else
 	cp 00-series-generic series
	touch series
fi

# Get vanilla kernel
echo catching vanilla sources
./scripts/get_vanilla.sh || exit 1

# Doing the real preparation
echo preparing working directory $SOURCE
./scripts/prepare_workingdir.sh || exit 1

echo done.


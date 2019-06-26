#!/bin/bash

if [ ! -e series ]
then
	echo "ERROR: couldnt find series file .. or you are in wrong working directory." >&2
	exit 1
fi

# Check series file
for i in $(grep -v "^#" series |grep patches\. | cut -d' ' -f1)
do
	if [ ! -f "$i" ]
	then
		echo "ERROR: couldnt find patch $i mentioned in series file." >&2
		exit 1
	fi
done

# check each patch to be in series file

for i in patches\.*/*
do
	if [ "${i#*\/}" = "*" ]
	then
		exit 0;
	fi

	if ! (grep -F "$i" *series* &>/dev/null)
	then
		echo "ERROR: patch $i is not in series file."
		exit 1
	fi
done


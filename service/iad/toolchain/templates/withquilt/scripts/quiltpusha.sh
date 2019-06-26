#!/bin/bash

if [ -e /usr/bin/quilt ]
then
	echo "INFO: using real quilt"
	if (grep "^patches" patches/series &>/dev/null)
	then
		quilt push -aq
	else
		echo "NO PATCHES THERE"
		exit 0
	fi
	exit $?;
fi

# manually apply
echo "WARNING.... NOT using quilt but internal applier...WARNING"

if ! [ -e patches/series ]
then
	echo "ERROR: no series file in place" >&2
	exit 1
fi

mkdir .pc &>/dev/null
echo "2" &>.pc/.version

export OLDIDF="$IFS"
export IFS="
"
for i in $(<patches/series )
do
	if (echo "$i" |grep "^#" &>/dev/null)
	then
		continue;
	fi
	if (echo "$i" | grep "^ " &>/dev/null)
	then 
		continue;
	fi
	if (echo "$i" | grep "^$" &>/dev/null)
	then
		continue;
	fi
	NAME="${i% \-p*}"
	DEPTH="${i#*patch }"
	if [ "$NAME" = "$DEPTH" ]
	then
		export DEPTH="-p1"
	fi
	if [ -e "patches/$NAME" ]
	then
		echo "INFO: applying patch $NAME"

		echo "$NAME" >>.pc/applied-patches

		patch $DEPTH < patches/$NAME
		if [ $? -ne 0 ]
		then
			echo "ERROR: patch $NAME did not apply, aborting..." >&2
			export IFS="$OLDIFS"
			exit 1;
		fi
	else
		echo "ERROR: could not find patch patches/$NAME" >&2
		exit 1
	fi
done
export IFS="$OLDIFS"


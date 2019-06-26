#!/bin/bash

source ./scripts/project_defaults.inc

export SOURCE

if [ -z "$SOURCE" ]
then
        export SOURCE="$PWD/${DEFAULT_SOURCE}"
        echo "WARNING: \$SOURCE is not set... using defaults..($SOURCE)"
fi

export VERSION

if [ -z "$VERSION" ]
then
	export VERSION="${DEFAULT_VERSION}"
	echo "WARNING: No \$VERSION given... using default ($VERSION)"
fi

if [ ! -e "${DEFAULT_CATCHCMD%%\ *}" ]
then
	if [ ! -e "$DEFAULT_TARSRC" ]
	then
		echo "ERROR: no ${DEFAULT_CATCHCMD%%\ *} found.. please install this tool." >&2
		exit 1
	fi
fi

mkdir $SOURCE
cd $SOURCE

if [ -n "$DEFAULT_TARSRC" ] && [ -e "../$DEFAULT_TARSRC" ]
then
	echo "INFO: using integrated tar-Ball"
	case $DEFAULT_TARSRC in
		*tar.bz2)
			tar --strip-components 1 -xjf ../${DEFAULT_TARSRC}
			;;
		*tar.gz|*.tgz)
			tar --strip-components 1 -xzf ../${DEFAULT_TARSRC}
			;;
		*)
			echo "ERROR: cannot compute $DEFAULT_TARSRC file" >&2
			cd ..
			rmdir $SOURCE
			exit 1
			;;
	esac
else
	if [ -z "$DEFAULT_CATCHCMD" ]
	then
		echo "ERROR: no DEFAULT_CATCHCMD given and not tarball defined" >&2
		cd ..
		rmdir $SOURCE
		exit 1
	fi
	echo "INFO: using ${DEFAULT_CATCHCMD%%\ *}"
	echo issueing $DEFAULT_CATCHCMD
	eval $DEFAULT_CATCHCMD
fi

cd ..


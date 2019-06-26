#!/bin/bash

source ./scripts/project_defaults.inc

export VERSION

if [ -z "$VERSION" ]
then
        export VERSION="${DEFAULT_VERSION}"
        echo "WARNING: No \$VERSION given... using default ($VERSION)"
fi

if [ -z "$SOURCE" ]
then
        export SOURCE="$PWD/${DEFAULT_SOURCE}"
        echo "WARNING: \$SOURCE is not set... using defaults..($SOURCE)"
fi


if [ -e "${SOURCE}/patches" ]
then
	echo "ERROR: ${SOURCE}/patches dir already present." >&2
	exit 1
fi

echo "Checking patches...."
./scripts/checkpatches.sh || exit 1

echo "Setting up quilt....."
export OLDROOT=$PWD
cd "${SOURCE}"
ln -sf $OLDROOT patches



#!/bin/bash
#
# SPDX-License-Identifier: Zyxel
# (C) 2018 Sphairon GmbH (a ZyXEL company)
#

# enforce safe shell settings
set -eu -o pipefail

SCRIPT_ROOT=$(dirname "${BASH_SOURCE[0]}")
source "$SCRIPT_ROOT/setup.bash"

github_download() {
    local repo=$1
    local file=$2
    local url
    local dest

    url="https://raw.githubusercontent.com/ztombol/${repo}/master/${file}"
    dest="$TEMPLATES_BATS_HELPER/${repo}/${file}"

    echo "Downloading $url"
    wget -q -O "$dest" "$url"
}

github_download 'bats-support' 'src/error.bash'
github_download 'bats-support' 'src/lang.bash'
github_download 'bats-support' 'src/output.bash'
github_download 'bats-support' 'load.bash'
github_download 'bats-support' 'README.md'
github_download 'bats-support' 'LICENSE'
github_download 'bats-assert' 'src/assert.bash'
github_download 'bats-assert' 'load.bash'
github_download 'bats-assert' 'README.md'
github_download 'bats-assert' 'LICENSE'
github_download 'bats-file' 'src/file.bash'
github_download 'bats-file' 'src/temp.bash'
github_download 'bats-file' 'load.bash'
github_download 'bats-file' 'README.md'
github_download 'bats-file' 'LICENSE'

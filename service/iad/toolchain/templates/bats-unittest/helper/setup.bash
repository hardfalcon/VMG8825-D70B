#
# SPDX-License-Identifier: Zyxel
# (C) 2018 Sphairon GmbH (a ZyXEL company)
#

SCRIPT_ROOT=$(dirname "${BASH_SOURCE[0]}")
TEMPLATES_ROOT=$(readlink -f "$SCRIPT_ROOT/../..")
TEMPLATES_BATS="$TEMPLATES_ROOT/bats-unittest"
TEMPLATES_BATS_HELPER="$TEMPLATES_BATS/helper"

export LC_ALL=C

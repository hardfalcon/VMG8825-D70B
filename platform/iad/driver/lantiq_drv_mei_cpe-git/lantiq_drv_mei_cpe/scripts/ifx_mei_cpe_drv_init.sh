#!/bin/sh

bindir=/opt/lantiq/bin 

# Base Address on VR9 Evaluation Board (EASY80920)
mei_base_0=0x1E116000

# VR9 IRQ Line:
mei_irq_0=0x37

# configure the MEI CPE driver
${bindir}/mei_cpe_drv_test -i $mei_base_0 -o $mei_irq_0 -n 0 > /dev/null


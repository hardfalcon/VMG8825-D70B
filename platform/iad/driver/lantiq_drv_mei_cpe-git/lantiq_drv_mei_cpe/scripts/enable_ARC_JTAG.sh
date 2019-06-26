# Reset Control Unit (RCU) setting
# Set bit 20 of RST_REQ register: Enable DSP JTAG
io -4 -o 0x1F203010 0x00100000

# GPIO module settings
# Set bit 11 of P0_OD: Open Drain Mode at Bit 11
#   Normal Mode, output is actively driven for 0 and 1 state
io -4 -o 0x1E100B24 0x00000800

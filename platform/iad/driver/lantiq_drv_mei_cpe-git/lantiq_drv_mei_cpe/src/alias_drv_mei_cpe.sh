#
# alias  definitions for the MEI CPE driver configuration
#

echo ... run alias defs for drv_mei_cpe

MEI_CPE_PROCFS_BASE_NAME=/proc/driver/mei_cpe

# common defines
alias dfe_stat='more $MEI_CPE_PROCFS_BASE_NAME/status/*'
alias dfe_vers='more $MEI_CPE_PROCFS_BASE_NAME/version'

# common config
alias lcfg='more $MEI_CPE_PROCFS_BASE_NAME/config'
alias bcfg='echo BOOTMODE $1 > $MEI_CPE_PROCFS_BASE_NAME/config'

# trace output handling
alias t_trace='echo TRACE $1 > $MEI_CPE_PROCFS_BASE_NAME/config'
alias t_log='echo LOG $1 > $MEI_CPE_PROCFS_BASE_NAME/config'
alias t_mei='echo T_MEI $1 > $MEI_CPE_PROCFS_BASE_NAME/config'
alias t_rom='echo T_ROM $1 > $MEI_CPE_PROCFS_BASE_NAME/config'

# boot handler: mailbox addresses
alias mb_arc2me='echo MB_ARC2ME $1 > $MEI_CPE_PROCFS_BASE_NAME/config'
alias mb_me2arc='echo MB_ME2ARC $1 > $MEI_CPE_PROCFS_BASE_NAME/config'

# boot handler: waittime - ready for download evt
alias w_rdy_f_dl='echo W_RDY_F_DL_MS $1 > $MEI_CPE_PROCFS_BASE_NAME/config'

# boot handler: waittime - ACK next FW block
alias w_ack_next='echo W_ACK_NEXT_FW_MS $1 > $MEI_CPE_PROCFS_BASE_NAME/config'

# boot handler: waittime - DownLoad start
alias w_dl_start='echo W_DL_START_MS $1 > $MEI_CPE_PROCFS_BASE_NAME/config'

# mailbox waittime: in online mode waittime of the mailbox
alias w_ol_mb='echo W_DFE_RESP_MS $1 > $MEI_CPE_PROCFS_BASE_NAME/config'

# waittime - mailbox block timeout
alias block_tout='echo DL_BLOCK_TOUT $1 > $MEI_CPE_PROCFS_BASE_NAME/config'

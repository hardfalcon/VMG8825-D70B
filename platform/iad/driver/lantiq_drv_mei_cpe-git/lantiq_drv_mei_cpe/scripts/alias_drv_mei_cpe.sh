#
# alias  definitions for the CPE MMEI Driver configuration
#

echo ... run alias defs for drv_mei_cpe

VRX_PROCFS_BASE_NAME=/proc/driver/drv_mei_cpe

# common defines
alias drv_vers='cat $VRX_PROCFS_BASE_NAME/version'
alias drv_cfg='cat $VRX_PROCFS_BASE_NAME/config'
alias line_info='cat $VRX_PROCFS_BASE_NAME/status/* | grep $1'
alias line_states='cat $VRX_PROCFS_BASE_NAME/status/* | grep bOpen'
alias dev_states='cat $VRX_PROCFS_BASE_NAME/status/* | grep ++'

# trace and log output handling
alias t_com='echo P_COM $1 $2 > $VRX_PROCFS_BASE_NAME/config'
alias t_mei='echo P_MEI $1 $2 > $VRX_PROCFS_BASE_NAME/config'
alias t_boot='echo P_BOOT $1 $2 > $VRX_PROCFS_BASE_NAME/config'
alias t_gdl='echo P_GDL $1 $2 > $VRX_PROCFS_BASE_NAME/config'

# waittime - mailbox block timeout
alias block_tout='echo T_OUT - - $1 > $VRX_PROCFS_BASE_NAME/config'
# waittime for modem ready evt
alias w_m_rdy='echo T_OUT $1 - - > $VRX_PROCFS_BASE_NAME/config'


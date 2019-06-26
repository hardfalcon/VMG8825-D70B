#ifndef _UGW_DIRECTCONNECT_DP_API_H_
#define _UGW_DIRECTCONNECT_DP_API_H_

#ifdef CONFIG_LTQ_DATAPATH /* for GRX500/GRX750 */
#include <net/datapath_api.h>
#else /* #ifdef CONFIG_LTQ_DATAPATH */
#include <net/datapath_api_common.h>
#endif /* #ifndef CONFIG_LTQ_DATAPATH */

/** \file directconnect_dp_api.h  This file defines all the APIs and associated data structures for DirectConnect (DC) compliant peripherals from common DirectConnect DP Library (referred in short as DC_DP_Lib). */

/** \defgroup DirectConnect_Datapath_Lib_Defines Direct Connect Defines
  \brief This section groups all the constant defines used in DirectConnect data structures.
*/

/** \defgroup DirectConnect_Datapath_Lib_Enums Direct Connect Enums
  \brief This section groups all the enumeration definitions used in DirectConnect data structures.
*/

/** \defgroup DirectConnect_Datapath_Lib_Unions Direct Connect Unions
  \brief This section groups all the Union type data structures definitions used in DirectConnect.
*/

/** \defgroup DirectConnect_Datapath_Lib_Structs Direct Connect Structures
  \brief This section groups all the Struct type data structures definitions used in DirectConnect.
*/

/** \defgroup DirectConnect_Datapath_Lib_APIs Direct Connect APIs
  \brief This section groups all the APIs definitions used in DirectConnect.
*/

/** \defgroup DirectConnect_Datapath_Lib_Power_Management_Wrapper_APIs  Direct Connect Power Management APIs
  \brief This section provides all the Power Management Wrapper API (Applicable if UGW CoC framework is used).
*/

/** \addtogroup DirectConnect_Datapath_Lib_Defines */
/* @{ */

/**
  \brief DC DP API version code
*/
#define DC_DP_API_VERSION_CODE        0x040104

/**
  \brief DC DP API version
*/
#define DC_DP_API_VERSION(a,b,c)      (((a) << 16) + ((b) << 8) + (c))

/**
  \brief Deregister
*/
#define DC_DP_F_DEREGISTER       0x00000001

/**
  \brief Alloc flag as FASTPATH
*/
#define DC_DP_F_FASTPATH         0x00000010

/**
  \brief Alloc flag as LITEPATH
*/
#define DC_DP_F_LITEPATH         0x00000020

/**
  \brief Alloc flag as SWPATH
*/
#define DC_DP_F_SWPATH           0x00000040

/**
  \brief Alloc flag as FASTPATH DSL
*/
#define DC_DP_F_FAST_DSL         0x00000100

/**
  \brief Alloc flag as shared resource
*/
#define DC_DP_F_SHARED_RES       0x00000200

/**
  \brief Alloc flag as Multiport support
*/
#define DC_DP_F_MULTI_PORT       0x00000400

/**
  \brief Register device flag; Specify if peripheral driver want to allocate SW Tx ring
*/
#define DC_DP_F_ALLOC_SW_TX_RING		0x00000010

/**
  \brief Register device flag; Specify if peripheral driver want to allocate SW Rx ring
*/
#define DC_DP_F_ALLOC_SW_RX_RING		0x00000020

/**
  \brief Register device flag; Specify if peripheral driver dont want to allocate HW Tx ring (may be for debugging)
*/
#define DC_DP_F_DONT_ALLOC_HW_TX_RING		0x00000040

/**
  \brief Register device flag; Specify if peripheral driver dont want to allocate HW Rx ring (may be for debugging)
*/
#define DC_DP_F_DONT_ALLOC_HW_RX_RING		0x00000080

/**
  \brief Register device flags; Specify it if peripheral/dev QoS class is required in DC peripheral
*/
#define DC_DP_F_QOS				0x00001000

/**
  \brief Register device flags: Specify it if peripheral driver requires to disable inter-subif forwarding (not in use)
*/
#define DC_DP_F_DISABLE_INTER_SUBIF_FORWARDING	0x00002000

/**
  \brief Register_device flags: Specify it if peripheral driver requires to disable intra-subif forwarding (not in use)
*/
#define DC_DP_F_DISABLE_INTRA_SUBIF_FORWARDING	0x00004000

/**
  \brief DC ring flags: Specify it if the ring is used only for buffer enqueuing in Tx/Rx direction
*/
#define DC_DP_F_PKTDESC_ENQ			0x00000001

/**
  \brief DC ring flags: Specify it if the ring is used only for buffer return in Tx/Rx direction
*/
#define DC_DP_F_PKTDESC_RET			0x00000002

/**
  \brief Dc ring flags: Specify it if the ring is used for both buffer enqueuing or buffer return in Tx/Rx direction
*/
#define DC_DP_F_RET_RING_SAME_AS_ENQ	        0x00000003

/**
  \brief DC Counter mode flags: Specify it if incremental DC counter is to be used.
*/
#define DC_DP_F_DCCNTR_MODE_INCREMENTAL	        0x01

/**
  \brief DC Counter mode flags: Specify it if cumulative DC counter is to be used.
*/
#define DC_DP_F_DCCNTR_MODE_CUMULATIVE	        0x02

/**
  \brief DC Counter mode flags: Specify it if DC counter in Little-endian mode is to be used.
*/
#define DC_DP_F_DCCNTR_MODE_LITTLE_ENDIAN	0x04

/**
  \brief DC Counter mode flags: Specify it if DC counter in Big-endian mode is to be used.
*/
#define DC_DP_F_DCCNTR_MODE_BIG_ENDIAN		0x08

/**
  \brief Host capability: Scatter/gather IO; Mapped to NETIF_F_SG
*/
#define DC_DP_F_HOST_CAP_SG 			0x00000001

/**
  \brief Host capability: Can checksum TCP/UDP over IPv4; Mapped to NETIF_F_IP_CSUM
*/
#define DC_DP_F_HOST_CAP_IP_CSUM 		0x00000002

/**
  \brief Host capability: Can checksum all the packets; Mapped to NETIF_F_HW_CSUM
*/
#define DC_DP_F_HOST_CAP_HW_CSUM 		0x00000004

/**
  \brief Host capability: Can checksum TCP/UDP over IPv6; Mapped NETIF_F_IPV6_CSUM
*/
#define DC_DP_F_HOST_CAP_IPV6_CSUM 		0x00000008

/**
  \brief Host capability: Receive checksumming offload; Mapped NETIF_F_RXCSUM
*/
#define DC_DP_F_HOST_CAP_RXCSUM 		0x00000010

/**
  \brief Host capability: TCPv4 segmentation; Mapped to NETIF_F_TSO
*/
#define DC_DP_F_HOST_CAP_TSO 			0x00000020

/**
  \brief Host capability: TCPv6 segmentation; Mapped to NETIF_F_TSO6
*/
#define DC_DP_F_HOST_CAP_TSO6 			0x00000040

/**
  \brief Host capability: large receive offload; Mapped to NETIF_F_LRO
*/
#define DC_DP_F_HOST_CAP_LRO 			0x00000080

/**
  \brief Host capability: Tx FCS calculation and insertion
*/
#define DC_DP_F_HOST_CAP_TX_FCS			0x00000100

/**
  \brief Host capability: Rx FCS automatic verfication
*/
#define DC_DP_F_HOST_CAP_RX_FCS			0x00000200

/**
  \brief Host capability: Specify when host can transmit frame without Tx FCS
*/
#define DC_DP_F_HOST_CAP_TX_WO_FCS		0x00000400

/**
  \brief Host capability: Specify when host can accept frame without Rx FCS
*/
#define DC_DP_F_HOST_CAP_RX_WO_FCS		0x00000800

/**
  \brief Host capability: Tx PMAC insertion
*/
#define DC_DP_F_HOST_CAP_TX_PMAC		0x00001000

/**
  \brief Host capability: Specify when host can accept with Rx PMAC
*/
#define DC_DP_F_HOST_CAP_RX_PMAC		0x00002000

/**
  \brief Host capability: Sepcify when host can accept without RX PMAC
*/
#define DC_DP_F_HOST_CAP_RX_WO_PMAC		0x00004000

/**
  \brief Host capability: Specify when host can do HW QoS
*/
#define DC_DP_F_HOST_CAP_HW_QOS			0x00008000

/**
  \brief Host capability: Specify when host can do HW QoS on WAN interface
*/
#define DC_DP_F_HOST_CAP_HW_QOS_WAN		0x00010000

/**
  \brief Host capability: Fragementation exception handling (Applicable for GRX350/550 FASTPATH)
*/
#define DC_DP_F_HOST_CAP_RX_FRAG_HANDLING_RESTRICTED 	0x00020000

/**
  \brief Host capability: Specify when host supports 2DW desc format (Applicable for GRX330 FASTPATH)
*/
#define DC_DP_F_HOST_CAP_DESC_2DW		0x00040000

/**
  \brief Host capability: Specify when host supports device QoS, like - WMM for WLAN etc.
*/
#define DC_DP_F_HOST_CAP_DEVQOS				0x00080000

/**
  \brief Host capability: Specify when host cpu is BIG endian.
*/
#define DC_DP_F_HOST_CAP_CPU_BIG_ENDIAN			0x00100000

/**
  \brief Host capability: Specify when host support SKB CHAIN (for future LITEPATH/SWPATH performance boost).
*/
#define DC_DP_F_HOST_CAP_SKB_CHAIN			0x00200000

/**
  \brief Device request: Specify when device expects without Tx FCS.
*/
#define DC_DP_F_DEV_REQ_TX_WO_FCS			0x00000001

/**
  \brief Device request: Specify when device expects without Rx FCS.
*/
#define DC_DP_F_DEV_REQ_RX_WO_FCS			0x00000002

/**
  \brief Device request: Specify when device expects Tx PMAC.
*/
#define DC_DP_F_DEV_REQ_TX_PMAC				0x00000004

/**
  \brief Device request: Specify when device expects with Rx PMAC.
*/
#define DC_DP_F_DEV_REQ_RX_PMAC				0x00000008

/**
  \brief Device capability: Specify when device supports SKB chain on receive.
*/
#define DC_DP_F_DEV_CAP_RX_SKB_CHAIN			0x00000010

/**
  \brief Device request: Specify when device expects Rx fragmentation handling.
*/
#define DC_DP_F_DEV_REQ_RX_FRAG_HANDLING		0x00000020

/**
  \brief Device request: Specify when device expects 2DW desc (Applicable in GRX330 FASTPATH).
*/
#define DC_DP_F_DEV_REQ_DESC_2DW			0x00000040

/**
  \brief Device request: Specify when device expects Rx in little endian (Applicatable in SW ring handling).
*/
#define DC_DP_F_DEV_REQ_RX_LTTLE_ENDIAN			0x00000080

/**
  \brief Device request: Specify when device expects Tx in little endian (Applicable in SW ring handling).
*/
#define DC_DP_F_DEV_REQ_TX_LTTLE_ENDIAN			0x00000100

/**
  \brief Device request: Specify when device expects SWPATH->LITEPATH internal forwarding.
*/
#define DC_DP_F_DEV_REQ_LITEPATH_TX_SHORTCUT		0x00000200

/**
  \brief Device request: Specify when device expects LITEPATH->SWPATH internal forwarding.
*/
#define DC_DP_F_DEV_REQ_LITEPATH_RX_SHORTCUT		0x00000400

/**
  \brief Register subif flags; Specify to register already registered subif/vap as LitePath Partial offload
*/
#define DC_DP_F_REGISTER_LITEPATH			0x00000100

/**
  \brief Register subif flags; De-register already registered LitePath subif/logical netdev from LitePath
*/
#define DC_DP_F_DEREGISTER_LITEPATH			0x00000200

/**
 * @brief disconn_if flags; client disconnected (3addr) - remove from acceleration (by mac or netif)
 */
#define DC_DP_F_CLIENT_DISCONNECT			0x00000000

/**
 * @brief disconn_if flags; client connected - remove from
 *  	  acceleration (by mac or netif) and trigger fake arp
 *  	  type 1
 */
#define DC_DP_F_CLIENT_CONNECT			0x00000001

/**
 * @brief disconn_if flags; extender connected - remove from
 *  	  acceleration (by mac or netif) and trigger fake arp
 *  	  type 2
 */
#define DC_DP_F_CLIENT_CONNECT_EXTENDER		0x00000010

/**
  \brief Xmit flags: Specify it as dc_dp_xmit() flags value if xmit to litepath
*/
#define DC_DP_F_XMIT_LITEPATH		0x01

/**
  \brief Xmit flags: Specify it as dc_dp_xmit() flags value if xmit to ATM interface, to be used by DP Lib
*/
#define DC_DP_F_XMIT_OAM		0x02

/**
  \brief Rx CB flags: Specify it as dc_dp_rx_fn_t flags value if received from litepath
*/
#define DC_DP_F_RX_LITEPATH		0x01

/**
  \brief Specify to enable SoC->peripheral enqueue
*/
#define DC_DP_F_ENQ_ENABLE		0x00000001

/**
  \brief Specify to disable SoC->peripheral enqueue
*/
#define DC_DP_F_ENQ_DISABLE		0x00000002

/**
  \brief Specify to enable peripheral->SoC dequeue
*/
#define DC_DP_F_DEQ_ENABLE		0x00000004

/**
  \brief Specify to disable peripheral->SoC dequeue
*/
#define DC_DP_F_DEQ_DISABLE		0x00000008

/**
  \brief Specify it if intra subif session is per-forwarding stage
*/
#define DC_DP_F_PREFORWARDING		0x01

/**
  \brief Specify it if intra subif session is post-forwarding stage
*/
#define DC_DP_F_POSTFORWARDING  	0x02

/**
  \brief Handle ring sw flags: Specify it if SWPATH ring handling for Tx return
*/
#define DC_DP_F_TX_COMPLETE		0x0001

/**
  \brief Handle ring sw flags: Specify it if SWPATH ring handling for Rx
*/
#define DC_DP_F_RX 			0x0002

/**
  \brief Handle ring sw flags: Specify it if fragmentation exception ring handling for Rx
*/
#define DC_DP_F_RX_FRAG_EXCEPT          0x0004

/**
  \brief Peripheral to SoC path (e.g. DMA1-TX for GRX500) default packet buffer size.
*/
#define DC_DP_PKT_BUF_SIZE_DEFAULT	2048

/**
  \brief Multicast (MC) module register request (if applicable to peripheral driver).
*/
#define DC_DP_F_MC_REGISTER			0x01

/**
  \brief Multicast module de-register request (if applicable to peripheral driver).
*/
#define DC_DP_F_MC_DEREGISTER			0x02

/**
  \brief  Multicast module register update MACs request
*/
#define DC_DP_F_MC_UPDATE_MAC_ADDR		0x08

/**
  \brief Multicast module cleanup request (if applicable to peripheral driver).
*/
#define DC_DP_F_MC_FW_RESET			0x10

/**
  \brief Multicast module re-learning request (if applicable to peripheral driver).
*/
#define DC_DP_F_MC_NEW_STA			0x20

/**
  \brief A new multicast group membership add request.
*/
#define DC_DP_MC_F_ADD				0x01

/**
  \brief An existing multicast group membership delete request
*/
#define DC_DP_MC_F_DEL				0x02

/**
   \brief An existing multicast group membership update request
*/
#define DC_DP_MC_F_UPD				0x03

/**
   \brief Maximum number of mac address
*/
#define DC_DP_MAX_MAC				64

/**
  \brief Number of Device QoS class/WiFi WMM Class/TID
*/
#define DC_DP_MAX_DEV_CLASS			8

/**
  \brief stats flags: specify it if request for subif specific stats
*/
#define DC_DP_F_SUBIF_LOGICAL                 0x00001000

/**
  \brief Power Saving (Cpufreq) callback register request
*/
#define DC_DP_F_PS_REGISTER			0x01

/**
  \brief Power Saving (PS) callback de-register request
*/
#define DC_DP_F_PS_DEREGISTER			0x02

/**
  \brief Power Saving (PS) notifier list type - Transition Notifier
*/
#define DC_DP_PS_TRANSITION_NOTIFIER		(0)
/**
  \brief Power Saving (PS) notifier list type - Policy Notifier
*/
#define DC_DP_PS_POLICY_NOTIFIER		(1)

/** \brief PS (CPUFreq) Operation Success */
#define DC_DP_PS_SUCCESS				0
/** \brief PS (CPUFreq) Operation Denied */
#define DC_DP_PS_DENIED					1
/** \brief Called function just return without doing anything; used only in callback functions */
#define DC_DP_PS_NOACTIVITY				3
/** \brief It is used if callback function is not defined */
#define DC_DP_PS_NOTDEFINED				4

/**
  \brief PS (Cpufreq) notifier list Add Operation
*/
#define DC_DP_PS_LIST_ADD				1
/**
  \brief PS (Cpufreq) notifier list Delete Operation
*/
#define DC_DP_PS_LIST_DEL				0

/* @} */


/** \addtogroup DirectConnect_Datapath_Lib_Enums */
/* @{ */

/** \brief Definition of the DC DP API status.
*/
enum dc_dp_api_status {
	DC_DP_FAILURE = -1,
	DC_DP_SUCCESS = 0,
};

/** \brief Definition of the IP Type used.
*/
enum dc_dp_iptype {
  /** IPv4 Type selector */
	IPV4 = 0,
  /** IPv6 Type selector */
	IPV6 = 1,
  /** None IP selector */
	INVALID,
};

/** \brief Definition of DC acceleration type
*/
enum dc_dp_accel_type {
	/** No Offload/Soft Path */
	DC_DP_ACCEL_NO_OFFLOAD,
	/** Partial Offload/Lite Path */
	DC_DP_ACCEL_PARTIAL_OFFLOAD,
	/** Full Offload/Fast Path */
	DC_DP_ACCEL_FULL_OFFLOAD,
};

/** \brief Definition of DC Tx/Rx ring type
*/
enum dc_dp_ring_type {
	/** Tx/Rx ring not used (e.g., Lite Path only) */
	DC_DP_RING_NONE = 0,
	/** Tx/Rx ring as SW DC Mode1 (e.g., Soft Path) */
	DC_DP_RING_SW_MODE1,
	/** Tx/Rx ring as HW DC Mode0 (e.g., GRX350 Fast Path) */
	DC_DP_RING_HW_MODE0,
	/** Tx/Rx ring as HW DC Mode1 (e.g., GRX750 Fast Path) */
	DC_DP_RING_HW_MODE1,
};

/**
   \brief Definition of DC Mode type.
*/
enum dc_dp_mode_type {
	/** DC Mode 0 */
	DC_DP_MODE_TYPE_0 = 0,
	/** DC Mode 1 */
	DC_DP_MODE_TYPE_1,
};

/** \brief Definition of DC direction type
*/
enum dc_dp_dir_type {
	/** Dev to SoC direction */
	DC_DP_DIR_DEV2SOC = 0,
	/** SoC to Dev direction */
	DC_DP_DIR_SOC2DEV,
};

/** \brief Definition of Power Management module
*/
enum dc_dp_power_module {
	DC_DP_MAX_PM = -1
};

/** \brief Definition of Power Management state
*/
enum dc_dp_power_state {
	/** Power State Invalid. */
	DC_DP_PS_UNDEF,
	/** Power State D0. normal operation freq */
	DC_DP_PS_D0,
	/** Power State D1.  intermediate freq */
	DC_DP_PS_D1,
	/** Power State D2.  intermediate freq */
	DC_DP_PS_D2,
	/** Power State D3. lowest freq */
	DC_DP_PS_D3,
	/** Power State don't care - Ignored by PS*/
	DC_DP_PS_D0D3,
	/** Power State BOOST highest freq, time limited because of thermal aspects */
	DC_DP_PS_BOOST,
};

/* @} */

/** \addtogroup DirectConnect_Datapath_Lib_Structs */
/* @{ */

/** \brief DirectConnect Data Path Device attributes.
 */
struct dc_dp_dev {
	char dev_name[IFNAMSIZ]; /*!< [in] Netdevice Name string (e.g. ptm0) */
	enum dc_dp_accel_type dc_accel_used; /*!< [out] DC acceleration type between SoC and Peripheral - Returned by DC Lib */
	enum dc_dp_ring_type dc_tx_ring_used; /*!< [out] DC Tx ring type between SoC and Peripheral - Returned by DC Lib */
	enum dc_dp_ring_type dc_rx_ring_used; /*!< [out] DC Rx ring type between SoC and Peripheral - Returned by DC Lib */
	uint32_t dev_cap_req; /*!< [in] Peripheral Requirements & Capability, as defined in DC_DP_F_DEV_* flags */
	uint32_t dc_cap; /*!< [out] DC Capability being used, as defined in DC_DP_F_CAP_* flags */
	void *dev_ctx; /*!< [in] Private device context  */
};

/**
   \brief Ring recovery stats Structure
   \note If stats difference exceeds pre-configured threshold the system would be rebooted to recover from loss of descriptiors.
	FIXME : will be generalzed for 4 ring.
*/
struct dc_dp_recovery_stats {
	uint32_t soc2dev_announced_desc_cum; /*!< Announced cumulative desc count by DC Counter */
	uint32_t soc2dev_to_be_pulled_counter; /*!< Desc count to be pulled from DC SoC (e.g. CBM for GRX500) Dequeue port */
	uint32_t soc2dev_to_be_freed_buffer; /*!< Buffer count to be freed to DC SoC (e.g. CBM for GRX500) Dequeue Free port */
	uint32_t dc_rx_outstanding_desc; /*!< DC client driver/FW HD ring outstanding decsriptors-
					 Add num_desc when written to HD ring and subtract num_desc based on DC counter value received. */
	//FIXME : For non-GRX500 : Expand dev2soc fields.??
};

/** \brief  Rx function callback - basic data struct for ACA Peripherals.
   \param[in] rx_if  Rx If netdevice pointer
   \param[in] tx_if  Tx If netdevice pointer - optional
   \param[in] subif  Tx/Rx SubIf pointer
   \param[in] skb  Pointer to pointer to packet buffer, like sk_buff
   \param[in] len  Length of the packet (optional as also present in skb->len)
   \param[in] flags:
       DC_DP_F_RX_LITEPATH - recieved packet through litepath
   \return 0 if OK / -1 if error / > 0, if DC DP layer needs to do LitePath Rx
   \note The receive callback is MUST to register and is invoked by DirectConnect datapath driver to pass the packets to the peripheral driver
   \note If the Peripheral Driver does not pass the packet to stack, and does
   not free the Rx buffer, it needs to return > 0 to indicate to DC DP Library to
   send packet to accelerator - Valid for LitePath Rx case only.
*/
typedef int32_t (*dc_dp_rx_fn_t) (
	struct net_device *rx_if,
	struct net_device *tx_if,
	struct dp_subif *subif,
	struct sk_buff **skb,
	int32_t len,
	uint32_t flags
); /*!< Rx function callback */

/**
   \brief   Get Meta-SubInterface Integer Identifier (e.g. Station Id) callback. Mostly applicable to DC WLAN peripheral only.
   \param[in] netif  Network interface through whireturn packet to dst MAC address mac_addr will be transmitted
   \param[in] skb  Pointer to pointer to packet buffer, like sk_buff
   \param[in] subif_data  VCC information for DSL nas interface, must provide valid subif_data, otherwise set to NULL
   \param[in] mac_addr  MAC Address of Station
   \param[in,out] subif  subif->port_id as input and sub-interface including Meta Sub Interface Id (e.g. STA Id) returned
   \param[in] flags  Reserved for future use
   \return 0 if OK, -1 on ERROR
   \note : Optional function - Not needed in case Client driver does not have peripheral specific mapping of Meta Sub Interface in the sub-interfce field.
*/
typedef int32_t(*dc_dp_get_netif_subinterface_fn_t) (
	struct net_device *netif,
	struct sk_buff *skb,
	void *subif_data,
	char *mac_addr,
	struct dp_subif *subif,
	uint32_t flags
); /*!< Get Meta SubInterface Id callback. */

/** \brief DirectConnect Data Path Device/Peripheral specific field info
 * structure including pos in descriptor DWORD format
 */
struct dc_dp_field_value_pos {
	uint32_t field_val; /*< Field value */
	uint32_t mask; /*< bit mask for the field value */
	uint32_t dw; /*< DWORD number dw for setting/reading the field from desc */
	uint32_t lsh; /*< Left shift the value to set in desc/HD before OR-ing */
};

/** \brief DirectConnect Data Path Device/Peripheral specific field info
 * structure as per field
 */
struct dc_dp_field_value {
	int32_t num_fields; /*< Num of fields */
	struct dc_dp_field_value_pos *fields; /*< Pointer to array of dc_dp_field_value_pos structure */
};

/** \brief DirectConnect Data Path Device/Peripheral specific field info
 * structure in descriptor DWORD format.
 */
struct dc_dp_fields_dw {
	uint32_t dw; /*< DWORD number for setting/reading the desc value */
	uint32_t desc_val; /*< Desc value */
	uint32_t desc_mask; /*< bit mask in the desc */
};

/** \brief DirectConnect Data Path Device/Peripheral specific field info
 * structure in descriptor DWORD format.
 */
struct dc_dp_fields_value_dw {
	int num_dw; /*< Num of DWORD */
	struct dc_dp_fields_dw *dw; /*< Pointer to array of dc_dp_fields_dw structure */
};

/** \brief  Get DC Peripheral specific Desc Information callback, which is
   invoked by DC library to the Peripheral driver. Peripheral driver indicates
   the field value and mask it wants to set in the desc/HD DWORDs
   \param[in] port_id	Port Number for which information is sought
   \param[in] skb	Pointer to skbuff for which Peripheral specific info is
   requested/
   \param[out] desc_fields  Pointer to dc_dp_fields_value_dw structure
   \param[in] flags  : Reserved
   \return 0 if OK / -1 if error
   \remark The Acceleration layer can treat this info as 'black box' if required
*/
typedef int32_t (*dc_dp_get_dev_specific_desc_info_t) (
	int32_t port_id,   /* Port Identifier */
	struct sk_buff *skb,
	struct dc_dp_fields_value_dw *desc_fields,
	uint32_t flags
);

/** \brief  Get DC Peripheral specific Desc Information, that will be passed in
    the HD to the peripheral (and read back from HD from the peripheral) - it
    indicates the mapping of the fields as well (not in use)
   \param[in] port_id	Port Number for which information is sought
   \param[in] skb	Pointer to skbuff for which Peripheral specific info is requested
   \param[out] desc_fields Pointer to dc_dp_field_value structure
   \param[in] flags  : Reserved
   \return 0 if OK / -1 if error
   \remark The Acceleration layer can treat this info as 'black box' if required
   and combine it into a single peripheral/vendor specific field if required.
*/
int32_t dc_dp_get_dev_specific_desc_fields (
	int32_t port_id,   /* Port Identifier */
	struct sk_buff *skb,
	struct dc_dp_field_value *desc_fields,
	uint32_t flags
);

/**
   \brief   Get ring/Buffer recovery stats callback
   \param[in] netif  Pointer to Linux netdevice structure
   \param[in] port_id  Port Identifier [either netdevice or port_id - one value to be passed].
   \param[out] stats  Pointer to dc_dp_recovery_stats structure
   \param[in] flags  Reserved for future use
   \return 0 if OK, -1 on ERROR
   \note Optional recovery stats callback. The DC datapath driver can tally up the buffer and
   ring stats on the DC peripheral and on the host SoC side, and trigger recovery - for exampe,
   through rebooting system if significant buffrs lost
*/
typedef int32_t (*dc_dp_get_recovery_stats_fn_t) (
	struct net_device *netif,
	int32_t port_id,
	struct dc_dp_recovery_stats *stats,
	uint32_t flags
);

/** \brief  Multicast module callback to add/delete a mcast group membership to/from a DirectConnect interface.
   \param[in] grp_id  Multicast group id.
   \param[in] dev  Registered net device.
   \param[in] mc_stream  Multicast stream information.
   \param[in] flags  :
       DC_DP_MC_F_ADD - Add a new mcast group membership to a DirectConnect interface.
       DC_DP_MC_F_UPD - Update an existing mcast group membership to a DirectConnect interface.
       DC_DP_MC_F_DEL - Delete an existing mcast group membership from a DirectConnect interface.
   \return none
   \note Group Identifier is allocated and managed by Multicast Subsystem.
*/
typedef void (*dc_dp_mcast_callback_fn_t) (
	uint32_t grp_id,
	struct net_device *dev,
	void *mc_stream,
	uint32_t flags
);

/**
   \brief IP address data structure - used in Multicast registration.
*/
struct dc_dp_ip_addr {
	enum dc_dp_iptype ip_type; /*!< IPv4 or IPv6 Type */
	union {
		struct in_addr ip4_addr; /*!< IPv4 address */
		struct in6_addr ip6_addr; /*!< IPv6 address */
	} u;  /*!< Union name u */
};

/**
   \brief Mutlicast stream (5-tuple) structure
*/
struct dc_dp_mcast_stream {
	struct net_device *mem_dev;	/*!< Member Netdevice */
	struct dc_dp_ip_addr src_ip; /*!< Source ip : can be ipv4 or ipv6 */
	struct dc_dp_ip_addr dst_ip; /*!< Destination ip - GA : can be ipv4 or ipv6 */
	uint32_t proto;	/*!< Protocol type : Mostly UDP for Multicast */
	uint32_t src_port; /*!< Source port */
	uint32_t dst_port; /*!< Destination port */
	union {
		unsigned char mac_addr[MAX_ETH_ALEN]; /*!< LAN/WLAN Mac address */
		unsigned char src_mac[MAX_ETH_ALEN]; /*!< LAN/WLAN source Mac address */
	};
	uint32_t num_joined_macs; /*!< Number of Joined MACs */
	unsigned char macaddr[DC_DP_MAX_MAC][MAX_ETH_ALEN]; /*!< Lan/wlan array of joined Mac Address */
};

/**
   \brief DirectConnect Power Saving (CoC) Thresholds values for different power states.
*/
struct dc_dp_ps_threshold {
	int32_t th_d0; /*!< Power State D0 (Highest Power) Threshold Level */
	int32_t th_d1; /*!< Power State D1 Threshold Level */
	int32_t th_d2; /*!< Power State D2 Threshold Level */
	int32_t th_d3; /*!< Power State D3 (Lowest Power) Threshold Level */
};

/**
   \brief DirectConnect Power State Module Information to be used for registration.
*/
struct dc_dp_ps_module_info {
	char                *module_name;  /*!< Power State registeirng module Name */
	enum dc_dp_power_state           power_feature_state; /*!< Current Power State */
	int32_t (*dc_dp_ps_state_get) (enum dc_dp_power_state *pmcu_state);  /*!< Callback to query current power state */
	int32_t (*dc_dp_ps_state_switch) (int32_t pmcu_pwr_state_ena); /*!< Callback to enable power state */
};

/**
   \brief Wake-on-LAN Config Structure
*/
struct dc_dp_wol_cfg {
	uint8_t wol_mac[MAX_ETH_ALEN]; /*!< Wake-on-LAN MAC address - part of Magic packet (16 times repeat)*/
	bool wol_passwd_enable; /*!< Wake-on-LAN password enable */
	uint8_t wol_passwd[MAX_ETH_ALEN]; /*!< Wake-on-LAN password */
};

/* @} */

/** \addtogroup DirectConnect_Datapath_Lib_Structs */
/* @{ */

/**
   \brief DirectConnect Datapath Lib Registration Callback. - It is supplied by individual DC peripheral driver.
*/
struct dc_dp_cb {
	dc_dp_rx_fn_t rx_fn;   /*!< Rx function callback */
	dp_stop_tx_fn_t stop_fn;  /*!< Stop Tx function callback for Tx flow control - Optional (NULL) */
	dp_restart_tx_fn_t restart_fn;  /*!< Start Tx function callback for Tx flow control - Optional (NULL)*/
	dc_dp_get_netif_subinterface_fn_t get_subif_fn;  /*!< Get Subinterface metaid callback - Optional (for non-WLAN) (NULL) */
	dc_dp_get_dev_specific_desc_info_t get_desc_info_fn; /*!< Get device/peripheral specific field info for
							 desc/HD */
	dp_reset_mib_fn_t reset_mib_fn;  /*!< reset registered device's network mib counters - Optional (NULL) */
	dp_get_mib_fn_t get_mib_fn;  /*!< Retrieve registered device's network mib counters  */
	dc_dp_get_recovery_stats_fn_t recovery_fn; /*!< Get Recovery related stats - Optional (NULL) */
};

/**
   \brief DirectConnect Buffer pools/Chunks Data structure.
   \\FIXME : For Rx direction only. Max Size in a pool : 4 MB (get_freepages).
*/
struct dc_dp_buf_pool {
	void *pool; /*!< Pointer to pool */
	void *phys_pool; /*!< Physical address to pool */
	uint32_t size; /*!< Size of pool in bytes - Must be multiple of individual buffer size */
};

/** \brief This data structure describes the ring attributes.
 */
/* FIXME : Any budget for burst handling mainly for sw ring handling? in form of num of desc or in fastpath it could be timer based for umt hw? */
struct dc_dp_ring {
	void *base; /*!< [out] Base address of ring */
	void *phys_base; /*!< [out] Physical Base address of ring */
	int32_t size; /*!< [in/out] Size of ring in terms of entries : FIXME : Zero implies ring is unused. */
	int32_t desc_dwsz; /*!< [in/out] Size of descriptor in DW. */
	int32_t flags; /*!< [in]  DC_DP_F_PKTDESC_ENQ, DC_DP_F_PKTDESC_RET
		          DC_DP_F_RET_RING_SAME_AS_ENQ*/
	void  *ring; /*!< [out] Place holder for ring meta-structure - only used
			internally in the lower layers */
};

/**
 * \brief Directconnect DirectPath Ring structure encompassing all connection rings between SoC and Peripheral.
 * Size is passed by Peripheral Driver. All resources allocated by DC driver
 * and passed back.
 *
 * For DC Mode 0: (GRX350/GRX550)
 *
 * SoC -> Peripheral Pkt direction (Tx)
 * -----------------------------------
 *
 * soc2dev - CBM Dequeue Port Base. SoC -> Peripheral direction and enqueue
 *  	counter update in Peripheral memory - incremental
 *
 * soc2dev_ret is CBM Buffer return port base, Peripheral -> SoC. SoC detects in HW buffer return and does not
 * 	need any enqueue counter update
 *
 * Above Ring Sizes are fixed
 *
 * Peripheral -> SoC Pkt direction (Rx)
 * -----------------------------------
 *
 * dev2soc - It is the DMA1-Tx channel ring (Peripheral -> SoC). Enqueue counter is  ignored as HW DMA engine polls.
 *	Peripheral may track on its own for its own writes!
 *
 * dev2soc_ret - Same as dev2soc_ring and points to DMA1-TX channel base. Can be ignored
 * 	in that mode. Size is also the same
 *
 * For DC Mode 1: (GRX750/PUMA7)
 *
 * SoC -> Peripheral Pkt direction (Tx)
 * -----------------------------------
 *
 * soc2dev - SoC -> Peripheral direction enqueue (desc/buffer) and enqueue counter
 *      update (incremental) in Peripheral memory
 *
 * soc2dev_ret - Peripheral -> SoC direction enqueue (desc/buffer return) and enqueue counter
 *      update (incremental) in SoC memory.
 *
 * Above Ring Sizes are equal. Currently, GRX750/PUMA7 may allocate all	peripheral rings as equal
 *
 * Peripheral -> SoC Pkt direction (Rx)
 * -----------------------------------
 *
 * dev2soc - Peripheral -> SoC direction enqueue (desc/buffer) and enqueue counter
 *      update (incremental) in SoC memory
 *
 * dev2soc_ret - SoC -> Peripheral direction enqueue (desc/buffer return) and enqueue counter
 *      update (incremental) in Peripheral memory.
 *
 * For DC Mode 2 (SW/SW with LitePath):
 *
 * Same as DC Mode 1 except its possible that the return ring is merged with the
 * ring for enqueue of Desc/Pkt. Counters of the Return ring will still be used
 * by DC counter update.
 *
 * SoC -> Peripheral Pkt direction (Tx)
 * -----------------------------------
 *
 * soc2dev - SoC -> Peripheral direction enqueue (desc/buffer) and enqueue counter
 *      update (incremental) in Peripheral memory
 *
 * soc2dev_ret - Peripheral -> SoC direction enqueue (desc/buffer return) and enqueue counter
 *      update (incremental) in SoC memory. This ring may not be allocated and
 *      then the return desc/pkt is written on the soc2dev ring.
 *
 * Above Ring Sizes are equal. Currently, GRX750/PUMA7 may allocate all	peripheral rings as equal
 *
 * Peripheral -> SoC Pkt direction (Rx)
 * -----------------------------------
 *
 * dev2soc - Peripheral -> SoC direction enqueue (desc/buffer) and enqueue counter
 *      update (incremental) in SoC memory
					  *
 * dev2soc_ret - SoC -> Peripheral direction enqueue (desc/buffer return) and enqueue counter
 *      update (incremental) in Peripheral memory. This ring may not be allocated and
 *      then the return desc/pkt is written on the dev2soc ring.
 * \note  soc2dev and soc2dev_ret rings must be of same size
 * \note  dev2soc and dev2soc_ret rings must be of same size
 */
struct dc_dp_ring_res {
	struct dc_dp_ring soc2dev; /*!< Params of Tx ring - SoC to Dev */
	struct dc_dp_ring soc2dev_ret; /*!< Params of Tx Return ring - SoC to Dev (Optional : Mode2)*/
	struct dc_dp_ring dev2soc;  /*!< Params of Rx ring - Dev to SoC */
	struct dc_dp_ring dev2soc_ret; /*!< Params of Rx Return ring - Dev to SoC (Optional : Mode0 / Mode2) */
	struct dc_dp_ring dev2soc_except; /*!< Params of Rx exception ring - Dev to SoC (Optional : Mode1 / Mode2) */
};

/**
 * \brief Direct Connect DC counters (Hardware based or FW/SW simulated) attributes.
 *
 * \note For SW Mode of WAVE500 today, dev2soc_base is a register in WAVE memory, but
 * recommended to write to SoC memory for generic handling for all periherals,
 * and for Driver to not have to read over PCIe which is slower..
 * \remark Some SoC Accelerators may need 2 separate counter update locations, while others need 1. Hence 2 distinct counter bases in Dev2SoC direction.
*/
/* FIXME : Is it better in more sturutured form? */
struct dc_dp_dccntr {
	int32_t soc_write_dccntr_mode; /*!< [in] DC Counter Write mode in peripheral memory : Incremental/Cumulative and Big/Little endian */
	int32_t dev_write_dccntr_mode; /*!< [out] Dc Counter Read mode from SoC memory: Incremental/Cumulative and Big/Little endian */

	void *soc2dev_enq_phys_base; /*!< [in] Physical Base address of DC Counter write location (in DC peripheral memory) for soc2dev ring enq counter. It MUST be provided by peripheral driver. */
	void *soc2dev_enq_base; /*!< [in] Base address of DC Counter write location (in DC peripheral memory) for soc2dev ring enq counter. */
	int32_t soc2dev_enq_dccntr_len; /*!< [in] DC Counter len (in bytes) to be written to soc2dev_enq_base. Optionally, it may include Tx and Rx return counters, where each counter is length of 4 bytes. */

	void *soc2dev_ret_deq_phys_base; /*!< [in] Physical Base address of DC Counter write location (in DC peripheral memory) for soc2dev_ret ring deq counter. It MUST be provided by peripheral driver. */
	void *soc2dev_ret_deq_base; /*!< [in] Base address of DC Counter write location (in DC peripheral memory) for soc2dev_ret ring deq counter. */
	int32_t soc2dev_ret_deq_dccntr_len; /*!< [in] DC Counter len (in bytes) to be written to soc2dev_ret_deq_base. */

	void *dev2soc_deq_phys_base; /*!< [in] Physical Base address of DC Counter write location (in DC peripheral memory) for dev2soc ring deq counter. It MUST be provided by peripheral driver. */
	void *dev2soc_deq_base; /*!< [in] Base address of DC Counter write location (in DC peripheral memory) for dev2soc ring deq counter. */
	int32_t dev2soc_deq_dccntr_len; /*!< [in] DC Counter len (in bytes) to be written to dev2soc_deq_base. */

	void *dev2soc_ret_enq_phys_base; /*!< [in] Physical Base address of DC Counter write location (in DC peripheral memory) for dev2soc_ret ring enq counter. It MUST be provided by peripheral driver. */
	void *dev2soc_ret_enq_base; /*!< [in] Base address of DC Counter write location (in DC peripheral memory) for dev2soc_ret ring enq counter. */
	int32_t dev2soc_ret_enq_dccntr_len; /*!< [in] DC Counter len (in bytes) to be written to dev2soc_ret_enq_base. */

	void *dev2soc_except_deq_phys_base; /*!< [in] Physical Base address of DC Counter write location (in DC peripheral memory) for dev2soc exception ring deq counter. It MUST be provided by peripheral driver. */
	void *dev2soc_except_deq_base; /*!< [in] Base address of DC Counter write location (in DC peripheral memory) for dev2soc exception ring deq counter. */
	int32_t dev2soc_except_deq_dccntr_len; /*!< [in] DC Counter len (in bytes) to be written to dev2soc_except_deq_base. */

	void *soc2dev_deq_phys_base; /*!< [out] Physical Base address of DC Counter read location (in SoC memory) for soc2dev ring deq counter - Optional based on DCMode0,1 etc. */
	void *soc2dev_deq_base; /*!< [out] Base address of DC Counter read location (in SoC memory) for soc2dev ring deq counter - Optional based on DCMode0,1 etc. */
	int32_t soc2dev_deq_dccntr_len; /*!< [out] DC Counter len (in bytes) to be read from soc2dev_deq_base. */

	void *soc2dev_ret_enq_phys_base; /*!< [out] Physical Base address of DC Counter read location (in SoC memory) for soc2dev_ret ring enq counter - Optional based on DCMode0,1 etc. */
	void *soc2dev_ret_enq_base; /*!< [out] Base address of DC Counter read location (in SoC memory) for soc2dev_ret ring enq counter - Optional based on DCMode0,1 etc. */
	int32_t soc2dev_ret_enq_dccntr_len; /*!< [out] DC Counter len (in bytes) to be read form soc2dev_ret_enq_base. */

	void *dev2soc_enq_phys_base; /*!< [out] Physical Base address of DC Counter read location (in SoC memory) for dev2soc ring enq counter - Optional based on DCMode0,1 etc. */
	void *dev2soc_enq_base; /*!< [out] Base address of DC Counter read location (in SoC memory) for dev2soc ring enq counter - Optional based on DCMode0,1 etc. */
	int32_t dev2soc_enq_dccntr_len; /*!< [out] DC Counter len (in bytes) to be read from dev2soc_enq_base. */

	void *dev2soc_ret_deq_phys_base; /*!< [out] Physical Base address of DC Counter read location (in SoC memory) for dev2soc_ret ring deq counter - Optional based on DCMode0,1 etc */
	void *dev2soc_ret_deq_base; /*!< [out] Base address of DC Counter read location (in SoC memory) for dev2soc_ret ring deq counter - Optional based on DCMode0,1 etc. */
	int32_t dev2soc_ret_deq_dccntr_len; /*!< [out] DC Counter len (in bytes) to be read from dev2soc_ret_deq_base. */

	void *dev2soc_except_enq_phys_base; /*!< [in/out] Physical Base address of DC Counter read location (in Peripheral/SoC memory) for dev2soc exception ring enq counter - Optional based on DCMode0,1 etc. */
	void *dev2soc_except_enq_base; /*!< [in/out] Base address of DC Counter read location (in Peripheral/SoC memory) for dev2soc exception ring enq counter - Optional based on DCMode0,1 etc. */
	int32_t dev2soc_except_enq_dccntr_len; /*!< [in/out] DC Counter len (in bytes) to be read from dev2soc_except_enq_base. */

	int32_t flags; /*!< Reserved */
};

/**
   \brief DirectConnect Datapath Lib Resource structure.
	Caller needs to allocate the DC counter structure array.
*/
struct dc_dp_res {
	uint32_t num_bufs_req; /*!< [in] Number of buffers to allocate (Directconnect Peripheral -> SoC direction) */
	int32_t num_bufpools; /*!< [out]  Number of buffer pools/chunks allocated for the desired no of buffers  */
	struct dc_dp_buf_pool *buflist; /*!< [out] Allocated list of buffer chunks from which packet buffers are carved out.
									Caller needs to free the memory given by buflist pointer. */

	uint32_t tx_num_bufs_req; /*!< [in] Number of buffers to allocate (Soc -> Directconnect Peripheral direction) */
	int32_t tx_num_bufpools; /*!< [out]  Number of buffer pools/chunks allocated for the desired no of buffers  */
	struct dc_dp_buf_pool *tx_buflist; /*!< [out] Allocated list of buffer chunks from which packet buffers are carved out.
									Caller needs to free the memory given by buflist pointer. */
	uint32_t tx_desc_start_idx; /*!< [out] Tx Prefill counter to be set */

	struct dc_dp_ring_res rings; /*!< [in/out] All the communication rings depending on DCMode0/DCMode1/SWModes */
	int32_t		num_dccntr; /*!< [in] Number of DC counter units used - Could be HW or SW . Eg. VRX518 DSL Bonding will use 2 x DC counter. */
	struct dc_dp_dccntr	*dccntr; /*!< [in/out] array of number of DC counter structures. Caller needs to allocate and free memory of dccntr pointer. */
	void		*dev_priv_res; /*!< [in] Pointer to any dev specific resource structure */
};

/**
   \brief DirectConnect Datapath Lib Counter write/read mode structure.
*/
struct dc_dp_counter_mode {
	uint32_t soc2dev_write;	/*!< Supported DC Counter write mode, as defined in DC_DP_F_DCCNTR_MODE flags */
	uint32_t dev2soc_write;	/*!< Supported DC Counter read mode, as defined in DC_DP_F_DCCNTR_MODE flags */
};

/**
   \brief DirectConnect Datapath Lib SoC capability structure.
*/
/* FIXME : can we use same object for all path? */
struct dc_dp_host_cap {
	uint32_t version;			/*!< DC API version */
	struct dc_dp_fastpath {
		int32_t support;		/*!< Supported DC FASTPATH or not? */
		enum dc_dp_mode_type hw_dcmode;	/*!< Supported HW DC Mode 0 or DC Mode 1 */
		struct dc_dp_counter_mode hw_cmode; /*!< Supported DC Counter read/write mode : BIG and/or LITTLE, Incremental and/or cumulative */
		uint32_t hw_cap;		/*!< Supported capability in HW as defined in DC_DP_F_CAP_* flags */
	} fastpath;
	struct dc_dp_litepath {
		int32_t support;		/*!< Supported DC LITEPATH or not? */
		uint32_t hw_cap;		/*!< Supported capability in HW as defined in DC_DP_F_CAP_* flags */
	} litepath;
	struct dc_dp_swpath {
		int32_t support; 		/*!< Supported DC SWPATH or not? */
		enum dc_dp_mode_type sw_dcmode;	/*!< Supported SW DC Mode 1 or any other, if exists */
		struct dc_dp_counter_mode sw_cmode; /*!< Supported DC Counter read/write mode : BIG and/or LITTLE, Incremental and/or cumulative */
		uint32_t sw_cap;		/*!< Supported capability in SW as defined in DC_DP_F_CAP_* flags */
	} swpath;
};

/* @} */


/** \addtogroup DirectConnect_Datapath_Lib_APIs */
/* @{ */

/** \brief  DirectConnect Datapath Allocate Data structure port may map to an exclusive netdevice
   \param[out] cap  Pointer to DC SoC capability structure
   \param[in] flags  : Reserved
   \return  0 if OK / -1 if error
*/
/* FIXME : can we add port specific capability as well? */
int32_t
dc_dp_get_host_capability (
	struct dc_dp_host_cap *cap,
	uint32_t flags
);

/** \brief  DirectConnect Datapath Allocate Data structure port may map to an exclusive netdevice
    like in the case of ethernet LAN ports.	In other cases like WLAN, the physical port is a Radio port,
    while netdevices are Virtual Access Points (VAPs). In this case, the AP netdevice can be passed.
    Alternately, driver_port & driver_id will be used to identify this port.
   \param[in] owner  Kernel module pointer which owns the port
   \param[in] dev_port  Physical Port Number of this device managed by the driver
   \param[in] dev  Pointer to Linux netdevice structure (optional)
   \param[in] port_id  Optional port_id number requested. Usually, 0 and allocated by driver
   \param[in] flags  : Use Datapath driver flags for Datapath Port Alloc
	-  DC_DP_F_FASTPATH : Allocate the port as h/w acclerable
	-  DC_DP_F_FAST_DSL : Allocate the DSL port as h/w acclerable
	-  DC_DP_F_LITEPATH : Allocate the port as partial accelerable/offload
	-  DC_DP_F_SWPATH : Allocate the port as non-accelerable
	-  DC_DP_F_MULTI_PORT : Allocate the port as multiport device
	-  DC_DP_F_SHARED_RES : Allocate the port as shared resources
	-  DC_DP_F_DEREGISTER : Deallocate the already allocated port
   \return  Returns allocated Port number for given netdevice. / -1 if error
*/
int32_t
dc_dp_alloc_port (
	struct module *owner,
	uint32_t dev_port,
	struct net_device *dev,
	int32_t port_id,
	uint32_t flags
);

/** \brief  Higher Layer Driver reservation API
   \param[in] owner  Kernel module pointer which owns the port
   \param[in] port_id  Datapath Port Identifier on which to reserve
   \param[in] dev  Pointer to Linux netdevice structure (Optional)
   \param[in] dev_req Peripheral Device Requirements and Capability, as defined in DC_DP_F_DEV_* flags
   \param[in] flags  : Special input flags to reserve mode routine
              - DC_DP_F_DEREGISTER : Free the reservation, if any.
   \return  0 if OK / -1 if error
   \note Optionally, DC peripheral driver may invoke this API to reserve DC fastpath mode.
*/
int32_t
dc_dp_reserve_fastpath (
	struct module *owner,
	int32_t port_id,
	struct net_device *dev,
	uint32_t dev_req,
	uint32_t flags
);

/** \brief  Higher layer Driver Datapath registration API
   \param[in] owner  Kernel module pointer which owns the port
   \param[in] port_id  Datapath Port Identifier on which to register
   \param[in] dev  Pointer to Linux netdevice structure
   \param[in] datapathcb  Callback registration structure
   \param[in,out] resources  Buffer, Tx/Rx ring and DC counter resources.
   \param[in,out] devspec  Device Specification - netdevice name, DC Mode, Context Pointer.
   \param[in] flags  : Special input flags to register device routine
		- DC_DP_F_DEREGISTER : Deregister the device
		- DC_DP_F_ALLOC_SW_TX_RING : Specify it if peripheral driver want to allocate sw tx ring
		- DC_DP_F_ALLOC_SW_RX_RING : Specify it if peripheral driver want to allocate sw rx ring
		- DC_DP_F_DONT_ALLOC_HW_TX_RING : Specify it if peripheral driver don't want to allocate hw tx ring
		- DC_DP_F_DONT_ALLOC_HW_RX_RING : Specify it if peripheral driver don't want to allocate hw rx ring
		- DC_DP_F_QOS : Specify it if peripheral/dev QoS class is required in DC peripheral
	        - DC_DP_F_SHARED_RES : Specify it if peripheral driver want to re-use/update dc resources, multi-port/bodning case respectively
   \return 0 if OK / -1 if error
   \note This is the first registration to be invoked by any DC peripheral. Subsequently additional registrations like Multicast, Ring-Recovery or Power Saving (PS) to be done.
*/
int32_t
dc_dp_register_dev (
	struct module *owner,
	uint32_t port_id,
	struct net_device *dev,
	struct dc_dp_cb *datapathcb,
	struct dc_dp_res *resources,
	struct dc_dp_dev *devspec,
	uint32_t flags
);

/** \brief  Allocates datapath subif number to a sub-interface netdevice.
    Sub-interface value must be passed to the driver.
    The port may map to an exclusive netdevice like in the case of ethernet LAN ports.
   \param[in] owner  Kernel module pointer which owns the port
   \param[in] dev  Pointer to Linux netdevice structure. Optional for VRX518(ATM) driver.
   \param[in] subif_name  Pointer to device name, this is MUST if Linux netdevice is NULL.
   \param[in,out] subif_id  Pointer to subif_id structure including port_id
   \param[in] flags :
       DC_DP_F_DEREGISTER - De-register already registered subif/vap
       DC_DP_F_REGISTER_LITEPATH - Register already registered subif/vap as LitePath Partial offload
       DC_DP_F_DEREGISTER_LITEPATH - De-register already registered LitePath subif/logical netdev from LitePath
   \return 0 if OK / -1 if error
   \note Sub-Interface is applicable for logical or virtual interfaces like VAP (SSID) or VLAN.
   \note LITEPATH register/deregister only works on Partial offload or NOT, does deregister subif.
   \note if subinterface has to be deregistered completely, DC_DP_F_DEREGISTER flag must be passed.
*/
int32_t
dc_dp_register_subif (
	struct module *owner,
	struct net_device *dev,
	const uint8_t *subif_name,
	struct dp_subif *subif_id,
	uint32_t flags
);

/** \brief  Transmit packet to low-level Datapath driver
   \param[in] rx_if  Rx If netdevice pointer (optional)- MUST be set when received net_device is known.
   \param[in] rx_subif  Rx SubIf pointer (Optional) - MUST be set when atleast received {PortId, <SubifId>} are known.
   \param[in] tx_subif  Tx SubIf pointer
   \param[in] skb  Pointer to packet buffer like sk_buff
   \param[in] len  Length of packet to transmit
   \param[in] flags :
       DC_DP_F_XMIT_LITEPATH - send packet through litepath
       DC_DP_F_XMIT_OAM - Send packet through ATM only
   \return 0 if OK / -1 if error
   \note Either rx_if or rx_subif would be passed in this routine if LitePath
   \note skb is freed by underlying layer if error
*/
int32_t
dc_dp_xmit (
	struct net_device *rx_if,
	struct dp_subif *rx_subif,
	struct dp_subif *tx_subif,
	struct sk_buff *skb,
	int32_t len,
	uint32_t flags
);

/** \brief  Handle Tx-Confirm and/or Rx interrupts (for SWPATH - w/ or w/o LITEPATH only)
   \param[in] owner  Kernel module pointer which owns the port.
   \param[in] port_id  Datapath Port Id (PMAC Port No).
   \param[in] dev  Pointer to Linux netdevice structure - (Optional).
   \param[in] ring Pointer to Ring  which needs processing.
   \param[in] flags  Special input flags to interrupt handling routine
		- DC_DP_F_TX_COMPLETE : To handle Tx return interrupt
		- DC_DP_F_RX : To handle Rx interrupt
		- DC_DP_F_RX_FRAG_EXCEPT : To handle Rx exception (fragmentation) interrupt
   \return 0 if OK / -1 if error
   \note This processing will be called from a tasklet (Device specific)
*/
int32_t
dc_dp_handle_ring_sw (
	struct module *owner,
	uint32_t port_id,
	struct net_device *dev,
	struct dc_dp_ring *ring,
	uint32_t flags
);

/** \brief  Allocate skbuff buffer in DDR/SRAM
   \param[in] len  Length of the buffer required
   \param[in] subif  Pointer to sub-interface for which allocation is requried
   \param[in] flags  Reserved
   \return skb pointer if OK / NULL if error
*/
struct sk_buff* dc_dp_alloc_skb (
	uint32_t len,
	struct dp_subif *subif,
	uint32_t flags
);

/** \brief  Free the allocated buffer
   \param[in] subif  Pointer to sub-interface for which free is requried
   \param[in] skb  Pointer to packet sk_buff structure to be freed
   \return 0 if OK / -1 if error
*/
int32_t dc_dp_free_skb (
	struct dp_subif *subif,
	struct sk_buff *skb
);

/** \brief  Allocate buffers in HW/FW/SW
   \param[in] subif  Pointer to sub-interface for which allocation is requried
   \param[in] buf_len  Length of the buffer required
   \param[in] num_bufs_req  Number of buffers to allocate
   \param[out] num_bufpools  Number of buffer pools/chunks allocated for the desired no of buffers
   \param[out] buflist  Allocated list of buffer chunks from which packet buffers are carved out.
                        Caller needs to free the memory given by buflist pointer.
   \param[in] dir  direction, where enum values are as descrbed in enum dc_dp_dir_type.
   \param[in] flags  Reserved
   \return 0 if OK / -1 if error
*/
/* FIXME : Any use? */
int32_t dc_dp_alloc_bufs (
	struct dp_subif *subif,
	uint32_t buf_len,
	uint32_t num_bufs_req,
	uint32_t *num_bufpools,
	struct dc_dp_buf_pool **buflist,
	enum dc_dp_dir_type dir,
	uint32_t flags
);

/** \brief  Free the allocated buffers from HW/FW/SW
   \param[in] subif  Pointer to sub-interface for which allocation is requried
   \param[in] num_bufpools  Number of buffer pools/chunks allocated for the desired no of buffers
   \param[in] buflist  Allocated list of buffer chunks from which packet buffers are carved out.
                        Caller needs to free the memory given by buflist pointer.
   \param[in] dir  direction, where enum values are as descrbed in enum dc_dp_dir_type.
   \param[in] flags  Reserved
   \return 0 if OK / -1 if error
*/
/* FIXME : Any use? */
int32_t dc_dp_free_bufs (
	struct dp_subif *subif,
	uint32_t num_bufpools,
	struct dc_dp_buf_pool *buflist,
	enum dc_dp_dir_type dir,
	uint32_t flags
);

/** \brief  Allow Acceleration subsystem to learn about session when driver shortcuts
   forwarding without going to stack.
   \param[in] subif  Pointer to sub-interface for which free is requried
   \param[in] skb  Pointer to packet with skb->dev pointing to RxIf in PREFORWARDING
	   and TxIf in POSTFORWARDING
   \param[in] flags : DC_DP_F_PREFORWARDING/DC_DP_F_POSTFORWARDING
   \return 0 if OK / -1 if error
   \remark skb->dev needs to be filled based on Rx and Tx Netif in the call
   \remark Peripheral driver needs to call once in PREFORWARDING and once in
   POSTFORWARDING
   \remark this will call PPA/HIL learning hook as required
*/
int32_t dc_dp_add_session_shortcut_forward (
	struct dp_subif *subif,
	struct sk_buff *skb,
	uint32_t flags
);

/** \brief  Set port id in Host SoC DMA Descriptor.
   \param[in,out] skb  Pointer to sk_buff structure
   \param[in] port_id  port identifier value
   \param[in] flags  Reserved
   \return  0 if OK / -1 if error
   \note  This utility function will set and return DMA Decriptor's from given port value.
*/
static inline int32_t
dc_dp_set_ep_pkt (
	struct sk_buff *skb,
	int32_t	port_id,
	uint32_t flags
)
{
	struct dma_tx_desc_1 *desc_1;

	if (!skb) {
		return DC_DP_FAILURE;
	}

	desc_1 = (struct dma_tx_desc_1 *) &skb->DW1;
	desc_1->field.ep = port_id;

	return DC_DP_SUCCESS;
}

/** \brief  Get port id from DMA Desc DWORD 1 (DW1).
   \param[in] skb  Pointer to sk_buff structure
   \param[out] port_id  Port identifier value
   \param[in] flags  : Reserved
   \return  0 if OK / -1 if error
   \note This utility funciton will extract port identifier from given DMA Descriptor's DWord-1.
*/
static inline int32_t
dc_dp_get_ep_pkt (
	struct sk_buff *skb,
	int32_t *port_id,
	uint32_t flags
)
{
	struct dma_tx_desc_1 *desc_1;

	if (!skb || !port_id) {
		return DC_DP_FAILURE;
	}

	desc_1 = (struct dma_tx_desc_1 *) &skb->DW1;
	*port_id = desc_1->field.ep;

	return DC_DP_SUCCESS;
}

/** \brief  Set sub-interface id in DMA Desc DWORD 0 (DW0).
   \param[in] port_id  port identifier value
   \param[in,out] skb  Pointer to sk_buff structure
   \param[in] subif_id  subinterface id value
   \param[in] flags : Reserved
   \return  0 if OK / -1 if error
   \note This funciton will return DMA Descriptior's DWord-0 by setting the given sub-interface id value.
*/
static inline int32_t
dc_dp_set_subifid_pkt (
	int32_t	port_id,
	struct sk_buff *skb,
	int32_t subif_id,
	uint32_t flags
)
{
	struct dma_tx_desc_0 *desc_0;

	if (!skb)
		return DC_DP_FAILURE;

	desc_0 = (struct dma_tx_desc_0 *) &skb->DW0;
	desc_0->field.dest_sub_if_id = subif_id;

	return DC_DP_SUCCESS;
}

/** \brief  Get sub-interface id from DMA Desc DWORD 0 (DW0).
   \param[in] port_id  port identifier value
   \param[in] skb  Pointer to sk_buff structure
   \param[out] subif_id  subinterface id value
   \param[in] flags  : Reserved
   \return  0 if OK / -1 if error
   \note This utility function will extract sub-interface id from given DMA Descriptor DWord-0.
*/
static inline int32_t
dc_dp_get_subifid_pkt (
	int32_t	port_id,
	struct sk_buff *skb,
	int32_t *subif_id,
	uint32_t flags
)
{
	struct dma_tx_desc_0 *desc_0;

	if (!skb || !subif_id)
		return DC_DP_FAILURE;

	desc_0 = (struct dma_tx_desc_0 *) &skb->DW0;
	*subif_id = desc_0->field.dest_sub_if_id;

	return DC_DP_SUCCESS;
}

/** \brief  Set complete mark into DMA Desc DWORD 3 (DW3).
   \param[in] port_id  port identifier value
   \param[in,out] skb  Pointer to sk_buff structure
   \param[in] flags  : Reserved
   \return  0 if OK / -1 if error
   \note This utility macro will set complete mark into DMA Descriptor DWord-3,
   to indicate that the peripheral specific desc fileds are all set.
*/
static inline int32_t
dc_dp_set_mark_pkt (
	int32_t	port_id,
	struct sk_buff *skb,
	uint32_t flags
)
{
	struct dma_tx_desc_3 *desc_3;

	if (!skb)
		return DC_DP_FAILURE;

	desc_3 = (struct dma_tx_desc_3 *) &skb->DW3;
	desc_3->field.c = 1;

	return DC_DP_SUCCESS;
}

/** \brief  Get special mark from DMA Desc DWORD 3 (DW3).
   \param[in] port_id  port identifier value
   \param[in] skb  Pointer to sk_buff structure
   \param[in] flags  : Reserved
   \return  1 if already marked / 0 if not marked / -1 if error
   \note This utility macro will get complete mark into DMA Descriptor DWord-3,
   to indicate that the peripheral specific desc fileds are all set.
*/
static inline int32_t
dc_dp_get_mark_pkt (
	int32_t	port_id,
	struct sk_buff *skb,
	uint32_t flags
)
{
	struct dma_tx_desc_3 *desc_3;

	if (!skb)
		return DC_DP_FAILURE;

	desc_3 = (struct dma_tx_desc_3 *) &skb->DW3;
	return (desc_3->field.c == 1) ? 1 : 0;
}

/** \brief  Disconnect a particular MAC addr or an network device - remove all MAC table entries
    and/or routing sessions which use the specified MAC address.
   \param[in] netif  netdevice pointer through which all entries must be removed from acceleration - optional
   \param[in] subif_id  Sub-interface identifier to remove on netif
   \param[in] mac_addr  MAC address to remove
   \param[in] flags  Reserved for future use
   \return 0 if OK / -1 if error
   \note One of subif_id, mac_addr or netif must be specified
*/
int32_t
dc_dp_disconn_if (
	struct net_device *netif,
	struct dp_subif *subif_id,
	uint8_t mac_addr[MAX_ETH_ALEN],
	uint32_t flags
);

/** \brief  Register/De-register a DirectConnect interface to MCAST helper module
   \param[in] dev  Net device to be registered, e.g., wlan0_0.
   \param[in] owner  Kernel module pointer which owns the port.
   \param[in] cb  Multicast callback function.
   \param[in] flags  :
       DC_DP_F_MC_REGISTER - Register a DirectConnect interface.
       DC_DP_F_MC_DEREGISTER - De-register already registered DirectConnect interface.
       DC_DP_F_MC_UPDATE_MAC_ADDR - Register a DirectConnect interface to get mcast SSM support.
       DC_DP_F_MC_FW_RESET - Cleanup request on already learned entries on the registered DirectConnect interface.
       DC_DP_F_MC_NEW_STA - Re-learn request on the registered DirectConnect interface.
   \return 0 if OK / -1 if error
   \note It can be skipped for specific peripheral driver if there is no notion
   of host connect/disconnect on the peripheral network/interface.
   \note Eg. WIFI has station association and disassociation which can be used
*/
int32_t
dc_dp_register_mcast_module (
	struct net_device *dev,
	struct module *owner,
	dc_dp_mcast_callback_fn_t cb,
	uint32_t flags
);

/** \brief  Provide a Priority (802.1D Priority) to Device QoS/WMM Class/TID map for the
 * given WiFi Radio/net_device
   \param[in] port_id  Port Id on which mapping is to be updated
   \param[in] netif  Pointer to stack network interface structure on which mapping is to be updated
   \param[in] prio2qos Array of priority to QoS/WMM Class/TID mapping values
   \return 0 if OK / -1 if error
   \note  One of port_id or netif must be specified
   \note The DC driver must  configure the Egress PMAC table to mark the WMM Class/TID in the descriptor DW1[7:4]
*/
int32_t
dc_dp_map_prio_devqos_class (
	int32_t	port_id,
	struct net_device *netif,
	uint8_t prio2qos[DC_DP_MAX_DEV_CLASS]
);

/** \brief  Mark Dev QoS Class/WMM AC/TID in given packet
   \param[in] port_id  Port Id on which mapping is to be updated
   \param[in] dst_netif  Pointer to stack network interface structure
   \param[in] skb  : pointer to network packet/sk_buff structure
   \return  Dev Class/WMM value marked if successful, -1 on error
   \note The WAVE/ DC Datapath driver needs to use skb->extmark/skb->priority to mark and return WMM Class/TID
*/
int32_t
dc_dp_mark_pkt_devqos (
	int32_t	port_id,
	struct net_device *dst_netif,
	struct sk_buff *skb
);

/** \brief  Get DirectConnect interface statistics. Either netdevice or subif_id has to be passed to this API.
   \param[in] netif  Pointer to Linux netdevice structure
   \param[in] subif_id  Datapath Port Number and Sub-Interface (if applicable else -1).
   \param[out] if_stats  Pointer to Linux rtnl_link_stats64 structure
   \param[in] flags  :
       DC_DP_F_SUBIF_LOGICAL - to be used when the interface is subif type
   \return 0 if OK / -1 if error
*/
int32_t
dc_dp_get_netif_stats (
	struct net_device *netif,
	struct dp_subif *subif_id,
	struct rtnl_link_stats64 *if_stats,
	uint32_t flags
);

/** \brief  Clear DirectConnect interface statistics. Either netdevice or subif_id has to be passed.
   \param[in] netif  Pointer to Linux netdevice structure
   \param[in] subif_id  Datapath Port Number and Sub-Interface (if applicable else -1).
   \param[in] flags  Flag Type to pass additional info such as
       DC_DP_F_SUBIF_LOGICAL - to be used when the interface is subif type
   \return 0 if OK / -1 if error
*/
int32_t
dc_dp_clear_netif_stats (
	struct net_device *netif,
	struct dp_subif *subif_id,
	uint32_t flags
);

/**
 * \brief Disable or Enable Enqueue and/or Dequeue to/from peripheral in SoC.
   \param[in] port_id  Datapath Port Id
   \param[in] flags  : What to enable or disable
		DC_DP_F_ENQ_ENABLE,
		DC_DP_F_ENQ_DISABLE,
		DC_DP_F_DEQ_DISABLE,
		DC_DP_F_DEQ_ENABLE
   \return 0 if OK / -1 if error
 */
int32_t
dc_dp_change_dev_status_in_soc (
	int32_t port_id,   /* Port Identifier */
	uint32_t flags
);

/**
 * \brief Flush the ring.
   \param[in] owner  Kernel module pointer which owns the port
   \param[in] port_id  Datapath Port Id
   \param[in] flags  : Reserved
   \return 0 if OK / -1 if error
 */
/* FIXME : Any use? */
int32_t
dc_dp_flush_ring (
	struct module *owner,
	int32_t port_id,
	uint32_t flags
);

/** \brief  Get Wake-on-LAN configuration.
   \param[in] port_id  Datapath Port Id
   \param[out] cfg  Wake-on-LAN Configuration
   \param[in] flags  : Reserved
   \return 0 if OK / -1 if error
*/
int32_t
dc_dp_get_wol_cfg (
	int32_t port_id,
	struct dc_dp_wol_cfg *cfg,
	uint32_t flags
);

/** \brief  Set Wake-on-LAN config
   \param[in] port_id  Datapath Port Id
   \param[in] cfg  Wake-on-LAN Configuration
   \param[in] flags  : Reserved
   \return  0 if OK / -1 if error
*/
int32_t
dc_dp_set_wol_cfg (
	int32_t port_id,
	struct dc_dp_wol_cfg *cfg,
	uint32_t flags
);

/** \brief  Enable/Disable Wake-on-LAN functionality
   \param[in] port_id  Datapath Port Id
   \param[in] enable  Enable Wake-on-LAN if 1, disable Wake-on-LAN if 0
   \return  0 if OK / -1 if error
*/
int32_t
dc_dp_set_wol_ctrl (
	int32_t port_id,
	uint32_t enable
);

/** \brief  Get Wake-on-LAN status
   \param[in] port_id  Datapath Port Id
   \return  1 if Enabled/ 0 if Disabled / -1 if error
*/
int32_t
dc_dp_get_wol_ctrl_status (
	int32_t port_id
);

/* @} */


/** \addtogroup DirectConnect_Datapath_Lib_Power_Management_Wrapper_API. FIXME : Can be made more generic and simplified. 2nd phase */
/* @{ */

/** \brief  Register/De-register to/from the CPUFreq framework
   \param[in] nb  Notifier function to register
   \param[in] notify_type  Notifier list type (DC_DP_PS_TRANSITION_NOTIFIER / DC_DP_PS_POLICY_NOTIFIER)
   \param[in] flags  :
	-  DC_DP_F_PS_REGISTER : Register to CPUFreq framework
	-  DC_DP_F_PS_DEREGISTER : De-register from CPUFreq framework
   \return  0 if OK / < 0 if error
*/
int32_t
dc_dp_register_power_notifier (
	struct notifier_block *nb,
	uint32_t notify_type,
	uint32_t flags
);

/** \brief  Request new system power state
   \param[in] module  Module identifier
   \param[in] module_nr Module Number
   \param[in] new_state  Power state as defined in dc_dp_power_state
   \return  DC_DP_PS_SUCCESS / DC_DP_PS_DENIED / DC_DP_PS_NOACTIVITY / DC_DP_PS_NOTDEFINED
*/
int32_t
dc_dp_req_power_state (
	enum dc_dp_power_module module,
	uint8_t module_nr,
	enum dc_dp_power_state new_state
);

/** \brief  Get Power Mgmt polling period per module
   \param[in] module  Module identifier
   \param[in] module_nr
   \return  Polling period
*/
int32_t
dc_dp_get_ps_poll_period (
	enum dc_dp_power_module module,
	uint8_t module_nr
);

/** \brief  Get power state related threshold values per module
   \param[in] module  Module identifier
   \param[in] module_nr
   \return  dc_dp_ps_threshold pointer if OK / NULL if error
*/
struct dc_dp_ps_threshold *
dc_dp_get_ps_threshold (
	enum dc_dp_power_module module,
	uint8_t module_nr
);

/** \brief  Add/Delete module from CPUFReq module list
   \param[in] head  Module list head pointer
   \param[in] add  DC_DP_PS_LIST_ADD / DC_DP_PS_LIST_DEL
   \return  0 if OK / < 0 if error
*/
int32_t
dc_dp_mod_ps_list (
	struct list_head *head,
	int32_t add
);

/** \brief  Convert frequency to power state
   \param[in] freq_khz frequency (in khz)
   \return  power state if OK / < 0 if error
*/
enum dc_dp_power_state
dc_dp_get_ps_from_khz (
	uint32_t freq_khz
);

/* @} */

#endif /* _UGW_DIRECTCONNECT_DP_API_H_ */

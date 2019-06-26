#ifndef _DTI_STATISTIC_IF_H
#define _DTI_STATISTIC_IF_H
/******************************************************************************

                              Copyright (c) 2015
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Status and Statistic definitions and declarations.
*/

/** \defgroup DTI_STATISTIC_IF DTI Status and Statistics

   This Group contains definitions of the Status and Statistic specific
   part of the Debug and Trace Interface (DTI) Protocol.

\ingroup DTI_PROTOCOL_COMMON
*/

#include "dti_agent_interface.h"
#include "dti_cli.h"

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   Includes
   ========================================================================= */

/** \addtogroup DTI_STATISTIC_IF
   *{ */

#define DTI_STAT_DEV_IF_NAME_LEN	16
#define DTI_STAT_CLI_IF_NAME_LEN	16

/**
   Contains the DTI System Information
*/
struct ltq_dti_system_info_s
{
	/** DTI Core version: 0xMa:Mi:St:Bu (<Major>:<Minor>:<Step>:<Build>)
	    --> DTI_AGENT_VER_STR */
	unsigned int core_version;
	/** DTI Protocol version: 0xMa:Mi:St:00 (<Major>:<Minor>:<Step>:00)
	    --> DTI_PROTOCOL_VER_STR */
	unsigned int protocol_version;

	/** Max packet length
	  Todo: clarify, DTI_MAX_PACKET_QUEUE_SIZE */
	unsigned int pkt_queue_size_byte;

	/** max number of worker threads
	    --> DTI_MAX_NUM_OF_WORKER */
	unsigned int max_worker;

	/** max number of device families
	    --> DTI_MAX_DEVICE_INTERFACES */
	unsigned int max_device_interfaces;

	/** max number of CLI interfaces
	    --> DTI_CLI_MAX_CLI_INTERFACES */
	unsigned int max_cli_interfaces;

	/** List of all device interfaces
	    Todo: check array len against "DTI_MAX_LEN_DEVICE_INTERFACE_NAME" */
	char dev_if_name[DTI_MAX_DEVICE_INTERFACES][DTI_STAT_DEV_IF_NAME_LEN];

	/** List of all CLI interfaces */
	struct {
		/** name of the CLI interface (registered by user)
		    Todo: check array len against "DTI_CLI_MAX_NAME_LEN" */
		char cli_if_name[DTI_STAT_CLI_IF_NAME_LEN];
		/** responce buffer size of this CLI interface */
		unsigned int responce_buffer_size;
	} cli_if[DTI_CLI_MAX_CLI_INTERFACES];

};


/** DTI Worker - statistics */
struct ltq_dti_status_ip_port_v4_s
{
	/** IP addr */
	unsigned int ipaddr;
	/** port */
	unsigned short ipport;
	/** padding */
	unsigned short pad;
};

/** DTI Agent - statistics */
struct ltq_dti_status_agent_s
{
	/** configure / valid indication */
	unsigned int b_valid;

	/** current worker */
	unsigned int curr_worker;
	/** current device interfaces */
	unsigned int curr_dev_if;
	/** current CLI interfaces */
	unsigned int curr_cli_if;
	/** CLI Event handling enabled */
	unsigned int cli_glb_evt_enabled;

	/** listener - IP:port */
	struct ltq_dti_status_ip_port_v4_s listen_ip;
};

/** DTI Worker - statistics */
struct ltq_dti_status_worker_thr_s
{
	/** worker_thr - thread state */
	int thr_state;

	/** worker - remote IP:port */
	struct ltq_dti_status_ip_port_v4_s remote_ip;

	/** CLI Event handling enabled */
	unsigned int cli_evt_enabled;
};

/** DTI Agent - statistics */
struct ltq_dti_statistic_agent_s
{
	/** Listener - connection request */
	unsigned int conn_req_listen_req_cnt;
	/** Listener - connection request done */
	unsigned int conn_req_listen_req_done_cnt;
	/** Listener - connection request failed (no worker) */
	unsigned int conn_req_listen_req_fail_cnt;
};


/** DTI Worker - statistics */
struct ltq_dti_statistic_worker_thr_s
{
	/** worker_thr - startup request count */
	unsigned int start_req_cnt;
	/** worker_thr - startup done count */
	unsigned int start_done_cnt;
	/** worker_thr - startup failed count */
	unsigned int start_fail_cnt;
	/** worker_thr - close count */
	unsigned int close_cnt;
};


/** DTI Worker - statistics */
struct ltq_dti_statistic_connection_s
{
	/** connection - incoming DTI packets */
	unsigned int packet_in_cnt;
	/** connection - outgoing DTI packets */
	unsigned int packet_out_cnt;
	/** connection - outgoing DTI packets send failed */
	unsigned int packet_out_err_cnt;
	/** connection - outgoing DTI packets discarded */
	unsigned int packet_out_discard_cnt;

	/** connection - incoming DTI Cntrl packets */
	unsigned int packet_in_cntrl_cnt;
	/** connection - outgoing DTI Cntrl packets */
	unsigned int packet_out_cntrl_cnt;

	/** connection - incoming DTI CLI packets */
	unsigned int packet_in_cli_cnt;
	/** connection - outgoing DTI CLI packets */
	unsigned int packet_out_cli_cnt;
	/** connection - outgoing DTI CLI Event packets */
	unsigned int packet_out_cli_evt_cnt;
	/** connection - outgoing DTI CLI Event packets */
	unsigned int packet_out_cli_evt_discard_cnt;

	/** connection - incoming DTI CLI Control packets */
	unsigned int packet_in_cli_cntrl_cnt;
	/** connection - outgoing DTI CLI Control packets */
	unsigned int packet_out_cli_cntrl_cnt;

	/** connection - incoming DTI Device packets */
	unsigned int packet_in_dev_cnt;
	/** connection - outgoing DTI Device packets */
	unsigned int packet_out_dev_cnt;
	/** connection - outgoing DTI Device Event packets */
	unsigned int packet_out_dev_evt_cnt;

	/** connection - incoming DTI packets - unknown */
	unsigned int packet_in_unknown_cnt;
	/** connection - outgoing DTI packets */
	unsigned int packet_out_unknown_cnt;

	/** connection - re-sync trigger */
	unsigned int conn_resync_cnt;
	/** connection - close trigger */
	unsigned int conn_close_cnt;

};

/**
   Contains the DTI System Information
*/
struct ltq_dti_system_status_s
{
	/** Agent Status */
	struct ltq_dti_status_agent_s status_agent;
	/** Agent Statistics */
	struct ltq_dti_statistic_agent_s stat_agent;

	/** Worker Status */
	struct ltq_dti_status_worker_thr_s status_worker[DTI_MAX_NUM_OF_WORKER];
	/** Worker Statistics */
	struct ltq_dti_statistic_worker_thr_s stat_worker_thr[DTI_MAX_NUM_OF_WORKER];
	/** Connection Statistics (worker) */
	struct ltq_dti_statistic_connection_s stat_conn[DTI_MAX_NUM_OF_WORKER];
};


/**
   Read out the DTI System Status

\param
   p_agt_ctx  points the DTI agent context
\param
   p_system_status  returns the DTI Agent System Status.

\return
   IFX_SUCCESS in case of success.
   IFX_ERROR  in case of failure.
*/
int dti_agent_system_status_get(
	DTI_AgentCtx_t *p_agt_ctx,
	struct ltq_dti_system_status_s *p_system_status);

/**
   Read out the DTI System Information

\param
   p_agt_ctx  points the DTI agent context
\param
   p_system_info  returns the DTI Agent System Information.

\return
   IFX_SUCCESS in case of success.
   IFX_ERROR  in case of failure.
*/
int dti_agent_system_info_get(
	DTI_AgentCtx_t *p_agt_ctx,
	struct ltq_dti_system_info_s *p_system_info);


/** *} */

#ifdef __cplusplus
}
#endif

#endif /* _DTI_STATISTIC_IF_H */




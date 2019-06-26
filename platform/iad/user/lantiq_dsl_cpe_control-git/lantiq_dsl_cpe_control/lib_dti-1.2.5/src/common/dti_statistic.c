/******************************************************************************

                              Copyright (c) 2015
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#include "dti_osmap.h"

#include "dti_agent_interface.h"
#include "dti_statistic_if.h"
#include "dti_statistic.h"
#include "dti_control.h"


#if (DTI_STAT_DEV_IF_NAME_LEN != DTI_MAX_LEN_DEVICE_INTERFACE_NAME)
#	error "DTI Statistic - missmatch DEV IF Name length (interal/statistic)"
#endif
#if (DTI_STAT_CLI_IF_NAME_LEN != DTI_CLI_MAX_NAME_LEN)
#	error "DTI Statistic - missmatch CLI IF Name length (interal/statistic)"
#endif


#if defined (DTI_INCLUDE_CORE_STATISTICS) && (DTI_INCLUDE_CORE_STATISTICS == 1)

/** Set the IP address in the 'IFXOS_sockAddr_t' - structure*/
#define IFXOS_SOC_ADDR_GET(a)	(((IFXOS_sockAddr_t*)(a))->sin_addr.s_addr)



static int status_stat_agent_get(
	DTI_AgentCtx_t *p_agt_ctx,
	struct ltq_dti_system_status_s *p_system_status)
{
	int i;

	p_system_status->status_agent.curr_worker = p_agt_ctx->numOfUsedWorker;
	p_system_status->status_agent.curr_dev_if = p_agt_ctx->numOfDevIf;
#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
	for (i = 0; i < DTI_CLI_MAX_CLI_INTERFACES; i++)
	{
		if (p_agt_ctx->cliControl[i].bRdyForCliSend == IFX_TRUE)
			{p_system_status->status_agent.curr_cli_if++;}
	}
#endif
	p_system_status->status_agent.cli_glb_evt_enabled = p_agt_ctx->bControlAutoCliMsgSupport;

	p_system_status->status_agent.listen_ip.ipaddr =
		DTI_ntohl(IFXOS_SOC_ADDR_GET(&p_agt_ctx->listenCon.sockAddr));
	p_system_status->status_agent.listen_ip.ipport =
		DTI_ntohs(IFXOS_SOC_ADDR_PORT_GET(&p_agt_ctx->listenCon.sockAddr));

	DTI_MemCpy(
		&p_system_status->stat_agent,
		&p_agt_ctx->stat_agent,
		sizeof(struct ltq_dti_statistic_agent_s));

	return IFX_SUCCESS;
}


static int status_stat_worker_get(
	DTI_AgentCtx_t *p_agt_ctx,
	struct ltq_dti_system_status_s *p_system_status)
{
	int i;

	for (i = 0; i < DTI_MAX_NUM_OF_WORKER; i++)
	{
		if (p_agt_ctx->pWorker[i] != IFX_NULL)
		{
			p_system_status->status_worker[i].thr_state = p_agt_ctx->pWorker[i]->thrResumeState;
#		if    defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) \
		   && defined(DTI_CLI_EVENT_SUPPORT) && (DTI_CLI_EVENT_SUPPORT == 1)
			p_system_status->status_worker[i].cli_evt_enabled =
				p_agt_ctx->pWorker[i]->cliEventCtx.bAutoCliMsgActive;
#		else
			p_system_status->status_worker[i].cli_evt_enabled = 0;
#		endif
			p_system_status->status_worker[i].remote_ip.ipaddr = DTI_ntohl(
				IFXOS_SOC_ADDR_GET(&p_agt_ctx->pWorker[i]->dtiProtocolServerCtx.dtiCon.sockAddr));
			p_system_status->status_worker[i].remote_ip.ipport = DTI_ntohs(
				IFXOS_SOC_ADDR_PORT_GET(&p_agt_ctx->pWorker[i]->dtiProtocolServerCtx.dtiCon.sockAddr));

			DTI_MemCpy(
				&p_system_status->stat_worker_thr[i],
				&p_agt_ctx->stat_worker_thr[i],
				sizeof(struct ltq_dti_statistic_worker_thr_s));

			DTI_MemCpy(
				&p_system_status->stat_conn[i],
				&p_agt_ctx->stat_conn[i],
				sizeof(struct ltq_dti_statistic_connection_s));
		}
	}

	return IFX_SUCCESS;
}
#endif	/* #if defined (DTI_INCLUDE_CORE_STATISTICS) && (DTI_INCLUDE_CORE_STATISTICS == 1) */

int dti_agent_system_status_get(
	DTI_AgentCtx_t *p_agt_ctx,
	struct ltq_dti_system_status_s *p_system_status)
{
#if defined (DTI_INCLUDE_CORE_STATISTICS) && (DTI_INCLUDE_CORE_STATISTICS == 1)

	if ((p_agt_ctx == IFX_NULL) || (p_system_status == IFX_NULL))
		{return IFX_ERROR;}

	DTI_MemSet(p_system_status, 0x0, sizeof(struct ltq_dti_system_status_s));
	p_system_status->status_agent.b_valid = (p_agt_ctx->bListenRun == IFX_TRUE) ? 1 : 0;
	if (p_system_status->status_agent.b_valid == 0)
		{return IFX_SUCCESS;}

	(void)status_stat_agent_get(p_agt_ctx, p_system_status);
	(void)status_stat_worker_get(p_agt_ctx, p_system_status);

	return IFX_SUCCESS;
#else
	return IFX_ERROR;
#endif	/* #if defined (DTI_INCLUDE_CORE_STATISTICS) && (DTI_INCLUDE_CORE_STATISTICS == 1) */

}


int dti_agent_system_info_get(
	DTI_AgentCtx_t *p_agt_ctx,
	struct ltq_dti_system_info_s *p_system_info)
{
#if defined (DTI_INCLUDE_CORE_STATISTICS) && (DTI_INCLUDE_CORE_STATISTICS == 1)
	int i;

	if ((p_agt_ctx == IFX_NULL) || (p_system_info == IFX_NULL))
		{return IFX_ERROR;}

	if (p_agt_ctx->bListenRun != IFX_TRUE)
		{return IFX_ERROR;}

	p_system_info->core_version = DTI_AGENT_VER;
	p_system_info->protocol_version = DTI_PROTOCOL_VER;
	p_system_info->max_worker = DTI_MAX_NUM_OF_WORKER;
	p_system_info->max_device_interfaces = DTI_MAX_DEVICE_INTERFACES;
	p_system_info->max_cli_interfaces = DTI_CLI_MAX_CLI_INTERFACES;

	for (i = 0; i < DTI_MAX_DEVICE_INTERFACES; i++)
	{
		if (p_agt_ctx->deviceInterface[i].bConfigured)
		{
			DTI_StrNCpy(
				p_system_info->dev_if_name[i],
				p_agt_ctx->deviceInterface[i].ifName,
				DTI_STAT_DEV_IF_NAME_LEN);
			p_system_info->dev_if_name[i][DTI_STAT_DEV_IF_NAME_LEN - 1] = '\0';
		}
		else
		{
			p_system_info->dev_if_name[i][0] = '\0';
		}
	}

#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
	for (i = 0; i < DTI_CLI_MAX_CLI_INTERFACES; i++)
	{
		if (DTI_StrLen(p_agt_ctx->cliControl[i].cliIfName) > 0)
		{
			DTI_StrNCpy(
				p_system_info->cli_if[i].cli_if_name,
				p_agt_ctx->cliControl[i].cliIfName,
				DTI_STAT_CLI_IF_NAME_LEN);
			p_system_info->cli_if[i].cli_if_name[DTI_STAT_CLI_IF_NAME_LEN - 1] = '\0';
			p_system_info->cli_if[i].responce_buffer_size =
				p_agt_ctx->cliControl[i].responceBufferSize;
		}
		else
		{
			p_system_info->cli_if[i].cli_if_name[0] = '\0';
			p_system_info->cli_if[i].responce_buffer_size = 0;
		}
	}
#else
	DTI_MemSet(p_system_info->cli_if, 0x0, sizeof(p_system_info->cli_if));
#endif

	return IFX_SUCCESS;

#else
	return IFX_ERROR;
#endif	/* #if defined (DTI_INCLUDE_CORE_STATISTICS) && (DTI_INCLUDE_CORE_STATISTICS == 1) */
}



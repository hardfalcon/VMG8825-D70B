/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   DSL CPE Control Application command line interface for pipe
*/

#include "dsl_cpe_control.h"

#ifndef DSL_CPE_REMOVE_PIPE_SUPPORT

#include "dsl_cpe_cli.h"

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_PIPE


#ifndef MAX_CLI_PIPES
#define MAX_CLI_PIPES    1
#endif
#ifndef MAX_CLI_PIPE_CMD_LEN
#define MAX_CLI_PIPE_CMD_LEN 1024
#endif

#define PIPE_NAME_FORMAT     "dsl_cpe%d_%s"
#define DTI_PIPE_NAME_FORMAT "dsl_cpe_dti%d_%s"

#define PIPE_DEBUG   1

typedef struct Pipe_env_s Pipe_env_t;
struct Pipe_env_s
{
   /** reference to registered CLI */
   DSL_CLI_Context_t *pCLIContext;
   /** running status of pipe task */
   volatile DSL_boolean_t bRun;
   /** thread id of the pipe task */
   DSL_CPE_ThreadCtrl_t control;
   /** number of this pipe interface */
   DSL_int_t number;
   /** name of event pipe */
   DSL_char_t namePipeEvent[32];
   /** command buffer */
   DSL_char_t cmd_buffer[MAX_CLI_PIPE_CMD_LEN];
   DSL_CPE_File_t *pipe_in;
   DSL_CPE_File_t *pipe_out;
};

#ifdef INCLUDE_DTI_AGENT_STANDALONE_SUPPORT
static Pipe_env_t PipeEnv[MAX_CLI_PIPES + 1];
#else
static Pipe_env_t PipeEnv[MAX_CLI_PIPES];
#endif /* INCLUDE_DTI_AGENT_STANDALONE_SUPPORT */

static DSL_Error_t DSL_CPE_Pipe_Exit(DSL_void_t *pContext);
static DSL_int_t DSL_CPE_Pipe_Task(DSL_CPE_Thread_Params_t *params);
static DSL_Error_t DSL_CPE_Pipe_Event(DSL_void_t *pContext, DSL_char_t *pMessage);
static DSL_Error_t DSL_CPE_Pipe_CreateCliEntry(DSL_CPE_Control_Context_t *pContext,
                                Pipe_env_t                *pPipeEnv,
                                DSL_char_t                *pipeNameFormat,
                                DSL_int_t                  nPipeNum);

/******************************************************************************/

DSL_Error_t DSL_CPE_Pipe_Init(DSL_CPE_Control_Context_t *pContext)
{
   DSL_int_t i = 0;
   DSL_Error_t ret = DSL_SUCCESS;

   for (i = 0; i < MAX_CLI_PIPES; i++)
   {
      ret = DSL_CPE_Pipe_CreateCliEntry(pContext, &PipeEnv[i], PIPE_NAME_FORMAT, i);
      if (ret < DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "DSL_CPE_Pipe_Init: could not create "
            PIPE_NAME_FORMAT DSL_CPE_CRLF, i, " pipe"));
      }
   }

#ifdef INCLUDE_DTI_AGENT_STANDALONE_SUPPORT
   ret = DSL_CPE_Pipe_CreateCliEntry(pContext,
                                     &PipeEnv[i],
                                     DTI_PIPE_NAME_FORMAT,
                                     i);
   if (ret < DSL_SUCCESS)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "DSL_CPE_Pipe_Init: could not create "
         DTI_PIPE_NAME_FORMAT DSL_CPE_CRLF, MAX_CLI_PIPES, " pipe"));
   }
#endif /* INCLUDE_DTI_AGENT_STANDALONE_SUPPORT */

   return ret;
}

DSL_Error_t DSL_CPE_Pipe_CreateCliEntry(DSL_CPE_Control_Context_t *pContext, 
                                Pipe_env_t                *pPipeEnv,
                                DSL_char_t                *pipeNameFormat,
                                DSL_int_t                  nPipeNum)
{
   DSL_Error_t ret = DSL_SUCCESS;
   DSL_char_t Name[16];
   
   memset(pPipeEnv, 0, sizeof(Pipe_env_t));

   /* remember own number */
   pPipeEnv->number = nPipeNum;

   DSL_CPE_snprintf(Name, sizeof(Name), pipeNameFormat, nPipeNum, "cmd");
   ret = DSL_CPE_PipeCreate(Name);

   if (ret < DSL_SUCCESS)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "DSL_CPE_PipeCreate: could not create %s pipe"
         DSL_CPE_CRLF, Name));
   }

   DSL_CPE_snprintf(Name, sizeof(Name), pipeNameFormat, nPipeNum, "ack");
   ret = DSL_CPE_PipeCreate(Name);

   if (ret < DSL_SUCCESS)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "DSL_CPE_PipeCreate: could not create %s pipe"
         DSL_CPE_CRLF, Name));
   }

   DSL_CPE_snprintf(pPipeEnv->namePipeEvent, sizeof(pPipeEnv->namePipeEvent),
      pipeNameFormat, nPipeNum, "event");

   ret = DSL_CPE_PipeCreate(pPipeEnv->namePipeEvent);
   
   if (ret < DSL_SUCCESS)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "DSL_CPE_PipeCreate: could not create %s pipe"
         DSL_CPE_CRLF, pPipeEnv->namePipeEvent));
   }

   DSL_CPE_snprintf(Name, sizeof(Name), "tPipe_%d", nPipeNum);

   pPipeEnv->bRun = DSL_TRUE;

   /* start task to handle pipe */
   DSL_CPE_ThreadInit(&pPipeEnv->control, Name, DSL_CPE_Pipe_Task,
      DSL_CPE_PIPE_STACK_SIZE, DSL_CPE_PIPE_PRIORITY, (DSL_uint32_t)pContext,
      (DSL_uint32_t)pPipeEnv);

   return DSL_CPE_CLI_Register(&pPipeEnv->pCLIContext,
      pPipeEnv,
      DSL_CPE_Pipe_Exit,
      DSL_CPE_Pipe_Event);
}

static DSL_Error_t DSL_CPE_Pipe_Exit(DSL_void_t *pContext)
{
   Pipe_env_t *env = pContext;

   DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
      "DSL_CPE_Pipe_Exit(%d)" DSL_CPE_CRLF, env->number));

   /*env->bRun = DSL_FALSE;*/
   DSL_CPE_ThreadShutdown(&env->control, 1000);
   if (env->pipe_in != DSL_NULL)
   {
      DSL_CPE_PipeClose(env->pipe_in);
      env->pipe_in = DSL_NULL;
   }

   /* wait for task to exit */
   /*DSL_CPE_MSecSleep(200);*/
   return DSL_SUCCESS;
}

/**
   Split the command buffer in single lines and execute them
*/
static DSL_int_t DSL_CPE_Pipe_Exec(DSL_Context_t * pContext, DSL_char_t *pCommand,
                                    DSL_int_t len, DSL_CPE_File_t *out)
{
   DSL_int_t ret=0;
   DSL_char_t *arg = DSL_NULL;

   DSL_char_t *line, *tokbuf;
   DSL_char_t *cmd, *cmd_tok;

   line = strtok_r(pCommand, "" DSL_CPE_CRLF , &tokbuf);
   if (line == DSL_NULL)
   {
      DSL_CPE_CLI_CommandExecute (0, DSL_NULL, DSL_NULL, out);
   }
   else
   {
      while (line != DSL_NULL)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, ("line = *%s*" DSL_CPE_CRLF, line));

         if (strlen(line)>0)
         {
            /* get command name */
            cmd = strtok_r(line, " \t" DSL_CPE_CRLF , &cmd_tok);
            if (cmd == DSL_NULL)
            {
               goto skip;
            }

            DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, ("cmd = *%s*" DSL_CPE_CRLF, cmd));

            arg = strtok_r(NULL, "" DSL_CPE_CRLF , &cmd_tok);

            DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, ("arg ="));
            if (arg != DSL_NULL)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, ("*%s*" DSL_CPE_CRLF , arg));
            }
            else
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, ("*NULL*" DSL_CPE_CRLF));
            }

            if (DSL_CPE_CliDeviceCommandExecute(DSL_CPE_GetGlobalContext(), -1, cmd, arg, out) == 1)
            {
               /* "quit" was found, ignore following commands */
               return 1;
            }
         }

skip:
         line = strtok_r(NULL, "" DSL_CPE_CRLF , &tokbuf);
      };
   }

   return ret;
}

/**
   Task for handling the commands received in the ..._cmd pipe
*/
static DSL_int_t DSL_CPE_Pipe_Task(DSL_CPE_Thread_Params_t *params)
{
   DSL_Context_t * pContext = (DSL_Context_t *)params->nArg1;
   Pipe_env_t *pPipeEnv = (Pipe_env_t *)params->nArg2;
   DSL_char_t pipe_name_cmd[50];
   DSL_char_t pipe_name_ack[50];
   DSL_char_t *pPipeFormat = DSL_NULL;
   DSL_int_t len = 0;
   DSL_int_t ret = 0;

   if (pPipeEnv == DSL_NULL) {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (
         "DSL_CPE_Pipe_Task() started failed, pipe env is NULL" DSL_CPE_CRLF));
      return -1;
   }

   DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (
      " DSL_CPE_Pipe_Task(%d) started" DSL_CPE_CRLF , pPipeEnv->number));

   if (pPipeEnv->number == MAX_CLI_PIPES)
   {
      pPipeFormat = DTI_PIPE_NAME_FORMAT;
   }
   else
   {
      pPipeFormat = PIPE_NAME_FORMAT;
   }

   DSL_CPE_snprintf(pipe_name_ack, sizeof(pipe_name_ack), pPipeFormat,
      pPipeEnv->number, "ack");
   DSL_CPE_snprintf(pipe_name_cmd, sizeof(pipe_name_cmd), pPipeFormat,
      pPipeEnv->number, "cmd");

   do
   {
      pPipeEnv->pipe_in = DSL_CPE_PipeOpen(pipe_name_cmd, DSL_TRUE, DSL_TRUE);
      if (pPipeEnv->pipe_in == DSL_NULL)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (
            "DSL_CPE_Pipe_Task() cannot open %s, errno=%d" DSL_CPE_CRLF , pipe_name_cmd, errno));
         ret = -1;
         goto error;
      }
      if (!pPipeEnv->bRun)
      {
         break;
      }

      memset(&pPipeEnv->cmd_buffer[0], 0,
         sizeof(pPipeEnv->cmd_buffer));

      len = sizeof(pPipeEnv->cmd_buffer);
#ifndef _lint
      if (len>0)
#endif /* _lint*/
      {
         ret = DSL_CPE_FRead(&pPipeEnv->cmd_buffer[0], 1, (DSL_uint32_t)len, pPipeEnv->pipe_in);

         if (!pPipeEnv->bRun)
         {
            break;
         }
         if (ret>0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (
               "Pipe%d: cmd len %d, feof=%d ferror=%d " DSL_CPE_CRLF "%s" DSL_CPE_CRLF,
               pPipeEnv->number, len,
               feof(pPipeEnv->pipe_in),
               ferror(pPipeEnv->pipe_in),
               pPipeEnv->cmd_buffer));

            DSL_CPE_PipeClose(pPipeEnv->pipe_in);
            pPipeEnv->pipe_in = DSL_NULL;

            pPipeEnv->pipe_out = DSL_CPE_PipeOpen(pipe_name_ack, DSL_FALSE, DSL_TRUE);
            if (pPipeEnv->pipe_out == DSL_NULL)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (
                  "DSL_CPE_Pipe_Task() cannot open %s" DSL_CPE_CRLF, pipe_name_ack));
               ret = -1;
               goto error;
            }

            DSL_CPE_Pipe_Exec(pContext, pPipeEnv->cmd_buffer,
                              len, pPipeEnv->pipe_out);
            DSL_CPE_FFlush(pPipeEnv->pipe_out);
            DSL_CPE_PipeClose(pPipeEnv->pipe_out);
            pPipeEnv->pipe_out = DSL_NULL;
         }
         else
         {
            DSL_CPE_PipeClose(pPipeEnv->pipe_in);
            pPipeEnv->pipe_in = DSL_NULL;
            DSL_CPE_MSecSleep(50);
         }
      }
   } while (pPipeEnv->bRun);

error:

   if (pPipeEnv->pipe_out != DSL_NULL)
   {
      DSL_CPE_PipeClose(pPipeEnv->pipe_out);
      pPipeEnv->pipe_out = DSL_NULL;
   }
   if (pPipeEnv->pipe_in != DSL_NULL)
   {
      DSL_CPE_PipeClose(pPipeEnv->pipe_in);
      pPipeEnv->pipe_in = DSL_NULL;
   }

   DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (
      " DSL_CPE_Pipe_Task(%d) exit" DSL_CPE_CRLF , pPipeEnv->number));

   return ret;
}


static DSL_Error_t DSL_CPE_Pipe_Event(DSL_void_t *pContext, DSL_char_t *pMessage)
{
   DSL_CPE_File_t *file;
   Pipe_env_t *env = pContext;

   file = DSL_CPE_PipeOpen(env->namePipeEvent, DSL_FALSE, DSL_FALSE);
   if (file != DSL_NULL)
   {
      if (fprintf(file, "%s" DSL_CPE_CRLF, pMessage) <= 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (
            "Pipe[%d]Event: Error %d during print!" DSL_CPE_CRLF, env->number, errno));
      }

      DSL_CPE_PipeClose(file);
   }

   return DSL_SUCCESS;
}

#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
DSL_Error_t DSL_CPE_Pipe_StaticResourceUsageGet(DSL_uint32_t *pStatResUsage)
{
   if (pStatResUsage == DSL_NULL)
   {
      return DSL_ERROR;
   }
   else
   {
      *pStatResUsage = sizeof(PipeEnv);
      return DSL_SUCCESS;
   }
}
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/

#endif /* DSL_CPE_REMOVE_PIPE_SUPPORT */


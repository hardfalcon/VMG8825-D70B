/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#if defined(WIN32) && !defined(NUCLEUS_PLUS)

/** \file
   This file contains the IFXOS Layer implementation for Win32 - 
   Pipes.
*/

/* ============================================================================
   IFX Win32 adaptation - Global Includes
   ========================================================================= */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <errno.h>
#include <fcntl.h>

#include "ifx_types.h"
#include "ifxos_pipe.h"
#include "ifxos_debug.h"
#include "ifxos_memory_alloc.h"
#include "ifxos_print_io.h"

/* ============================================================================
   IFX Win32 adaptation - User Space, XXX
   ========================================================================= */

/** define the pipe specific name (base dir, name) */
#define IFXOS_SYS_PIPE_PREFIX  "\\\\.\\pipe"
#define BUFSIZE 512

#define IFXOS_SYS_PIPE_TABLE_LENGTH       16

/** Pipe descriptor table entry containing all necessary information pipe */
typedef struct _IFX_PipeEntry_t
{
   /* server pipe handle */
   IFXOS_Pipe_t fpServer;
   /* client pipe handle */
   IFXOS_Pipe_t fpClient;
   /* Weather IFXOS_PipeOpen has been called for this pipe. Server is always
    * opened first.
    */
   IFX_boolean_t bServerOpened;
   IFX_char_t* pName;
} IFX_PipeEntry_t;

static IFX_PipeEntry_t pipeTable[IFXOS_SYS_PIPE_TABLE_LENGTH];

/**
   Return index of pipe descriptor in pipeTable by its name
\param pName Name of the pipe descriptor
\return
   Pipe descriptor index or -1 if not found
*/
static IFX_int_t IFXOS_PipeIndexGetByName (IFX_char_t *pName)
{
   IFX_int_t i;

   for (i = 0; i < IFXOS_SYS_PIPE_TABLE_LENGTH; i++)
   {
      if (pipeTable[i].pName != IFX_NULL &&
          strcmp (pipeTable[i].pName, pName) == 0)
      {
         return i;
      }
   }
   return -1;
}

/**
   Return index of pipe descriptor in pipeTable by its server pipe handle
\param pName Name of the pipe descriptor
\return
   Pipe descriptor index or -1 if not found
*/
static IFX_int_t IFXOS_PipeIndexGetByServerHandle (IFXOS_Pipe_t fp)
{
   IFX_int_t i;

   for (i = 0; i < IFXOS_SYS_PIPE_TABLE_LENGTH; i++)
   {
      if (pipeTable[i].fpServer == fp)
      {
         return i;
      }
   }
   return -1;
}

/**
   Add pipe descriptor to pipeTable
\param fp Pipe handle
\param pName Name of the pipe descriptor
\return
   Pipe descriptor index or -1 if table full
*/
static IFX_int_t IFXOS_PipeTableAdd (IFXOS_Pipe_t fp, IFX_char_t* pName)
{
   IFX_int_t i;

   for (i = 0; i < IFXOS_SYS_PIPE_TABLE_LENGTH; i++)
   {
      if (pipeTable[i].pName == IFX_NULL)
      {
         pipeTable[i].pName = pName;
         pipeTable[i].fpServer = fp;
         return i;
      }
   }
   /* no space */
   return -1;
}

/**
   Remove pipe descriptor from pipeTable
\param index Pipe index in pipeTable
\return
   none
*/
static IFX_void_t IFXOS_PipeTableRemove (IFX_int_t index)
{
   if (index >= IFXOS_SYS_PIPE_TABLE_LENGTH || index < 0)
      return;
   pipeTable[index].bServerOpened = IFX_FALSE;
   pipeTable[index].fpClient = INVALID_HANDLE_VALUE;
   pipeTable[index].fpServer = INVALID_HANDLE_VALUE;
   pipeTable[index].pName = IFX_NULL;
   return;
}

/** \addtogroup IFXOS_PIPES_WIN32_APPL
@{ */

#if ( defined(IFXOS_HAVE_PIPE) && (IFXOS_HAVE_PIPE == 1) )

#if ( defined(IFXOS_HAVE_PIPE_CREATE) && (IFXOS_HAVE_PIPE_CREATE == 1) )
/**
   Win32 - Create a pipe.

\param
   pName - pipe name

\return
   - IFX_SUCCESS on success
   - IFX_ERROR on failure
*/
IFX_int_t IFXOS_PipeCreate(IFX_char_t *pName)
{
   IFX_char_t pipepath[256];
   IFXOS_Pipe_t fp;
   IFX_uint_t i;

   if (IFXOS_PipeIndexGetByName(pName) != -1)
   {
      IFXOS_PRN_USR_DBG_NL( IFXOS, IFXOS_PRN_LEVEL_LOW,
         ("IFXOS - Pipe - pipe %s exists" IFXOS_CRLF, pName));
      return IFX_SUCCESS;
   }

   IFXOS_SNPrintf(pipepath, sizeof(pipepath), IFXOS_SYS_PIPE_PREFIX"\\%s",
      pName);
   /* replace slash with backslash */
   for (i = 0; i < strlen(pipepath); i++)
      if (pipepath[i] == '/')
         pipepath[i] = '\\';

   IFXOS_PRN_USR_DBG_NL( IFXOS, IFXOS_PRN_LEVEL_LOW,
      ("IFXOS - Pipe - create pipe %s" IFXOS_CRLF, pipepath));

   fp = CreateNamedPipe(pipepath,
      PIPE_ACCESS_DUPLEX,  /* bidirectional */
      PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
      1,
      BUFSIZE,
      BUFSIZE,
      0,
      NULL);
   if (fp == INVALID_HANDLE_VALUE)
   {
      IFXOS_PRN_USR_ERR_NL( IFXOS, IFXOS_PRN_LEVEL_ERR,
         ("IFXOS ERROR - Pipe - CreateNamedPipe %s failed (errno=%d)"
           IFXOS_CRLF,
           pipepath, GetLastError()));
      return IFX_ERROR;
   }
   if (IFXOS_PipeTableAdd(fp, pName) == -1)
   {
      IFXOS_PRN_USR_ERR_NL( IFXOS, IFXOS_PRN_LEVEL_ERR,
         ("IFXOS ERROR - Creating pipe %s failed. IFXOS pipe descriptor table"
          "full",
           IFXOS_CRLF,
           pName));
      CloseHandle(fp);
      return IFX_ERROR;
   }
   return IFX_SUCCESS;
}
#endif

/**
   Win32 - Open a pipe.

\param
   pName    - pipe name.
\param
   reading  - if set, open the pipe for read.
\param
   blocking - if set, open the pipe in blocking mode.

\return
   - pointer to IFXOS_Pipe_t structure
   - in case of error the return value is NULL
*/
IFXOS_Pipe_t *IFXOS_PipeOpen(
                     IFX_char_t *pName, 
                     IFX_boolean_t reading, 
                     IFX_boolean_t blocking)
{
   DWORD          dwMode;
   IFX_int_t      index;
   IFX_uint_t     i;
   IFX_char_t pipepath[256];

   IFXOS_PRN_USR_DBG_NL( IFXOS, IFXOS_PRN_LEVEL_LOW,
      ("IFXOS - Pipe - open pipe %s" IFXOS_CRLF, pName));

   index = IFXOS_PipeIndexGetByName(pName);
   if (index == -1)
   {
      IFXOS_PRN_USR_DBG_NL( IFXOS, IFXOS_PRN_LEVEL_LOW,
         ("IFXOS - Pipe - pipe %s doesn't exist" IFXOS_CRLF, pName));
      return IFX_NULL;
   }

   if (!pipeTable[index].bServerOpened)
   {
      /* Server is opened first. Set server pipe parameters. Don't look at
       * "reading" parameter. Pipe is always bidirectional. */
      dwMode = PIPE_READMODE_MESSAGE | (blocking ? PIPE_WAIT : PIPE_NOWAIT);
      if (SetNamedPipeHandleState(pipeTable[index].fpServer, &dwMode, NULL, NULL) ==
         0)
      {
         IFXOS_PRN_USR_ERR_NL(IFXOS, IFXOS_PRN_LEVEL_ERR,
            ("IFXOS ERROR - Pipe - SetNamedPipeHandleState %s failed (errno=%d)"
              IFXOS_CRLF,
              pName, GetLastError()));
         /* Delete the pipe so that it can be created again */
         CloseHandle(pipeTable[index].fpServer);
         IFXOS_PipeTableRemove(index);
         return IFX_NULL;
      }
      pipeTable[index].bServerOpened = IFX_TRUE;
      /* client pipe not created yet, mark it in table */
      pipeTable[index].fpClient = INVALID_HANDLE_VALUE;
      /* return server pipe handle */
      return &pipeTable[index].fpServer;
   }

   /* Server opened, open client */
   if (pipeTable[index].fpClient == INVALID_HANDLE_VALUE)
   {
      IFXOS_SNPrintf(pipepath, sizeof(pipepath), IFXOS_SYS_PIPE_PREFIX"\\%s",
         pName);
      /* replace slash with backslash */
      for (i = 0; i < strlen(pipepath); i++)
         if (pipepath[i] == '/')
            pipepath[i] = '\\';

      /* Enable both reading and writing, disregarding "reading" parameter,
       * because changing access rights requires
       * reopening the file (CreateFile call). For CreateFile to succeed second
       * time, server
       * should call ConnectNamedPipe (and DisconnectNamedPipe afterwards).
       * ConnectNamedPipe is a blocking function, waiting for client to call
       * CreateFile. Hence it requires 2 threads (client and server). This would
       * make implementation too complicated.
       */
      pipeTable[index].fpClient = CreateFile(pipepath,
         GENERIC_READ | GENERIC_WRITE,
         0, /* only one handle is created, so no need for sharing */
         NULL,           /* default security attributes */
         OPEN_EXISTING,  /* opens existing pipe */
         0,              /* default attributes */
         NULL);          /* no template file */

      if (pipeTable[index].fpClient == INVALID_HANDLE_VALUE)
      {
         IFXOS_PRN_USR_ERR_NL( IFXOS, IFXOS_PRN_LEVEL_ERR,
            ("IFXOS ERROR - Pipe - CreateFile %s failed (errno=%d)"
              IFXOS_CRLF,
              pipepath, GetLastError()));
         return IFX_NULL;
      }
   }

   /* Client pipe opened, set parameter */
   dwMode = PIPE_READMODE_MESSAGE | (blocking ? PIPE_WAIT : PIPE_NOWAIT);
   if (SetNamedPipeHandleState(pipeTable[index].fpClient, &dwMode, NULL, NULL) == 0)
   {
      IFXOS_PRN_USR_ERR_NL(IFXOS, IFXOS_PRN_LEVEL_ERR,
         ("IFXOS ERROR - Pipe - SetNamedPipeHandleState %s failed (errno=%d)"
           IFXOS_CRLF,
           pipepath, GetLastError()));
      /* Although setting client handle parameter failed, we can't close it,
       * because opening it won't be possible. The only way is to close
       * server pipe and recreate it. */
      return IFX_NULL;
   }
   return &pipeTable[index].fpClient;
}

/**
   Win32 - Close a pipe.

\param
   pPipe    - handle of the pipe stream

\return
   - IFX_SUCCESS on success
   - IFX_ERROR on failure
*/
IFX_int_t IFXOS_PipeClose(IFXOS_Pipe_t *pPipe)
{
   DWORD lpFlags;
   BOOL bResult = 1;

   if (GetNamedPipeInfo(*pPipe, &lpFlags, NULL, NULL, NULL) == 0)
   {
      IFXOS_PRN_USR_ERR_NL( IFXOS, IFXOS_PRN_LEVEL_ERR,
         ("IFXOS ERROR - Pipe - GetNamedPipeInfo failed (errno=%d)"
           IFXOS_CRLF,
           GetLastError()));
      return IFX_ERROR;
   }
   if ((lpFlags & PIPE_SERVER_END) > 0)
   {
      /* server pipe */
      FlushFileBuffers(*pPipe);
      DisconnectNamedPipe(*pPipe);
      bResult = CloseHandle(*pPipe);
      /* clean table entry */
      IFXOS_PipeTableRemove(IFXOS_PipeIndexGetByServerHandle(*pPipe));
   }
   return (bResult != 0) ? IFX_SUCCESS : IFX_ERROR;
}

#if ( defined(IFXOS_HAVE_PIPE_WRITE) && (IFXOS_HAVE_PIPE_WRITE == 1) )
/**
   Win32 - Print to a pipe.

\param
   streamPipe  - handle of the pipe stream.
\param
   format      - points to the printf format string.
   
\return
   For success - Number of written bytes.
   For error   - negative value.
*/
IFX_int_t IFXOS_PipePrintf(
                     IFXOS_Pipe_t      *streamPipe, 
                     const IFX_char_t  *format, ...)
{
   IFX_char_t s[512];
   IFX_int32_t len = 0;
   va_list ptr;
   DWORD cbWritten;

   va_start(ptr, format);
   len = vsnprintf (s, sizeof(s), format, ptr);
   if (len == -1)
   {
      IFXOS_PRN_USR_ERR_NL( IFXOS, IFXOS_PRN_LEVEL_ERR,
         ("IFXOS ERROR - Pipe - message to write too big" IFXOS_CRLF));
      vprintf(format, ptr);
   }
   va_end(ptr);

   if (WriteFile(*streamPipe, s, strlen(s), &cbWritten, NULL) == 0)
   {
      IFXOS_PRN_USR_ERR_NL( IFXOS, IFXOS_PRN_LEVEL_ERR,
         ("IFXOS ERROR - Pipe - WriteFile failed (errno=%d)"
           IFXOS_CRLF, GetLastError()));
      return -1;
   }
   return cbWritten;
}
#endif

#if ( defined(IFXOS_HAVE_PIPE_BUFFER_WRITE) && (IFXOS_HAVE_PIPE_BUFFER_WRITE == 1) )
/**
   Win32 - Write to a pipe.

\param
   pDataBuf          - Points to the buffer for data to be sent. [i]
\param
   bufferSize_byte   - pDataBuf size [byte]
\param
   pPipe             - handle of the pipe stream.

\return
   For success - Number of written bytes.
   For error   - negative value.

*/
IFX_int_t IFXOS_PipeWrite(
                     IFX_void_t     *pDataBuf,
                     IFX_uint32_t   bufferSize_byte,
                     IFXOS_Pipe_t   *pPipe)
{
   DWORD cbWritten;

   if (WriteFile(*pPipe, pDataBuf, bufferSize_byte, &cbWritten, NULL) == 0)
   {
      IFXOS_PRN_USR_ERR_NL( IFXOS, IFXOS_PRN_LEVEL_ERR,
         ("IFXOS ERROR - Pipe - WriteFile failed (errno=%d)"
           IFXOS_CRLF, GetLastError()));
      return -1;
   }
   return cbWritten;
}
#endif /* IFXOS_HAVE_PIPE_BUFFER_WRITE */

#if ( defined(IFXOS_HAVE_PIPE_READ) && (IFXOS_HAVE_PIPE_READ == 1) )
/**
   Win32 - Read from pipe .

\param
   pDataBuf          - Points to the buffer used for get the data. [o]
\param
   elementSize_byte  - Element size of one element to read [byte]
\param
   elementCount      - Number of elements to read
\param
   pPipe             - handle of the pipe stream.

\return
   Number of bytes read

\attention
   If an error occurs, or the end-of-file is reached, the return value is
   a short item count (or zero) (see errno)
*/
IFX_int_t IFXOS_PipeRead(
                     IFX_void_t     *pDataBuf, 
                     IFX_uint32_t   elementSize_byte,  
                     IFX_uint32_t   elementCount, 
                     IFXOS_Pipe_t   *pPipe)
{
   DWORD  cbRead;
   DWORD error;

   if (ReadFile(*pPipe, pDataBuf, elementSize_byte * elementCount, &cbRead,
      NULL) == 0)
   {
      error = GetLastError();
      if (error != ERROR_NO_DATA
         && error != ERROR_PIPE_LISTENING
         && error != ERROR_BROKEN_PIPE)
      {
         IFXOS_PRN_USR_ERR_NL( IFXOS, IFXOS_PRN_LEVEL_ERR,
            ("IFXOS ERROR - Pipe - ReadFile failed (errno=%d)"
              IFXOS_CRLF, error));
         return -1;
      }
      else
         return 0;
   }
   return cbRead;
}
#endif

#endif      /* #if ( defined(IFXOS_HAVE_PIPE) && (IFXOS_HAVE_PIPE == 1) ) */

/** @} */

#endif      /* #ifdef WIN32 */


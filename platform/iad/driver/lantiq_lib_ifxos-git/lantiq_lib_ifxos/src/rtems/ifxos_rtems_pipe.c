/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef RTEMS

/** \file
   This file contains the IFXOS Layer implementation for RTEMS Application
   Pipes.
*/

/* ============================================================================
   RTEMS adaptation - Global Includes
   ========================================================================= */

#include "ifx_types.h"
#include "ifxos_rt_if_check.h"
#include "ifxos_pipe.h"

/* ============================================================================
   RTEMS adaptation - User Space, Pipes
   ========================================================================= */

/** \addtogroup IFXOS_PIPES_RTEMS_APPL
@{ */

#if ( defined(IFXOS_HAVE_PIPE) && (IFXOS_HAVE_PIPE == 1) )

#if ( defined(IFXOS_HAVE_PIPE_CREATE) && (IFXOS_HAVE_PIPE_CREATE == 1) )
/**
   Create a pipe.

\attention
   Not implemented yet!

\param
   pName - pipe name

\return
   - IFX_SUCCESS on success
   - IFX_ERROR on failure
*/
IFX_int_t IFXOS_PipeCreate(
                     IFX_char_t *pName)
{
   /*
      Customer-ToDo:
      Fill with your customer OS implementation like:
      - ...
   */

   IFXOS_RETURN_IF_POINTER_NULL(pName, IFX_ERROR);

   return IFX_ERROR;
}
#endif

/**
   Open a pipe.

\attention
   Not implemented yet!

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
   /*
      Customer-ToDo:
      Fill with your customer OS implementation like:
      - ...
   */

   IFXOS_RETURN_IF_POINTER_NULL(pName, IFX_ERROR);

   return IFX_ERROR;
}

/**
   Close a pipe.

\attention
   Not implemented yet!

\param
   pPipe    - handle of the pipe stream

\return
   - IFX_SUCCESS on success
   - IFX_ERROR on failure
*/
IFX_int_t IFXOS_PipeClose(IFXOS_Pipe_t *pPipe)
{
   /*
      Customer-ToDo:
      Fill with your customer OS implementation like:
      - ...
   */

   IFXOS_RETURN_IF_POINTER_NULL(pPipe, IFX_ERROR);

   return IFX_ERROR;
}

#if ( defined(IFXOS_HAVE_PIPE_WRITE) && (IFXOS_HAVE_PIPE_WRITE == 1) )
/**
   Print to a pipe.

\attention
   Not implemented yet!

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
   /*
      Customer-ToDo:
      Fill with your customer OS implementation like:
      - ...
   */

   IFXOS_RETURN_IF_POINTER_NULL(streamPipe, IFX_ERROR);
   IFXOS_RETURN_IF_POINTER_NULL(format, IFX_ERROR);

   return IFX_ERROR;
}
#endif

#if ( defined(IFXOS_HAVE_PIPE_READ) && (IFXOS_HAVE_PIPE_READ == 1) )
/**
   Read from pipe .

\attention
   Not implemented yet!

\param
   pDataBuf          - Points to the buffer used for get the data. [o]
\param
   elementSize_byte  - Element size of one element to read [byte]
\param
   elementCount      - Number of elements to read
\param
   pPipe             - handle of the pipe stream.

\return
   Number of read elements

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
   /*
      Customer-ToDo:
      Fill with your customer OS implementation like:
      - ...
   */

   IFXOS_RETURN_IF_POINTER_NULL(pPipe, IFX_ERROR);
   IFXOS_RETURN_IF_POINTER_NULL(pDataBuf, IFX_ERROR);

   return IFX_ERROR;
}
#endif

#endif      /* #if ( defined(IFXOS_HAVE_PIPE) && (IFXOS_HAVE_PIPE == 1) ) */

/** @} */

#endif      /* #ifdef RTEMS */


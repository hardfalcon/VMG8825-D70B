#ifndef _DRV_VMMC_CON_PRIV_H
#define _DRV_VMMC_CON_PRIV_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************
   Module      : drv_vmmc_con_priv.h
   Description : This file contains the declaration of all Connection module
******************************************************************************/

/* include the public header for this module */
#include "drv_vmmc_con.h"

/* define signal array inputs */
#define MAX_MODULE_SIGNAL_INPUTS 5


/** Data structure for one DSP signal input.
 It contains the signal array number for its input, a pointer to the next
 input connected to the same signal, the muting information and a
 pointer to the module to which the input is connected (used as previous
 pointer */
typedef struct _MODULE_SIGNAL
{
   /** Signal array index value from ECMD_IX_SIG connected to this input. */
   IFX_uint8_t i;
   /** Signal array index value used when this input is muted. */
   IFX_uint8_t i_mute;
   /** If not 0 this input is in muted state and uses the index from i_mute
       instead of i.  */
   IFX_uint8_t mute;
   /* Parent module. In case of muting, it must be set to modified */
   struct VMMCDSP_MODULE *pParent;
   /** Which output this input is connected to. */
   struct VMMCDSP_MODULE *pOut;
   /** Points to the next input to which the outSig is connected to.
       This input signal is within a linked list of input signals that are
       all connected to one output signal. */
   struct _MODULE_SIGNAL *pNext;
} VMMCDSP_MODULE_SIGNAL_t;

/** Data structure for one DSP module, like analog line module (ALM), Coder,
    PCM. The signaling module inherits the member and adds an additional
    input list */
typedef struct VMMCDSP_MODULE
{
   /** array of structures for each input of this module */
   VMMCDSP_MODULE_SIGNAL_t in[MAX_MODULE_SIGNAL_INPUTS];
   /** flag that indicates changes to the inputs (0 means no change) */
   IFX_uint8_t modified;
   /** the signal array index of this module's standard output */
   IFX_uint8_t nSignal;
   /** pointer to the first input signal which connects to the output */
   VMMCDSP_MODULE_SIGNAL_t *pInputs;
   /** flag that indicates that the standard output is muted */
   IFX_uint8_t nMute;
   /** the signal array index of this module's second output
       (only used for signaling modules) */
   IFX_uint8_t nSignal2;
   /** pointer to the first input signal which connects to the second output
       (only used for signaling modules) */
   VMMCDSP_MODULE_SIGNAL_t *pInputs2;
   /** flag that indicates that the second output is muted
       (only used for signaling modules) */
   IFX_uint8_t nMute2;
   /** defines the module type, value out of \ref VMMCDSP_MT */
   VMMCDSP_MT nModType;
   /** flag that indicates the current sampling mode off, WB, NB or automatic */
   VMMC_CON_SAMPLING sampling_mode;
   /** channel that this module is assigned to.
       Needed as link for module configuration from conference member list. */
   VMMC_CHANNEL *pCh;
} VMMCDSP_MODULE_t;

struct VMMC_CON
{
   struct VMMCDSP_MODULE  modAlm;
   struct VMMCDSP_MODULE  modCod;
   struct VMMCDSP_MODULE  modSig;
#ifdef VMMC_FEAT_PCM
   struct VMMCDSP_MODULE  modPcm;
#endif /* VMMC_FEAT_PCM */
#ifdef DECT_SUPPORT
   struct VMMCDSP_MODULE  modDect;
#endif /* DECT_SUPPORT */
};

/** Conference Member List Element structure.
    This is an element of the single linked list that contains the FW Modules
    connected in a conference.

    The element contains a pointer to the module struct defined above and
    additionally it comprises a pointer to the next list element (next) that
    is used for list traversal.
    An IFX_NULL pointer as next pointer indicates the end of the list. */
struct CONF_LIST_ELEMENT
{
   struct VMMCDSP_MODULE    *pModule;
   struct CONF_LIST_ELEMENT *next;
};
typedef struct CONF_LIST_ELEMENT CONF_LIST_ELEMENT_t;

#endif

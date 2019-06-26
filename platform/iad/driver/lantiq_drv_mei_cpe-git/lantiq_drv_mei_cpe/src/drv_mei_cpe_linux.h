#ifndef _DRV_MEI_CPE_LINUX_H
#define _DRV_MEI_CPE_LINUX_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : This file contains the includes and the defines
                 specific to the linux OS
   Remarks     : Please use the compiler switches here if you have
                 more than one OS.
   ========================================================================== */

#ifdef LINUX

/* ============================================================================
   Global Includes
   ========================================================================= */

/* Linux Includes*/

#ifdef __KERNEL__
#include <linux/kernel.h>
#endif
#include <linux/module.h>

#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/crc32.h>
#include <linux/vmalloc.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
   #if (MEI_SUPPORT_DEVICE_VR10_320 == 1)
      #include <linux/irq.h>
   #else
      #include <asm/ifx/irq.h>
   #endif
#else
   #include <linux/irq.h>
   #include <net/net_namespace.h>
   #include <linux/platform_device.h>
   #include <linux/of_irq.h>
   #include <linux/of_address.h>
#endif

#if (MEI_DRV_IFXOS_ENABLE == 0)

#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/types.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
   #include <asm/ifx/ifx_types.h>
#else
   #include <ifx_types.h>
#endif

#endif /* #if (MEI_DRV_IFXOS_ENABLE == 0)*/

#include <linux/dma-mapping.h>
#include <linux/dma-direction.h>
#include <asm/dma-mapping.h>
#include <linux/types.h>
#include <linux/pci.h>

/* ============================================================================
   typedefs interrupt wrapping (Linux)
   ========================================================================= */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,28))

/**
   typedef for return of Linux Interrupt Service Routine (Kernel Version < 2.4.28)
   - return type of the ISR is void.
*/
typedef void irqreturn_t;

/**
   define for return of Linux Interrupt Service Routine (Kernel Version < 2.4.28)
   - return type of the ISR is void, so no return.
*/
#define IRQ_RETVAL(x)

#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
typedef irqreturn_t (*usedIsrHandler_t)(int, void *, struct pt_regs *);
#else
typedef irqreturn_t (*usedIsrHandler_t)(int, void *);
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#define MEI_DRVOS_SIGNAL_PENDING             0
#else
#define MEI_DRVOS_SIGNAL_PENDING             signal_pending(current)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
#   ifndef PDE_DATA
#     define PDE_DATA(inode) PDE(inode)->data
#  endif
#endif

/**
   Function typedef for the Linux request_threaded_irq()
*/
typedef int (*MEI_RequestIrq_WrapLinux_t)( unsigned int usedIrq,
                                             usedIsrHandler_t usedIsrHandler,
                                             usedIsrHandler_t usedIsrThreadedHandler,
                                             unsigned long usedFlags,
                                             const char *pUsedDevName,
                                             void *pUsedDevId );

/**
   Function typedef for the Linux free_irq()
*/
typedef void (*MEI_FreeIrq_WrapLinux_t)( unsigned int usedIrq,
                                           void *usedDevId );


/**
   Function typedef for the Linux Interrupt Service Routine
*/
typedef irqreturn_t (*MEI_IntServRoutine_WrapLinux_t)(int, void *, struct pt_regs *);


/**
   Function typedef for the Linux Interrupt enable intEnable()
*/
typedef void (*MEI_IntEnable_WrapLinux_t)(int IRQNum);

/**
   Function typedef for the Linux Interrupt disable intDisable()
*/
typedef void (*MEI_IntDisable_WrapLinux_t)(int IRQNum);


/* ============================================================================
   global function (LINUX) - declarations
   ========================================================================= */

/* set wrapper functions for the interrupt handling */
extern int MEI_FctRequestIrqSet(MEI_RequestIrq_WrapLinux_t pRequestIrqFct);
extern int MEI_FctFreeIrqSet(MEI_FreeIrq_WrapLinux_t pFreeIrqFct);

#if (MEI_DRV_IFXOS_ENABLE == 0)

#define MEI_DRVOS_HAVE_DRV_SELECT   1

typedef struct semaphore    MEI_DRVOS_sema_t;
typedef wait_queue_head_t   MEI_DRVOS_selectQueue_t;
typedef struct file         MEI_DRVOS_select_OSArg_t;
typedef poll_table          MEI_DRVOS_selectTable_t;

/** LINUX Kernel - EVENT, type for event handling. */
typedef struct
{
   IFX_boolean_t bValid;
   /** event object */
   wait_queue_head_t object;
   /** wakeup condition flag (used for Kernel Version 2.6) */
   int bConditionFlag;
} MEI_DRVOS_event_t;

static inline int mei_sema_init(struct semaphore *sema)
{
   sema_init((sema),1);
   return IFX_SUCCESS;
}

static inline int mei_sema_delete(struct semaphore *sema)
{
   return 0;
}

static inline int mei_sema_lock(struct semaphore *sema)
{
   down((sema));
   return IFX_SUCCESS;
}

static inline int mei_sema_unlock(struct semaphore *sema)
{
   up((sema));
   return IFX_SUCCESS;
}

#define MEI_DRVOS_SemaphoreInit(id)          mei_sema_init((id))
#define MEI_DRVOS_SemaphoreDelete(id)        mei_sema_delete((id))
#define MEI_DRVOS_SemaphoreLock(id)          mei_sema_lock((id))
#define MEI_DRVOS_SemaphoreUnlock(id)        mei_sema_unlock((id))

#define MEI_DRVOS_SEL_WAKEUP_TYPE_RD          0

#define MEI_DRVOS_EVENT_INIT_VALID(pEvent)   ((((pEvent) != NULL) && ((pEvent)->bValid == IFX_TRUE)) ? IFX_TRUE : IFX_FALSE)

static inline int mei_init_waitqueue_head(MEI_DRVOS_event_t *pEventId)
{
   if (pEventId == NULL)
   {
      return IFX_ERROR;
   }

   pEventId->bConditionFlag = 0;
   pEventId->bValid = IFX_TRUE;

   init_waitqueue_head (&pEventId->object);

   return IFX_SUCCESS;
}

static inline int mei_wake_up_interruptible(MEI_DRVOS_event_t *pEventId)
{
   if (MEI_DRVOS_EVENT_INIT_VALID(pEventId) == IFX_FALSE)
   {
      return IFX_ERROR;
   }

   pEventId->bConditionFlag = 1;
   wake_up_interruptible(&pEventId->object);

   return IFX_SUCCESS;
}

static inline int mei_poll_wait(struct file *pDrvSelectOsArg, MEI_DRVOS_event_t *queue, poll_table *pDrvSelectTable)
{
   if (MEI_DRVOS_EVENT_INIT_VALID(queue) == IFX_FALSE)
   {
      return IFX_ERROR;
   }

   poll_wait( (struct file *)pDrvSelectOsArg,
              &(queue->object),
              (poll_table *)pDrvSelectTable );

   return IFX_SUCCESS;
}

#define MEI_DRVOS_SelectQueueInit(queue)                  mei_init_waitqueue_head((queue))
#define MEI_DRVOS_SelectQueueWakeUp(queue, drvSelType)    mei_wake_up_interruptible((queue))
#define MEI_DRVOS_SelectQueueAddTask(a,b,c)               mei_poll_wait((a),(b),(c))
#define MEI_DRVOS_EventInit(ev)                           mei_init_waitqueue_head((ev))

static inline int mei_event_delete(MEI_DRVOS_event_t *pEventId)
{
   if (MEI_DRVOS_EVENT_INIT_VALID(pEventId) == IFX_FALSE)
   {
      return IFX_ERROR;
   }

   pEventId->bConditionFlag = 0;
   pEventId->bValid = IFX_FALSE;

   return IFX_SUCCESS;
}

static inline int mei_interruptible_sleep_on_timeout(MEI_DRVOS_event_t *pEventId, unsigned int waitTime_ms)
{
   if (MEI_DRVOS_EVENT_INIT_VALID(pEventId) == IFX_FALSE)
   {
      return IFX_ERROR;
   }

#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   interruptible_sleep_on_timeout(&pEventId->object, (HZ * (waitTime_ms)) / 1000);
#  else
   wait_event_interruptible_timeout(pEventId->object,(pEventId->bConditionFlag == 1),((HZ * (waitTime_ms)) / 1000));
#  endif
   pEventId->bConditionFlag = 0;
   return IFX_SUCCESS;
}

#define MEI_DRVOS_EventDelete(ev)             mei_event_delete((ev))
#define MEI_DRVOS_EventWakeUp(ev)             mei_wake_up_interruptible((ev))
#define MEI_DRVOS_EventWait_timeout(_event_, _time_to_wait_) \
           mei_interruptible_sleep_on_timeout((_event_), (_time_to_wait_))

#define MEI_DRVOS_Malloc(block_size)      kmalloc((block_size), GFP_KERNEL)
#define MEI_DRVOS_Free(ptr_block)         kfree((ptr_block))

#define MEI_DRVOS_VirtMalloc(virt_size)   vmalloc((virt_size))
#define MEI_DRVOS_VirtFree(v_ptr)         vfree((v_ptr))

#define MEI_DRVOS_GetElapsedTime_ms(refTime_ms)  MEI_DRVOS_ElapsedTimeMSecGet((refTime_ms))
#define MEI_DRVOS_Wait_ms(sleepTime_ms)          MEI_DRVOS_MSecSleep((sleepTime_ms))

/* Max length of the Thread/Task name string.*/
#define MEI_DRVOS_THREAD_NAME_LEN                16

/** NULL pointer */
#ifndef IFX_NULL
#define IFX_NULL         ((void *)0)
#endif


/**
   User argument structure for the user thread start routine.
   Here the user can provide its own thread/task arguments to the thread function.
*/
typedef struct MEI_DRVOS_ThreadParams_s
{
   /** user argument 1 */
   IFX_ulong_t   nArg1;
   /** user argument 2 */
   IFX_ulong_t   nArg2;
   /** name of the thread/task */
   IFX_char_t     pName[MEI_DRVOS_THREAD_NAME_LEN];

   /** control - signal the run state */
   volatile IFX_boolean_t  bRunning;
   /** control - set to shutdown the thread */
   volatile IFX_boolean_t  bShutDown;
   /** points to the internal system object - for debugging */
   IFX_void_t              *pSysObject;
} MEI_DRVOS_ThreadParams_t;

/**
   Function type of the user thread/task function.
*/
typedef IFX_int32_t (*MEI_DRVOS_ThreadFunction_t)(MEI_DRVOS_ThreadParams_t *);

/**
   LINUX Kernel Thread - Control struct for thread handling.
*/
typedef struct
{
   /** Contains the user and thread control parameters */
   MEI_DRVOS_ThreadParams_t    thrParams;

   /** Points to the thread start routine */
   MEI_DRVOS_ThreadFunction_t  pThrFct;

   /** Kernel thread process ID */
   IFX_int32_t             tid;

   /** requested kernel thread priority */
   IFX_int32_t             nPriority;

   /** LINUX specific internal data - completion handling */
   struct completion       thrCompletion;

   /** flag indicates that the structure is initialized */
   IFX_boolean_t           bValid;
} MEI_DRVOS_ThreadCtrl_t;

/**
   LINUX Kernel Thread - function type LINUX Kernel Thread Start Routine.
*/
typedef int (*MEI_DRVOS_KERNEL_THREAD_StartRoutine)(void *);

/** LINUX Kernel Thread - thread options */
#define MEI_DRVOS_DRV_THREAD_OPTIONS                   (CLONE_FS | CLONE_FILES)

/** MEI_DRVOS Thread Delete - for user thread end wait forever */
#define MEI_DRVOS_THREAD_DELETE_WAIT_FOREVER     0xFFFFFFFF

/** LINUX Kernel Thread - internal poll time for check thread end */
#define MEI_DRVOS_THREAD_DOWN_WAIT_POLL_MS       10

/**
   Check the init status of the given mutex object
*/
#define MEI_DRVOS_THREAD_INIT_VALID(P_THREAD_ID)\
   (((P_THREAD_ID)) ? (((P_THREAD_ID)->bValid == IFX_TRUE) ? IFX_TRUE : IFX_FALSE) : IFX_FALSE)


static inline int mei_thread_priority_modify(unsigned int new_priority)
{
   return IFX_SUCCESS;
}

#define MEI_DRVOS_ThreadPriorityModify(new_priority) mei_thread_priority_modify((new_priority))

/**
   LINUX Kernel - Creates a new thread / task.

\par Implementation
   - Allocate and setup the internal thread control structure.
   - setup the LINUX specific thread parameter (see "init_completion").
   - start the LINUX Kernel thread with the internal stub function (see "kernel_thread")

\param
   pThrCntrl         Pointer to thread control structure. This structure has to
                     be allocated outside and will be initialized.
\param
   pName             specifies the 8-char thread / task name.
\param
   pThreadFunction   specifies the user entry function of the thread / task.
\param
   nArg1             first argument passed to thread / task entry function.
\param
   nArg2             second argument passed to thread / task entry function.

\return
   - IFX_SUCCESS thread was successful started.
   - IFX_ERROR thread was not started
*/
IFX_int32_t MEI_DRVOS_ThreadInit(
               MEI_DRVOS_ThreadCtrl_t *pThrCntrl,
               IFX_char_t *pName,
               MEI_DRVOS_ThreadFunction_t pThreadFunction,
               IFX_ulong_t nArg1,
               IFX_ulong_t nArg2);

/**
   LINUX Kernel - Shutdown and terminate a given thread.
   Therefore the thread delete functions triggers the user thread function
   to shutdown. In case of not response (timeout) the thread will be canceled.

\par Implementation
   - force a shutdown via the shutdown flag and wait.
   - wait for completion (see "wait_for_completion").
   - free previous allocated internal data.

\param
   pThrCntrl   Thread control struct.

\return
   - IFX_SUCCESS thread was successful deleted - thread control struct is freed.
   - IFX_ERROR thread was not deleted
*/
IFX_int32_t MEI_DRVOS_ThreadDelete(
               MEI_DRVOS_ThreadCtrl_t *pThrCntrl);


/**
   LINUX Kernel - Map the physical address to a virtual memory space.
   For virtual memory management this is required.

\par Implementation
   - check if the given physical memory region is free (see "check_mem_region")
   - reserve the given physical memory region (see "request_mem_region")
   - map the given physical memory region - no cache (see "ioremap_nocache")

\attention
   This sequence will reserve the requested memory region, so no following user
   can remap the same area after this.
\attention
   Other users (driver) which have map the area before (without reservation)
   will still have access to the area.

\param
   physicalAddr         The physical address for mapping [I]
\param
   addrRangeSize_byte   Range of the address space to map [I]
\param
   pName                The name of the address space, for administration [I]
\param
   ppVirtAddr           Returns the pointer to the virtual mapped address [O]

\return
   IFX_SUCCESS if the mapping was successful and the ppVirtAddr is set, else
   IFX_ERROR   if something was wrong.

*/
IFX_int32_t MEI_DRVOS_Phy2VirtMap(
               IFX_ulong_t physicalAddr,
               IFX_ulong_t    addrRangeSize_byte,
               IFX_char_t     *pName,
               IFX_uint8_t    **ppVirtAddr);

/**
   LINUX Kernel - Release the virtual memory range of a mapped physical address.
   For virtual memory management this is required.

\par Implementation
   - unmap the given physical memory region (see "iounmap")
   - release the given physical memory region (see "release_mem_region")

\param
   pPhysicalAddr        Points to the physical address for release mapping [IO]
                        (Cleared if success)
\param
   addrRangeSize_byte   Range of the address space to map [I]
\param
   ppVirtAddr           Provides the pointer to the virtual mapped address [IO]
                        (Cleared if success)

\return
   IFX_SUCCESS if the release was successful. The physicalAddr and the ppVirtAddr
               pointer is cleared, else
   IFX_ERROR   if something was wrong.
*/
IFX_int32_t MEI_DRVOS_Phy2VirtUnmap(
               IFX_ulong_t    *pPhysicalAddr,
               IFX_ulong_t    addrRangeSize_byte,
               IFX_uint8_t    **ppVirtAddr);

/**
   LINUX Kernel - Copy a block form driver space (kernel) TO USER space (application).

\par Implementation
   Copy data from kernel to user space by use of the kernel function "copy_to_user"

\param
   pTo         Points to the source (in driver space)
\param
   pFrom       Points to the destination (in user space)
\param
   size_byte   Block size to copy [byte]

\return
   IFX_NULL if an error occured, else pTo
*/
IFX_void_t *MEI_DRVOS_CpyToUser(
               IFX_void_t *pTo,
               const IFX_void_t *pFrom,
               IFX_uint32_t size_byte);

/**
   LINUX Kernel - Copy a block FORM USER space (application) to driver space (kernel).

\par Implementation
   Copy data from user to kernel space by use of the kernel function "copy_from_user"

\param
   pTo         Points to the source (in user space).
\param
   pFrom       Points to the destination (in driver space).
\param
   size_byte   Block size to copy [byte].

\return
   IFX_NULL if an error occured, else pTo
*/
IFX_void_t *MEI_DRVOS_CpyFromUser(
               IFX_void_t *pTo,
               const IFX_void_t *pFrom,
               IFX_uint32_t size_byte);

/**
   LINUX Kernel - Sleep a given time in [ms].

\attention
   The sleep requires a "sleep wait". "busy wait" implementation will not work.

\par Implementation
   Within the Kernel we use the LINUX scheduler to set the thread into "sleep".

\param
   sleepTime_ms   Time to sleep [ms]

\return
   None.

\remark
   Available in
   - Driver space
*/
IFX_void_t MEI_DRVOS_MSecSleep(
               IFX_time_t sleepTime_ms);

/**
   LINUX Kernel - Get the elapsed time in [ms].

\par Implementation
   Based on the HZ and jiffies defines we calculate the elapsed time since
   startup or based on the given ref-time.

\param
   refTime_ms  Reference time to calculate the elapsed time in [ms].

\return
   Elapsed time in [ms] based on the given reference time

\remark
   Provide refTime_ms = 0 to get the current elapsed time. For messurement provide
   the current time as reference.
*/
IFX_time_t MEI_DRVOS_ElapsedTimeMSecGet(
               IFX_time_t refTime_ms);

#define MEI_DRVOS
#define MEI_DRVOS_PRN_LEVEL_ERR

/** Define the used CR/LF sequence */
#define MEI_DRVOS_CRLF                              "\n\r"

   /** Kernel - Error Print on Appl-Level (formated) */
#if (MEI_SUPPORT_DEBUG_LOGGER == 1)
#  define MEI_DRVOS_ERR_PRINT_USR(fmt, args...) MEI_debugLogSend(fmt "\r", ##args)
#  define MEI_DRVOS_PRINT_INT_RAW(fmt, args...) MEI_debugLogSend(fmt, ##args)
#else
#  define MEI_DRVOS_ERR_PRINT_USR(fmt, args...)       printk(fmt "\r", ##args)
#  define MEI_DRVOS_PRINT_INT_RAW(fmt, args...)       printk(fmt, ##args)
#endif /* (MEI_SUPPORT_DEBUG_LOGGER == 1) */

#define MEI_PRINT_USR                     MEI_DRVOS_ERR_PRINT_USR
#define MEI_PRINT_INT                     MEI_DRVOS_ERR_PRINT_USR
#define MEI_PRINT_INT_RAW                 MEI_DRVOS_PRINT_INT_RAW

#define MEI_DRVOS_PRN_USR_ERR_NL(module_name, dbg_level, print_message) \
            do { \
                  { (void) MEI_DRVOS_ERR_PRINT_USR print_message ; }\
            } while(0)


#define MEI_DRV_OS_LITTLE_ENDIAN          1234
#define MEI_DRV_OS_BIG_ENDIAN             4321

#if defined ( __LITTLE_ENDIAN )
#  define MEI_DRV_OS_BYTE_ORDER          MEI_DRV_OS_LITTLE_ENDIAN
#elif defined ( __BIG_ENDIAN )
#  define MEI_DRV_OS_BYTE_ORDER          MEI_DRV_OS_BIG_ENDIAN
#endif

/** default task stack size */
#ifndef MEI_DRVOS_DEFAULT_STACK_SIZE
#define MEI_DRVOS_DEFAULT_STACK_SIZE      0
#endif /* #ifndef MEI_DRVOS_DEFAULT_STACK_SIZE*/

#endif /* #if (MEI_DRV_IFXOS_ENABLE == 0)*/

#define MEI_DRVOS_DMA_T dma_addr_t

#define MEI_DRVOS_DMA_Malloc(hwdev, size, ptr_phy) \
               dma_alloc_coherent((hwdev), (size), (ptr_phy), GFP_ATOMIC)

#define MEI_DRVOS_DMA_Free(hwdev, size, ptr_virt, ptr_phy) \
               dma_free_coherent((hwdev), (size), (ptr_virt), (ptr_phy))

#endif   /* LINUX */
#endif   /* _DRV_MEI_CPE_LINUX_H */


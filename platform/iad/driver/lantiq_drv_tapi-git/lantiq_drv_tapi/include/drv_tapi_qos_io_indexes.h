#ifndef DRV_TAPI_QOS_IO_INDEXES_H
#define DRV_TAPI_QOS_IO_INDEXES_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_tapi_qos_io_indexes.h  TAPI ioctl indexes to the QOS driver.

   \remarks
      This file contains the automatically generated code, please be careful
      during editing.
*/

/*
   TAPI QOS ioctl command indexes

   \remarks
      !!!IMPORTANT!!!
      Never touch that data between  'IOCTL_QOS_INDEXES_START'
        and 'IOCTL_QOS_INDEXES_END' directly.
      Use 'drv_tapi_qos_io.xml' and 'make ioctl' instead!
*/
enum {
/* IOCTL_QOS_INDEXES_START */
   FIO_QOS_START_IDX,
   FIO_QOS_ON_SOCKET_START_IDX,
   FIO_QOS_ACTIVATE_IDX,
   FIO_QOS_STOP_IDX,
   FIO_QOS_CLEAN_IDX
/* IOCTL_QOS_INDEXES_END */
};

#endif /* DRV_TAPI_QOS_IO_INDEXES_H */

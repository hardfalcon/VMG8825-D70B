#ifndef __SAS_SUPPORT_H__
#define __SAS_SUPPORT_H__

/* Pull macros and settings from current toolchain */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <features.h>
#include <sys/cdefs.h>

/* Generic helper definitions for shared library support. */
/* Visibility attributes are supported only in gcc 4.x */
#if __GNUC_PREREQ(4, 0)
#define SAS_API_EXPORT      __attribute__ ((visibility("default")))
#define SAS_API_LOCAL       __attribute__ ((visibility("hidden")))
#else /* SAS_DSO_COMPILE */
#define SAS_API_EXPORT
#define SAS_API_LOCAL
#endif /* SAS_DSO_COMPILE */

#define SAS_API_DEPRECATED  __attribute__ ((deprecated))

#endif //* __SAS_SUPPORT_H__  */

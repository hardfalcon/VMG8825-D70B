/**
 * @file  sas_osip_dns.h
 * @brief Implements osip_dns_record_t routines
 * @copyright 2009-2018 Sphairon GmbH (a ZyXEL company)
 *
 * SPDX-License-Identifier: Zyxel
 */

#include <osip2/internal.h>
#include <osip2/osip.h>

void osip_dns_record_release(osip_dns_record_t* dns_record)
{
  if (dns_record->query != NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "removing DNS record %s\n", dns_record->query));
    osip_free(dns_record->query);
  }
  osip_free(dns_record);
}

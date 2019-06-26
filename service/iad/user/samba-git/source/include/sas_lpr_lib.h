/**
 * @file	sas_lpr_lib.h 
 * @brief	Declaration of a simple Line Printer Remote (LPR) library
 * 
 * This file contains the declaration part of a simple Line Printer
 * Remote (LPR) library
 *
 * @author		Thomas Haak <thomas.haak@sphairon.com> +49(351)8925511
 * @copyright	Sphairon Access Systems GmbH, D-02605 Bautzen, Philipp-Reis-Str. 1
 * @date		23.10.2007
 * @version		1.0.0
 *
 ******************************************************************************/

#ifndef __SAS_LPR_LIB_H__
#define __SAS_LPR_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

/* TH: Only needed if used outside of Samba - #ifndef DEBUG
#define DEBUG(level, body) (void)(printf body)
#endif*/
	
/** sas_lpr public functions */

/**
 * @brief Checks whether sas_lpr socket has been already opened 
 *        or not
 * 
 * @retval 0	not opened 
 * @retval 1	opened 
 */
extern int sas_lpr_is_open();

/**
 * @brief Opens the sas_lpr
 *  
 * @param hostname    Name or IP address of the print host 
 * @param printername Name of the printer device 
 *  
 * @retval 0	Success
 * @retval -1	Error
 */
extern int sas_lpr_open(char* hostname, char* printername);

/**
 * @brief Writes print data to the sas_lpr
 *
 * @param data_buf   Buffer containing the data for printing 
 * @param numtowrite Number of data bytes in the buffer 
 *                   that shall be transfered to the printer
 * 
 * @retval 0	Success 
 * @retval -1	Error 
 */
extern int sas_lpr_write(const char* data_buf, int numtowrite);

/**
 * @brief Closes the sas_lpr
 * 
 * @retval 0	Success
 * @retval -1	Error
 */
extern int sas_lpr_close();

#ifdef __cplusplus
}
#endif

#endif /* __SAS_LPR_LIB_H__ */

/**
 * @file	sas_lpr_lib.c 
 * @brief	Implementation of a simple Line Printer Remote (LPR) library
 * 
 * This file contains the implementation part of a simple Line Printer
 * Remote (LPR) library
 *
 * @author		Thomas Haak <thomas.haak@sphairon.de> +49(351)8925511
 * @copyright	Sphairon Access Systems GmbH, D-02605 Bautzen, Philipp-Reis-Str. 1
 * @date		23.10.2007
 * @version		1.0.0
 *
 ******************************************************************************/

/**
 * Please refer to http://www.ietf.org/rfc/rfc1179.txt 
 * for the Line Printer Daemon protocol description 
 */

/**
 * Some of this code has been adapted from Chris Gonnerman's 
 * Universal LPR 
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "includes.h" /** Samba includes, give us everything we need */
extern int DEBUGLEVEL; /** Required for Samba debug messages */

#include "sas_lpr_lib.h"

/**
 * @brief Queue name for line printing device
 */
#define SAS_LPR_QUEUE "lp"

/**
 * @brief Blocksize for data transfer to lpd
 */
#define SAS_LPR_BLOCKSIZE 2048

/**
 * @brief Timeout for lpr socket communication
 */
int lpr_socket_timeout = 60; /** seconds */

/**
 * @brief Socket descriptor for shortcut to LPR socket
 */
int lpr_socket = 0;

/**
 * sas_lpr "private" function prototypes
 */

int sas_lpr_open_socket(char* hostname);
int sas_lpr_bind_socket(struct sockaddr_in *localaddr);
int sas_lpr_prepare_job(char* printername);
int sas_lpr_finish_job();
int sas_lpr_converse(char *message, int len, int sock);
void sas_lpr_converse_error(int rc, char *hostname);
int sas_lpr_net_write(int fd, const char *ptr, int nbytes);
int sas_lpr_net_read(int fd, char *ptr, int nbytes);
int sas_lpr_canread(int fd);

/**
 * public functions of sas_lpr module
 */

int sas_lpr_is_open()
{
    return lpr_socket == 0 ? 0 : 1;
}

int sas_lpr_open(char* hostname, char* printername)
{	
    /** Open lpr socket */
	
	DEBUG(3, ("sas_lpr_open: opening lpr socket to printer '%s' on host '%s'...\n", printername, hostname));
	
	if ((lpr_socket = sas_lpr_open_socket(hostname)) < 0)
	{
		DEBUG(3, ("sas_lpr_open: FAILED to open lpr socket\n"));
		return -1;
	}
	
	DEBUG(3, ("sas_lpr_open: successfully opened lpr socket\n"));
	
    /** Prepare for print job */
	
	DEBUG(3, ("sas_lpr_open: preparing print job...\n"));
	
	if (sas_lpr_prepare_job(printername) < 0)
	{
		DEBUG(3, ("sas_lpr_open: FAILED to prepare print job\n"));
		return -1;
	}
	
	DEBUG(3, ("sas_lpr_open: successfully prepared print job\n"));
	
	return 0;
}

int sas_lpr_write(const char* data_buf, int numtowrite)
{
	int readsize;
	int nleft;
	
	DEBUG(3, ("sas_lpr_write: writing data to the lpr...\n"));
	
	nleft = numtowrite;
	readsize = (nleft > SAS_LPR_BLOCKSIZE ? SAS_LPR_BLOCKSIZE : nleft);
	
	while(nleft > 0)
	{
		if(sas_lpr_net_write(lpr_socket, data_buf, readsize) == -1)
		{
			close(lpr_socket);
			lpr_socket = 0;
			DEBUG(3, ("sas_lpr_write: FAILED errno %d writing data to lpr socket\n", errno));
			return -1;
		}
		
		DEBUG(3, ("sas_lpr_write: %d bytes written to lpr socket\n", readsize));
		
        /** Move data pointer forward */
		data_buf += readsize;
		
		nleft -= readsize;
		readsize = (nleft > SAS_LPR_BLOCKSIZE ? SAS_LPR_BLOCKSIZE : (int)nleft);
	}
	
	return 0;
}

int sas_lpr_close()
{
    if (lpr_socket != 0) {
        sas_lpr_finish_job();
        
        DEBUG(3, ("sas_lpr_close: closing the lpr socket\n")); 
        close(lpr_socket);
        lpr_socket = 0;
    }
	
	return 0;
}

/**
 * "private" functions of sas_lpr module
 */

int sas_lpr_open_socket(char* hostname)
{
	struct hostent *host;
	struct sockaddr_in localaddr, remoteaddr;
	struct servent *sp;
	unsigned long inaddr;
	
	DEBUG(3, ("sas_lpr_open_socket: (hostname=%s)\n", hostname));
	
    /** Initialisation */
	
	memset((void *)&localaddr, 0, sizeof(struct sockaddr_in));
	localaddr.sin_family = AF_INET;
	
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	memset((void *)&remoteaddr, 0, sizeof(struct sockaddr_in));
	remoteaddr.sin_family = AF_INET;
	
    /** Look up remote host */
	
	if ((inaddr = inet_addr(hostname)) != INADDR_NONE)
	{
		memcpy((void *)&(remoteaddr.sin_addr), (void*)&inaddr, sizeof(inaddr));
	}
	else
	{
		if ((host = gethostbyname(hostname)) == NULL)
		{
			errno = 0; /** Means look at h_errno */
			return -1;
		}
		memcpy((void *)&(remoteaddr.sin_addr), host->h_addr_list[0],
			host->h_length);
	}
	
    /** Look up the remote port */

	if ((sp = getservbyname("printer", "tcp")) == NULL)
		remoteaddr.sin_port = htons(515);
	else
		remoteaddr.sin_port = sp->s_port;

    /** Create socket */

	if ((lpr_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		DEBUG(3, ("sas_lpr_open_socket: cannot create client socket.\n"));
		return -1;
	}

    /** Bind to socket */

	if (sas_lpr_bind_socket(&localaddr) == -1)
	{
		DEBUG(3, ("sas_lpr_open_socket: cannot bind to socket %d.\n",
			lpr_socket));
		close(lpr_socket);
		lpr_socket = 0;
		return -1;
	}
	
	DEBUG(3, ("sas_lpr_open_socket: trying to connect to: sock=%d remoteaddr=%x, remoteport=%d.\n",
			lpr_socket,remoteaddr.sin_addr,remoteaddr.sin_port));
	
	if (connect(lpr_socket, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr)) == -1)
	{
		DEBUG(3, ("sas_lpr_open_socket: cannot connect: sock=%d remoteaddr=%x, remoteport=%d.\n",
			lpr_socket,remoteaddr.sin_addr,remoteaddr.sin_port));
		close(lpr_socket);
		lpr_socket = 0;
		return -1;
	}
	else
	{
		return lpr_socket;
	}
}

int sas_lpr_bind_socket(struct sockaddr_in *localaddr)
{
	int trys_left, port_index;

    /** @todo Implement strict RFC 1179 compliance, i.e. solve
     *        issue that source port needs to be in the range
     *        721...731 (inclusive), but in this port range only
     *        root can open sockets */

#if STRICT_RFC1179_COMPLIANCE
    /** This is for strict rfc compliance */
	
	DEBUG(3, ("sas_lpr_bind_socket -- strict mode selected.\n"));
	
	trys_left = 11;
	port_index = time(NULL) % 11;

    /** Choose a "random" source address, repeat until well done */

	while (trys_left > 0)
	{
		localaddr->sin_port = htons(721 + port_index);

        DEBUG(3, ("sas_lpr_bind_socket: try bind: sock=%d, localaddr=%x, localport=%d)\n",
		lpr_socket, localaddr->sin_addr, localaddr->sin_port));

		if (bind(lpr_socket, (struct sockaddr *)localaddr,
			sizeof(struct sockaddr_in)) != -1)
				break;
		if (errno == EACCES)
		{
			DEBUG(3, ("sas_lpr should be installed setuid root\n"));
			return -1;
		}
		port_index = (port_index + 1) % 11;
		trys_left--;
	}
	
	if (trys_left <= 0)
	{
		errno = EAGAIN; /** Overloaded for our purposes */
		DEBUG(3, ("sas_lpr_bind_socket:  can't get rfc-strict reserved port (none available)\n"));
		return -1;
	}
#else
    /* 
     * No strict rfc compiance, we use a source port above 1023 in
     * order to allow non-root users printing as well
     */

    localaddr->sin_port = 1024; /** Using fixed source port above 1023 */
    DEBUG(3, ("sas_lpr_bind_socket: try bind: sock=%d, localaddr=%x, localport=%d)\n",
		lpr_socket, localaddr->sin_addr, localaddr->sin_port));

	if (bind(lpr_socket, (struct sockaddr *)localaddr,
			sizeof(struct sockaddr_in)) != -1) {
        return 0; /** Success */
    }
    else {
        DEBUG(3, ("sas_lpr_bind_socket: socket bind FAILED!\n"));
        return -1;
    }
#endif

	return 0;
}

int sas_lpr_prepare_job(char* printername)
{
	DEBUG(3, ("sas_lpr_prepare_job: send control file and prepare for receiving data...\n"));
	
	char cf_buf[SAS_LPR_BLOCKSIZE];	/** Control file buffer */
	char cf_filename[128];			/** Name of the control file */
	char df_filename[128];			/** Name of the data file */
	char my_hostname[128];
	char *my_username;
	//char *queue_name = SAS_LPR_QUEUE;
    char *queue_name = printername; /** Use the printer name for naming the queue */
    char message[128];				/** lpr message buffer */
	int cf_len;						/** Length of the control file */
	int rc;							/** Result code */
	char *filename;
	size_t numtowrite;				/** Number of bytes to be written to lpr */
	int seqno = 0;					/** Sequence number in order to make
                                     * filenames unique (we always use 0 in the
                                     * hope that this isn't a problem, i.e.
                                     * never more than one job in the queue...)
                                     */
	
    /** Initialisation */
	
	filename = "sas_lpr_doc";
	if ((my_username = getenv("USER")) == NULL)
		my_username = "user";
	if (gethostname(my_hostname, 128) == -1)
	{
        DEBUG(3, ("sas_lpr_prepare_job: FAILED to get hostname\n"));
        return -1;
    }

    /** Give up priviledges */
	setuid(getuid());
	
	/**
     * RFC 1179 says all servers must be able to accept control file
     * first... so open w-i-d-e, here it is 
	 */
	
    /** Prepare file names */
	snprintf(cf_filename, 128, "cfA%03d%s", seqno, my_hostname); /* TODO: how unique does this file name need to be? */
	snprintf(df_filename, 128, "dfA%03d%s", seqno, my_hostname); /* TODO: how unique does this file name need to be? */
	
    /** Send 'receive a printer job' command */
	DEBUG(3, ("sas_lpr_prepare_job: sending 'receive a printer job' command...\n"));
	snprintf(message, 128, "\02%s\n", queue_name);
	if ((rc = sas_lpr_converse(message, strlen(message), lpr_socket)) != 0)
	{
		close(lpr_socket);
		lpr_socket = 0;
		sas_lpr_converse_error(rc, my_hostname);
		DEBUG(3, ("sas_lpr_prepare_job: FAILED to send 'receive a printer job' command\n"));
		return -1;
	}

    /** Compose control file contents */
	snprintf(cf_buf, SAS_LPR_BLOCKSIZE, "H%s\nP%s\nJ%s\nC%s\nf%s\nU%s\nN%s\n",
		my_hostname, my_username, filename, filename, df_filename, df_filename,
		filename);
	
	cf_len = strlen(cf_buf);
	
    /** Send 'receive control file' command */
	DEBUG(3, ("sas_lpr_prepare_job: sending 'receive control file' command...\n"));
	snprintf(message, 128, "\02%d %s\n", cf_len, cf_filename);
	if ((rc = sas_lpr_converse(message, strlen(message), lpr_socket)) != 0)
	{
		close(lpr_socket);
		lpr_socket = 0;
		sas_lpr_converse_error(rc, my_hostname);
		DEBUG(3, ("sas_lpr_prepare_job: FAILED to send 'receive control file' command\n"));
		return -1;
	}
	
    /** Send control file contents */
	DEBUG(3, ("sas_lpr_prepare_job: sending control file contents...\n"));
	if (sas_lpr_net_write(lpr_socket, cf_buf, cf_len) == -1)
	{
		close(lpr_socket);
		lpr_socket = 0;
		DEBUG(3, ("sas_lpr_prepare_job: FAILED with errno %d writing control file\n", errno));
		return -1;
	}
	
    /** Acknowledge 'receive control file' command */
	DEBUG(3, ("sas_lpr_prepare_job: acknowledge 'receive control file' command...\n"));
	message[0] = '\0';
	if ((rc = sas_lpr_converse(message, 1, lpr_socket)) != 0)
	{
		close(lpr_socket);
		lpr_socket = 0;
		sas_lpr_converse_error(rc, my_hostname);
		DEBUG(3, ("sas_lpr_prepare_job: FAILED in receiving control file\n"));
		return -1;
	}
	
	/**
     * We don't know the total number of bytes to be printed 
     * therefore 'Count' should be cleared to 0 (see rfc1179) 
	 */
	numtowrite = 0;

    /** Send 'receive data file' command */
	DEBUG(3, ("sas_lpr_prepare_job: sending 'receive data file' command...\n"));
	snprintf(message, 128, "\03%ld %s\n", (long)numtowrite, df_filename);
	if ((rc = sas_lpr_converse(message, strlen(message), lpr_socket)) != 0)
	{
		close(lpr_socket);
		lpr_socket = 0;
		sas_lpr_converse_error(rc, my_hostname);
		DEBUG(3, ("sas_lpr_prepare_job: FAILED to send 'receive data file' command\n"));
		return -1;
	}
	
	return 0;
}

int sas_lpr_finish_job()
{
	char my_hostname[128];
	int rc;					/** Result code */
	char message[128]; 		/** LPD control command */
	
	if (gethostname(my_hostname, 128) == -1)
	{
		DEBUG(3, ("sas_lpr_finish_job: FAILED to get hostname\n"));
		return -1;
	}

    /**
     * Since we didn't define the number of bytes in the print data 
     * we also shouldn't acknowledge the reception - see rfc1179 
     */
#if 0
    /** Acknowledge 'receive data file' command */
	DEBUG(3, ("sas_lpr_finish_job: acknowledge 'receive data file' command...\n"));
	message[0] = '\0';
	if ((rc = sas_lpr_converse(message, 1, lpr_socket)) != 0)
	{
		close(lpr_socket);
		lpr_socket = 0;
		sas_lpr_converse_error(rc, my_hostname);
		DEBUG(3, ("sas_lpr_prepare_job: FAILED in receiving data file\n"));
		return -1;
	}
#endif

	DEBUG(3, ("sas_lpr_finish_job: successfully finished job\n"));
	
	return 0;
}

int sas_lpr_converse(char *message, int len, int socket)
{
	char ch;
	
	ch = message[len-1];
	message[len-1] = '\0';
	DEBUG(3, ("sas_lpr_converse(message=[%x]%s, len=%d, sock=%d)\n", message[0], message+1, len, socket));
	message[len-1] = ch;
	
	if(sas_lpr_net_write(socket, message, len) == -1)
		return -1;
	
	if(sas_lpr_net_read(socket, &ch, 1) == -1)
		return -1;
	
	return ch;
}

void sas_lpr_converse_error(int rc, char *hostname)
{
	DEBUG(3, ("converse_error(rc=%d, hostname=%s)\n", rc, hostname));
	
	if(rc == -1)
		DEBUG(3, ("sas_lpr:  errno %d from server %s\n", errno, hostname));
	else
		DEBUG(3, ("sas_lpr:  return code %d from server %s\n", rc, hostname));
}

/**
 * @brief	Write nbytes from ptr to fd
 *
 * @retval 0 success 
 * @retval -1 error 
                                */
int sas_lpr_net_write(int fd, const char *ptr, int nbytes)
{
	int	nleft, nwritten;
	
	DEBUG(4, ("sas_lpr_net_write: fd=%d, ptr=%lx, nbytes=%d\n", fd,
		(unsigned long)ptr, nbytes));
	
	nleft = nbytes;
	
	while (nleft > 0)
	{
		nwritten = write(fd, ptr, nleft);
		
		if (nwritten <= 0)
		{
			DEBUG(3, ("sas_lpr_net_write() FAILED!\n"));
			return -1;
		}
		
		nleft -= nwritten;
		ptr   += nwritten;
	}
	
	return 0;
}

/**
 * @brief	Read up to nbytes into ptr from fd
 *
 * @retval >=0 actual bytes read 
 * @retval -1 error 
                                             */
int sas_lpr_net_read(int fd, char *ptr, int nbytes)
{
	int	nleft, nread;
	
	DEBUG(3, ("net_read(fd=%d, ptr=%lx, nbytes=%d)\n", 
		fd, (unsigned long)ptr, nbytes));
	
	nleft = nbytes;
	while (nleft > 0)
	{
		if(!sas_lpr_canread(fd))
		{
			DEBUG(3, ("timeout in sas_lpr_net_read()\n"));
			errno = ETIMEDOUT;
			return -1;
		}
		nread = read(fd, ptr, nleft);
		if (nread < 0)
		{
			DEBUG(3, ("sas_lpr_net_read() FAILED!\n"));
			return(nread); /** error, return < 0 */
		}
		else if (nread == 0)
			break; /** EOF */
		
		nleft -= nread;
		ptr   += nread;
	}
	return(nbytes - nleft); /** return >= 0 */
}

int sas_lpr_canread(int fd)
{
	fd_set r, e;
	struct timeval tv;
	int rc;
	
	DEBUG(3, ("sas_lpr_canread: fd=%d\n", fd));
	
	FD_ZERO(&r);
	FD_ZERO(&e);
	
	FD_SET(fd, &r);
	FD_SET(fd, &e);
	
	tv.tv_sec = lpr_socket_timeout;
	tv.tv_usec = 0;
	
	rc = select(fd+1, &r, NULL, &e, &tv);
	
	DEBUG(3, ("sas_lpr_canread(): select() returned %d\n", rc));
	DEBUG(3, ("sas_lpr_canread(): FD_ISSET(%d, &r) = %d\n", fd,
		FD_ISSET(fd, &r)));
	DEBUG(3, ("sas_lpr_canread(): FD_ISSET(%d, &e) = %d\n", fd,
		FD_ISSET(fd, &e)));
	
	if (!rc || FD_ISSET(fd, &e))
		return 0;
	
	if (FD_ISSET(fd, &r))
		return 1;
	
	/** shouldn't reach here... */
	return 0;
}

/**
 * Line Printer Daemon (LPD) protocol commands - see also rfc1179
 */

#if 0 /** @todo currently command encapsulation is not implemented */
int lpd_cmd_rx_print_job(char* queue_name)
{
	char lpd_cmd_msg[128];	/** LPD command message */
	int rc;					/** Result code */
	
    /** Send 'receive a printer job' command */
	DEBUG(3, ("sas_lpr: sending 'receive a printer job' command...\n"));
	snprintf(lpd_cmd_msg, 128, "\02%s\n", queue_name);
	if ((rc = sas_lpr_converse(lpd_cmd_msg, strlen(lpd_cmd_msg), lpr_socket)) != 0)
	{
		close(lpr_socket);
		lpr_socket = 0;
		sas_lpr_converse_error(rc, my_hostname);
		DEBUG(3, ("sas_lpr: FAILED to send 'receive a printer job' command\n"));
		return -1;
	}
}
#endif


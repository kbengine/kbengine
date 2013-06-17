/*
 * get_mib2.h -- definitions for the get_mib2() function
 *
 * V. Abell <abe@cc.purdue.edu>
 * Purdue University Computing Center
 */


/*
 * Copyright 1995 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Victor A. Abell <abe@cc.purdue.edu>
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither Victor A  Abell nor Purdue University are responsible for
 *    any consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to Victor A. Abell and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

/*
 * Altered for sigar:
 * - remove static stuff to make thread-safe by Doug MacEachern (3/11/05)
 */

#if	!defined(GET_MIB2_H)
#define	GET_MIB2_H


/*
 * Required header files
 */

#include <stropts.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stream.h>
#include <sys/tihdr.h>
#include <sys/tiuser.h>
#include <inet/mib2.h>
#include <inet/led.h>


/*
 * Miscellaneous definitions
 */

#define	GET_MIB2_ARPDEV		"/dev/arp"	/* ARP stream devi9ce */
#define	GET_MIB2_ERRMSGL	1024		/* ErrMsg buffer length */
#define	GET_MIB2_TCPSTREAM	"tcp"		/* TCP stream name */
#define	GET_MIB2_UDPSTREAM	"udp"		/* UDP stream name */


/*
 * get_mib2() response codes
 *
 * 	-1		End of MIB2 information
 *	 0		Next MIB2 structure returned
 *	>0		Error code
 */

#define	GET_MIB2_EOD		-1	/* end of data */
#define	GET_MIB2_OK		0	/* function succeeded */
#define	GET_MIB2_ERR_ACK	1	/* getmsg() ACK error received */
#define	GET_MIB2_ERR_ARPOPEN	2	/* error opening ARPDEV */
#define	GET_MIB2_ERR_CLOSE	3	/* MIB2 access close error */
#define	GET_MIB2_ERR_GETMSGD	4	/* error getting message data */
#define	GET_MIB2_ERR_GETMSGR	5	/* error getting message reply */
#define	GET_MIB2_ERR_NODATA	6	/* data expected; not received */
#define	GET_MIB2_ERR_NOSPC	7	/* no malloc() space */
#define	GET_MIB2_ERR_NOTOPEN	8	/* MIB2 access not open */
#define	GET_MIB2_ERR_OPEN	9	/* MIB2 access open error */
#define	GET_MIB2_ERR_PUTMSG	10	/* error putting request message */
#define	GET_MIB2_ERR_TCPPUSH	11	/* error pushing TCPSTREAM */
#define	GET_MIB2_ERR_UDPPUSH	12	/* error pushing UDPSTREAM */

#define	GET_MIB2_ERR_MAX	13	/* maximum error number + 1 */


typedef struct {
    char *db;       /* data buffer */
    int db_len;     /* data buffer length */
    char *smb;      /* stream message buffer */
    size_t smb_len; /* size of Smb[] */
    int sd;         /* stream device descriptor */
    char errmsg[GET_MIB2_ERRMSGL];      /* error message buffer */
    struct T_optmgmt_ack *op_ack;       /* message ACK pointer */ 
    struct strbuf ctlbuf;               /* streams control buffer */
    struct T_error_ack *err_ack;        /* message error pointer */
    struct opthdr *op;                  /* message option pointer */
    struct T_optmgmt_req *req;          /* message request pointer */
} solaris_mib2_t;

/*
 * Function prototypes
 */

int close_mib2(				/* close acccess to MIB2 information */
        solaris_mib2_t *mib2 
	);
int get_mib2(				/* get MIB2 information */
        solaris_mib2_t *mib2, 
	struct opthdr **opt,			/* opthdr pointer return (see
						 * <sys/socket.h> */
	char **data,				/* data buffer return address */
	int *datalen				/* data buffer length return
						 * address */
	);
int open_mib2(				/* open acccess to MIB2 information */
        solaris_mib2_t *mib2 
	);

#endif	/* !defined(GET_MIB2_H) */

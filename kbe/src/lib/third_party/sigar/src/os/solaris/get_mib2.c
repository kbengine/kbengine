/*
 * get_mib2() -- get MIB2 information from Solaris 2.[3-7] kernel
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

#if 0 /*ndef lint -Wall -Werror*/ 
static char copyright[] =
"@(#) Copyright 1995 Purdue Research Foundation.\nAll rights reserved.\n";
#endif

#include "get_mib2.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/*
 * close_mib2() - close MIB2 access
 *
 * return:
 *
 *	exit = GET_MIB2_OK if close succeeded
 *	       GET_MIB2_* is the error code.
 */

int
close_mib2(solaris_mib2_t *mib2)
{
    if (mib2->sd < 0) {
        (void) strcpy(mib2->errmsg, "close_mib2: socket not open");
        return(GET_MIB2_ERR_NOTOPEN);
    }
    if (close(mib2->sd)) {
        (void) sprintf(mib2->errmsg, "close_mib2: %s", strerror(errno));
        return(GET_MIB2_ERR_CLOSE);
    }
    mib2->sd = -1;
    if (mib2->db_len && mib2->db) {
        mib2->db_len = 0;
        free((void *)mib2->db);
        mib2->db = NULL;
    }
    if (mib2->smb_len && mib2->smb) {
        mib2->smb_len = 0;
        free((void *)mib2->smb);
        mib2->smb = NULL;
    }
    return(GET_MIB2_OK);
}


/*
 * get_mib2() - get MIB2 data
 *
 * return:
 *
 *	exit = GET_MIB2_OK if get succeeded, and:
 *			*opt = opthdr structure address
 *			*data = data buffer address
 *			*datalen = size of data buffer
 *	       GET_MIB2_* is the error code for failure.
 */

int
get_mib2(solaris_mib2_t *mib2,
         struct opthdr **opt,
         char **data,
         int *datalen)
{
    struct strbuf d;		/* streams data buffer */
    int err;			/* error code */
    int f;				/* flags */
    int rc;				/* reply code */

    /*
     * If MIB2 access isn't open, open it and issue the preliminary stream
     * messages.
     */
    if (mib2->sd < 0) {
	/*
	 * Open access.  Return on error.
	 */
        if ((err = open_mib2(mib2))) {
            return(err);
        }
	/*
	 * Set up message request and option.
	 */
        mib2->req = (struct T_optmgmt_req *)mib2->smb;
        mib2->op = (struct opthdr *)&mib2->smb[sizeof(struct T_optmgmt_req)];
        mib2->req->PRIM_type = T_OPTMGMT_REQ;
        mib2->req->OPT_offset = sizeof(struct T_optmgmt_req);
        mib2->req->OPT_length = sizeof(struct opthdr);

#if	defined(MI_T_CURRENT)
        mib2->req->MGMT_flags = MI_T_CURRENT;
#else	/* !defined(MI_T_CURRENT) */
# if	defined(T_CURRENT)
        mib2->req->MGMT_flags = T_CURRENT;
# else	/* !defined(T_CURRENT) */
#error	"Neither MI_T_CURRENT nor T_CURRENT are defined."
# endif	/* defined(T_CURRENT) */
#endif	/* defined(MI_T_CURRENT) */

        mib2->op->level = MIB2_IP;
        mib2->op->name = mib2->op->len = 0;
        mib2->ctlbuf.buf = mib2->smb;
        mib2->ctlbuf.len = mib2->req->OPT_offset + mib2->req->OPT_length;
	/*
	 * Put the message.
	 */
        if (putmsg(mib2->sd, &mib2->ctlbuf, (struct strbuf *)NULL, 0) == -1) {
            (void) sprintf(mib2->errmsg,
                           "get_mib2: putmsg request: %s", strerror(errno));
            return(GET_MIB2_ERR_PUTMSG);
        }
	/*
	 * Set up to process replies.
	 */
        mib2->op_ack = (struct T_optmgmt_ack *)mib2->smb;
        mib2->ctlbuf.maxlen = mib2->smb_len;
        mib2->err_ack = (struct T_error_ack *)mib2->smb;
        mib2->op = (struct opthdr *)&mib2->smb[sizeof(struct T_optmgmt_ack)];
    }
    /*
     * Get the next (first) reply message.
     */
    f = 0;
    if ((rc = getmsg(mib2->sd, &mib2->ctlbuf, NULL, &f)) < 0) {
        (void) sprintf(mib2->errmsg, "get_mib2: getmsg(reply): %s",
                       strerror(errno));
        return(GET_MIB2_ERR_GETMSGR);
    }
    /*
     * Check for end of data.
     */
    if (rc == 0
	&&  mib2->ctlbuf.len >= sizeof(struct T_optmgmt_ack)
	&&  mib2->op_ack->PRIM_type == T_OPTMGMT_ACK
	&&  mib2->op_ack->MGMT_flags == T_SUCCESS
	&&  mib2->op->len == 0)
    {
        err = close_mib2(mib2);
        if (err) {
            return(err);
        }
        return(GET_MIB2_EOD);
    }
    /*
     * Check for error.
     */
    if (mib2->ctlbuf.len >= sizeof(struct T_error_ack)
	&&  mib2->err_ack->PRIM_type == T_ERROR_ACK)
    {
        (void) sprintf(mib2->errmsg,
                       "get_mib2: T_ERROR_ACK: len=%d, TLI=%#x, UNIX=%#x",
                       mib2->ctlbuf.len,
                       (int)mib2->err_ack->TLI_error,
                       (int)mib2->err_ack->UNIX_error);
        return(GET_MIB2_ERR_ACK);
    }
    /*
     * Check for no data.
     */
    if (rc != MOREDATA
	||  mib2->ctlbuf.len < sizeof(struct T_optmgmt_ack)
	||  mib2->op_ack->PRIM_type != T_OPTMGMT_ACK
	||  mib2->op_ack->MGMT_flags != T_SUCCESS)
    {
        (void) sprintf(mib2->errmsg,
                       "get_mib2: T_OPTMGMT_ACK: "
                       "rc=%d len=%d type=%#x flags=%#x",
                       rc, mib2->ctlbuf.len,
                       (int)mib2->op_ack->PRIM_type,
                       (int)mib2->op_ack->MGMT_flags);
        return(GET_MIB2_ERR_NODATA);
    }
    /*
     * Allocate (or enlarge) the data buffer.
     */
    if (mib2->op->len >= mib2->db_len) {
        mib2->db_len = mib2->op->len;
        if (mib2->db == NULL) {
            mib2->db = (char *)malloc(mib2->db_len);
        }
        else {
            mib2->db = (char *)realloc(mib2->db, mib2->db_len);
        }
        if (mib2->db == NULL) {
            (void) sprintf(mib2->errmsg,
                           "get_mib2: no space for %d byte data buffer",
                           mib2->db_len);
            return(GET_MIB2_ERR_NOSPC);
        }
    }
    /*
     * Get the data part of the message -- the MIB2 part.
     */
    d.maxlen = mib2->op->len;
    d.buf = mib2->db;
    d.len = 0;
    f = 0;
    if ((rc = getmsg(mib2->sd, NULL, &d, &f)) < 0) {
        (void) sprintf(mib2->errmsg, "get_mib2: getmsg(data): %s",
                       strerror(errno));
        return(GET_MIB2_ERR_GETMSGD);
    }
    if (rc) {
        (void) sprintf(mib2->errmsg,
                       "get_mib2: getmsg(data): rc=%d maxlen=%d len=%d: %s",
                       rc, d.maxlen, d.len, strerror(errno));
        return(GET_MIB2_ERR_GETMSGD);
    }
    /*
     * Compose a successful return.
     */
    *opt = mib2->op;
    *data = mib2->db;
    *datalen = d.len;
    return(GET_MIB2_OK);
}


/*
 * open_mib2() - open access to MIB2 data
 *
 * return:
 *
 *	exit = GET_MIB2_OK if open succeeded
 *	       GET_MIB2_* is the error code for failure.
 */

int
open_mib2(solaris_mib2_t *mib2)
{
    /*
     * It's an error if the stream device is already open.
     */
    if (mib2->sd >= 0) {
        (void) strcpy(mib2->errmsg, "open_mib2: MIB2 access already open");
        return(GET_MIB2_ERR_OPEN);
    }
    /*
     * Open the ARP stream device, push TCP and UDP on it.
     */
    if ((mib2->sd = open(GET_MIB2_ARPDEV, O_RDWR, 0600)) < 0) {
        (void) sprintf(mib2->errmsg, "open_mib2: %s: %s", GET_MIB2_ARPDEV,
                       strerror(errno));
        return(GET_MIB2_ERR_ARPOPEN);
    }
    if (ioctl(mib2->sd, I_PUSH, GET_MIB2_TCPSTREAM) == -1) {
        (void) sprintf(mib2->errmsg, "open_mib2: push %s: %s",
                       GET_MIB2_TCPSTREAM, strerror(errno));
        return(GET_MIB2_ERR_TCPPUSH);
    }
    if (ioctl(mib2->sd, I_PUSH, GET_MIB2_UDPSTREAM) == -1) {
        (void) sprintf(mib2->errmsg, "open_mib2: push %s: %s",
                       GET_MIB2_UDPSTREAM, strerror(errno));
        return(GET_MIB2_ERR_UDPPUSH);
    }
    /*
     * Allocate a stream message buffer.
     */
    mib2->smb_len = sizeof(struct opthdr) + sizeof(struct T_optmgmt_req);
    if (mib2->smb_len < (sizeof (struct opthdr) + sizeof(struct T_optmgmt_ack))) {
        mib2->smb_len = sizeof (struct opthdr) + sizeof(struct T_optmgmt_ack);
    }
    if (mib2->smb_len < sizeof(struct T_error_ack)) {
        mib2->smb_len = sizeof(struct T_error_ack);
    }
    if ((mib2->smb = (char *)malloc(mib2->smb_len)) == NULL) {
        (void) strcpy(mib2->errmsg,
                      "open_mib2: no space for stream message buffer");
        return(GET_MIB2_ERR_NOSPC);
    }
    /*
     * All is OK.  Return that indication.
     */
    return(GET_MIB2_OK);
}

/* Extracted from anet.c to work properly with Hiredis error reporting.
 *
 * Copyright (c) 2006-2011, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010-2011, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 * Copyright (c) 2014, Ren Bin <232811979 at 163 dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "fmacros.h"
#include <sys/types.h>
#define HIREDIS_WIN
#ifndef HIREDIS_WIN
#	include <sys/socket.h>
#	include <sys/select.h>
#	include <sys/un.h>
#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <arpa/inet.h>
#	include <unistd.h>
#	include <netdb.h>
#	include <poll.h>
#else
#	include <WinSock2.h>
#endif//HIREDIS_WIN

#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>

#include "net.h"
#include "sds.h"

/* Defined in hiredis.c */
void __redisSetError(redisContext *c, int type, const char *str);

static void __redisSetErrorFromErrno(redisContext *c, int type, const char *prefix) {
    char buf[128];
    size_t len = 0;

#ifndef HIREDIS_WIN
	if (prefix != NULL)
		len = snprintf(buf,sizeof(buf),"%s: ",prefix);
    strerror_r(errno,buf+len,sizeof(buf)-len);
#else
	LPVOID lpMsgBuf;
	if (prefix != NULL)
		len = sprintf_s(buf,sizeof(buf),"%s: ",prefix);

	FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS, 
		NULL, 
		errno, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		(LPTSTR) &lpMsgBuf, 
		0, 
		NULL 
		);
	strcpy_s(buf+len,sizeof(buf)-len,(char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
#endif
    __redisSetError(c,type,buf);
}

static int redisSetReuseAddr(redisContext *c, int fd) {
    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) == -1) {
        __redisSetErrorFromErrno(c,REDIS_ERR_IO,NULL);
#ifndef HIREDIS_WIN
        close(fd);
#else
		errno = WSAGetLastError();
		closesocket(fd);
#endif
        return REDIS_ERR;
    }
    return REDIS_OK;
}

#ifndef HIREDIS_WIN
static int redisCreateSocket(redisContext *c, int type) {
    int s;
    if ((s = socket(type, SOCK_STREAM, 0)) == -1) {
        __redisSetErrorFromErrno(c,REDIS_ERR_IO,NULL);
        return REDIS_ERR;
    }
    if (type == AF_INET) {
        if (redisSetReuseAddr(c,s) == REDIS_ERR) {
            return REDIS_ERR;
        }
    }
    return s;
}
#endif

static int redisSetBlocking(redisContext *c, int fd, int blocking) {
	int flags;

#ifndef HIREDIS_WIN
    /* Set the socket nonblocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        __redisSetErrorFromErrno(c,REDIS_ERR_IO,"fcntl(F_GETFL)");
        close(fd);
        return REDIS_ERR;
    }

    if (blocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1) {
        __redisSetErrorFromErrno(c,REDIS_ERR_IO,"fcntl(F_SETFL)");
        close(fd);
        return REDIS_ERR;
    }
#else
	flags = blocking == 0;

	if (ioctlsocket(fd, FIONBIO, &flags) != NO_ERROR)
	{
		errno = WSAGetLastError();
		__redisSetErrorFromErrno(c,REDIS_ERR_IO,"ioctlsocket(FIONBIO)");
		closesocket(fd);
		return REDIS_ERR;
	}
#endif
	return REDIS_OK;
}

static int redisSetTcpNoDelay(redisContext *c, int fd) {
    int yes = 1;
	if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&yes, sizeof(yes)) == -1) {
#ifndef HIREDIS_WIN
		__redisSetErrorFromErrno(c,REDIS_ERR_IO,"setsockopt(TCP_NODELAY)");
        close(fd);
#else
		errno = WSAGetLastError();
		__redisSetErrorFromErrno(c,REDIS_ERR_IO,"setsockopt(TCP_NODELAY)");
		closesocket(fd);
#endif
        return REDIS_ERR;
    }
    return REDIS_OK;
}

#define __MAX_MSEC (((LONG_MAX) - 999) / 1000)

static int redisContextWaitReady(redisContext *c, int fd, const struct timeval *timeout) {
#ifndef HIREDIS_WIN
	struct pollfd   wfd[1];
	long msec;

	msec          = -1;
	wfd[0].fd     = fd;
	wfd[0].events = POLLOUT;

	/* Only use timeout when not NULL. */
	if (timeout != NULL) {
		if (timeout->tv_usec > 1000000 || timeout->tv_sec > __MAX_MSEC) {
			close(fd);
			return REDIS_ERR;
		}

		msec = (timeout->tv_sec * 1000) + ((timeout->tv_usec + 999) / 1000);

		if (msec < 0 || msec > INT_MAX) {
			msec = INT_MAX;
		}
	}

	if (errno == EINPROGRESS) {
		int res;

		if ((res = poll(wfd, 1, msec)) == -1) {
			__redisSetErrorFromErrno(c, REDIS_ERR_IO, "poll(2)");
			close(fd);
			return REDIS_ERR;
		} else if (res == 0) {
			errno = ETIMEDOUT;
			__redisSetErrorFromErrno(c,REDIS_ERR_IO,NULL);
			close(fd);
			return REDIS_ERR;
		}

		if (redisCheckSocketError(c, fd) != REDIS_OK)
			return REDIS_ERR;

		return REDIS_OK;
	}

	__redisSetErrorFromErrno(c,REDIS_ERR_IO,NULL);
	close(fd);
	return REDIS_ERR;
#else
	if (errno == WSAEINPROGRESS || errno == WSAEWOULDBLOCK )
	{
		int res;
		fd_set wfds,efds;
		FD_ZERO(&wfds);
		FD_ZERO(&efds);
		FD_SET(fd,&wfds);
		FD_SET(fd,&efds);

		if((res = select(0,NULL,&wfds,&efds,timeout)) == SOCKET_ERROR)
		{
			errno = WSAGetLastError();
			__redisSetErrorFromErrno(c, REDIS_ERR_IO, "select(2)");
			closesocket(fd);
			return REDIS_ERR;
		}
		else if(res == 0)
		{
			errno = WSAETIMEDOUT;
			__redisSetErrorFromErrno(c,REDIS_ERR_IO,NULL);
			closesocket(fd);
			return REDIS_ERR;
		}

		if (redisCheckSocketError(c, fd) != REDIS_OK)
			return REDIS_ERR;

		return REDIS_OK;
	}
	__redisSetErrorFromErrno(c,REDIS_ERR_IO,NULL);
	closesocket(fd);
	return REDIS_ERR;
#endif
}

int redisCheckSocketError(redisContext *c, int fd) {
    int err = 0;
    size_t errlen = sizeof(err);

	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&err, &errlen) == -1) {
#ifndef HIREDIS_WIN
		__redisSetErrorFromErrno(c,REDIS_ERR_IO,"getsockopt(SO_ERROR)");
		close(fd);
#else
		errno = WSAGetLastError();
		__redisSetErrorFromErrno(c,REDIS_ERR_IO,"getsockopt(SO_ERROR)");
		closesocket(fd);
#endif
        return REDIS_ERR;
    }

    if (err) {
        errno = err;
        __redisSetErrorFromErrno(c,REDIS_ERR_IO,NULL);
#ifndef HIREDIS_WIN
		close(fd);
#else
		closesocket(fd);
#endif
        return REDIS_ERR;
    }

    return REDIS_OK;
}

int redisContextSetTimeout(redisContext *c, struct timeval tv) {
#ifndef HIREDIS_WIN
    if (setsockopt(c->fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv)) == -1) {
        __redisSetErrorFromErrno(c,REDIS_ERR_IO,"setsockopt(SO_RCVTIMEO)");
        return REDIS_ERR;
    }
    if (setsockopt(c->fd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv)) == -1) {
        __redisSetErrorFromErrno(c,REDIS_ERR_IO,"setsockopt(SO_SNDTIMEO)");
        return REDIS_ERR;
    }
#else
	unsigned long millisecond = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	if (setsockopt(c->fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&millisecond, sizeof(millisecond)) == -1) {
		__redisSetErrorFromErrno(c,REDIS_ERR_IO,"setsockopt(SO_RCVTIMEO)");
		return REDIS_ERR;
	}
	if (setsockopt(c->fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&millisecond, sizeof(millisecond)) == -1) {
		__redisSetErrorFromErrno(c,REDIS_ERR_IO,"setsockopt(SO_SNDTIMEO)");
		return REDIS_ERR;
	}
#endif
	return REDIS_OK;
}

int redisContextConnectTcp(redisContext *c, const char *addr, int port, struct timeval *timeout) {

#ifndef HIREDIS_WIN
    int s, rv;
    char _port[6];  /* strlen("65535"); */
    struct addrinfo hints, *servinfo, *p;
    int blocking = (c->flags & REDIS_BLOCK);
    snprintf(_port, 6, "%d", port);
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(addr,_port,&hints,&servinfo)) != 0) {
        __redisSetError(c,REDIS_ERR_OTHER,gai_strerror(rv));
        return REDIS_ERR;
    }
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((s = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)	{
            continue;
		}

        if (redisSetBlocking(c,s,0) != REDIS_OK)
            goto error;
		if (connect(s,p->ai_addr,p->ai_addrlen) == -1) {
			if (errno == EHOSTUNREACH) {
				close(s);
                continue;
            } else if (errno == EINPROGRESS && !blocking) {
                /* This is ok. */
            } else {
                if (redisContextWaitReady(c,s,timeout) != REDIS_OK)
                    goto error;
            }
        }
        if (blocking && redisSetBlocking(c,s,1) != REDIS_OK)
            goto error;
        if (redisSetTcpNoDelay(c,s) != REDIS_OK)
            goto error;

        c->fd = s;
        c->flags |= REDIS_CONNECTED;
        rv = REDIS_OK;
        goto end;
    }
    if (p == NULL) {
        char buf[128];
		snprintf(buf,sizeof(buf),"Can't create socket: %s",strerror(errno));
        __redisSetError(c,REDIS_ERR_OTHER,buf);
        goto error;
    }

error:
    rv = REDIS_ERR;
end:
    freeaddrinfo(servinfo);
    return rv;  // Need to return REDIS_OK if alright
#else
	int s, rv;
	char errbuf[128];
	int blocking = (c->flags & REDIS_BLOCK);
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;  
	sa.sin_port = htons(port);  
	sa.sin_addr.s_addr=inet_addr(addr);
	if(sa.sin_addr.s_addr == INADDR_NONE || sa.sin_addr.s_addr == INADDR_ANY)
	{
		struct hostent *he;
		he = gethostbyname(addr);
		if (he == NULL) {
			char buf[128];
			sprintf_s(buf,sizeof(buf),"Can't resolve: %s", addr);
			__redisSetError(c,REDIS_ERR_OTHER,buf);
			return REDIS_ERR;
		}
		memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
	}

	if ((s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == -1)	{
		LPVOID lpMsgBuf;
		errno = WSAGetLastError();
		FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS, 
			NULL, 
			errno, 
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPTSTR) &lpMsgBuf, 
			0, 
			NULL 
			); 
		sprintf_s(errbuf,sizeof(errbuf),"Can't create socket: %s",lpMsgBuf);
		LocalFree(lpMsgBuf);
		__redisSetError(c,REDIS_ERR_OTHER,errbuf);
		goto error;
	}

	if (redisSetBlocking(c,s,0) != REDIS_OK)
		goto error;
	if (connect(s,(PSOCKADDR)&sa,sizeof(sa)) == -1) {
		errno = WSAGetLastError();
		if (errno == WSAEHOSTUNREACH) {
			LPVOID lpMsgBuf;
			closesocket(s);
			FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS, 
				NULL, 
				errno, 
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
				(LPTSTR) &lpMsgBuf, 
				0, 
				NULL 
				); 
			sprintf_s(errbuf,sizeof(errbuf),"Can't connect: %s",lpMsgBuf);
			LocalFree(lpMsgBuf);
			__redisSetError(c,REDIS_ERR_OTHER,errbuf);
			goto error;
		} else if ((errno == WSAEINPROGRESS || errno == WSAEWOULDBLOCK) && !blocking) {
			/* This is ok. */
		} else {
			if (redisContextWaitReady(c,s,timeout) != REDIS_OK)
				goto error;
		}
	}
	if (blocking && redisSetBlocking(c,s,1) != REDIS_OK)
		goto error;
	if (redisSetTcpNoDelay(c,s) != REDIS_OK)
		goto error;

	c->fd = s;
	c->flags |= REDIS_CONNECTED;
	rv = REDIS_OK;
	goto end;
error:
	rv = REDIS_ERR;
end:
	return rv;  // Need to return REDIS_OK if alright
#endif
}

#ifndef HIREDIS_WIN
int redisContextConnectUnix(redisContext *c, const char *path, struct timeval *timeout) {
    int s;
    int blocking = (c->flags & REDIS_BLOCK);
    struct sockaddr_un sa;

    if ((s = redisCreateSocket(c,AF_LOCAL)) < 0)
        return REDIS_ERR;
    if (redisSetBlocking(c,s,0) != REDIS_OK)
        return REDIS_ERR;

    sa.sun_family = AF_LOCAL;
    strncpy(sa.sun_path,path,sizeof(sa.sun_path)-1);
    if (connect(s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        if (errno == EINPROGRESS && !blocking) {
            /* This is ok. */
        } else {
            if (redisContextWaitReady(c,s,timeout) != REDIS_OK)
                return REDIS_ERR;
        }
    }

    /* Reset socket to be blocking after connect(2). */
    if (blocking && redisSetBlocking(c,s,1) != REDIS_OK)
        return REDIS_ERR;

    c->fd = s;
    c->flags |= REDIS_CONNECTED;
    return REDIS_OK;
}
#endif
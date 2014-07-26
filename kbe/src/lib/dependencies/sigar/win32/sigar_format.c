/*
 * Copyright (c) 2007-2008 Hyperic, Inc.
 * Copyright (c) 2009 SpringSource, Inc.
 * Copyright (c) 2010 VMware, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Utility functions to provide string formatting of SIGAR data */

#include "sigar.h"
#include "sigar_private.h"
#include "sigar_util.h"
#include "sigar_os.h"
#include "sigar_format.h"

#include <errno.h>
#include <stdio.h>

#ifndef WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(_AIX)
#include <sys/socket.h>
#endif
#include <pwd.h>
#include <grp.h>

/* sysconf(_SC_GET{PW,GR}_R_SIZE_MAX) */
#define R_SIZE_MAX 1024

int sigar_user_name_get(sigar_t *sigar, int uid, char *buf, int buflen)
{
    struct passwd *pw = NULL;
    /* XXX cache lookup */

# ifdef HAVE_GETPWUID_R
    struct passwd pwbuf;
    char buffer[R_SIZE_MAX];

    if (getpwuid_r(uid, &pwbuf, buffer, sizeof(buffer), &pw) != 0) {
        return errno;
    }
    if (!pw) {
        return ENOENT;
    }
# else
    if ((pw = getpwuid(uid)) == NULL) {
        return errno;
    }
# endif

    strncpy(buf, pw->pw_name, buflen);
    buf[buflen-1] = '\0';

    return SIGAR_OK;
}

int sigar_group_name_get(sigar_t *sigar, int gid, char *buf, int buflen)
{
    struct group *gr;
    /* XXX cache lookup */

# ifdef HAVE_GETGRGID_R
    struct group grbuf;
    char buffer[R_SIZE_MAX];

    if (getgrgid_r(gid, &grbuf, buffer, sizeof(buffer), &gr) != 0) {
        return errno;
    }
# else
    if ((gr = getgrgid(gid)) == NULL) {
        return errno;
    }
# endif

    if (gr && gr->gr_name) {
        strncpy(buf, gr->gr_name, buflen);
    }
    else {
        /* seen on linux.. apache httpd.conf has:
         * Group #-1
         * results in uid == -1 and gr == NULL.
         * wtf getgrgid_r doesnt fail instead? 
         */
        sprintf(buf, "%d", gid);
    }
    buf[buflen-1] = '\0';

    return SIGAR_OK;
}

int sigar_user_id_get(sigar_t *sigar, const char *name, int *uid)
{
    /* XXX cache lookup */
    struct passwd *pw;

# ifdef HAVE_GETPWNAM_R
    struct passwd pwbuf;
    char buf[R_SIZE_MAX];

    if (getpwnam_r(name, &pwbuf, buf, sizeof(buf), &pw) != 0) {
        return errno;
    }
# else
    if (!(pw = getpwnam(name))) {
        return errno;
    }
# endif

    *uid = (int)pw->pw_uid;
    return SIGAR_OK;
}

#endif /* WIN32 */

static char *sigar_error_string(int err)
{
    switch (err) {
      case SIGAR_ENOTIMPL:
        return "This function has not been implemented on this platform";
      default:
        return "Error string not specified yet";
    }
}

SIGAR_DECLARE(char *) sigar_strerror(sigar_t *sigar, int err)
{
    char *buf;

    if (err < 0) {
        return sigar->errbuf;
    }

    if (err > SIGAR_OS_START_ERROR) {
        if ((buf = sigar_os_error_string(sigar, err)) != NULL) {
            return buf;
        }
        return "Unknown OS Error"; /* should never happen */
    }

    if (err > SIGAR_START_ERROR) {
        return sigar_error_string(err);
    }

    return sigar_strerror_get(err, sigar->errbuf, sizeof(sigar->errbuf));
}

char *sigar_strerror_get(int err, char *errbuf, int buflen)
{
    char *buf = NULL;
#ifdef WIN32
    DWORD len;

    len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        err,
                        MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), /* force english */
                        (LPTSTR)errbuf,
                        (DWORD)buflen,
                        NULL);
#else

#if defined(HAVE_STRERROR_R) && defined(HAVE_STRERROR_R_GLIBC)
    /*
     * strerror_r man page says:
     * "The GNU version may, but need not, use the user supplied buffer"
     */
    buf = strerror_r(err, errbuf, buflen);
#elif defined(HAVE_STRERROR_R)
    if (strerror_r(err, errbuf, buflen) < 0) {
        buf = "Unknown Error";
    }
#else
    /* strerror() is thread safe on solaris and hpux */
    buf = strerror(err);
#endif

    if (buf != NULL) {
        SIGAR_STRNCPY(errbuf, buf, buflen);
    }
    
#endif
    return errbuf;
}

void sigar_strerror_set(sigar_t *sigar, char *msg)
{
    SIGAR_SSTRCPY(sigar->errbuf, msg);
}

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

void sigar_strerror_printf(sigar_t *sigar, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vsnprintf(sigar->errbuf, sizeof(sigar->errbuf), format, args);
    va_end(args);
}

/* copy apr_strfsize */
SIGAR_DECLARE(char *) sigar_format_size(sigar_uint64_t size, char *buf)
{
    const char ord[] = "KMGTPE";
    const char *o = ord;
    int remain;

    if (size == SIGAR_FIELD_NOTIMPL) {
        buf[0] = '-';
        buf[1] = '\0';
        return buf;
    }

    if (size < 973) {
        sprintf(buf, "%3d ", (int) size);
        return buf;
    }

    do {
        remain = (int)(size & 1023);
        size >>= 10;

        if (size >= 973) {
            ++o;
            continue;
        }

        if (size < 9 || (size == 9 && remain < 973)) {
            if ((remain = ((remain * 5) + 256) / 512) >= 10) {
                ++size;
                remain = 0;
            }
            sprintf(buf, "%d.%d%c", (int) size, remain, *o);
            return buf;
        }

        if (remain >= 512) {
            ++size;
        }

        sprintf(buf, "%3d%c", (int) size, *o);

        return buf;
    } while (1);
}


SIGAR_DECLARE(int) sigar_uptime_string(sigar_t *sigar, 
                                       sigar_uptime_t *uptime,
                                       char *buffer,
                                       int buflen)
{
    char *ptr = buffer;
    int time = (int)uptime->uptime;
    int minutes, hours, days, offset = 0;

    /* XXX: get rid of sprintf and/or check for overflow */
    days = time / (60*60*24);

    if (days) {
        offset += sprintf(ptr + offset, "%d day%s, ",
                          days, (days > 1) ? "s" : "");
    }

    minutes = time / 60;
    hours = minutes / 60;
    hours = hours % 24;
    minutes = minutes % 60;

    if (hours) {
        offset += sprintf(ptr + offset, "%2d:%02d",
                          hours, minutes);
    }
    else {
        offset += sprintf(ptr + offset, "%d min", minutes);
    }

    return SIGAR_OK;
}

/* threadsafe alternative to inet_ntoa (inet_ntop4 from apr) */
int sigar_inet_ntoa(sigar_t *sigar,
                    sigar_uint32_t address,
                    char *addr_str)
{
    char *next=addr_str;
    int n=0;
    const unsigned char *src =
        (const unsigned char *)&address;

    do {
        unsigned char u = *src++;
        if (u > 99) {
            *next++ = '0' + u/100;
            u %= 100;
            *next++ = '0' + u/10;
            u %= 10;
        }
        else if (u > 9) {
            *next++ = '0' + u/10;
            u %= 10;
        }
        *next++ = '0' + u;
        *next++ = '.';
        n++;
    } while (n < 4);

    *--next = 0;

    return SIGAR_OK;
}

static int sigar_ether_ntoa(char *buff, unsigned char *ptr)
{
    sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X",
            (ptr[0] & 0xff), (ptr[1] & 0xff), (ptr[2] & 0xff),
            (ptr[3] & 0xff), (ptr[4] & 0xff), (ptr[5] & 0xff));
    return SIGAR_OK;
}

SIGAR_DECLARE(int) sigar_net_address_equals(sigar_net_address_t *addr1,
                                            sigar_net_address_t *addr2)
                                            
{
    if (addr1->family != addr2->family) {
        return EINVAL;
    }

    switch (addr1->family) {
      case SIGAR_AF_INET:
        return memcmp(&addr1->addr.in, &addr2->addr.in, sizeof(addr1->addr.in));
      case SIGAR_AF_INET6:
        return memcmp(&addr1->addr.in6, &addr2->addr.in6, sizeof(addr1->addr.in6));
      case SIGAR_AF_LINK:
        return memcmp(&addr1->addr.mac, &addr2->addr.mac, sizeof(addr1->addr.mac));
      default:
        return EINVAL;
    }
}

#if !defined(WIN32) && !defined(NETWARE) && !defined(__hpux)
#define sigar_inet_ntop inet_ntop
#define sigar_inet_ntop_errno errno
#else
#define sigar_inet_ntop(af, src, dst, size) NULL
#define sigar_inet_ntop_errno EINVAL
#endif

SIGAR_DECLARE(int) sigar_net_address_to_string(sigar_t *sigar,
                                               sigar_net_address_t *address,
                                               char *addr_str)
{
    switch (address->family) {
      case SIGAR_AF_INET6:
        if (sigar_inet_ntop(AF_INET6, (const void *)&address->addr.in6,
                            addr_str, SIGAR_INET6_ADDRSTRLEN))
        {
            return SIGAR_OK;
        }
        else {
            return sigar_inet_ntop_errno;
        }
      case SIGAR_AF_INET:
        return sigar_inet_ntoa(sigar, address->addr.in, addr_str);
      case SIGAR_AF_UNSPEC:
        return sigar_inet_ntoa(sigar, 0, addr_str); /*XXX*/
      case SIGAR_AF_LINK:
        return sigar_ether_ntoa(addr_str, &address->addr.mac[0]);
      default:
        return EINVAL;
    }
}

SIGAR_DECLARE(sigar_uint32_t) sigar_net_address_hash(sigar_net_address_t *address)
{
    sigar_uint32_t hash = 0;
    unsigned char *data;
    int i=0, size, elts;

    switch (address->family) {
      case SIGAR_AF_UNSPEC:
      case SIGAR_AF_INET:
        return address->addr.in;
      case SIGAR_AF_INET6:
        data = (unsigned char *)&address->addr.in6;
        size = sizeof(address->addr.in6);
        elts = 4;
        break;
      case SIGAR_AF_LINK:
        data = (unsigned char *)&address->addr.mac;
        size = sizeof(address->addr.mac);
        elts = 2;
        break;
      default:
        return -1;
    }

    while (i<size) {
        int j=0;
        int component=0;
        while (j<elts && i<size) {
            component = (component << 8) + data[i];
            j++; 
            i++;
        }
        hash += component;
    }

    return hash;
}

SIGAR_DECLARE(const char *)sigar_net_connection_type_get(int type)
{
    switch (type) {
      case SIGAR_NETCONN_TCP:
        return "tcp";
      case SIGAR_NETCONN_UDP:
        return "udp";
      case SIGAR_NETCONN_RAW:
        return "raw";
      case SIGAR_NETCONN_UNIX:
        return "unix";
      default:
        return "unknown";
    }
}

SIGAR_DECLARE(const char *)sigar_net_connection_state_get(int state)
{
    switch (state) {
      case SIGAR_TCP_ESTABLISHED:
        return "ESTABLISHED";
      case SIGAR_TCP_SYN_SENT:
        return "SYN_SENT";
      case SIGAR_TCP_SYN_RECV:
        return "SYN_RECV";
      case SIGAR_TCP_FIN_WAIT1:
        return "FIN_WAIT1";
      case SIGAR_TCP_FIN_WAIT2:
        return "FIN_WAIT2";
      case SIGAR_TCP_TIME_WAIT:
        return "TIME_WAIT";
      case SIGAR_TCP_CLOSE:
        return "CLOSE";
      case SIGAR_TCP_CLOSE_WAIT:
        return "CLOSE_WAIT";
      case SIGAR_TCP_LAST_ACK:
        return "LAST_ACK";
      case SIGAR_TCP_LISTEN:
        return "LISTEN";
      case SIGAR_TCP_CLOSING:
        return "CLOSING";
      case SIGAR_TCP_IDLE:
        return "IDLE";
      case SIGAR_TCP_BOUND:
        return "BOUND";
      case SIGAR_TCP_UNKNOWN:
      default:
        return "UNKNOWN";
    }
}

SIGAR_DECLARE(char *) sigar_net_interface_flags_to_string(sigar_uint64_t flags, char *buf)
{
    *buf = '\0';

    if (flags == 0) {
        strcat(buf, "[NO FLAGS] ");
    }
    if (flags & SIGAR_IFF_UP) {
        strcat(buf, "UP ");
    }
    if (flags & SIGAR_IFF_BROADCAST) {
        strcat(buf, "BROADCAST ");
    }
    if (flags & SIGAR_IFF_DEBUG) {
        strcat(buf, "DEBUG ");
    }
    if (flags & SIGAR_IFF_LOOPBACK) {
        strcat(buf, "LOOPBACK ");
    }
    if (flags & SIGAR_IFF_POINTOPOINT) {
        strcat(buf, "POINTOPOINT ");
    }
    if (flags & SIGAR_IFF_NOTRAILERS) {
        strcat(buf, "NOTRAILERS ");
    }
    if (flags & SIGAR_IFF_RUNNING) {
        strcat(buf, "RUNNING ");
    }
    if (flags & SIGAR_IFF_NOARP) {
        strcat(buf, "NOARP ");
    }
    if (flags & SIGAR_IFF_PROMISC) {
        strcat(buf, "PROMISC ");
    }
    if (flags & SIGAR_IFF_ALLMULTI) {
        strcat(buf, "ALLMULTI ");
    }
    if (flags & SIGAR_IFF_MULTICAST) {
        strcat(buf, "MULTICAST ");
    }

    return buf;
}

#ifdef WIN32
#define NET_SERVICES_FILE "C:\\windows\\system32\\drivers\\etc\\services"
#else
#define NET_SERVICES_FILE "/etc/services"
#endif

static int net_services_parse(sigar_cache_t *names, char *type)
{
    FILE *fp;
    char buffer[8192], *ptr;
    char *file;


    if (!(file = getenv("SIGAR_NET_SERVICES_FILE"))) {
        file = NET_SERVICES_FILE;
    }

    if (!(fp = fopen(file, "r"))) {
        return errno;
    }

    while ((ptr = fgets(buffer, sizeof(buffer), fp))) {
        int port;
        char name[256], proto[56];
        sigar_cache_entry_t *entry;

        while (sigar_isspace(*ptr)) {
            ++ptr;
        }
        if ((*ptr == '#') || (*ptr == '\0')) {
            continue;
        }

        if (sscanf(ptr, "%s%d/%s", name, &port, proto) != 3) {
            continue;
        }
        if (!strEQ(type, proto)) {
            continue;
        }

        entry = sigar_cache_get(names, port);
        if (!entry->value) {
            entry->value = strdup(name);
        }
    }

    fclose(fp);
    return SIGAR_OK;
}

SIGAR_DECLARE(char *)sigar_net_services_name_get(sigar_t *sigar,
                                                 int protocol, unsigned long port)
{
    sigar_cache_entry_t *entry;
    sigar_cache_t **names;
    char *pname;

    switch (protocol) {
      case SIGAR_NETCONN_TCP:
        names = &sigar->net_services_tcp;
        pname = "tcp";
        break;
      case SIGAR_NETCONN_UDP:
        names = &sigar->net_services_udp;
        pname = "udp";
        break;
      default:
        return NULL;
    }

    if (*names == NULL) {
        *names = sigar_cache_new(1024);
        net_services_parse(*names, pname);
    }

    if ((entry = sigar_cache_find(*names, port))) {
        return (char *)entry->value;
    }
    else {
        return NULL;
    }
}

SIGAR_DECLARE(int) sigar_cpu_perc_calculate(sigar_cpu_t *prev,
                                            sigar_cpu_t *curr,
                                            sigar_cpu_perc_t *perc)
{
    double diff_user, diff_sys, diff_nice, diff_idle;
    double diff_wait, diff_irq, diff_soft_irq, diff_stolen;
    double diff_total;

    diff_user = curr->user - prev->user;
    diff_sys  = curr->sys  - prev->sys;
    diff_nice = curr->nice - prev->nice;
    diff_idle = curr->idle - prev->idle;
    diff_wait = curr->wait - prev->wait;
    diff_irq = curr->irq - prev->irq;
    diff_soft_irq = curr->soft_irq - prev->soft_irq;
    diff_stolen = curr->stolen - prev->stolen;

    diff_user = diff_user < 0 ? 0 : diff_user;
    diff_sys  = diff_sys  < 0 ? 0 : diff_sys;
    diff_nice = diff_nice < 0 ? 0 : diff_nice;
    diff_idle = diff_idle < 0 ? 0 : diff_idle;
    diff_wait = diff_wait < 0 ? 0 : diff_wait;
    diff_irq = diff_irq < 0 ? 0 : diff_irq;
    diff_soft_irq = diff_soft_irq < 0 ? 0 : diff_soft_irq;
    diff_stolen = diff_stolen < 0 ? 0 : diff_stolen;

    diff_total =
        diff_user + diff_sys + diff_nice + diff_idle +
        diff_wait + diff_irq + diff_soft_irq +
        diff_stolen;

    perc->user = diff_user / diff_total;
    perc->sys  = diff_sys / diff_total;
    perc->nice = diff_nice / diff_total;
    perc->idle = diff_idle / diff_total;
    perc->wait = diff_wait / diff_total;
    perc->irq = diff_irq / diff_total;
    perc->soft_irq = diff_soft_irq / diff_total;
    perc->stolen = diff_stolen / diff_total;

    perc->combined =
        perc->user + perc->sys + perc->nice + perc->wait;

    return SIGAR_OK;
}

/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define INCL_DOSERRORS
#include "apr_arch_file_io.h"
#include "apr_file_io.h"
#include <errno.h>
#include <string.h>
#include "apr_errno.h"

static int errormap[][2] = {
    { NO_ERROR,                   APR_SUCCESS      },
    { ERROR_FILE_NOT_FOUND,       APR_ENOENT       },
    { ERROR_PATH_NOT_FOUND,       APR_ENOENT       },
    { ERROR_TOO_MANY_OPEN_FILES,  APR_EMFILE       },
    { ERROR_ACCESS_DENIED,        APR_EACCES       },
    { ERROR_SHARING_VIOLATION,    APR_EACCES       },
    { ERROR_INVALID_PARAMETER,    APR_EINVAL       },
    { ERROR_OPEN_FAILED,          APR_ENOENT       },
    { ERROR_DISK_FULL,            APR_ENOSPC       },
    { ERROR_FILENAME_EXCED_RANGE, APR_ENAMETOOLONG },
    { ERROR_INVALID_FUNCTION,     APR_EINVAL       },
    { ERROR_INVALID_HANDLE,       APR_EBADF        },
    { ERROR_NEGATIVE_SEEK,        APR_ESPIPE       },
    { ERROR_NO_SIGNAL_SENT,       ESRCH            },
    { ERROR_NO_DATA,              APR_EAGAIN       },
    { SOCEINTR,                 EINTR           },
    { SOCEWOULDBLOCK,           EWOULDBLOCK     },
    { SOCEINPROGRESS,           EINPROGRESS     },
    { SOCEALREADY,              EALREADY        },
    { SOCENOTSOCK,              ENOTSOCK        },
    { SOCEDESTADDRREQ,          EDESTADDRREQ    },
    { SOCEMSGSIZE,              EMSGSIZE        },
    { SOCEPROTOTYPE,            EPROTOTYPE      },
    { SOCENOPROTOOPT,           ENOPROTOOPT     },
    { SOCEPROTONOSUPPORT,       EPROTONOSUPPORT },
    { SOCESOCKTNOSUPPORT,       ESOCKTNOSUPPORT },
    { SOCEOPNOTSUPP,            EOPNOTSUPP      },
    { SOCEPFNOSUPPORT,          EPFNOSUPPORT    },
    { SOCEAFNOSUPPORT,          EAFNOSUPPORT    },
    { SOCEADDRINUSE,            EADDRINUSE      },
    { SOCEADDRNOTAVAIL,         EADDRNOTAVAIL   },
    { SOCENETDOWN,              ENETDOWN        },
    { SOCENETUNREACH,           ENETUNREACH     },
    { SOCENETRESET,             ENETRESET       },
    { SOCECONNABORTED,          ECONNABORTED    },
    { SOCECONNRESET,            ECONNRESET      },
    { SOCENOBUFS,               ENOBUFS         },
    { SOCEISCONN,               EISCONN         },
    { SOCENOTCONN,              ENOTCONN        },
    { SOCESHUTDOWN,             ESHUTDOWN       },
    { SOCETOOMANYREFS,          ETOOMANYREFS    },
    { SOCETIMEDOUT,             ETIMEDOUT       },
    { SOCECONNREFUSED,          ECONNREFUSED    },
    { SOCELOOP,                 ELOOP           },
    { SOCENAMETOOLONG,          ENAMETOOLONG    },
    { SOCEHOSTDOWN,             EHOSTDOWN       },
    { SOCEHOSTUNREACH,          EHOSTUNREACH    },
    { SOCENOTEMPTY,             ENOTEMPTY       },
    { SOCEPIPE,                 EPIPE           }
};

#define MAPSIZE (sizeof(errormap)/sizeof(errormap[0]))

int apr_canonical_error(apr_status_t err)
{
    int rv = -1, index;

    if (err < APR_OS_START_SYSERR)
        return err;

    err -= APR_OS_START_SYSERR;

    for (index=0; index<MAPSIZE && errormap[index][0] != err; index++);
    
    if (index<MAPSIZE)
        rv = errormap[index][1];
    else
        fprintf(stderr, "apr_canonical_error: Unknown OS/2 error code %d\n", err );
        
    return rv;
}

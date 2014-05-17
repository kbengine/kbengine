#include "apr.h"
#include "apr_file_io.h"
#include "apr.h"

#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif

int main(void)
{
    char buf[256];
    apr_file_t *err;
    apr_pool_t *p;

    apr_initialize();
    atexit(apr_terminate);

    apr_pool_create(&p, NULL);
    apr_file_open_stdin(&err, p);

    while (1) {
        apr_size_t length = 256;
        apr_file_read(err, buf, &length);
    }
    exit(0); /* just to keep the compiler happy */
}

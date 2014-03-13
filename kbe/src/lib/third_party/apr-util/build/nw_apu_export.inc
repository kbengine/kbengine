
/* Must include apu.h first so that we can undefine
    the standard prototypes macros after it messes with
    them. */
#include "apu.h"

#undef APU_DECLARE
#undef APU_DECLARE_NONSTD
#undef APU_DECLARE_DATA

/* Preprocess all of the standard APR headers. */
#include "apr_anylock.h"
#include "apr_base64.h"
#include "apr_buckets.h"
#include "apr_crypto.h"
#include "apr_date.h"
#include "apr_dbd.h"
#include "apr_dbm.h"
#include "apr_dbm_private.h"
#include "apr_hooks.h"
#include "apr_ldap.h"
#include "apr_ldap_init.h"
#include "apr_ldap_option.h"
#include "apr_ldap_rebind.h"
#include "apr_ldap_url.h"
#include "apr_md4.h"
#include "apr_md5.h"
#include "apr_memcache.h"
#include "apr_optional.h"
#include "apr_optional_hooks.h"
#include "apr_queue.h"
#include "apr_reslist.h"
#include "apr_rmm.h"
#include "apr_sdbm.h"
#include "apr_sha1.h"
#include "apr_strmatch.h"
#include "apr_thread_pool.h"
#include "apr_uri.h"
#include "apr_uuid.h"
#include "apr_xlate.h"
#include "apr_xml.h"
#include "apu_version.h"
#include "apu_want.h"

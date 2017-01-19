AC_OUTPUT

if test -n "$MONGOC_PRERELEASE_VERSION"; then
cat << EOF
 *** IMPORTANT *** 

 This is an unstable version of libmongoc.
 It is for test purposes only.

 Please, DO NOT use it in a production environment.
 It will probably crash and you will lose your data.

 Additionally, the API/ABI may change during the course
 of development.

 Thanks,

   The libmongoc team.

 *** END OF WARNING ***

EOF
fi

echo "
libmongoc $MONGOC_VERSION was configured with the following options:

Build configuration:
  Enable debugging (slow)                          : ${enable_debug}
  Compile with debug symbols (slow)                : ${enable_debug_symbols}
  Enable GCC build optimization                    : ${enable_optimizations}
  Enable automatic binary hardening                : ${enable_hardening}
  Enable automatic init and cleanup                : ${enable_automatic_init_and_cleanup}
  Code coverage support                            : ${enable_coverage}
  Cross Compiling                                  : ${enable_crosscompile}
  Fast counters                                    : ${enable_rdtscp}
  Shared memory performance counters               : ${enable_shm_counters}
  SASL                                             : ${sasl_mode}
  SSL                                              : ${enable_ssl}
  Libbson                                          : ${with_libbson}

Documentation:
  man                                              : ${enable_man_pages}
  HTML                                             : ${enable_html_docs}
"

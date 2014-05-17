## jwsmtp library source directory

# define NDEBUG to stop asserts in the production code.
# if you want asserts uncomment the -DNDEBUG line below.
# CPPFLAGS=-DNDEBUG
# CXXFLAGS=

headers = jwsmtp.h mailer.h compat.h base64.h

h_sources  = jwsmtp.h mailer.h compat.h base64.h
cc_sources = mailer.cpp compat.cpp base64.cpp

library_includedir=$(includedir)/$(JWSMTP_LIBRARY_NAME)-$(JWSMTP_API_VERSION)/$(JWSMTP_LIBRARY_NAME)
library_include_HEADERS = $(headers)

lib_LTLIBRARIES= libjwsmtp.la
libjwsmtp_la_SOURCES= $(h_sources) $(cc_sources)
libjwsmtp_la_LDFLAGS= -release $(JWSMTP_API_VERSION)


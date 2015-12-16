#!/bin/sh

# This file illustrates how to generate a useful TAGS file via etags
# for emacs.  This should be invoked from the top source directory i.e.:
#   > build/MakeEtags
# and will create a TAGS file in the top source directory.

# This script falls under the Apache License.
# See http://www.apache.org/docs/LICENSE

# Once you have created ./TAGS in emacs you'll need to setup
# tag-table-alist with an entry to assure it finds the single ./TAGS
# file from the many source directories.  Something along these lines:
# (setq tag-table-alist
#	'(("/home/me/work/apr-x.y/" . "/home/me/work/apr-x.y/")
#	  ("/home/me/work/apr-util-x.y/" . "/home/me/work/apr-util-x.y/")
#	  ("/home/me/work/httpd-x.y/" . "/home/me/work/httpd-x.y/")
#	 ))

# This requires a special version of etags, i.e. the
# one called "Exuberant ctags" available at:
#    http://ctags.sourceforge.net/
# Once that is setup you'll need to point to the
# executable here:

etags=${ETAGS-etags}

# Exuberant etags is necessary since it can ignore some defined symbols
# that obscure the function signatures.

ignore=AP_DECLARE,AP_DECLARE_NONSTD,__declspec,APR_DECLARE,APR_DECLARE_NONSTD
ignore=$ignore,APU_DECLARE,APU_DECLARE_NONSTD

# Create an etags file at the root of the source
# tree, then create symbol links to it from each
# directory in the source tree.  By passing etags
# absolute pathnames we get a tag file that is
# NOT portable when we move the directory tree.

find . -name '*.[ch]' -print | $etags -I "$ignore"  -L -


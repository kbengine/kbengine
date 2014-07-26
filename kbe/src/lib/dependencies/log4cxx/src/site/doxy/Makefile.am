# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
EXTRA_DIST = Doxyfile mainpage.dox license_notice_footer.txt

# DOC is defined if installer requests doc generation.


if DOC

htmldest = $(pkgdatadir)/html

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(htmldest)
	cp -dpR @manual_dest@/* $(DESTDIR)$(htmldest)

# Automake's "distcheck" is sensitive to having files left over
# after "make uninstall", so we have to clean up the install hook.
uninstall-local:
	rm -rf $(DESTDIR)$(htmldest)

dox: @manual_dest@/index.html

if LATEX_DOC

pdf: @PACKAGE@.pdf
@PACKAGE@.pdf:
	$(MAKE) -C ./latex pdf
	ln -s ./latex/refman.ps	 @PACKAGE@.ps
	ln -s ./latex/refman.pdf @PACKAGE@.pdf
	
endif

else

# We repeat the three targets in both the "if" and "else" clauses
# of the conditional, because the generated makefile will contain
# references to the targets (target "install" depends on target
# "install-datahook", for example), and some make programs get upset
# if no target exists.

install-data-hook:
uninstall-local:
dox:

endif


all-local: dox


@manual_dest@/index.html: Doxyfile mainpage.dox
	"@DOXYGEN@"

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)

# Make tarfile to distribute the HTML documentation.
doc-dist: dox
	rm -rf $(distdir)
	mkdir $(distdir)
	mkdir $(distdir)/docs
	mkdir $(distdir)/docs/html
	cp @manual_dest@/* $(distdir)/docs/html
	tar -czf $(top_builddir)/$(PACKAGE)-docs-$(VERSION).tar.gz -C $(distdir) docs
	rm -rf $(distdir)


clean-local:
	$(RM) -r latex
	$(RM) -r @manual_dest@ man @PACKAGE@.ps @PACKAGE@.pdf

dist-hook:
	-rm -f $(distdir)/Doxyfile


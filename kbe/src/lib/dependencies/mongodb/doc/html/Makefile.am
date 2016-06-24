BUILT_HTML_FILES = $(patsubst doc/%.page,doc/html/%.html,$(wildcard doc/*.page))
CLEANFILES += $(wildcard doc/html/*.html)
CLEANFILES += $(wildcard doc/html/*.css)
CLEANFILES += $(wildcard doc/html/*.js)
CLEANFILES += $(wildcard doc/html/*.png)

%.html: ../%.page
	$(AM_V_GEN)$(YELP_BUILD) html -o doc/html/ doc

html: $(BUILT_HTML_FILES)

YELP_HTML_FILES = \
	doc/html/C.css \
	doc/html/jquery.js \
	doc/html/jquery.syntax.brush.clang.js \
	doc/html/jquery.syntax.core.js \
	doc/html/jquery.syntax.js \
	doc/html/jquery.syntax.layout.yelp.js \
	doc/html/yelp-note-tip.png \
	doc/html/yelp-note-warning.png \
	doc/html/yelp-note.png \
	doc/html/yelp.js

EXTRA_DIST += $(wildcard doc/html/*.html)
EXTRA_DIST += $(wildcard doc/html/*.css)
EXTRA_DIST += $(wildcard doc/html/*.js)

BUILT_MAN_FILES = $(patsubst doc/%.page,doc/man/%.3,$(wildcard doc/*.page))
CLEANFILES += $(wildcard doc/man/*.3)

dist_man_MANS = $(BUILT_MAN_FILES)
# Automake (at least up to 1.10) mishandles dist_man_MANS inside conditionals.
# Unlike with other dist primaries, the files are not distributed if the
# conditional is false.
# Work the bug around until it is fixed:
dist_noinst_DATA = $(dist_man_MANS)

man3_MANS = $(BUILT_MAN_FILES)

%.3: ../%.page
	$(AM_V_GEN)./doc/mallard2man.py 3 $^

man: $(BUILT_MAN_FILES)
EXTRA_DIST += $(wildcard doc/man/*.3) \
              doc/mallard2man.py

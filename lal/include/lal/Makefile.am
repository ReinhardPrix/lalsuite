.PHONY: force-header-link-generation

EXTRA_DIST = header-link-stamp
BUILT_SOURCES = header-link-stamp
DISTCLEANFILES = header-link-stamp *.h *.i

header-link-stamp: force-header-link-generation
	$(AM_V_at)for file in $(DISTCLEANFILES); do \
		rm -f "$$file"; \
	done; \
	for dir in lib swig; do \
		( cd "$(top_builddir)/$$dir" && $(MAKE) $(AM_MAKEFLAGS) header-links ) || exit 1; \
	done; \
	echo "$@" > $@

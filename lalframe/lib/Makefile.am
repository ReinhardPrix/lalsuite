BUILT_SOURCES =
MOSTLYCLEANFILES =
include $(top_srcdir)/gnuscripts/lalsuite_header_links.am
include $(top_srcdir)/gnuscripts/lalsuite_vcs_info.am

FRAMECSRCS = LALFrameUFrameC.h LALFrameC.c
FRAMELSRCS = LALFrameUFrameL.h LALFrameL.c
FRAMEUSRCS =

if FRAMEC
FRAMEUSRCS += $(FRAMECSRCS)
endif
if FRAMEL
FRAMEUSRCS += $(FRAMELSRCS)
endif

LDADD = liblalframe.la

bin_PROGRAMS = \
	lalfr-cksum \
	lalfr-stat \
	lalfr-dump \
	lalfr-print \
	lalfr-fmt \
	lalfr-cat \
	lalfr-split \
	lalfr-cut \
	lalfr-paste \
	lalfr-stream \
	lalfr-vis \
	lalframe_version \
	$(END_OF_LIST)

man_MANS = \
	lalfr-cksum.1 \
	lalfr-stat.1 \
	lalfr-dump.1 \
	lalfr-print.1 \
	lalfr-fmt.1 \
	lalfr-cat.1 \
	lalfr-split.1 \
	lalfr-cut.1 \
	lalfr-paste.1 \
	lalfr-stream.1 \
	lalfr-vis.1

lalfr_cksum_SOURCES = cksum.c
lalfr_stat_SOURCES = stat.c
lalfr_dump_SOURCES = dump.c
lalfr_print_SOURCES = print.c
lalfr_fmt_SOURCES = fmt.c
lalfr_cat_SOURCES = cat.c utils.c utils.h
lalfr_split_SOURCES = split.c utils.c utils.h
lalfr_cut_SOURCES = cut.c utils.c utils.h
lalfr_paste_SOURCES = paste.c utils.c utils.h
lalfr_stream_SOURCES = stream.c
lalfr_vis_SOURCES = vis.c
lalframe_version_SOURCES = version.c

pkginclude_HEADERS = \
	FrameCalibration.h \
	LALFrStream.h \
	LALFrameConfig.h \
	LALFrameIO.h \
	LALFrameL.h \
	LALFrameU.h \
	LALFrameVCSInfo.h \
	LALFrameVCSInfoHeader.h

lib_LTLIBRARIES = liblalframe.la

liblalframe_la_SOURCES = \
	$(FRAMEUSRCS) \
	LALFrameU.c \
	LALFrameIO.c \
	LALFrStream.c \
	LALFrStreamRead.c \
	LALFrStreamLegacy.c \
	FrameCalibration.c \
	$(END_OF_LIST)

nodist_liblalframe_la_SOURCES = \
	LALFrameBuildInfoHeader.h \
	LALFrameVCSInfo.c \
	$(END_OF_LIST)

noinst_HEADERS = \
	LALFrStreamReadTS_source.c \
	LALFrStreamReadFS_source.c \
	LALFrameIO_source.c

liblalframe_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(LIBVERSION)

EXTRA_DIST = \
	$(MANS) \
	$(FRAMECSRCS) \
	$(FRAMELSRCS) \
	config.h.in \
	LALFrameC.c \
	LALFrameL.c \
	LALFrameConfig.h.in

DISTCLEANFILES = config.h

vcs_build_info_source = LALFrameVCSInfo.c
vcs_info_sources = LALFrameVCSInfoHeader.h
build_info_source = LALFrameBuildInfoHeader.h
build_info_dependencies = $(liblalframe_la_DEPENDENCIES)

TESTS = \
	lalframe_version \
	$(END_OF_LIST)

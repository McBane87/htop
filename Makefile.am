
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = htop
dist_man_MANS = htop.1
EXTRA_DIST = $(dist_man_MANS) htop.desktop htop.png scripts/MakeHeader.py \
install-sh autogen.sh missing
applicationsdir = $(datadir)/applications
applications_DATA = htop.desktop
pixmapdir = $(datadir)/pixmaps
pixmap_DATA = htop.png

htop_CFLAGS = -DSYSCONFDIR=\"$(sysconfdir)\"

myhtopsources = AvailableMetersPanel.c CategoriesPanel.c CheckItem.c \
ClockMeter.c ColorsPanel.c ColumnsPanel.c CPUMeter.c CRT.c \
DisplayOptionsPanel.c FunctionBar.c Hashtable.c Header.c htop.c ListItem.c \
LoadAverageMeter.c MemoryMeter.c Meter.c MetersPanel.c Object.c Panel.c \
BatteryMeter.c Process.c ProcessList.c RichString.c ScreenManager.c Settings.c \
IOPriorityPanel.c SignalsPanel.c String.c SwapMeter.c TasksMeter.c TraceScreen.c \
UptimeMeter.c UsersTable.c Vector.c AvailableColumnsPanel.c AffinityPanel.c \
HostnameMeter.c OpenFilesScreen.c Affinity.c IOPriority.c IncSet.c utils.c

if HAVE_LINUX
myhtopsources += sysdeps/linux.c
endif

if HAVE_SOLARIS
myhtopsources += sysdeps/solaris.c
endif

myhtopheaders = AvailableColumnsPanel.h AvailableMetersPanel.h \
CategoriesPanel.h CheckItem.h ClockMeter.h ColorsPanel.h ColumnsPanel.h \
IOPriorityPanel.h CPUMeter.h CRT.h DisplayOptionsPanel.h FunctionBar.h \
Hashtable.h Header.h htop.h ListItem.h LoadAverageMeter.h MemoryMeter.h \
BatteryMeter.h Meter.h MetersPanel.h Object.h Panel.h ProcessList.h RichString.h \
ScreenManager.h Settings.h SignalsPanel.h String.h SwapMeter.h TasksMeter.h \
TraceScreen.h UptimeMeter.h UsersTable.h Vector.h Process.h AffinityPanel.h \
HostnameMeter.h OpenFilesScreen.h Affinity.h IOPriority.h IncSet.h

SUFFIXES = .h

BUILT_SOURCES = $(myhtopheaders)
htop_SOURCES = $(myhtopheaders) $(myhtopsources) config.h htop-sysdeps.h utils.h

.c.h:
	scripts/MakeHeader.py $<

cppcheck:
	cppcheck -q -v . --enable=all -DHAVE_CGROUP -DHAVE_OPENVZ -DHAVE_TASKSTATS

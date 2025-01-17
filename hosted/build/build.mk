MYDIR := $(dir $(lastword $(MAKEFILE_LIST)))

app_sources := AppMain.cc Printer.cc EbbRTStackRegistrations.cc EbbRTSliceToVolumeRegistration.cc EbbRTCoeffInit.cc
target := AppMain

EBBRT_APP_LINK := -L $(MYDIR)lib $(MYDIR)lib/libzlib.so.1.2.7 -lgeometry++ -lcommon++ -limage++ -ltransformation++ -lniftiio -lznz -lm -lboost_system -lboost_serialization -lgsl -lgslcblas -L /usr/lib/x86_64-linux-gnu/ -lz

include $(abspath ../../../ebbrthosted.mk)
include $(abspath ../../../irtk.mk)

-include $(shell find -name '*.d')

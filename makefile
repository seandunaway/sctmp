CXXFLAGS += -Wno-old-style-cast -Wno-unsafe-buffer-usage -Wno-unused-parameter
LDLIBS += -lgdi32 -lkernel32 -luser32 -ldwmapi

include ../scmake/makefile

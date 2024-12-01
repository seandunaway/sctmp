destdir ?= /Volumes/[C]%Windows%11/SierraChart/Data/ /Volumes/[C]%Windows%11/SierraChartREPLAY/Data/
host ?= windows-11
port ?= 22903 22904
sierrachart ?= /Users/sean/src/_lib/sierrachart
xwin ?= /Users/sean/src/_lib/xwin
source_map ?= z:/src

CXX = clang++
CXXFLAGS += -target $(arch)-pc-windows-msvc -O3 -shared -fuse-ld=lld
CXXFLAGS += -Weverything -Wno-c++98-compat -Wno-missing-prototypes -Wno-old-style-cast -Wno-unsafe-buffer-usage -Wno-unused-parameter
CXXFLAGS += $(addprefix -isystem, $(header))
LDFLAGS += $(addprefix -L, $(addsuffix /$(arch), $(library)))
LDLIBS += -lgdi32 -lkernel32 -luser32 -ldwmapi

ifndef strip
CXXFLAGS += -O0 -g -fstandalone-debug -fdebug-prefix-map=/Users/sean/src=$(source_map)
endif

header += $(xwin)/splat/crt/include
header += $(shell find $(xwin)/splat/sdk/include -maxdepth 1 -type d)
header += $(sierrachart)/ACS_Source
library += $(xwin)/splat/crt/lib
library += $(shell find $(xwin)/splat/sdk/lib -maxdepth 1 -type d)

src = $(wildcard *.cpp)
aarch64 = $(src:.cpp=_arm64.dll)
x86_64 = $(src:.cpp=_64.dll)

default: aarch64 x86_64
aarch64: $(aarch64)
x86_64: $(x86_64)

%_arm64.dll: arch = aarch64
%_arm64.dll: %.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	rm $*_arm64.lib

%_64.dll: arch = x86_64
%_64.dll: %.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	rm $*_64.lib

clean:
	rm -f $(aarch64) $(aarch64:.dll=.lib) $(aarch64:.dll=.pdb) $(x86_64) $(x86_64:.dll=.lib) $(x86_64:.dll=.pdb)

install: $(wildcard *.dll)
	$(foreach dir, $(destdir), $(foreach dll, $^, cp $(dll) $(subst %,\ , $(dir));))

uninstall: unload
	$(foreach dir, $(destdir), cd $(subst %,\ ,$(dir)) && rm -f $(aarch64) $(x86_64);)

unload:
	$(foreach h, $(host), $(foreach p, $(port), scdll -a $(h) -p $(p) unload;))
	sleep 1

load:
	$(foreach h, $(host), $(foreach p, $(port), scdll -a $(h) -p $(p)  load;))

reload: unload install load

.PHONY: default aarch64 x86_64 clean install uninstall unload load reload

BUILDDIR := build
SRCDIR   := src

SHELL := /bin/bash
ifeq ("$(shell clang++ --version > /dev/null 2>&1 ; echo $$?)", "0")
CXX := clang++
endif
ifeq ("clang++", "$(CXX)")
START := clang
else
START := ada
endif

ifeq ($(CXX), clang++)
ifeq ("$(shell ld.lld --version > /dev/null 2>&1 ; echo $$?)", "0")
LINKERFLAGS := -fuse-ld=lld
else
LINKERFLAGS := -fuse-ld=gold
endif
else
LINKERFLAGS := ""
endif

CPP = $(CXX)
TARGET = superboids
CPPFLAGS = -MP -MD
CXXLIBS = -lpthread
WARNINGS = -W -Wall -Wextra -Wshadow -Wstrict-overflow -Wmissing-braces -Wextra-tokens -Wambiguous-member-template -Wbind-to-temporary-copy
GCCWARNINGS = -Wall -Wextra -Wshadow -Wstrict-overflow -Wmissing-braces
CXXFLAGS = -stdlib=libstdc++ -std=c++14 -fno-strict-aliasing -fPIC -flto

CXXSOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS    := $(CXXSOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
DEPS       := $(CXXSOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.d)

all: $(START)

clang: CXXFLAGS+=-O3 $(WARNINGS)
clang: LDFLAGS += $(LINKERFLAGS)
clang: $(TARGET)

ada: CXX=g++
ada: CXXFLAGS=-std=c++14 -fno-strict-aliasing -flto -fPIC -O3 $(GCCWARNINGS)
ada: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(CXXLIBS) $(OBJECTS) -o $(TARGET)

at_once: 
	$(CXX) $(CXXFLAGS) $(CXXLIBS) $(CXXSOURCES) -o $(TARGET)

debug: CXXFLAGS+=-g
debug: CXXFLAGS+=-O0
debug: CXXFLAGS+=$(WARNINGS)
debug: CPPFLAGS+=-DDEBUG
debug: $(TARGET)

copy:
	@rsync -aulv $(TARGET) ada:superboids/

clean:
	@if !(for i in $(OBJECTS) $(DEPS); do [ -e $$i ] && rm $$i && echo $$i removed.; done) then \
		echo "Nothing removed"; \
	fi \
###	[ -e file ] returns true if file exists.

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(OBJECTS): $(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.remove_binary:
	rm $(TARGET)

eita:
	@echo $(OBJECTS)
	@echo $(DEPS)
	@echo $(CXXSOURCES)

-include $(DEPS)


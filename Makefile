BUILDDIR := build
SRCDIR   := src

CXX ?= clang++
CXX =  clang++ ##############
CPP = clang++
TARGET = superboids
CPPFLAGS = -MP -MD
CXXLIBS = -lpthread
WARNINGS = -W -Wall -Wextra -Wshadow -Wstrict-overflow -Wmissing-braces -Wextra-tokens -Wambiguous-member-template -Wbind-to-temporary-copy
GCCWARNINGS = -Wall -Wextra -Wshadow -Wstrict-overflow -Wmissing-braces
CXXFLAGS = -stdlib=libstdc++ -std=c++14 -fno-strict-aliasing -fPIC -flto

ifneq ("$(wildcard /usr/bin/ld.lld)", "")
LDFLAGS += -fuse-ld=lld
else
LDFLAGS += -fuse-ld=gold
endif

CXXSOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS    := $(CXXSOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
DEPS       := $(CXXSOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.d)

all: CXXFLAGS+=-O3 $(WARNINGS)
all: $(TARGET)

ada: CXX=g++
ada: CXXFLAGS=-std=c++14 -fno-strict-aliasing -flto -fPIC -O3 $(GCCWARNINGS)
ada: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(CXXLIBS) $(OBJECTS) -o $(TARGET)

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


# ifneq ("$(wildcard $(HOME)/.clang/usr/bin/clang++)", "")
# CXX=$(HOME)/.clang/usr/bin/clang++
# export LD_LIBRARY_PATH=$(HOME)/.clang/lib/x86_64-linux-gnu/:$(HOME)/.clang/lib/:$(HOME)/.clang/usr/lib/:$(HOME)/.clang/usr/lib/x86_64-linux-gnu
# endif

CXX?=clang++
CXX=clang++ ##############
TARGET=superboids
CPPFLAGS=-MP -MD
CXXLIBS=-lpthread
WARNINGS=-W -Wall -Wextra -Wshadow -Wstrict-overflow -Wmissing-braces -Wextra-tokens -Wambiguous-member-template -Wbind-to-temporary-copy
CXXFLAGS=-stdlib=libstdc++ -std=c++14 -fno-strict-aliasing -fPIC -flto
CXXSOURCES:=Argument.cpp Box.cpp CellNeighbors.cpp Date.cpp Distance.cpp export.cpp lines.cpp load.cpp main.cpp Miniboid.cpp nextstep.cpp parameters.cpp Neighbor.cpp Superboid.cpp SuperboidContainer.cpp TwistNeighbor.cpp
CXXLINKER+=-fuse-ld=gold
OBJECTS=$(CXXSOURCES:.cpp=.o)
DOT_LL=$(CXXSOURCES:.cpp=.ll)
DEPS=$(CXXSOURCES:.cpp=.d)

all: CXXFLAGS+=-O3 $(WARNINGS)
all: $(TARGET)

ada: CXX=g++
ada: CXXFLAGS=-std=c++14 -fno-strict-aliasing -flto -fPIC -O3
ada: $(TARGET)

avx: CXXFLAGS+=-emit-llvm
avx: $(OBJECTS)
	llc-5.0 $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXLINKER) $(CXXLIBS) $(OBJECTS) -o $(TARGET)

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
	@if !(for i in $(OBJECTS) $(DEPS) $(TARGET); do [ -e $$i ] && rm $$i && echo $$i removed.; done) then \
		echo "Nothing removed"; \
	fi \
###	[ -e file ] returns true if file exists.

.remove_binary:
	rm $(TARGET)

-include $(DEPS)

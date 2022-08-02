print-%  : ; @echo $* = $($*)

CXX := clang++-13

FLAGS := -std=c++17 -Wno-deprecated-register -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded

LD_FLAGS := #-L

LD_LIBS := -pthread

INCLUDES := #

BUILD = ./bin/release
CXXFLAGS = $(FLAGS) $(INCLUDES) -O3 -Wall

STRIP := strip

ifneq (, $(findstring debug,$(MAKECMDGOALS)))
	BUILD = ./bin/debug
	CXXFLAGS = $(FLAGS) $(INCLUDES) -g -Weverything
	STRIP = echo
endif

.PHONY: all clean

all: post-build
debug: all
release: all

pre-build:
	@mkdir -p $(BUILD) # prep dist 

post-build: main-build
	$(STRIP) $(BUILD)/*

main-build: pre-build
	@$(MAKE) --no-print-directory $(BUILD)/StdStringRealloc
	@$(MAKE) --no-print-directory $(BUILD)/StdStringRealloc_LLVM_STL
	
clean:
	@rm -r ./bin


$(BUILD)/StdStringRealloc: ./StdStringRealloc.cpp
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LD_FLAGS) $(LD_LIBS)
	@echo "$@"

$(BUILD)/StdStringRealloc_LLVM_STL: ./StdStringRealloc.cpp
	@$(CXX) -stdlib=libc++ $(CXXFLAGS) -I/usr/lib/llvm-13/include/c++/v1/ -nostdinc++ -o $@ $^ $(LD_FLAGS) -lc++ $(LD_LIBS)
	@echo "$@"

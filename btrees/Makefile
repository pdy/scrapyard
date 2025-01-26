print-%  : ; @echo $* = $($*)

CXX := g++

FLAGS := -std=c++20 -Wno-deprecated-register -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded

LD_FLAGS := #-L

LD_LIBS := -pthread

INCLUDES := -isystemcmdline 

BUILD = ./bin/release
CXXFLAGS = $(FLAGS) $(INCLUDES) -O3 -Wall
STRIP := strip
ifneq (, $(findstring debug,$(MAKECMDGOALS)))
	BUILD = ./bin/debug
	CXXFLAGS = $(FLAGS) $(INCLUDES) -g -Wall#-Weverything
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
	#@$(MAKE) --no-print-directory $(BUILD)/btrees
	@$(MAKE) --no-print-directory $(BUILD)/binary_tree
	@$(MAKE) --no-print-directory $(BUILD)/bst
	
clean:
	@rm -r ./bin


$(BUILD)/%: ./%.cpp
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LD_FLAGS) $(LD_LIBS)
	@echo "$@"

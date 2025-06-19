print-%  : ; @echo $* = $($*)

#CXX := clang++
CXX := g++

FLAGS := -std=c++20 -Wno-deprecated-register -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-unsafe-buffer-usage

LD_FLAGS := #-L

LD_LIBS := -lssl -lcrypto -pthread

INCLUDES := -isystemcmdline -isystem/usr/include/openssl

BUILD = ./bin/release
CXXFLAGS = $(FLAGS) $(INCLUDES) -O3 -Wall
STRIP := strip
ifneq (, $(findstring debug,$(MAKECMDGOALS)))
	BUILD = ./bin/debug
#	CXXFLAGS = $(FLAGS) $(INCLUDES) -g3 -fsanitize=address -Wall#-Weverything
	CXXFLAGS = $(FLAGS) $(INCLUDES) -g3 -Wall#-Weverything
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
#	@$(MAKE) --no-print-directory $(BUILD)/log_merger
	@$(MAKE) --no-print-directory $(BUILD)/log_merger_2
	@$(MAKE) --no-print-directory $(BUILD)/log_merger_3
	
clean:
	@rm -r ./bin


$(BUILD)/%: ./%.cpp
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LD_FLAGS) $(LD_LIBS)
	@echo "$@"

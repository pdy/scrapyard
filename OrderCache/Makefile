print-%  : ; @echo $* = $($*)

CXX := g++

FLAGS := -std=c++17 -Wno-deprecated-register -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded

LD_FLAGS := #-L

LD_LIBS := -pthread

INCLUDES := -isystemcmdline 

BUILD = ./bin/release
CXXFLAGS = $(FLAGS) $(INCLUDES) -O3 -Wall
STRIP := strip
ifneq (, $(findstring debug,$(MAKECMDGOALS)))
	BUILD = ./bin/debug
	CXXFLAGS = $(FLAGS) $(INCLUDES) -g #-Weverything
	STRIP = echo
endif

OBJ_PATH := $(BUILD)/obj

.PHONY: all clean

all: post-build
debug: all
release: all

pre-build:
	@mkdir -p $(BUILD) # prep dist 
	@mkdir -p $(OBJ_PATH) # prep dist 

post-build: main-build
	$(STRIP) $(BUILD)/MainOrderCache

main-build: pre-build
	@$(MAKE) --no-print-directory $(BUILD)/MainOrderCache
	
clean:
	@rm -r ./bin


SRCS := $(wildcard ./*.cpp)
OBJS := $(patsubst ./%.cpp,$(OBJ_PATH)/%.o, $(SRCS))

#$(BUILD)/%: ./%.cpp
#	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LD_FLAGS) $(LD_LIBS)
#	@echo "$@"

$(BUILD)/MainOrderCache: $(OBJS)	
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LD_FLAGS) $(LD_LIBS) 
	@echo "$<"

$(OBJ_PATH)/%.o: ./%.cpp
	@$(CXX) $(CXXFLAGS) $(TEST_FLAGS) -c -o $@ $^ 
	@echo "$<"

print-%  : ; @echo $* = $($*)

CXX := clang++
STRIP := strip

FLAGS := -std=c++17 -Wno-deprecated-register -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded

LD_FLAGS := -L./glfw/lib

LD_LIBS := -pthread -lglfw3 -ldl -lX11

INCLUDES := -isystem./imgui -isystem./glfw/include -isystem./glad/include

SRC_FILES := $(wildcard ./*.cpp) \
	$(wildcard ./imgui/*.cpp) \
	$(wildcard ./imgui/bindings/*.cpp) \
	$(wildcard ./glad/src/*.c)

ifeq ($(MAKECMDGOALS),debug)
	BUILD = ./bin/debug
	CXXFLAGS = $(FLAGS) $(INCLUDES) -g -Weverything
	STRIP = echo
else
	BUILD = ./bin/release
	CXXFLAGS = $(FLAGS) $(INCLUDES) -O3 -Wall
endif

.PHONY: all clean

all: dist PROGRAMS strip
debug: all

dist:
	@mkdir -p $(BUILD)/
	
clean:
	@rm -r ./bin

strip:
	$(STRIP) $(BUILD)/*

PROGRAMS: $(BUILD)/ImguiTests 

$(BUILD)/ImguiTests: $(SRC_FILES)
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LD_FLAGS) $(LD_LIBS)
	@echo "$<"


#   $(SRC_FILES)
#	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LD_FLAGS) $(LD_LIBS)
#	@echo "$<"


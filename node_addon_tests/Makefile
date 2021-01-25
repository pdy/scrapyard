print-%  : ; @echo $* = $($*)

CXX := clang++
STRIP := strip

FLAGS := -fPIC -std=c++17 -Wno-deprecated-register -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded

LD_FLAGS := #-L

LD_LIBS := -pthread

INCLUDES := 

ifeq ($(MAKECMDGOALS),debug)
	BUILD = ./bin/debug
	CXXFLAGS = $(FLAGS) $(INCLUDES) -g -Weverything
	STRIP = echo
else
	BUILD = ./bin/release
	CXXFLAGS = $(FLAGS) $(INCLUDES) -O3 -Wall
endif

SRC_FILES := $(wildcard ./*.cpp)

OBJ_DIR := $(BUILD)/obj
OBJS := $(patsubst ./%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

.PHONY: all clean

all: dist PROGRAMS strip
debug: all

dist:
	@mkdir -p $(BUILD) $(OBJ_DIR)
	
clean:
	@rm -r ./bin


PROGRAMS: $(BUILD)/node_addon_tests $(BUILD)/test_addon.so

strip:
	$(STRIP) $(BUILD)/node_addon_tests


$(BUILD)/node_addon_tests: $(OBJ_DIR)/node_addon_tests.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LD_FLAGS) $(LD_LIBS)
	@echo "$<"

$(BUILD)/test_addon.so: $(OBJ_DIR)/test_addon.o
	@$(CXX) -o $@ $^ $(LD_FLAGS) $(LD_LIBS) -shared
	@echo "$<"

$(OBJ_DIR)/%.o: ./%.cpp
	@$(CXX) $(CXXFLAGS) -c -o $@ $^ 
	@echo "$<"

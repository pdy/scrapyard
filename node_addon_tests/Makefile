print-%  : ; @echo $* = $($*)

CXX := clang++

INCLUDES := 

ifeq ($(MAKECMDGOALS),debug)
	BUILD = ./bin/debug
	CXXFLAGS = $(FLAGS) $(INCLUDES) -g -Weverything
	STRIP = echo
else
	BUILD = ./bin/release
	CXXFLAGS = $(FLAGS) $(INCLUDES) -O3 -Wall
endif

FLAGS := -fPIC -std=c++17 -Wno-deprecated-register -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded
LD_FLAGS := #-L$(BUILD)/
LD_LIBS := -pthread #-l:test_addon.so 
SO_LD_LIBS := $(LD_LIBS) -shared 

SRC_FILES := $(wildcard ./*.cpp)

OBJ_DIR := $(BUILD)/obj
OBJS := $(patsubst ./%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

.PHONY: all clean 

all: dist addon 
debug: all

dist:
	@mkdir -p $(BUILD) $(OBJ_DIR)
	
clean:
	@rm -r ./bin


addon: $(BUILD)/test_addon.so	

$(BUILD)/test_addon.so: $(OBJS)
	@$(CXX) -o $@ $^ $(LD_FLAGS) $(SO_LD_LIBS)
	@echo "$<"

$(OBJ_DIR)/%.o: ./%.cpp
	@$(CXX) $(CXXFLAGS) -c -o $@ $^ 
	@echo "$<"

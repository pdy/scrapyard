print-%  : ; @echo $* = $($*)

CXX := clang++
C := clang
STRIP := strip

FLAGS := -std=c++17 -Wno-deprecated-register -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded

LD_FLAGS := -L./glfw/lib

LD_LIBS := -pthread -lglfw3 -ldl -lX11

INCLUDES := -isystem./imgui -isystem./glfw/include -isystem./glad/include

ifeq ($(MAKECMDGOALS),debug)
	BUILD = ./bin/debug
	CXXFLAGS = $(FLAGS) $(INCLUDES) -g -Weverything
	STRIP = echo
else
	BUILD = ./bin/release
	CXXFLAGS = $(FLAGS) $(INCLUDES) -O3 -Wall
endif

SRC_IMGUI := $(wildcard ./imgui/*.cpp)
SRC_GLAD := $(wildcard ./glad/src/*.c)
SRC_FILES := $(wildcard ./*.cpp)

OBJ_DIR := $(BUILD)/obj
OBJ_IMGUI_DIR := $(OBJ_DIR)/imgui
OBJ_GLAD_DIR := $(OBJ_DIR)/glad
OBJS := $(patsubst ./%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
OBJS_GLAD := $(patsubst ./glad/src/%.c,$(OBJ_GLAD_DIR)/%.o,$(SRC_GLAD))
OBJS_IMGUI := $(patsubst ./imgui/%.cpp,$(OBJ_IMGUI_DIR)/%.o,$(SRC_IMGUI))

.PHONY: all clean

all: dist PROGRAMS
debug: all

dist:
	@mkdir -p $(BUILD)/ $(OBJ_DIR) $(OBJ_IMGUI_DIR) $(OBJ_GLAD_DIR)
	
clean:
	@rm -r ./bin

strip:
	$(STRIP) $(BUILD)/ImguiTests

PROGRAMS: $(BUILD)/ImguiTests 

$(BUILD)/ImguiTests: $(OBJS) $(OBJS_GLAD) $(OBJS_IMGUI)
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LD_FLAGS) $(LD_LIBS)
	@echo "$<"

$(OBJ_DIR)/%.o: ./%.cpp
	@$(CXX) $(CXXFLAGS) -c -o $@ $^ 
	@echo "$<"

$(OBJ_IMGUI_DIR)/%.o: ./imgui/%cpp
	@$(CXX) $(CXXFLAGS) -c -o $@ $^
	@echo "$<"

$(OBJ_GLAD_DIR)/%.o: ./glad/src/%.c
	@$(C) $(INCLUDES) -c -o $@ $^
	@echo "$<"



print-%  : ; @echo $* = $($*)

CXX := clang++
STRIP := strip

# -frtti is required by boost, which is linked staticly
MANUAL_FLAGS := -std=c++14 -frtti -fexceptions -Wno-deprecated-register
FLAGS := $(MANUAL_FLAGS)

LD_FLAGS := -L./3rd/boost/lib

BOOST_LINKING := -lboost_system -lboost_filesystem -lboost_program_options -lboost_log -lboost_log_setup -lboost_thread
GDAL_LINKING := -lgdal -lkmlbase -lkmlengine -lkmldom -lexpat -lz -lxerces-c -lcurl -ldl

LD_LIBS := -Wl,-Bstatic $(BOOST_LINKING) -Wl,-Bdynamic -pthread $(GDAL_LINKING) 

INCLUDES := -I/usr/include/
INCLUDES += -I./3rd/boost/include
ROOT_BUILD := ./build

ifeq ($(MAKECMDGOALS),debug)
	BUILD = $(ROOT_BUILD)/debug
	CXXFLAGS = $(FLAGS) $(INCLUDES) -g -Wall
	STRIP = echo
else
	BUILD = $(ROOT_BUILD)/release
	CXXFLAGS = $(FLAGS) $(INCLUDES) -O3 -Wall	
endif

OBJ := $(BUILD)/obj

.PHONY: all clean 

DESTBIN := $(BUILD)/bin

all: dist $(DESTBIN)/GdalMess strip data
debug: all

dist:
	@mkdir -p $(OBJ) $(DESTBIN)
	
clean:
	@rm -r $(BUILD) $(ROOT_BUILD)

data:
	@cp -r src/data $(DESTBIN)/

strip:
	$(STRIP) $(DESTBIN)/GdalMess

OBJS := $(OBJ)/GdalMess.o $(OBJ)/Application.o

$(DESTBIN)/GdalMess: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LD_FLAGS) $(LD_LIBS)

$(OBJ)/%.o: ./src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

$(OBJ)/Application.o: ./src/application/Application.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

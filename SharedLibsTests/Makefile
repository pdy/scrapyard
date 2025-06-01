print-%  : ; @echo $* = $($*)

#CXX := clang++
CXX := g++

FLAGS := -std=c++20 -Wno-deprecated-register -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded

OSSL_111s_PATH := /home/songoku/repo/openssl_1.1.1/build_111s
OSSL_111s_INCLUDES := -isystem$(OSSL_111s_PATH)/include 
#OSSL_111s_LIBS := -L$(OSSL_111s_PATH)/lib -Wl,-Bstatic -lssl -lcrypto

OSSL_111w_PATH := /home/songoku/repo/openssl_1.1.1/build_111w
OSSL_111w_INCLUDES := -isystem$(OSSL_111w_PATH)/include
#OSSL_111w_LIBS := -L$(OSSL_111w_PATH)/lib -Wl,-Bstatic -lssl -lcrypto

LD_FLAGS := -L$(OSSL_111s_PATH)/lib
LD_SHARED_FLAGS := -L$(OSSL_111w_PATH)/lib

#LD_LIBS := -Wl,-Bstatic -lssl -lcrypto -Wl,-Bdynamic -pthread -ldl
LD_LIBS := -Wl,-Bdynamic -lrsa_pem_to_der -pthread -ldl
LD_SHARED_LIBS := -Wl,-Bstatic -lssl -lcrypto -Wl,-Bdynamic -pthread -ldl

INCLUDES := -isystemcmdline 



BUILD = ./bin/release
CXXFLAGS = $(FLAGS) $(INCLUDES) -O3 -Wall
STRIP := strip
ifneq (, $(findstring debug,$(MAKECMDGOALS)))
	BUILD = ./bin/debug
	CXXFLAGS = $(FLAGS) $(INCLUDES) -g -Wall#-Weverything
	STRIP = echo
endif

LD_FLAGS += -L$(BUILD)

.PHONY: all clean shared

all: post-build
debug: all
release: all

pre-build:
	@mkdir -p $(BUILD) # prep dist 

post-build: main-build
#	$(STRIP) $(BUILD)/*

main-build: pre-build
	@$(MAKE) --no-print-directory $(BUILD)/SharedLibsTests
	
clean:
	@rm -r ./bin

$(BUILD)/SharedLibsTests: SharedLibsTests.cpp shared
	$(CXX) $(CXXFLAGS) $(OSSL_111s_INCLUDES) -o $@ $< $(LD_FLAGS) $(LD_LIBS) 
	#@echo "$@"

shared: $(BUILD)/rsa_pem_to_der.o
	$(CXX) -shared -o $(BUILD)/librsa_pem_to_der.so $^ $(LD_SHARED_FLAGS) $(LD_SHARED_LIBS)
#	@echo "Not building shared right now"

$(BUILD)/rsa_pem_to_der.o : rsa_pem_to_der.cpp
	$(CXX) -c $(CXXFLAGS) $(OSSL_111w_INCLUDES) -fPIC -o $@ $^ $(LD_SHARED_FLAGS) $(LD_LIBS)

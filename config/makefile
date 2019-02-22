TARGET_NAME       := HelloWorld

HEADER_PATH       := ./header
SOURCE_PATH       := ./source
OBJECT_PATH       := ./object
TARGET_PATH       := ./target

CC                := g++

INCLUDES          := -I/usr/include
INCLUDES          += -I/usr/local/include
INCLUDES          += -I/opt/FabricEngine-2.3.0-Linux-x86_64/include

LINKS             := -L/usr/lib
LINKS             += -L/usr/local/lib
LINKS             += -L/opt/FabricEngine-2.3.0-Linux-x86_64/lib -lFabricCore

DEFINES           := -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
DEFINES           += -DFEC_SHARED -DFEC_PROVIDE_STL_BINDINGS

CCFLAGS           := -O3 -m64 -fopenmp -std=c++11
LDFLAGS           := -fopenmp

CPP_NAME          := $(notdir $(wildcard $(SOURCE_PATH)/*.cpp))        # *.cpp
OBJ_LIST          := $(addprefix $(OBJECT_PATH)/,$(CPP_NAME:.cpp=.o))  # object/*.o
DEP_LIST          := $(addprefix $(OBJECT_PATH)/,$(CPP_NAME:.cpp=.d))  # object/*.d

#.SILENT:

all: $(TARGET_PATH)/$(TARGET_NAME)

# .o -> .so
$(TARGET_PATH)/$(TARGET_NAME): $(OBJ_LIST)
    $(CC) -o $@ $< $(LDFLAGS) $(LINKS)

# .cpp -> .o
$(OBJECT_PATH)/%.o: $(SOURCE_PATH)/%.cpp
    $(CC) -o $@ $< -c $(DEFINES) $(CCFLAGS) $(INCLUDES) -MMD

-include $(DEP_LIST)

clean:
	rm -f $(OBJECT_PATH)/*
	rm -f $(TARGET_PATH)/*

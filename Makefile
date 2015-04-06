# Multi-platform makefile for SPARToR

# Get system name
UNAME := $(shell sh -c 'uname -s 2>/dev/null || echo not')

# Same for all platforms, probably
CC = gcc #tcc
MKDIRP = mkdir -p
OBJDIR = objects
SRCS = $(wildcard engine/*.c)
SRCS += $(wildcard engine/mt19937ar/*.c)
include game/Makefile-include
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

GITCOMMIT := $(shell sh -c "git branch -v | grep '^\*' | sed 's/\s\+/ /g' | cut -d' ' -f2,3")

FLAGS = -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-overlength-strings -pedantic -DGLEW_STATIC
FLAGS += -DGITCOMMIT='"$(GITCOMMIT)"'
FLAGS += -std=c99


INC = -Iengine -Igame

# Only useful on certain platforms
OBJSRES =
WINDRES =
POSTCC =


ifeq ($(UNAME),Linux)
	EXE_NAME = spartor
	FLAGS += `sdl2-config --cflags`
	LIBS = -lm -lSDL2 -lSDL2_net -lSDL2_image -lGL -lGLU -lGLEW
endif
ifeq ($(UNAME),Darwin)
	EXE_NAME = platforms/mac/spartor.app/Contents/MacOS/spartor
	FLAGS += `sdl2-config --cflags`
	LIBS = -lm `sdl2-config --libs` -lSDL2_net -lSDL2_image -framework OpenGL -lGLEW
	POSTCC = cp -R -f platforms/mac/spartor.app .
endif
ifneq (,$(findstring MINGW,$(UNAME)))
	EXE_NAME = spartor.exe
	OBJSRES = game/icon.o
	WINDRES = windres
	FLAGS += -mwindows
	LIBS = -L/usr/local/lib -L/usr/lib \
	       -lmingw32 -lSDL2main -lSDL2 -lSDL2_net -lSDL2_image -lglew32 -lopengl32 -lglu32 -lm
	INC += -I/usr/local/include/SDL2 -I/usr/include
	POSTCC = cp platforms/win/*.dll .
endif

all: $(EXE_NAME)

$(EXE_NAME): $(OBJS) $(OBJSRES)
	$(CC) -o $@ $(OBJS) $(OBJSRES) $(FLAGS) $(INC) $(LIBS) $(XLIBS)
	$(POSTCC)

-include $(DEPS)

$(OBJDIR)/%.o: %.c
	@$(MKDIRP) $(dir $@)
	$(CC) $(FLAGS) $(INC) -MM -MT $@ -MF $(patsubst %.o,%.d,$@) $<
	$(CC) -o $@ -c $(FLAGS) $(INC) $<

.rc.o:
	$(WINDRES) $^ -o $@
%.o : %.rc
	$(WINDRES) $^ -o $@

clean:
	-$(RM) $(OBJS) $(OBJSRES)

distclean: clean
	-$(RM) $(EXE_NAME)

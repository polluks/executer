CC=vc
RM=rm -f

CONFIG=+aos68k
CFLAGS=-I. -Isrc
LDFLAGS=-lamiga

TARGET=Executer

SRCS= src/executer.c \
	src/libraries.c \
	src/arexx.c

ifeq ($(ENABLE_MUI), 1)
CFLAGS+=-DENABLE_MUI
SRCS+=src/window-mui.c
TARGET=Executor_mui
else
SRCS+=src/window-gadtools.c
endif

OBJS=$(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CONFIG) $(LDFLAGS) $(OBJS) -o $@

$(OBJS): %.o: %.c
	$(CC) $(CONFIG) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

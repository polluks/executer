CC=vc
RM=rm -f

CONFIG=+aos68k
CFLAGS=-I. -Isrc
LDFLAGS=-lamiga

TARGET=Executer

SRCS= src/executer.c \
	src/libraries.c \
	src/arexx.c \
	src/window.c

OBJS=$(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CONFIG) $(LDFLAGS) $(OBJS) -o $@

$(OBJS): %.o: %.c
	$(CC) $(CONFIG) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

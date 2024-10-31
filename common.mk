TARGET=Executer

SRCS= src/executer.c \
	src/libraries.c \
	src/arexx.c \
	src/notify.c \
	src/spawn.c \
	src/prefs.c \
	src/utility.c

ifeq ($(ENABLE_MUI), 1)
CFLAGS+=-DENABLE_MUI
SRCS+=src/mui/window.c \
	src/mui/classes.c \
	src/mui/objects.c \
	src/mui/application-class.c \
	src/mui/main-window-class.c \
	src/mui/list-class.c
else
SRCS+=src/gadtools/window.c
SRCS+=src/gadtools/window-main.c
SRCS+=src/gadtools/window-edit.c
endif

OBJS=$(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

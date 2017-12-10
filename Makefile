
CFLAGS   = -W -Wall -Wextra
CPPFLAGS = -DSUNRISET_LIB
LDLIBS   = -lm

OBJS     = sun.o sunriset.o
EXEC     = sun

all: $(EXEC)

$(EXEC): $(OBJS)

clean:
	$(RM) *.o

distclean: clean
	$(RM) *~


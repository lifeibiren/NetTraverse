CC=gcc
AS=as
LD=ld

ASFLAGS=
CFLAGS=-g -O3
LDFALGS=-lm
OBJECTS= ctx.o ctx_switch.o ctx_create.o
all: ctx

ctx: ${OBJECTS}
	gcc -o ctx ${OBJECTS} ${LDFALGS} ${CFLAGS}
clean:
	rm -rf ctx *.o

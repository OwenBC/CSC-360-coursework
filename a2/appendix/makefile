CC=gcc
# CFLAGS=-g -D_REENTRANT
CFLAGS=-g -D_REENTRANT -Wall -Werror -std=c18
OBJECTS_A=cp-problem-sem.o
OBJECTS_B=cp-problem-mcv.o
# LIBS=-lpthread -lrt
LIBS=-lpthread

all:cp-problem-mcv cp-problem-sem

cp-problem-sem: $(OBJECTS_A)
	$(CC) -o cp-problem-sem $(OBJECTS_A) $(LIBS) $(CFLAGS)

cp-problem-mcv: $(OBJECTS_B)
	$(CC) -o cp-problem-mcv $(OBJECTS_B) $(LIBS) $(CFLAGS)

cp-problem-sem.o: cp-problem-sem.c
	$(CC) $(CFLAGS) -c cp-problem-sem.c

cp-problem-mcv.o: cp-problem-mcv.c
	$(CC) $(CFLAGS) -c cp-problem-mcv.c

clean:
	-rm -f $(OBJECTS_A) cp-problem-sem $(OBJECTS_B) cp-problem-mcv

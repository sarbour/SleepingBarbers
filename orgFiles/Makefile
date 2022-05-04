# the compiler: gcc for C program, define as g++ for C++
CC = g++

# compiler flags:
# -g     - this flag adds debugging information to the executable file
# -Wall  - this flag is used to turn on most compiler warnings
CFLAGS = -g -Wall -lpthread

all: sleepingBarbers

sleepingBarbers: driver.o Shop_org.o
	$(CC) $(CFLAGS) driver.o Shop_org.o -o sleepingBarbers

driver.o: Shop_org.h

Shop_org.o: Shop_org.h

clean:
	$(RM) sleepingBarbers driver.o Shop_org.o
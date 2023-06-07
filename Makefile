CFLAGS = -O2

OBJS = racetroute.o sendpacketudp.o sendpackettcp.o sendpacketicmp.o \
       getinferface.o printpacket.o printsent.o
SRCS = racetroute.c sendpacketudp.c sendpackettcp.c sendpacketicmp.c \
       getinferface.c printpacket.c printsent.c analyze.c

all: racetroute analyze

linux: all
	echo "sudo /sbin/setcap cap_net_raw=eip /home/iljitsch/racetroute"

racetroute: $(OBJS)

$(OBJS): racetroute.h Makefile

clean:
	rm *.o racetroute analyze

analyze: analyze.o
analyze.o: Makefile

tar: $(SRCS) racetroute.h Makefile
	touch racetroute.tar
	rm racetroute.tar
	co $(SRCS)
	tar cvf racetroute.tar *.c *.h Makefile

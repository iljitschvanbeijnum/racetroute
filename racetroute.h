#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in_systm.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#include <sys/select.h>
#include <fcntl.h>

// sudo /sbin/setcap cap_net_raw=eip /home/vanbeijnum/racetroute
// sudo /sbin/setcap cap_net_raw=eip racetroute

#define RUNTIME 3
#define MAXPACKET 9216
#define PORTRANGE 100
#define NUMPACKET 100
#define MPMAXTTL 60

#define TESTPROTO 7
#define TESTUDP 1
#define TESTTCP 2
#define TESTICMP 4
#define TESTNOSUM 8
#define TESTSPORT 16
#define TESTDPORTM1 32
#define TESTDPORT6 64
#define TESTDATA 128

#define MAXTT 20

extern int done;
extern int verbosity;

struct udphdr
  {
    unsigned short sport;
    unsigned short dport;
    unsigned short len;
    unsigned short sum;
  };

struct tcphdr
  {
    unsigned short sport;
    unsigned short dport;
    unsigned int seq;
    unsigned int ack;
    unsigned int resflagwin;
    unsigned short sum;
    unsigned short urg;
  };

union ipaddr
  {
    unsigned char byte[4];
    unsigned int addr;
  };

struct udata
  {
    struct in_addr *destaddr;
    char *target;
    int sport;
    int dport;
    int id;
    int hwater;
    int oo;
    int numoo;
    int idhwater;
    int numidoo;
    int startts;
    int icmptype;
    int lo;
    int n;
    int mod;
    int offset;
    int testtype;
    FILE *ofile;
    int numtt;
    int ttype[MAXTT];
    union ipaddr remoteaddr[NUMPACKET + 5];
    int num[NUMPACKET + 5];
    int group[NUMPACKET + 5];
    unsigned short recvid[NUMPACKET + 5];
    unsigned short returnid[NUMPACKET + 5];
  };

struct pdata
  {
    int n;
    int sttl;
    int ttl;
    int tos;
    int rhop;
    int proto;
    int process;
    int startid;
    int noname;
    int size;
    int unreachhops;
    unsigned int saddr;
    int status[NUMPACKET];
    unsigned short sport[NUMPACKET];
    unsigned short dport[NUMPACKET];
    unsigned short id[NUMPACKET];
    char target[80];
  };

struct palette
  {
    char *inord;
    char *ord1;
    char *ord1b;
    char *ord2;
    char *ord2b;
    char *star;
    char *excl;
    char *space;
    char *inconret;
    char *inordret;
    char *outordret;
    char *reset;
  };

int getinterface(char *ifin, char *ifout, unsigned int *ipaddr);
void printpacket(u_char *user, const u_char *bytes, int len);
void sendpacketudp(int sock, struct in_addr *destaddr, struct pdata *pdata, struct udata *udata);
void sendpackettcp(int sock, struct in_addr *destaddr, struct pdata *pdata, struct udata *udata);
void sendpacketicmp(int sock, struct in_addr *destaddr, struct pdata *pdata, struct udata *udata);
void printsent(unsigned char *bytes, int packetlen, struct udata *udata);


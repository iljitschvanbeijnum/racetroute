#include "racetroute.h"

int getinterface(char *ifin, char *ifout, unsigned int *ipaddr)
{
  unsigned char *bla;
  struct ifaddrs *ifap, *ifcur;

  *ipaddr = 0;

  if (getifaddrs(&ifap))
    {
      printf("Couldn't get interface address information\n");
      exit(1);
    }

  ifcur = ifap;
  do
    {
      // printf("Interface found: %s (%i)\n", ifcur->ifa_name, ifcur->ifa_flags & (IFF_UP | IFF_LOOPBACK));
      if (ifcur->ifa_flags & IFF_UP && ifcur->ifa_addr->sa_family == AF_INET &&
         ((strlen(ifin) == 0 && !(ifcur->ifa_flags & IFF_LOOPBACK)) ||
         (strlen(ifin) > 0 && strcmp(ifin, ifcur->ifa_name) == 0)))
        {
          strcpy(ifout, ifcur->ifa_name);
          *ipaddr = ((struct sockaddr_in *)ifcur->ifa_addr)->sin_addr.s_addr;
        }
      ifcur = ifcur->ifa_next;
    }
  while (!*ipaddr && ifcur);

  freeifaddrs(ifap);

  if (*ipaddr)
    {
      bla = (unsigned char *)ipaddr;
      if (verbosity > 2)
        printf("Using interface %s with address %i.%i.%i.%i\n", ifout, bla[0], bla[1], bla[2], bla[3]);
      return 0;
    }
  else
    {
      printf("No suitable interface found\n");
      exit(1);
    }
}

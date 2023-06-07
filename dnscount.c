#include "racetroute.h"

int ipid = 2048;
int verbosity = 1;
int done = 3;

int main(int argc, char *argv[])
{
  struct addrinfo hints, *resfirst, *rescur;
  int count[4], v4, v6, totv4, totv6;
  struct in_addr *destaddr;
  int sock;
  struct hostent *desthe;
  char *dest;
/*
  unsigned short sport = 34327;
*/
  unsigned short sport = 34300;
  unsigned short dport = 33435;
  pid_t pid;
  int i, j, k, l, tt, num, first, success, output;
  union ipaddr previous;
  struct pdata pdata;
  char ifin[20] = "";
  char ifout[20];
  char file[160] = "";
  char temp[160];
  FILE *f = NULL;
  struct timeval timeout, timeout2, timeout3;
  struct msghdr message;
  struct iovec vector;
  unsigned char packet[MAXPACKET];
  struct cmsghdr ancdata;
  struct udata udata;
  fd_set fds;
  struct timeval ts;
  int b0, b1, b2, b3;
  union ipaddr a1;
  struct in_addr a2;
  char *d2 = dest;

  vector.iov_base = packet;
  vector.iov_len = MAXPACKET;

  message.msg_name = 0;
  message.msg_namelen = 0;
  message.msg_iov = &vector;
  message.msg_iovlen = 1;
  message.msg_control = &ancdata;
  message.msg_controllen = sizeof(ancdata);

  pdata.n = 10;
  pdata.sttl = 1;
  pdata.size = 46;
  pdata.proto = 17;     // UDP
  pdata.process = 0;
  pdata.startid = 0;

  udata.ofile = NULL;
  udata.target = pdata.target;
  // udata.testtype = 1;
  udata.mod = 0;
  udata.offset = 0;
  udata.numtt = 0;

  pdata.noname = 0;

  output = 0;

  first = 0;
  for (i = 1; i < argc; i++)
    {
      if (strlen(argv[i]) == 2 && argv[i][0] == '-')
        {
          switch (argv[i][1])
            {
              // case 'a': pdata.noname = 1; break;
              case 'm':
                sscanf(argv[++i], "%d", &udata.mod);
                sscanf(argv[++i], "%d", &udata.offset);
                break;
              case 'v': sscanf(argv[++i], "%d", &verbosity); break;
              case 'i': strcpy(ifin, argv[++i]); break;
              case 't': sscanf(argv[++i], "%d", &pdata.sttl); break;
              case 's': sscanf(argv[++i], "%d", &pdata.size); break;
              case 'c': sscanf(argv[++i], "%d", &pdata.n); break;
              case 'f': sscanf(argv[++i], "%s", file); break;
              case 'p': sscanf(argv[++i], "%d", &pdata.process); break;
              case 'e': // sscanf(argv[++i], "%d", &udata.testtype); break;
                while (argv[i+1][0] >= '0' && strlen(argv[i+1]) < 5 && argv[i+1][0] <= '9')
                  sscanf(argv[++i], "%d", &udata.ttype[udata.numtt++]);
                break;
              case 'o': output = 1; break;
            }
        }
      else
        if (first <= 1)
          first = i;
    }
  if ((!first && strlen(file) == 0) || argc == 1)
    {
      printf("Usage: %s [-m modulus offset] [-p <process>] [-v <verbosity>] [-i <interface>] [-t <start ttl>] [-s <size>] [-c <packetcount>] [-e <test type(s)>] [-o] [-f] <destination(s) (file)>\n", argv[0]);
      exit(1);
    }

  if (pdata.process <= 0)
    pdata.process = -pdata.process + udata.offset;

  if (udata.numtt == 0)
    udata.ttype[udata.numtt++] = 1;

  if (pdata.process < 0 || pdata.process > 125)
    {
      printf("Process number must be between 0 and 125 (inclusive)\n");
      exit(1);
    }
  pdata.startid = pdata.process * 500 + 2000;

  if (pdata.n > NUMPACKET)
    {
      printf("Number of packets is out of range, must be <= %i\n", NUMPACKET);
      exit(1);
    }

  if (pdata.size < 28 || pdata.size > MAXPACKET)
    {
      printf("Size is out of range, must be >= 28 and <= %i\n", MAXPACKET);
      exit(1);
    }

  if (strlen(file) > 0)
    {
      f = fopen(file, "r");
      if (!f)
        {
          printf("couldn't open input file!\n");
          exit(1);
        }
      k = argc;

      if (udata.offset)
        for (i = 0; i < udata.offset; i++)
          fgets(temp, 160, f);
          // fscanf(f, "%s", temp);
    }
  else
    {
      k = first;
      f = NULL;
    }

  gettimeofday(&ts, NULL);
  udata.startts = ts.tv_sec;

  for (i = 0; i < 4; i++)
    count[i] = 0;
  totv4 = 0;
  totv6 = 0;

  // for (k = first; k < argc; k++)
  while ((f && !feof(f)) || (k < argc))
    {
      if (!f)
        {
          dest = argv[k];
          k++;
        }
      else
        {
          file[0] = 0;
          fgets(temp, 160, f);
          sscanf(temp, "%s", file);
          // fscanf(f, "%s", file);
          dest = file;

          if (udata.mod > 1)
            for (i = 1; i < udata.mod; i++)
              fgets(temp, 160, f);
              // fscanf(f, "%s", temp);
        }
      if (f && feof(f) && strlen(dest) == 0)
        break;

      if (strncmp(dest, "LISTEN", 6) != 0)
        {
          strcpy(pdata.target, dest);

/*
          if (pdata.noname)
            {
              d2 = dest;
              while (*d2)
                {
                  if (*d2 == '.')
                    *d2 = ' ';
                  d2++;
                }

              sscanf(dest, "%i %i %i %i", &b0, &b1, &b2, &b3);
              a1.byte[0] = b0;
              a1.byte[1] = b1;
              a1.byte[2] = b2;
              a1.byte[3] = b3;
              a2.s_addr = a1.addr;
            }
*/

/*
          desthe = gethostbyname(pdata.target);
          if (!desthe)
            {
              sprintf(pdata.target, "www.%s", dest);
              desthe = gethostbyname(pdata.target);
              if (!desthe)
                {
                  printf("Can't parse destination %s\n", dest);
                  if (!f)
                    exit(1);
                }
            }

          if (desthe)
            {
              destaddr = (struct in_addr *)desthe->h_addr_list[0];
*/



           memset(&hints, 0, sizeof(hints));
           hints.ai_family = PF_UNSPEC;
           hints.ai_socktype = 0;
           hints.ai_protocol = IPPROTO_TCP;

           v4 = 0;
           v6 = 0;

           if (getaddrinfo(pdata.target, NULL, &hints, &resfirst))
             {
               freeaddrinfo(resfirst);
               strcpy(dest, pdata.target);
               sprintf(pdata.target, "www.%s", dest);
               if (getaddrinfo(pdata.target, NULL, &hints, &resfirst))
                 {
                   printf("Can't parse destination %s\n", dest);
                   if (!f)
                     exit(1);
                 }
             }

           for (rescur = resfirst; rescur; rescur = rescur->ai_next)
             {
               // s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
               if (rescur->ai_family == PF_INET)
                 v4++;
               if (rescur->ai_family == PF_INET6)
                 v6++;
               if (verbosity > 2)
                 {
                   printf("Found address of type %i: ", rescur->ai_family);
                   if (rescur->ai_family == PF_INET)
                     printf("IPv4");
                   if (rescur->ai_family == PF_INET6)
                     printf("IPv6");
                   printf("\n");
                 }
              }
            freeaddrinfo(resfirst);

            if (v4)
	      {
                if (v6)
                  {
                    count[3]++;
                    if (verbosity > 1)
                      printf("%s has %i IPv6 and %i IPv4 addresses\n", pdata.target, v6, v4);
                  }
                else
                  {
                    count[1]++;
                    if (verbosity > 2)
                      printf("%s has %i IPv4 addresses\n", pdata.target, v4);
                  }
              }
            else
	      {
                if (v6)
                  {
                    count[2]++;
                    if (verbosity > 1)
                      printf("%s has %i IPv6 addresses\n", pdata.target, v6);
                  }
                else
                  {
                    count[0]++;
                    if (verbosity > 2)
                      printf("%s has no addresses\n", pdata.target);
                  }
              }
            totv4 += v4;
            totv6 += v6;

/*
            }
*/
        }
    }

  if (output)
    fclose(udata.ofile);
  if (f)
    fclose(f);

  printf("Number of destinations with no addresses: %i\n", count[0]);
  printf("Number of IPv4-only destinations:  %i\n", count[1]);
  printf("Number of IPv6-only destinations:  %i\n", count[2]);
  printf("Number of dual stack destinations: %i\n", count[3]);
  printf("Total addresses, IPv4: %i, IPv6: %i\n", totv4, totv6);
}


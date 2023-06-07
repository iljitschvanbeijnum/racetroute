#include "racetroute.h"

int ipid = 2048;
int verbosity = 1;
int done = 3;

int main(int argc, char *argv[])
{
  int sock;
  struct hostent *desthe;
  char *dest;
  unsigned short sport = 34300;
  unsigned short dport = 33435;
  pid_t pid;
  int i, j, k, l, pr, gr, rb1, rb2, tt, num, first, success, output;
  int stars;
  int minrid, maxrid, prevrid, reordrid;
  int prevflag;
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

  struct palette pl[2];
  int palette = 1;

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
  pdata.unreachhops = 3;

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
              case 'a': sscanf(argv[++i], "%d", &palette); break;
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
              case 'u': sscanf(argv[++i], "%d", &pdata.unreachhops); break;
              case 'e':
                while (argv[i+1][0] >= '0' && strlen(argv[i+1]) < 5 && argv[i+1][0] <= '9')
                  sscanf(argv[++i], "%d", &udata.ttype[udata.numtt++]);
                break;
              case 'o': output = 1; break;
            }
        }
      else
        if (first <= 1)
          first = i;
      if (palette > 1)
        palette = 1;
    }
  if ((!first && strlen(file) == 0) || argc == 1)
    {
      printf("Usage: %s [-a colorpalette] [-m modulus offset] [-p <process>] [-v <verbosity>] [-i <interface>] [-t <start ttl>] [-s <size>] [-c <packetcount>] [-e <test type(s)>] [-u unreachable-timeout] [-o] [-f] <destination(s) (file)>\n", argv[0]);
      exit(1);
    }

  pl[0].inord = " %i ";
  pl[0].ord1 = " %i ";
  pl[0].ord1b = "";
  pl[0].ord2 = " %i ";
  pl[0].ord2b = "";
  pl[0].star = " * ";
  pl[0].excl = " ! ";
  pl[0].space = "   ";
  pl[0].inconret = " %i ";
  pl[0].inordret = " %i ";
  pl[0].outordret = " %i ";
  pl[0].reset = "";

  pl[1].inord = " \033[39m%i ";
  pl[1].ord1 = " \033[31m%i ";
  pl[1].ord1b = "\033[31m";
  pl[1].ord2 = " \033[34m%i ";
  pl[1].ord2b = "\033[34m";
  pl[1].star = " \033[39m* ";
  pl[1].excl = " \033[39m! ";
  pl[1].space = "   ";
  pl[1].inconret = " \033[34m%i ";
  pl[1].inordret = " \033[35m%i ";
  pl[1].outordret = " \033[36m%i ";
  pl[1].reset = "\033[39m";

  if (pdata.n > 10)
    {
      pl[0].inord = " %2i ";
      pl[0].ord1 = " %2i ";
      pl[0].ord2 = " %2i ";
      pl[0].star = "  * ";
      pl[0].excl = "  ! ";
      pl[0].space = "    ";
      pl[0].inconret = " %2i ";
      pl[0].inordret = " %2i ";
      pl[0].outordret = " %2i ";

      pl[1].inord = " \033[39m%2i ";
      pl[1].ord1 = " \033[31m%2i ";
      pl[1].ord2 = " \033[34m%2i ";
      pl[1].star = " \033[39m * ";
      pl[1].excl = " \033[39m ! ";
      pl[1].space = "    ";
      pl[1].inconret = " \033[34m%2i ";
      pl[1].inordret = " \033[35m%2i ";
      pl[1].outordret = " \033[36m%2i ";
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

  for (tt = 0; tt < udata.numtt; tt++)
    {
      if ((udata.ttype[tt] & (TESTPROTO)) == TESTUDP)
        {
          if (verbosity > 5)
            {
              printf("Test %i: testing with UDP packets", tt);
              if (udata.ttype[tt] & TESTNOSUM)
                printf(", no checksum");
            }
        }
      else if ((udata.ttype[tt] & (TESTPROTO | TESTNOSUM)) == TESTTCP)
        {
          if (verbosity > 5)
            printf("Testing with TCP packets");
        }
      else if ((udata.ttype[tt] & (TESTPROTO | TESTNOSUM)) == TESTICMP)
        {
          if (verbosity > 5)
            printf("Testing with ICMP packets");
        }
      else
        {
          printf("Unsupported combination of test options selected. Supported options are:\n\n");
          printf("UDP: %i\n", TESTUDP);
          printf("TCP: %i\n", TESTTCP);
          printf("ICMP: %i\n", TESTICMP);
          printf("No UDP checksum: %i\n", TESTNOSUM);
          printf("Vary TCP/UDP source port / ICMP seq (+1): %i\n", TESTSPORT);
          printf("Vary TCP/UDP destination port / ICMP id (-1): %i\n", TESTDPORTM1);
          printf("Vary TCP/UDP destination port / ICMP id (+6): %i\n", TESTDPORT6);
          printf("Vary data: %i\n", TESTDATA);
          printf("ToS: bits 8 - 15, multiply ToS by 256 or DSCP by 1024\n");
          exit(1);
        }

      if (pdata.size < 28 || ((udata.ttype[tt] & TESTTCP) && pdata.size < 40))
        {
          printf("Packet size too small (must be >= 28, >= 42 for TCP, to allow modifying data)\n");
          exit(1);
        }

      if (verbosity > 5)
        {
          if (udata.ttype[tt] & TESTSPORT)
            printf(", source port +1");
          if (udata.ttype[tt] & TESTDPORT6)
            printf(", destination port +6");
          if (udata.ttype[tt] & TESTDPORTM1)
            printf(", destination port -1");
          if (udata.ttype[tt] & TESTDATA)
            printf(", varying data");
          printf("\n");
        }
    }

  gettimeofday(&ts, NULL);
  udata.startts = ts.tv_sec;

  if (output)
    {
      sprintf(file, "racetroute.%i.%i.csv", pdata.process, udata.startts);
      udata.ofile = fopen(file, "w");
      if (!udata.ofile)
        {
          printf("couldn't open output file!\n");
          exit(1);
        }
      fprintf(udata.ofile, "inout,target,testtype,timestamp,ihl,tos,iplen,idtag,id,offset,ttl,proto,hdrsum,src,dest,icmptype,icmpcode,icmpsum,icmpreserved,oihl,otos,oiplen,oidtag,oid,ooffset,ottl,oproto,ohdrsum,osrc,odest,sport,dport,udplen,udpsum\n");
    }

  // sock = socket(PF_INET, SOCK_RAW, pdata.proto);
  // sock = socket(PF_INET, SOCK_RAW, 0);

  sock = socket(PF_INET, SOCK_RAW, 1);
  if (sock < 0)
    {
      printf("could not create socket (you need to be root!)\n");
      exit(1);
    }
  int hincl = 1;                  /* 1 = on, 0 = off */
  setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &hincl, sizeof(hincl));

  getinterface(ifin, ifout, &pdata.saddr);

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

          if (pdata.noname || desthe)
            {
/*
              if (pdata.noname)
                udata.destaddr = &a2;
              else
*/
                udata.destaddr = (struct in_addr *)desthe->h_addr_list[0];

              for (tt = 0; tt < udata.numtt; tt++)
                {
                  udata.testtype = udata.ttype[tt];
                  done = pdata.unreachhops;
                  previous.addr = -1;
                  pdata.rhop = 0;
                  pdata.ttl = pdata.sttl;

                  if (verbosity > 5)
                    printf("Sending %i %i-byte packets with initial TTL %i to %s; interface %s\n", pdata.n, pdata.size, pdata.ttl, dest, ifout);

                  do
                    {
                      udata.sport = sport;
                      udata.dport = dport;
                      udata.id = pdata.startid;
                      udata.hwater = 0;
                      udata.oo = 0;
                      udata.numoo = 0;
                      udata.numidoo = 0;
                      udata.idhwater = 0;
                      udata.n = 0;
                      udata.lo = 0;

                      pdata.tos = udata.testtype >> 8;

                      fcntl(sock, F_SETFL, 0);

                      for (i = 0; i < pdata.n; i++)
                        {
                          pdata.status[i] = 0;
                          if (udata.testtype & TESTSPORT)
                            pdata.sport[i] = sport + i;
                          else
                            pdata.sport[i] = sport;
                          if (udata.testtype & TESTDPORTM1)
                            pdata.dport[i] = dport - i;
                          else if (udata.testtype & TESTDPORT6)
                            pdata.dport[i] = dport + i * 6;
                          else
                            pdata.dport[i] = dport;
                          pdata.id[i] = pdata.startid + i;
                        }

                      if (udata.testtype & TESTUDP)
                        sendpacketudp(sock, udata.destaddr, &pdata, &udata);
                      if (udata.testtype & TESTTCP)
                        sendpackettcp(sock, udata.destaddr, &pdata, &udata);
                      if (udata.testtype & TESTICMP)
                        sendpacketicmp(sock, udata.destaddr, &pdata, &udata);

                      fcntl(sock, F_SETFL, O_NONBLOCK);

                      timeout.tv_sec = 1;
                      timeout.tv_usec = 0;

                      gettimeofday(&timeout2, NULL);
                      timeout2.tv_sec += 2;
                      timeout2.tv_usec = 0;

                      do
                        {
                          i = recvmsg(sock, &message, 0);
                          if (i > 0)
                            printpacket((u_char *)&udata, packet, i);

                          if (udata.n >= pdata.n)
                            break;

                          FD_ZERO(&fds);
                          FD_SET(sock, &fds);
                          success = select(sock + 1, &fds, NULL, NULL, &timeout);

                          gettimeofday(&timeout3, NULL);
                        }
                      while (success && timeout3.tv_sec < timeout2.tv_sec);

                      if (timeout3.tv_sec >= timeout2.tv_sec || (udata.n > 0 && previous.addr == udata.remoteaddr[0].addr))
                        done--;

                      previous.addr = udata.remoteaddr[0].addr;

                      num = udata.n;
                      for (i = 0; i < udata.n - 1; i++)
                        {
                          udata.num[i] = 1;
                          for (j = i + 1; j < udata.n; j++)
                            if (udata.remoteaddr[j].addr && udata.remoteaddr[j].addr == udata.remoteaddr[i].addr)
                              {
                                num--;
                                udata.num[i]++;
                                if (verbosity > 2)
                                  udata.remoteaddr[j].addr = 0;
                              }
                        }
                      udata.num[udata.n] = 1;

                      if (udata.numoo && !pdata.rhop)
                        pdata.rhop = pdata.ttl - 1;



                      if (verbosity == 1 || verbosity == 2)
                        {

                          // group return packets based on source IP address
                          udata.group[0] = 0;
                          for (i = 0; i < udata.n; i++)
                            {
                              udata.group[i] = i;
                              for (j = 0; j < i; j++)
                                {
                                  if (udata.remoteaddr[j].addr == udata.remoteaddr[i].addr)
                                    {
                                      udata.group[i] = j;
                                      break;
                                    }
                                }
                            }

                          // go through all groups
                          // for (gr = 0; gr < udata.n; gr++)
                          stars = 1;
                          gr = 0;
                          do
                            {
                              // if this group exists, do output
                              if (udata.group[gr] == gr)
                                {
                                  // print hop number
                                  if (gr == 0)
                                    printf("%2i: ", pdata.ttl);
                                  else
                                    printf("    ");

                                  // check order and print packet number
                                  for (i = 0; i < pdata.n; i++)
                                    {
                                      pr = 1;
                                      for (j = 0; j < udata.n; j++)
                                        {
                                          if (udata.recvid[i] == udata.id + j)
                                            {
                                              pr = 0;
                                              if (udata.group[i] == gr)
                                                {
                                                  rb1 = 0;
                                                  rb2 = 0;
                                                  for (l = 0; l < i; l++)
                                                    if (udata.recvid[l] > udata.id + j)
                                                      {
                                                        if (udata.group[l] == gr)
                                                          rb1 = 1;
                                                        else
                                                          rb2 = 1;
                                                      }
                                                  if (rb1)
                                                    printf(pl[palette].ord1, j);
                                                  else if (rb2)
                                                    printf(pl[palette].ord2, j);
                                                  else
                                                    printf(pl[palette].inord, j);
                                                  prevflag = 1;
                                                }
                                              else
                                                printf("   ");
                                            }
                                        }
                                      if (pr)
                                        {
                                          if (stars > 0)
                                            {
		                              if (udata.icmptype == 11)
                                                printf("%s", pl[palette].star);
		                              else if (udata.icmptype == 3)
                                                printf("%s", pl[palette].excl);
                                              else
                                                printf("%s", pl[palette].space);
                                              stars++;
                                            }
                                          else
                                            printf("%s", pl[palette].space);
                                        }
                                    }
                                  if (udata.n != 0)
                                    {
                                      if (gr == 0 && udata.numoo != 0)
                                        printf("%s ", pl[palette].ord1b);
                                      else if (gr == 0)
                                        printf("%s ", pl[palette].reset);
                                      else
                                        printf("%s ", pl[palette].ord2b);
                                      printf("%i.%i.%i.%i", udata.remoteaddr[gr].byte[0], udata.remoteaddr[gr].byte[1], udata.remoteaddr[gr].byte[2], udata.remoteaddr[gr].byte[3]);
                                    }
                                  printf("%s", pl[palette].reset);
                                  // print return ID information
                                  if (udata.n > 0 && verbosity == 2)
                                    {
                                      minrid = 65535;
                                      maxrid = 0;
                                      prevrid = 0;
                                      reordrid = 0;
                                      for (i = 0; i < udata.n; i++)
                                        if (gr == udata.group[i])
                                          {
                                            if (udata.returnid[i] < minrid)
                                              minrid = udata.returnid[i];
                                            if (udata.returnid[i] > maxrid)
                                              maxrid = udata.returnid[i];
                                            if (udata.returnid[i] < prevrid)
                                              reordrid = 1;
                                            prevrid = udata.returnid[i];
                                          }
                                        if (maxrid == 0)
                                          printf("  ret-IDs: 0");
                                        else if (maxrid - minrid < 100 && reordrid == 0)
                                          printf("  ret-IDs: %i-%i (in-order)", minrid, maxrid);
                                        else if (maxrid - minrid < 100 && reordrid != 0)
                                          printf("  ret-IDs: %i-%i (reordered)", minrid, maxrid);
                                        else
                                          printf("  ret-IDs: %i-%i", minrid, maxrid);
                                      }
                                  printf("\n");
                                }
                              if (stars > 1)
                                stars = 0;
                              gr++;
                            }
                          while (gr < udata.n);
                        }





                      if (verbosity > 5)
                        {
                          if (udata.n == 0)
                            printf("hop %i: 0 out of %i packets received\n", pdata.ttl, pdata.n);
                          else if (num == 1)
                            printf("hop %i: %i of %i packets received from single IP address %i.%i.%i.%i\n", pdata.ttl, udata.n, pdata.n, udata.remoteaddr[0].byte[0], udata.remoteaddr[0].byte[1], udata.remoteaddr[0].byte[2], udata.remoteaddr[0].byte[3]);
                          else
                            {
                              printf("hop %i: %i out of %i packets received from %i different IP addresses\n", pdata.ttl, udata.n, pdata.n, num);
                              if (verbosity > 6)
                                {
                                  printf("\n");
                                  for (i = 0; i < udata.n; i++)
                                    if (udata.remoteaddr[i].addr)
                                      printf("hop %i: %i packets from %i.%i.%i.%i\n", pdata.ttl, udata.num[i], udata.remoteaddr[i].byte[0], udata.remoteaddr[i].byte[1], udata.remoteaddr[i].byte[2], udata.remoteaddr[i].byte[3]);
                                  printf("\n");
                                }
                            }
                        }

                      if (verbosity > 4 && udata.n != 0 && udata.numoo != 0)
                        printf("hop %i: %i packets out of %i reordered, average distance from expected order %1.1f\n", pdata.ttl, udata.numoo, udata.n, -udata.oo / (float)udata.n);
                      if (verbosity > 5 && udata.n != 0 && udata.numidoo > 1)
                        printf("ID value in ICMP messages out of order %i times\n", udata.numidoo);

                      if (udata.n <= 1)
                        done--;
		      else if (udata.icmptype == 3)
		        {
			if (done > 1)
			  done = 1;
			else
			  done = 0;
		        }
                      else if (udata.n > 3)
                        done = pdata.unreachhops;
                      pdata.ttl++;
                      if (pdata.ttl > MPMAXTTL)
                        done = -1;

                      pdata.startid += pdata.n;
                      if (pdata.startid > pdata.process * 500 + 2400)
                        pdata.startid = pdata.process * 500 + 2000;
                    }
                  while (done > 0);
                  // while (done > 0 && udata.numoo == 0);

                  if (verbosity > 0)
                    {
                      if (pdata.rhop > 0)
                        {
                          if (num > 1)
                            printf("Test %i: found multiaddress reordering at hop %i for %s\n", udata.testtype, pdata.rhop + 1, pdata.target);
                          else
                            printf("Test %i: found reordering at hop %i for %s\n", udata.testtype, pdata.rhop + 1, pdata.target);
                        }
                      else if (done < 0)
                        printf("Test %i: reached destination at hop %i for %s\n", udata.testtype, pdata.ttl - 1, pdata.target);
                      else
                        printf("Test %i: time out after hop %i for %s\n", udata.testtype, pdata.ttl - pdata.unreachhops - 1, pdata.target);
                    }
                }
            }
        }
      else
        {
          udata.lo = 1;
          fcntl(sock, F_SETFL, 0);
          while (1)
            {
              i = recvmsg(sock, &message, 0);
              if (i > 0)
                printpacket((u_char *)&udata, packet, i);
            }
        }
    }

  if (output)
    fclose(udata.ofile);
  if (f)
    fclose(f);
  close(sock);
}


#include "racetroute.h"

void sendpacketudp(int sock, struct in_addr *destaddr, struct pdata *pdata, struct udata *udata)
{
  unsigned char packet[MAXPACKET], printpacket[MAXPACKET];
  struct ip *ipheader = (struct ip *)packet;
  struct udphdr *udpheader = (struct udphdr *)(packet + sizeof(struct ip)); 
  unsigned int packetlen;
  char *payload = "iljitsch.com/racetroute";
  // struct sockaddr destsa;
  struct sockaddr_in destsa;
  int i, x, checksum, ret;
  unsigned short *chp, ii;

  bzero(packet, MAXPACKET);

  // destsa.sa_family = AF_INET;
  destsa.sin_family = AF_INET;
  // memcpy(&destsa.sa_data, destaddr, 4);
  memcpy(&destsa.sin_addr, destaddr, 4);
  i = 0;
  memcpy(&destsa.sin_port, &i, 2);

  packetlen = sizeof(struct ip);
  ipheader->ip_hl = 5;
  ipheader->ip_v = 4;
  ipheader->ip_tos = pdata->tos;
  // ipheader->ip_id = htons(ipid);
  ipheader->ip_ttl = pdata->ttl;
  ipheader->ip_p = pdata->proto;
  ipheader->ip_off = 0; /* host byte order! */
  ipheader->ip_src.s_addr = pdata->saddr;
  ipheader->ip_dst = *destaddr;

  packetlen += sizeof(struct udphdr);

  memcpy(packet + packetlen, payload, strlen(payload));
  // packetlen += strlen(payload) + PADDING;
  packetlen = pdata->size;

  ipheader->ip_len = packetlen; /* host byte order! */
  udpheader->len = htons(packetlen - sizeof(struct ip));

  for (i = 0; i < pdata->n; i++)
    {

/*
if (i & 1)
  packetlen = 46;
else
  packetlen = pdata->size;
ipheader->ip_len = packetlen;   // host byte order!
udpheader->len = htons(packetlen - sizeof(struct ip));
*/

      ipheader->ip_id = htons((unsigned short)pdata->id[i]);
      udpheader->sport = htons((unsigned short)pdata->sport[i]);
      udpheader->dport = htons((unsigned short)pdata->dport[i]);

      if (udata->testtype & TESTDATA)
        {
          ii = ntohs((unsigned short)i);
          memcpy(packet + packetlen - 2, &ii, 2);
        }

      udpheader->sum = 0;

      if (!(udata->testtype & TESTNOSUM))
        {
          checksum = 17;
          checksum += packetlen - sizeof(struct ip);
          checksum += ntohs(ipheader->ip_src.s_addr & 0xffff) + ntohs(ipheader->ip_src.s_addr >> 16);
          checksum += ntohs(ipheader->ip_dst.s_addr & 0xffff) + ntohs(ipheader->ip_dst.s_addr >> 16);
          for (chp = (void *)udpheader; chp < (unsigned short *)(((void *)udpheader) + packetlen - sizeof(struct ip) - 1); chp++)
            checksum += ntohs(*chp);
          if (packetlen & 1)
            checksum += ntohs(*chp) & 0xff00;
          checksum = (checksum & 0xffff) + (checksum >> 16);
          checksum = (checksum & 0xffff) + (checksum >> 16);
          udpheader->sum = htons(0xffff - checksum);
        }

      if (i == 0)
        memcpy(printpacket, packet, packetlen);

      ret = sendto(sock, packet, packetlen, 0, (struct sockaddr *)&destsa, sizeof(struct sockaddr_in));
      if (ret <= 0 && verbosity > 1)
        {
          printf("sendto failed, errno = %i\n", errno);
          // exit(1);
        }
    }

  printsent(printpacket, packetlen, udata);
}

#include "racetroute.h"

void sendpackettcp(int sock, struct in_addr *destaddr, struct pdata *pdata, struct udata *udata)
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
  char tcppkt[] = { 0xf3, 0xed, 0x00, 0x50, 0x00, 0xe3, 0x6b, 0xbe, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x02, 0xff, 0xff, 0xa4, 0xce, 0x00, 0x00, 0x02, 0x04, 0x05, 0xb4, 0x01, 0x03, 0x03, 0x03, 0x01, 0x01, 0x08, 0x0a, 0x3b, 0x86, 0xf2, 0x77, 0x00, 0x00, 0x00, 0x00, 0x04, 0x02, 0x00, 0x00 };
  struct tcphdr *tcphdr;

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
  ipheader->ip_p = 6; // TCP
  ipheader->ip_off = 0; /* host byte order! */
  ipheader->ip_src.s_addr = pdata->saddr;
  ipheader->ip_dst = *destaddr;

  tcphdr = (void *)ipheader + sizeof(struct ip);
  memcpy(tcphdr, tcppkt, 44);

  packetlen += sizeof(struct ip) + 44;

  ipheader->ip_len = packetlen; /* host byte order! */
  // tcphdr->len = htons(packetlen - sizeof(struct ip));

  for (i = 0; i < pdata->n; i++)
    {
      ipheader->ip_id = htons((unsigned short)pdata->id[i]);
      tcphdr->sport = htons((unsigned short)pdata->sport[i]);
      tcphdr->dport = htons((unsigned short)pdata->dport[i]);

      if (udata->testtype & TESTDATA)
        {
          ii = ntohs((unsigned short)i);
          memcpy(packet + packetlen - 2, &ii, 2);
        }

      tcphdr->sum = 0;

      checksum = 6;
      checksum += packetlen - sizeof(struct ip);
      checksum += ntohs(ipheader->ip_src.s_addr & 0xffff) + ntohs(ipheader->ip_src.s_addr >> 16);
      checksum += ntohs(ipheader->ip_dst.s_addr & 0xffff) + ntohs(ipheader->ip_dst.s_addr >> 16);
      for (chp = (void *)tcphdr; chp < (unsigned short *)(((void *)tcphdr) + packetlen - sizeof(struct ip) - 1); chp++)
        checksum += ntohs(*chp);
      if (packetlen & 1)
        checksum += ntohs(*chp) & 0xff00;
      checksum = (checksum & 0xffff) + (checksum >> 16);
      checksum = (checksum & 0xffff) + (checksum >> 16);
      tcphdr->sum = htons(0xffff - checksum);

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

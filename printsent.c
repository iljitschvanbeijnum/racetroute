#include "racetroute.h"

void printsent(unsigned char *bytes, int packetlen, struct udata *udata)
{
  struct ip *ip;
  struct udphdr *udp;
  struct tcphdr *tcp;
  struct icmp *icmp;
  int sport = 0;
  int dport = 0;
  int sum = 0;
  int len = 0;
  int id = -1;
  struct timeval ts;
  int timestamp;
  union ipaddr s, d;

  if (!udata->ofile)
    return;

  ip = (struct ip *)bytes;

  gettimeofday(&ts, NULL);
  timestamp = ts.tv_usec + (ts.tv_sec - udata->startts) * 1000000;

  if (ip->ip_p == 1)
    {
      icmp = (struct icmp *)(((u_char *)ip) + 4 * ip->ip_hl);
      if (icmp->icmp_type == 0)
        {
          sport = ntohs(icmp->icmp_hun.ih_idseq.icd_seq);
          dport = ntohs(icmp->icmp_hun.ih_idseq.icd_id);
          sum = ntohs(icmp->icmp_cksum);
        }
    }
  else if (ip->ip_p == 17)
    {
      udp = (struct udphdr *)(((u_char *)ip) + 4 * ip->ip_hl);
      sport = ntohs(udp->sport);
      dport = ntohs(udp->dport);
      sum = ntohs(udp->sum);
    }
  else if (ip->ip_p == 6)
    {
      tcp = (struct tcphdr *)(((u_char *)ip) + 4 * ip->ip_hl);
      sport = ntohs(tcp->sport);
      dport = ntohs(tcp->dport);
      sum = ntohs(tcp->sum);
    }

// "inout,target,testtype,timestamp,ihl,tos,iplen,idtag,id,offset,ttl,proto,hdrsum,src,dest,icmptype,icmpcode,icmpsum,icmpreserved,oihl,otos,oiplen,oidtag,oid,ooffset,ottl,oproto,ohdrsum,"osrc,odest,sport,dport,udplen,udpsum\n");

  fprintf(udata->ofile, "out,%s,%i,%i,0,0,0,id,-1,0,0,0,0,0.0.0.0,0.0.0.0,0,0,0,0,", udata->target, udata->testtype, timestamp);

  fprintf(udata->ofile, "%i,%i,%i,oid,%i,%i,%i,%i,%i,", ip->ip_hl, ip->ip_tos, ip->ip_len, ntohs(ip->ip_id), ntohs(ip->ip_off), ip->ip_ttl, ip->ip_p, ntohs(ip->ip_sum));

  s.addr = ip->ip_src.s_addr;
  d.addr = ip->ip_dst.s_addr;
  fprintf(udata->ofile, "%i.%i.%i.%i,%i.%i.%i.%i,", s.byte[0], s.byte[1], s.byte[2], s.byte[3], d.byte[0], d.byte[1], d.byte[2], d.byte[3]);

  fprintf(udata->ofile, "%i,%i,%i,%i\n", sport, dport, len, sum);
}

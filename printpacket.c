#include "racetroute.h"

void printpacket(u_char *user, const u_char *bytes, int len)
{
  struct ip *ip, *oip;
  struct udphdr *oudp;
  struct tcphdr *otcp;
  struct icmp *icmp;
  int sport = 0;
  int dport = 0;
  int sum = 0;
  int id = -1;
  int vp = 0;
  struct udata *udata;
  struct timeval ts;
  int timestamp;
  union ipaddr s, d;

  ip = (struct ip *)bytes;
  udata = (struct udata *)user;

  gettimeofday(&ts, NULL);
  timestamp = ts.tv_usec + (ts.tv_sec - udata->startts) * 1000000;

  udata->icmptype = 0;

  // handle ICMP
  if (ip->ip_p == 1)
    {
      icmp = (struct icmp *)(((u_char *)ip) + 4 * ip->ip_hl);
      udata->icmptype = icmp->icmp_type;
      if ((icmp->icmp_type == 3 || icmp->icmp_type == 11) && ntohs(ip->ip_len) >= 56)
        {
          oip = (struct ip *)(((u_char *)icmp) + 8);
          udata->returnid[udata->n] = ntohs(ip->ip_id);
          if (!udata->lo)
            id = udata->recvid[udata->n] = ntohs(oip->ip_id);
          if (oip->ip_p == 17)
            {
              oudp = (struct udphdr *)(((u_char *)oip) + 4 * oip->ip_hl);
              sport = ntohs(oudp->sport);
              dport = ntohs(oudp->dport);
              sum = ntohs(oudp->sum);
	    }
          else if (oip->ip_p == 6)
            {
              otcp = (struct tcphdr *)(((u_char *)oip) + 4 * oip->ip_hl);
              sport = ntohs(otcp->sport);
              dport = ntohs(otcp->dport);
              sum = ntohs(otcp->sum);
	    }
          if ((id >= udata->id && id < udata->id + NUMPACKET))
            vp = 1;
        }
      else
        oip = NULL;

      if (icmp->icmp_type == 0)
        {
          sport = ntohs(icmp->icmp_hun.ih_idseq.icd_seq);
          dport = ntohs(icmp->icmp_hun.ih_idseq.icd_id);
          sum = ntohs(icmp->icmp_cksum);
          if (ip->ip_src.s_addr == udata->destaddr->s_addr)
            {
              vp = 1;
              done = -1;
            }
        }

      // if ((icmp->icmp_type == 0 && ip->ip_src.s_addr == udata->destaddr->s_addr) || (icmp->icmp_type == 3 && ((id >= udata->id && id < udata->id + NUMPACKET))))
      //   done = -1;

      if (!vp)
        return;


// "inout,target,testtype,timestamp,ihl,tos,iplen,idtag,id,offset,ttl,proto,hdrsum,src,dest,icmptype,icmpcode,icmpsum,icmpreserved,oihl,otos,oiplen,oidtag,oid,ooffset,ottl,oproto,ohdrsum,"osrc,odest,sport,dport,udplen,udpsum\n");




  // if (udata->ofile && (udata->lo || (icmp->icmp_type == 0 || (icmp->icmp_type == 3 || icmp->icmp_type == 11) && ((id >= udata->id && id < udata->id + NUMPACKET) || ip->ip_src.s_addr == udata->destaddr->s_addr))))

  if (udata->ofile && (udata->lo || ((icmp->icmp_type == 0 || icmp->icmp_type == 3) && ip->ip_src.s_addr == udata->destaddr->s_addr) || (icmp->icmp_type == 11 && ((id >= udata->id && id < udata->id + NUMPACKET)))))
    {
      if (icmp->icmp_type == 0 || icmp->icmp_type == 3)
        done = -1;

// if (icmp->icmp_type == 0)
// fprintf(stderr, "ip: %i, udata: %i, id: %i, udata: %i\n", ip->ip_src.s_addr, udata->destaddr->s_addr, id, udata->id);
      fprintf(udata->ofile, "in,%s,%i,%i,%i,%i,%i,id,%i,%i,%i,%i,%i,", udata->target, udata->testtype, timestamp, ip->ip_hl, ip->ip_tos, ip->ip_len, ntohs(ip->ip_id), ntohs(ip->ip_off), ip->ip_ttl, ip->ip_p, ntohs(ip->ip_sum));

      s.addr = ip->ip_src.s_addr;
      d.addr = ip->ip_dst.s_addr;
      fprintf(udata->ofile, "%i.%i.%i.%i,%i.%i.%i.%i,", s.byte[0], s.byte[1], s.byte[2], s.byte[3], d.byte[0], d.byte[1], d.byte[2], d.byte[3]);

      fprintf(udata->ofile, "%i,%i,%i,%u,", icmp->icmp_type, icmp->icmp_code, ntohs(icmp->icmp_cksum), (unsigned int)ntohl(icmp->icmp_hun.ih_void));

      if (oip)
        {
          fprintf(udata->ofile, "%i,%i,%i,oid,%i,%i,%i,%i,%i,", oip->ip_hl, oip->ip_tos, oip->ip_len, ntohs(oip->ip_id), ntohs(oip->ip_off), oip->ip_ttl, oip->ip_p, ntohs(oip->ip_sum));
          s.addr = oip->ip_src.s_addr;
          d.addr = oip->ip_dst.s_addr;
          fprintf(udata->ofile, "%i.%i.%i.%i,%i.%i.%i.%i,", s.byte[0], s.byte[1], s.byte[2], s.byte[3], d.byte[0], d.byte[1], d.byte[2], d.byte[3]);
          fprintf(udata->ofile, "%i,%i,%i,%i\n", sport, dport, len, sum);
        }
      else
        {
          if (icmp->icmp_type == 0)
            fprintf(udata->ofile, "0,0,0,oid,-1,0,0,0,0,0.0.0.0,0.0.0.0,%i,%i,-1,-1\n", sport, dport);
          else
            fprintf(udata->ofile, "0,0,0,oid,-1,0,0,0,0,0.0.0.0,0.0.0.0,-1,-1,-1,-1\n");
	}
    }

  if (!udata->lo)
    {
      if (udata->hwater == 0)
        {
          udata->hwater = id - 1;
          // udata->firstts = ts.tv_usec;
        }

      udata->remoteaddr[udata->n].byte[0] = bytes[12];
      udata->remoteaddr[udata->n].byte[1] = bytes[13];
      udata->remoteaddr[udata->n].byte[2] = bytes[14];
      udata->remoteaddr[udata->n].byte[3] = bytes[15];
// printf("remoteaddr[%i]\n", udata->n);
      // udata->remoteaddr2[udata->n].addr = udata->remoteaddr[udata->n].addr;
      udata->n++;
    }

      if (verbosity > 6)
        {
   //       if (icmp->icmp_type == 0)
    //        printf("ICMP type %i ID %i from %i.%i.%i.%i\n", icmp->icmp_type, ntohs(ip->ip_id), bytes[12], bytes[13], bytes[14], bytes[15]);
     //     else
            printf("ICMP type %i ID %i from %i.%i.%i.%i for probe with ID %i (%i us)", icmp->icmp_type, ntohs(ip->ip_id), bytes[12], bytes[13], bytes[14], bytes[15], id, (unsigned int)(ts.tv_usec + (ts.tv_sec - udata->startts) * 1000000));
        }
            // else if (verbosity > 7)
              // printf("ICMP type %i from %i.%i.%i.%i with src, dst ports: %i, %i (%i us)", icmp->icmp_type, bytes[12], bytes[13], bytes[14], bytes[15], sport, dport, ts.tv_usec + (ts.tv_sec - udata->startts) * 1000000);
      if (!udata->lo && icmp->icmp_type != 0)
        {
          if (id < udata->hwater)
            {
              if (verbosity > 6)
                printf(" %i\n", id - udata->hwater);
              udata->oo += id - udata->hwater;
              udata->numoo++;
            }
          else
            {
           //   if (udata->hwater > 0 && id != udata->hwater + 1)
           //     puts("lost packet");
              udata->hwater = id;
              if (verbosity > 6)
                printf("\n");
            }
          if (ntohs(ip->ip_id) > udata->idhwater || (ntohs(ip->ip_id) < 20000 && udata->idhwater > 45000))
            udata->idhwater = ntohs(ip->ip_id);
          else if (ntohs(ip->ip_id) < udata->idhwater)
            udata->numidoo++;
        }
      else
        {
          if (verbosity > 6)
            printf("\n");
        }
    }

  // handle UDP
  if (udata->lo && ip->ip_p == 17)
    {
      oudp = (struct udphdr *)(((u_char *)ip) + 4 * ip->ip_hl);
      fprintf(udata->ofile, "in,%s,%i,%i,%i,%i,%i,id,%i,%i,%i,%i,%i,", udata->target, udata->testtype, timestamp, ip->ip_hl, ip->ip_tos, ip->ip_len, ntohs(ip->ip_id), ntohs(ip->ip_off), ip->ip_ttl, ip->ip_p, ntohs(ip->ip_sum));

      s.addr = ip->ip_src.s_addr;
      d.addr = ip->ip_dst.s_addr;
      fprintf(udata->ofile, "%i.%i.%i.%i,%i.%i.%i.%i,", s.byte[0], s.byte[1], s.byte[2], s.byte[3], d.byte[0], d.byte[1], d.byte[2], d.byte[3]);

      fprintf(udata->ofile, "0,0,0,0,0,0,0,oid,0,0,0,0,0,0.0.0.0,0.0.0.0,");
      oudp = (struct udphdr *)(((u_char *)oip) + 4 * oip->ip_hl);
      sport = ntohs(oudp->sport);
      dport = ntohs(oudp->dport);
      sum = ntohs(oudp->sum);
      fprintf(udata->ofile, "%i,%i,%i,%i\n", sport, dport, len, sum);
    }

  // handle TCP
  if (udata->lo && ip->ip_p == 6)
    {
      // oudp = (struct udphdr *)(((u_char *)ip) + 4 * ip->ip_hl);
      fprintf(udata->ofile, "in,%s,%i,%i,%i,%i,%i,id,%i,%i,%i,%i,%i,", udata->target, udata->testtype, timestamp, ip->ip_hl, ip->ip_tos, ip->ip_len, ntohs(ip->ip_id), ntohs(ip->ip_off), ip->ip_ttl, ip->ip_p, ntohs(ip->ip_sum));

      s.addr = ip->ip_src.s_addr;
      d.addr = ip->ip_dst.s_addr;
      fprintf(udata->ofile, "%i.%i.%i.%i,%i.%i.%i.%i,", s.byte[0], s.byte[1], s.byte[2], s.byte[3], d.byte[0], d.byte[1], d.byte[2], d.byte[3]);

      fprintf(udata->ofile, "0,0,0,0,0,0,0,oid,0,0,0,0,0,0.0.0.0,0.0.0.0,");
      otcp = (struct tcphdr *)(((u_char *)oip) + 4 * oip->ip_hl);
      sport = ntohs(otcp->sport);
      dport = ntohs(otcp->dport);
      sum = ntohs(otcp->sum);
      fprintf(udata->ofile, "%i,%i,%i,%i\n", sport, dport, len, sum);
    }
}

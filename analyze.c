#include "racetroute.h"

struct racetroute
  {
    char inout[10];
    char target[40];
    int testtype;
    int timestamp;
    int ihl;
    int tos;
    int iplen;
    int id;
    int offset;
    int ttl;
    int proto;
    int hdrsum;
    char src[20];
    char dest[20];
    int icmptype;
    int icmpcode;
    int icmpsum;
    int icmpreserved;
    int oihl;
    int otos;
    int oiplen;
    int oid;
    int ooffset;
    int ottl;
    int oproto;
    int ohdrsum;
    char osrc[20];
    char odest[20];
    int sport;
    int dport;
    int udplen;
    int udpsum;
  };

void readline(char *line, struct racetroute *r)
{
    char buf1[10], buf2[10];
    int i;

    i = 0;
    while (line[i])
      {
        if (line[i] == ',')
          line[i] = ' ';
        i++;
      }

    sscanf(line, "%s %s %i %i %i %i %i %s %i %i %i %i %i %s %s %i %i %i %i %i %i %i %s %i %i %i %i %i %s %s %i %i %i %i", r->inout, r->target, &r->testtype, &r->timestamp, &r->ihl, &r->tos, &r->iplen, buf1, &r->id, &r->offset, &r->ttl, &r->proto, &r->hdrsum, r->src, r->dest, &r->icmptype, &r->icmpcode, &r->icmpsum, &r->icmpreserved, &r->oihl, &r->otos, &r->oiplen, buf2, &r->oid, &r->ooffset, &r->ottl, &r->oproto, &r->ohdrsum, r->osrc, r->odest, &r->sport, &r->dport, &r->udplen, &r->udpsum);
}

int main(int argc, char *argv[])
{
  struct racetroute tmp, out, in[MAXPACKET];
  char line[500], current[40];
  int f, i, j, n, m, hop, testtype = 0, oidoo, idoo, idt, idt0, idtsmall, idtlarge, idtype, id0, ma, oidooh, idooh, mah, dest, desth, unr, unrh;
  FILE *infile;

  if (argc < 2)
    {
      printf("Usage: %s <input file> [<more input files]\n", argv[0]);
      exit(1);
    }

  printf("testtype,target,reorderhop,multiaddresshop,idreorderhop,unreachablehop,destinationhop,idtype\n");

  for (f = 1; f < argc; f++)
    {
      infile = fopen(argv[f], "r");
      if (!infile)
        printf("couldn't open file %s!\n", argv[f]);
      else
        {
          fgets(line, 500, infile);
          current[0] = 0;
          
          while (!feof(infile))
            {
              while (!feof(infile) && (strlen(line) == 0 || strncmp(line, "out,", 4) != 0))
                fgets(line, 500, infile);
              readline(line, &tmp);

              if (strcmp(current, tmp.target) != 0 || testtype != tmp.testtype)
                {
                  if (current[0])
                    {
                      // results per destination
                      printf("%i,%s,%i,%i,%i,%i,%i,%i\n", testtype, current, oidooh, mah, idooh, unrh, desth, idtype);
                    }

                  strcpy(current, tmp.target);
                  testtype = tmp.testtype;
                  // printf("now doing %s\n", current);
                  m = 0;
                  id0 = 0;
                  idtype = -1;
                  idoo = 0;
                  idooh = 0;
                  oidoo = 0;
                  oidooh = 0;
                  ma = 0;
                  mah = 0;
                  dest = 0;
                  desth = 0;
		  unr = 0;
                  unrh = 0;
                }

            //   while (!feof(infile))
              // while (!feof(infile) && (strlen(line) == 0 || strncmp(line, "out,", 4) != 0))
                {
                  while (!feof(infile) && (strlen(line) == 0 || strncmp(line, "out,", 4) != 0))
                    fgets(line, 500, infile);
                  readline(line, &out);

                  hop = out.ottl;
                  n = 0;
                  if (!feof(infile) && (strlen(line) == -1 || strncmp(line, "out,", 4) == 0))
                    fgets(line, 500, infile);

                  do
                    {
                      if ((strlen(line) > 0 && strncmp(line, "in,", 3) == 0))
                        readline(line, &in[n++]);

                      fgets(line, 500, infile);
                    }
                  while (!feof(infile) && (strlen(line) == 0 || strncmp(line, "in,", 3) == 0));

                  // results per hop
                  id0 = 0;
                  // printf("out + %i in\n", n);
                  if (n == 0)
                    {
                      if (!unr)
                        {
                          unrh = hop;
                        }
                      unr = 1;
                      // printf("timeout at hop %i\n", hop);
                    }
                  else
                    {
                      unr = 0;
                      unrh = 0;
                      if (in[0].proto == 1 && in[0].icmptype == 3)
                        {
                          if (!dest)
                            desth = hop;
                          dest = 1;
                        }
                      if (n > 1)
                        {
                          for (i = 1; i < n; i++)
                            {
                              id0 += in[i].id;
                              if (in[i].proto == 1 && in[i].icmptype == 3)
                                {
                                  if (!dest)
                                    desth = hop;
                                  dest = 1;
                                }
                              if (in[i-1].id > in[i].id || (in[i-1].id < 10000 && in[i].id > 55000))
                                {
                                  if (!idoo)
                                    {
                                      idooh = hop;
                                      if (n > 1)
                                        {
                                          idt = 0;
                                          idtsmall = 0;
                                          idtlarge = 0;
                                          idt0 = in[0].id;
                                          for (j = 1; j < n; j++)
                                            {
                                              idt0 += in[j].id;
                                              idt = abs(in[j].id - in[j-1].id);
                                              if (idt > 32768)
                                                idt -= 32768;
                                              if (idt < 10)
                                                idtsmall++;
                                              else
                                                idtlarge++;
// printf("id: %i  \n", in[j].id);
                                            }
// printf("idt0: %i, n: %i\n", idt0, n);
                                          if (idt0 == 0)
                                            idtype = 0;
                                          if (idtsmall >= idtlarge)
                                            idtype = 1;
                                          else
                                            idtype = 10;
                                        }
                                    }
                                  idoo = 1;
                                }
                              if (in[i-1].proto == 1 && in[i].proto == 1 && in[i-1].icmptype != 0 && in[i].icmptype != 0)
                                {
                                  if (in[i-1].oid > in[i].oid)
                                    {
                                      if (!oidoo)
                                        oidooh = hop;
                                      oidoo = 1;
                                    }
                                }
                              if (strcmp(in[i-1].src, in[i].src) != 0)
                                {
                                  if (!ma)
                                    mah = hop;
                                  ma = 1;
                                }
                            }
/*
                          if (
                            
*/
                        }
                    }

/*
                  if (oidoo)
                    printf("reordering at hop %i\n", hop);
                  if (idoo)
                    printf("return path reordering at hop %i\n", hop);
*/
                }

            }
        }
    }
}

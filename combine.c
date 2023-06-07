#include "racetroute.h"

#define MAXRES 200000

struct res
  {
    char target[MAXRES][20];
    char data[MAXRES][3][20];
  };

int main(int argc, char *argv[])
{
  char line[500], *line2;
  int f, i, j, n, next, found;
  struct res res;
  FILE *infile;

  if (argc != 5)
    {
      printf("Usage: %s <type> <input file 1> <input file 2> <input file 3>\n", argv[0]);
      exit(1);
    }

  printf("target,test,reorderhop,multiaddresshop,idreorderhop,unreachablehop,destinationhop\n");

  n = 0;
  next = 0;

  for (f = 0; f < 3; f++)
    {
      infile = fopen(argv[f+2], "r");
      if (!infile)
        {
          printf("couldn't open file %s!\n", argv[f]);
          exit(1);
        }
      else
        {
          do
            {
              fgets(line, 500, infile);
              line2 = line;
              while (*line2)
                {
                  line2++;
                  if (*line2 == ',')
                    {
                      *line2 = 0;
                      break;
                    }
                }
              if (*line2 == 0 && line != line2)
                {
                  line2++;
                  if (f == 0 || n == 0)
                    {
                      strcpy(res.target[n], line);
                      strcpy(res.data[0][n], line2);
                      strcpy(res.data[1][n], ",,,,,");
                      strcpy(res.data[2][n], ",,,,,");
                      n++;
                    }
                  else
                    {
                      found = 0;
                      for (j = next; j < n; j++)
                        if (strcmp(res.target[j], line) == 0)
                          {
                            found = j;
                            j = n;
                            break;
                          }
                      if (!found)
                        for (j = 0; j < next; j++)
                          if (strcmp(res.target[j], line) == 0)
                            {
                              found = j;
                              j = next;
                              break;
                            }
                      if (!found)
                        {
                          found = n++;
                          strcpy(res.target[0], ",,,,,");
                          strcpy(res.target[found], line);
                          strcpy(res.target[3-found], ",,,,,");
                        }
                      strcpy(res.data[f][found], line2);
                    }
                }
            }
          while (!feof(infile) && line != line2);
        }
    }

  for (i = 0; i < n; i++)
    printf("%s,%s,%s,%s\n", res.target[i], res.data[i][0], res.data[i][1], res.data[i][2]);

}

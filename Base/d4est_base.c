#include <d4est_base.h>

int d4est_asprintf(char **str, char* fmt, ...){
  va_list argp;
  va_start(argp, fmt);
  char one_char[1];
  int len = vsnprintf(one_char, 1, fmt, argp);
  if (len < 1){
    fprintf(stderr, "An encoding error occurred. Setting the input pointer to NULL.\n");
    *str = NULL;
    return len;
  }
  va_end(argp);
  *str = malloc(len+1);
  if (!str) {
    fprintf(stderr, "Couldn't allocate %i bytes.\n", len+1);
    return -1;
  }
  va_start(argp, fmt);
  vsnprintf(*str, len+1, fmt, argp);
  va_end(argp);
  return len;
}

/* from the openmpi gdb guide */
void d4est_mpi_gdb_stall()
{
    int i = 0;
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    printf("PID %d on %s ready for attach\n", getpid(), hostname);
    fflush(stdout);
    while (0 == i)
        sleep(5);
}

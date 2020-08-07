#include <sys/time.h>
#include <stdio.h>

int main() {
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  printf("seconds : %ld\n micro seconds : %ld\n",
    current_time.tv_sec, current_time.tv_usec);

  return 0;
}

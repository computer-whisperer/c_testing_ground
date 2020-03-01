#include <stdio.h>
#include <time.h>

#define RUN_COUNT 1000000000

int do_it_with_if(int i) {
  int j = (i * 233) % RUN_COUNT;
  if (i > j) {
    return 1;
  }
  return 0;
}

int do_it_with_no_if(int i) {
  int j = (i * 233) % RUN_COUNT;
  return i > j;
}

int output[RUN_COUNT];

int main() {
  int i;
  
  clock_t start = clock();
  for (i=0; i<RUN_COUNT; i++) {
    output[i] = do_it_with_if(i);
  }
  int time_1 = clock() - start;
  printf("With if:    %f ns\n", 1000000000.0*((float)time_1/(float)RUN_COUNT)/(float)CLOCKS_PER_SEC);
  
  start = clock();
  for (i=0; i<RUN_COUNT; i++) {
    output[i] = do_it_with_no_if(i);
  }
  int time_2 = clock() - start;
  printf("Without if: %f ns\n", 1000000000.0*((float)time_2/(float)RUN_COUNT)/(float)CLOCKS_PER_SEC);
  printf("%f%%\n", 100*(float)time_2/(float)time_1);
}

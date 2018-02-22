#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <assert.h>
size_t array[5*1024];

void toggle_benchmark(void* p) {
    asm volatile ("clflush 0(%0)\n"
      :
      : "c" (p)
      : "rax");
}

int main(int argc, char** argv)
{
  memset(array,-1,5*1024*sizeof(size_t));

  toggle_benchmark(array+1024);
  for (int i = 0; i < 5*1024; ++i)
  {
    size_t read = array[i];
  }
  toggle_benchmark(array+2*1024);

  return 1;
}

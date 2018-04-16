#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <assert.h>
#include "../cacheutils.h"

size_t array[5*1024];

size_t hit_histogram[80];
size_t miss_histogram[80];

size_t onlyreload(void* addr)
{
  size_t time = rdtsc();
  maccess(addr);
  size_t delta = rdtsc() - time;
  return delta;
}

size_t flushandreload(void* addr)
{
  size_t time = rdtsc();
  maccess(addr);
  size_t delta = rdtsc() - time;
  flush(addr);
  return delta;
}

int main(int argc, char** argv)
{
  memset(array,-1,5*1024*sizeof(size_t));
  maccess(array + 2*1024);
  sched_yield();
  for (int i = 0; i < 4*1024*1024; ++i)
  {
    size_t d = onlyreload(array+2*1024);
    hit_histogram[MIN(79,d/5)]++;
    sched_yield();
  }
  flush(array+1024);
  for (int i = 0; i < 4*1024*1024; ++i)
  {
    size_t d = flushandreload(array+2*1024);
    miss_histogram[MIN(79,d/5)]++;
    sched_yield();
  }
  printf(".\n");
  size_t hit_max = 0;
  size_t hit_max_i = 0;
  size_t miss_min_i = 0;
  for (int i = 0; i < 80; ++i)
  {
    FILE *fc = fopen("logs/calibration_hist_current", "a");
    assert(fc != NULL);
    fprintf(fc, "%3zu: %10zu %10zu\n",i*5,hit_histogram[i],miss_histogram[i]);
    fclose(fc);
    //printf("%3d: %10zu %10zu\n",i*5,hit_histogram[i],miss_histogram[i]);
    if (hit_max < hit_histogram[i])
    {
      hit_max = hit_histogram[i];
      hit_max_i = i;
    }
    if (miss_histogram[i] > 3 && miss_min_i == 0)
      miss_min_i = i;
  }
  if (miss_min_i > hit_max_i+4){
    FILE *fc = fopen("logs/calibration_hist_current", "a");
    assert(fc != NULL);
    fprintf(fc, "Flush+Reload possible!\n");
    fclose(fc);
    printf("Flush+Reload possible!\n");
  }

  else if (miss_min_i > hit_max_i+2){
    FILE *fc = fopen("logs/calibration_hist_current", "a");
    assert(fc != NULL);
    fprintf(fc, "Flush+Reload probably possible!\n");
    fclose(fc);
    printf("Flush+Reload probably possible!\n");
  }

  else if (miss_min_i < hit_max_i+2){
    FILE *fc = fopen("logs/calibration_hist_current", "a");
    assert(fc != NULL);
    fprintf(fc, "Flush+Reload maybe not possible!\n");
    fclose(fc);
    printf("Flush+Reload maybe not possible!\n");
  }

  else{
    FILE *fc = fopen("logs/calibration_hist_current", "a");
    assert(fc != NULL);
    fprintf(fc, "Flush+Reload not possible!\n");
    fclose(fc);
    printf("Flush+Reload not possible!\n");
  }

  size_t min = -1UL;
  size_t min_i = 0;
  for (int i = hit_max_i; i < miss_min_i; ++i)
  {
    if (min > (hit_histogram[i] + miss_histogram[i]))
    {
      min = hit_histogram[i] + miss_histogram[i];
      min_i = i;
    }
  }

  FILE *fc = fopen("logs/calibration_hist_current", "a");
  assert(fc != NULL);
  fprintf(fc, "The lower the threshold, the lower the number of false positives.\n");
  fprintf(fc, "Suggested cache hit/miss threshold: %zu\n",min_i * 5);
  fclose(fc);
  printf("The lower the threshold, the lower the number of false positives.\n");
  printf("Suggested cache hit/miss threshold: %zu\n",min_i * 5);
  return min_i * 5;
}

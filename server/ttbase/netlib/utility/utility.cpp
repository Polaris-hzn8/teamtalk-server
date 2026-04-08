/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: util.cpp
 Update Time: Mon 12 Jun 2023 17:04:14 CST
 brief: 各种工具类封装
*/

#include "utility.h"
#include <sstream>

uint64_t get_tick_count() {
#ifdef _MSC_VER
  LARGE_INTEGER liCounter;
  LARGE_INTEGER liCurrent;

  if (!QueryPerformanceFrequency(&liCounter))
    return GetTickCount();

  QueryPerformanceCounter(&liCurrent);
  return (uint64_t)(liCurrent.QuadPart * 1000 / liCounter.QuadPart);
#else
  struct timeval tval;
  uint64_t ret_tick;

  gettimeofday(&tval, NULL);

  ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
  return ret_tick;
#endif
}

void util_sleep(uint32_t millisecond) {
#ifdef _MSC_VER
  Sleep(millisecond);
#else
  usleep(millisecond * 1000);
#endif
}

void writePid() {
  uint32_t curPid;
#ifdef _MSC_VER
  curPid = (uint32_t)GetCurrentProcess();
#else
  curPid = (uint32_t)getpid();
#endif
  FILE* fp = fopen("server.pid", "w");
  assert(fp);
  char szPid[32];
  snprintf(szPid, sizeof(szPid), "%d", curPid);
  fwrite(szPid, strlen(szPid), 1, fp);
  fclose(fp);
}

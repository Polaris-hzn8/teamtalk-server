/*
 Reviser: Polaris_hzn8
 Email: lch2022fox@163.com
 filename: main.cpp
 Update Time: Sun 10 Aug 2025 12:09:22 CST
 brief: 
*/

#include <iostream>
#include <sys/signal.h>
#include "push_app.h"
#include "timer/Timer.hpp"

void writePid()
{
    uint32_t curPid;
#ifdef _WIN32
    curPid = (uint32_t) GetCurrentProcess();
#else
    curPid = (uint32_t) getpid();
#endif
    FILE* f = fopen("server.pid", "w");
    assert(f);
    char szPid[32];
    snprintf(szPid, sizeof(szPid), "%d", curPid);
    fwrite(szPid, strlen(szPid), 1, f);
    fclose(f);
}

int main(int argc, const char * argv[])
{
    // insert code here...
    printf("start push server...\n");
    signal(SIGPIPE, SIG_IGN);
    CPushApp::GetInstance()->Init();
    CPushApp::GetInstance()->Start();
    writePid();
    while (true) {
        S_Sleep(1000);
    }
    return 0;
}

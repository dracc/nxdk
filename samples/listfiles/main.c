#include <xboxrt/debug.h>
#include <pbkit/pbkit.h>
#include <hal/xbox.h>
#include <assert.h>
#include <fileapi.h>
#include <stdio.h>

void main(void)
{
    int i;

    switch(pb_init())
    {
        case 0: break;
        default:
            XSleep(2000);
            XReboot();
            return;
    }

    pb_show_debug_screen();

    WIN32_FIND_DATA findFileData;
    HANDLE hFind;

    hFind = FindFirstFile("C:\\*", &findFileData);
    assert(hFind != INVALID_HANDLE_VALUE);
    do {
        debugPrint("Found File: %s\n", findFileData.cFileName);

    } while (FindNextFile(hFind, &findFileData) != 0);
    debugPrint("error: %x\n", GetLastError());


    while(1) {
        XSleep(2000);
    }

    pb_kill();
    XReboot();
}

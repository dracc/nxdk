#ifndef __WINBASE_H__
#define __WINBASE_H__

#include <WinDef.h>

#ifdef __cplusplus
extern "C"
{
#endif

DWORD GetLastError (void);
void SetLastError (DWORD error);

#ifdef __cplusplus
}
#endif

#endif

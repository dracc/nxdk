#include "xgu.h"

#include <assert.h>
#include <pbkit/pbkit.h>

DWORD *pushbuffer_edge;

static DWORD* push_command(DWORD command, int nparams) {
    pb_push(pushbuffer_edge++, command, nparams);
    return pushbuffer_edge;
}

static DWORD* push_parameter(DWORD parameter) {
    *pushbuffer_edge++ = parameter;
}

DWORD* xgu_set_object(DWORD* p, int instance) {
    pb_push(p++, NV097_SET_OBJECT, 1);
    *p++ = instance;
    return p;
}

DWORD* xgu_enable_texture(DWORD* p, int stage_index, bool enabled) {
    assert(stage_index >= 0);
    assert(stage_index <= 3);
    pb_push(p++, NV20_TCL_PRIMITIVE_3D_TX_ENABLE(1), 1);
    *p++ = 0x0003ffc0 | enabled ? (1 << 30) : 0);
    return p;
}

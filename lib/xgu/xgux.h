#ifndef __XGUX_H
#define __XGUX_H

#include "xgu.h"

inline
void xgux_draw_arrays(XguPrimitiveType mode, unsigned int start, unsigned int count) {
    uint32_t *p = pb_begin();
    p = xgu_begin(p, mode);
    p = xgu_draw_arrays(p, start, count);
    p = xgu_end(p);
    p = pb_end(p);
}

inline
void xgux_set_clear_rect(unsigned int x, unsigned int y,
                         unsigned int width, unsigned int height) {
    uint32_t *p = pb_begin();
    p = xgu_set_clear_rect_horizontal(p, x, x+width);
    p = xgu_set_clear_rect_vertical(p, y, y+height);
    p = pb_end(p);
}

inline
void xgux_set_attrib_pointer(XguVertexArray index, XguVertexArrayType format, unsigned int size, unsigned int stride, const void* data)
{
    uint32_t *p = pb_begin();
    p = xgu_set_vertex_data_array_format(p, index, format, size, stride);
    p = xgu_set_vertex_data_array_offset(p, index, (uint32_t)data & 0x03ffffff);
    p = pb_end(p);
}

#endif

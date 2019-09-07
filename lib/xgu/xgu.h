#ifndef __XGU_H
#define __XGU_H

#include <assert.h>
#include <pbkit/pbkit.h>
// Hack until we fix pbkit/outer.h (or stop including it)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmacro-redefined"
#include "nv2a_regs.h"
#pragma GCC diagnostic pop

#include <stdint.h>

typedef enum {
  //FIXME: define NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_UB_D3D     0
  //FIXME: define NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S1         1
  XGU_FLOAT = NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F,
  //FIXME: define NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_UB_OGL     4
  //FIXME: define NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S32K       5
  //FIXME: define NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_CMP        6
} XguVertexArrayType;

typedef enum {
  XGU_END = NV097_SET_BEGIN_END_OP_END, // FIXME: Disallow this one?
  XGU_POINTS = NV097_SET_BEGIN_END_OP_POINTS,
  XGU_LINES = NV097_SET_BEGIN_END_OP_LINES,
  XGU_LINE_LOOP = NV097_SET_BEGIN_END_OP_LINE_LOOP,
  XGU_LINE_STRIP = NV097_SET_BEGIN_END_OP_LINE_STRIP,
  XGU_TRIANGLES = NV097_SET_BEGIN_END_OP_TRIANGLES,
  XGU_TRIANGLE_STRIP = NV097_SET_BEGIN_END_OP_TRIANGLE_STRIP,
  XGU_TRIANGLE_FAN = NV097_SET_BEGIN_END_OP_TRIANGLE_FAN,
  XGU_QUADS = NV097_SET_BEGIN_END_OP_QUADS,
  XGU_QUAD_STRIP = NV097_SET_BEGIN_END_OP_QUAD_STRIP,
  XGU_POLYGON = NV097_SET_BEGIN_END_OP_POLYGON
} XguPrimitiveType;

typedef enum {
  XGU_VERTEX_ARRAY = NV2A_VERTEX_ATTR_POSITION,
  XGU_NORMAL_ARRAY = NV2A_VERTEX_ATTR_NORMAL,
  XGU_COLOR_ARRAY = NV2A_VERTEX_ATTR_DIFFUSE,
  XGU_SECONDARY_COLOR_ARRAY = NV2A_VERTEX_ATTR_SPECULAR,
  XGU_FOG_ARRAY = NV2A_VERTEX_ATTR_FOG,
  XGU_TEXCOORD0_ARRAY = NV2A_VERTEX_ATTR_TEXTURE0,
  XGU_TEXCOORD1_ARRAY = NV2A_VERTEX_ATTR_TEXTURE1,
  XGU_TEXCOORD2_ARRAY = NV2A_VERTEX_ATTR_TEXTURE2,
  XGU_TEXCOORD3_ARRAY = NV2A_VERTEX_ATTR_TEXTURE3
} XguVertexArray;

static inline
DWORD* push_command(DWORD* p, DWORD command, int nparams) {
    pb_push(p++, command, nparams);
    return p;
}

static inline
DWORD* push_parameter(DWORD* p, DWORD parameter) {
    *p++ = parameter;
    return p;
}

static inline
DWORD* push_boolean(DWORD* p, bool enabled) {
  return push_parameter(p, enabled ? 1 : 0);
}

static inline
DWORD* push_command_boolean(DWORD* p, DWORD command, bool enabled) {
  p = push_command(p, command, 1);
  p = push_boolean(p, enabled);
  return p;
}

static inline
DWORD* push_command_parameter(DWORD* p, DWORD command, DWORD parameter) {
  p = push_command(p, command, 1);
  p = push_parameter(p, parameter);
  return p;
}

inline
DWORD* xgu_begin(DWORD* p, XguPrimitiveType type) {

  // Force people to use xgu_end instead
  assert(type != NV097_SET_BEGIN_END_OP_END);

  return push_command_parameter(p, NV097_SET_BEGIN_END, type);
}

inline
DWORD* xgu_end(DWORD* p) {
  return push_command_parameter(p, NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
}

inline
DWORD* xgu_clear_color(DWORD* p, DWORD color) {
    p = push_command_parameter(p, NV097_SET_COLOR_CLEAR_VALUE, color);
    return p;
}

inline
DWORD* xgu_clear(DWORD* p) {
    /* Should probably do some magic related to 
     * NV20_TCL_PRIMITIVE_3D_CLEAR_VALUE_HORIZ and
     * NV20_TCL_PRIMITIVE_3D_CLEAR_VALUE_DEPTH
     */
    return p;
}

inline
DWORD* xgu_set_object(DWORD* p, int instance) {
    p = push_command(p, NV097_SET_OBJECT, 1);
    p = push_parameter(instance);
    return p;
}

inline
DWORD* xgu_matrix_mode_modelview(DWORD* p) {
    p = push_command(p, NV097_SET_MODELVIEW_MATRIX, 0);
    return p;
}

inline
DWORD* xgu_matrix_mode_projection(DWORD* p) {
    p = push_command(p, NV097_SET_PROJECTION_MATRIX, 0);
    return p;
}

inline
DWORD* xgu_matrix_mode_texture(DWORD* p) {
    p = push_command(p, NV097_SET_TEXTURE_MATRIX, 0);
    return p;
}

inline
DWORD* xgu_set_viewport(int vpx, int vpy, int vpw, int vph, float znear, float zfar) {
    
}

inline
DWORD* xgu_enable_texture(DWORD* p, int stage_index, bool enabled) {
    assert(stage_index >= 0);
    assert(stage_index <= 3);
    p = push_command(p, NV20_TCL_PRIMITIVE_3D_TX_ENABLE(1), 1);
    p = push_parameter(p, 0x0003ffc0 | enabled ? (1 << 30) : 0);
    return p;
}

#define xgu_set_vertex_array_format(...) 0
#define xgu_set_vertex_array_offset(...) 0

inline
DWORD* xgu_elements16(DWORD* p, uint16_t* elements, unsigned int count) {
  assert(count % 2 == 0);
  // This one is a bit complicated
  assert(false);
  return p;
}

inline
DWORD* xgu_elements32(DWORD* p, uint32_t* elements, unsigned int count) {
  // This one is a bit complicated
  assert(false);
  return p;
}

#define xgu_draw_arrays(...) 0

inline
DWORD* xgu_set_alpha_test(DWORD* p, bool enabled) {
  return push_command_boolean(p, NV097_SET_ALPHA_TEST_ENABLE, enabled);
}

#endif

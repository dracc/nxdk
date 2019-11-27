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
#include <stdbool.h>
#include <string.h>

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

typedef enum {
    XGU_FIXED = NV097_SET_TRANSFORM_EXECUTION_MODE_MODE_FIXED,
    XGU_PROGRAM = NV097_SET_TRANSFORM_EXECUTION_MODE_MODE_PROGRAM
} XguExecMode;

typedef enum {
    XGU_RANGE_MODE_USER = 0 /*NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE_USER*/,
    XGU_RANGE_MODE_PRIVATE = 1 /*NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE_PRIVATE */
} XguExecRange;

typedef enum {
    XGU_CLEAR_Z = NV097_CLEAR_SURFACE_Z,
    XGU_CLEAR_STENCIL = NV097_CLEAR_SURFACE_STENCIL,
    XGU_CLEAR_COLOR = NV097_CLEAR_SURFACE_COLOR,
    XGU_CLEAR_R = NV097_CLEAR_SURFACE_R,
    XGU_CLEAR_G = NV097_CLEAR_SURFACE_G,
    XGU_CLEAR_B = NV097_CLEAR_SURFACE_B,
    XGU_CLEAR_A = NV097_CLEAR_SURFACE_A
} XguClearSurface;

/*
 * SFACTOR and DFACTOR share the same values for their respective defines.
 * Thus no duplicate table was created.
 */
typedef enum {
    XGU_FACTOR_ZERO = NV097_SET_BLEND_FUNC_SFACTOR_V_ZERO,
    XGU_FACTOR_ONE = NV097_SET_BLEND_FUNC_SFACTOR_V_ONE,
    XGU_FACTOR_SRC_COLOR = NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_COLOR,
    XGU_FACTOR_ONE_MINUS_SRC_COLOR = NV097_SET_BLEND_FUNC_SFACTOR_V_ONE_MINUS_SRC_COLOR,
    XGU_FACTOR_SRC_ALPHA = NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA,
    XGU_FACTOR_ONE_MINUS_SRC_ALPHA = NV097_SET_BLEND_FUNC_SFACTOR_V_ONE_MINUS_SRC_ALPHA,
    XGU_FACTOR_DST_ALPHA = NV097_SET_BLEND_FUNC_SFACTOR_V_DST_ALPHA,
    XGU_FACTOR_ONE_MINUS_DST_ALPHA = NV097_SET_BLEND_FUNC_SFACTOR_V_ONE_MINUS_DST_ALPHA,
    XGU_FACTOR_DST_COLOR = NV097_SET_BLEND_FUNC_SFACTOR_V_DST_COLOR,
    XGU_FACTOR_ONE_MINUS_DST_COLOR = NV097_SET_BLEND_FUNC_SFACTOR_V_ONE_MINUS_DST_COLOR,
    XGU_FACTOR_SRC_ALPHA_SATURATE = NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA_SATURATE,
    XGU_FACTOR_CONSTANT_COLOR = NV097_SET_BLEND_FUNC_SFACTOR_V_CONSTANT_COLOR,
    XGU_FACTOR_ONE_MINUS_CONSTANT_COLOR = NV097_SET_BLEND_FUNC_SFACTOR_V_ONE_MINUS_CONSTANT_COLOR,
    XGU_FACTOR_CONSTANT_ALPHA = NV097_SET_BLEND_FUNC_SFACTOR_V_CONSTANT_ALPHA,
    XGU_FACTOR_ONE_MINUS_CONSTANT_ALPHA = NV097_SET_BLEND_FUNC_SFACTOR_V_ONE_MINUS_CONSTANT_ALPHA
} XguBlendFactor;

typedef enum {
    XGU_BLUE = NV097_SET_COLOR_MASK_BLUE_WRITE_ENABLE,
    XGU_GREEN = NV097_SET_COLOR_MASK_GREEN_WRITE_ENABLE,
    XGU_RED = NV097_SET_COLOR_MASK_RED_WRITE_ENABLE,
    XGU_ALPHA = NV097_SET_COLOR_MASK_ALPHA_WRITE_ENABLE
} XguColorMask;

typedef enum {
    XGU_POINT = NV097_SET_FRONT_POLYGON_MODE_V_POINT,
    XGU_LINE = NV097_SET_FRONT_POLYGON_MODE_V_LINE,
    XGU_FILL = NV097_SET_FRONT_POLYGON_MODE_V_FILL
} XguPolygonMode;

typedef enum {
    XGU_CULL_FRONT = NV097_SET_CULL_FACE_V_FRONT,
    XGU_CULL_BACK = NV097_SET_CULL_FACE_V_BACK,
    XGU_CULL_FRONT_AND_BACK = NV097_SET_CULL_FACE_V_FRONT_AND_BACK
} XguCullFace;

typedef enum {
    XGU_TEXGEN_DISABLE = NV097_SET_TEXGEN_S_DISABLE,
    XGU_TEXGEN_EYE_LINEAR = NV097_SET_TEXGEN_S_EYE_LINEAR,
    XGU_TEXGEN_OBJECT_LINEAR = NV097_SET_TEXGEN_S_OBJECT_LINEAR,
    XGU_TEXGEN_SPHERE_MAP = NV097_SET_TEXGEN_S_SPHERE_MAP,
    XGU_TEXGEN_REFLECTION_MAP = NV097_SET_TEXGEN_S_REFLECTION_MAP,
    XGU_TEXGEN_NORMAL_MAP = NV097_SET_TEXGEN_S_NORMAL_MAP
} XguTexgen;

typedef enum {
    XGU_STENCIL_OP_KEEP = NV097_SET_STENCIL_OP_V_KEEP,
    XGU_STENCIL_OP_ZERO = NV097_SET_STENCIL_OP_V_ZERO,
    XGU_STENCIL_OP_REPLACE = NV097_SET_STENCIL_OP_V_REPLACE,
    XGU_STENCIL_OP_INCRSAT = NV097_SET_STENCIL_OP_V_INCRSAT,
    XGU_STENCIL_OP_DECRSAT = NV097_SET_STENCIL_OP_V_DECRSAT,
    XGU_STENCIL_OP_INVERT = NV097_SET_STENCIL_OP_V_INVERT,
    XGU_STENCIL_OP_INCR = NV097_SET_STENCIL_OP_V_INCR,
    XGU_STENCIL_OP_DECR = NV097_SET_STENCIL_OP_V_DECR
} XguStencilOp;

typedef enum {
    XGU_FRONT_CW = NV097_SET_FRONT_FACE_V_CW,
    XGU_FRONT_CCW = NV097_SET_FRONT_FACE_V_CCW
} XguFrontFace;

typedef enum {
    XGU_LMASK_OFF = NV097_SET_LIGHT_ENABLE_MASK_LIGHT0_OFF,
    XGU_LMASK_INFINITE = NV097_SET_LIGHT_ENABLE_MASK_LIGHT0_INFINITE,
    XGU_LMASK_LOCAL = NV097_SET_LIGHT_ENABLE_MASK_LIGHT0_LOCAL,
    XGU_LMASK_SPOT = NV097_SET_LIGHT_ENABLE_MASK_LIGHT0_SPOT
} XguLightMask;

#define XGU_MASK(mask, val) (((val) << (__builtin_ffs(mask)-1)) & (mask))

#define XGU_ATTRIBUTE_COUNT 16
#define XGU_TEXTURE_COUNT 4
#define XGU_WEIGHT_COUNT 4
#define XGU_LIGHT_COUNT 8

typedef union {
    float f[3];
    struct {
        float x, y, z;
    };
    struct {
        float r, g, b;
    };
} XguVec3;

typedef union {
    float f[4];
    struct {
        float x, y, z, w;
    };
    struct {
        float r, g, b, a;
    };
} XguVec4;

typedef union {
  float f[4*4];
  XguVec4 col[4];
} XguMatrix4x4;


/* ========================= *
 *  BEGIN PUSH HELPER BLOCK  *
 * ========================= */

static inline
uint32_t* push_command(uint32_t* p, uint32_t command, unsigned int parameter_count) {
    int nparams = parameter_count;
    assert(nparams > 0);
    pb_push(p++, command, nparams);
    return p;
}

static inline
uint32_t* push_parameter(uint32_t* p, uint32_t parameter) {
    *p++ = parameter;
    return p;
}

static inline
uint32_t* push_parameters(uint32_t* p, const uint32_t* parameters, unsigned int count) {
    for(unsigned int i = 0; i < count; i++) {
        p = push_parameter(p, parameters[i]);
    }
    return p;
}

static inline
uint32_t* push_boolean(uint32_t* p, bool enabled) {
    return push_parameter(p, enabled ? 1 : 0);
}

static inline
uint32_t* push_command_boolean(uint32_t* p, uint32_t command, bool enabled) {
    p = push_command(p, command, 1);
    p = push_boolean(p, enabled);
    return p;
}

static inline
uint32_t* push_float(uint32_t* p, float f) {
    return push_parameter(p, *(uint32_t*)&f);
}

static inline
uint32_t* push_floats(uint32_t* p, const float* f, unsigned int count) {
    return push_parameters(p, (const uint32_t*)f, count);
}

static inline
uint32_t* push_matrix2x2(uint32_t* p, float m[2*2]) {
    return push_floats(p, m, 2*2);
}

static inline
uint32_t* push_matrix4x4(uint32_t* p, float m[4*4]) {
    return push_floats(p, m, 4*4);
}

static inline
uint32_t* push_command_matrix2x2(uint32_t* p, uint32_t command, float m[2*2]) {
    p = push_command(p, command, 2*2);
    p = push_matrix2x2(p, m);
    return p;
}

static inline
uint32_t* push_command_matrix4x4(uint32_t* p, uint32_t command, float m[4*4]) {
    p = push_command(p, command, 4*4);
    p = push_matrix4x4(p, m);
    return p;
}

static inline
uint32_t* push_command_parameter(uint32_t* p, uint32_t command, uint32_t parameter) {
    p = push_command(p, command, 1);
    p = push_parameter(p, parameter);
    return p;
}

static inline
uint32_t* push_command_float(uint32_t* p, uint32_t command, float parameter) {
    p = push_command(p, command, 1);
    p = push_float(p, parameter);
    return p;
}


/* ========================= *
 * BEGIN XGU FUNCTIONS BLOCK *
 * ========================= */

inline
uint32_t* xgu_begin(uint32_t* p, XguPrimitiveType type) {

    // Force people to use xgu_end instead
    assert(type != NV097_SET_BEGIN_END_OP_END);

    return push_command_parameter(p, NV097_SET_BEGIN_END, type);
}

inline
uint32_t* xgu_end(uint32_t* p) {
    return push_command_parameter(p, NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
}

inline
uint32_t* xgu_no_operation(uint32_t* p, uint32_t param) {
    /* param is typically 0 */
    return push_command_parameter(p, NV097_NO_OPERATION, param);
}

inline
uint32_t* xgu_wait_for_idle(uint32_t* p) {
    return push_command(p, NV097_WAIT_FOR_IDLE, 0);
}

inline
uint32_t* xgu_set_viewport_offset(uint32_t* p, float x, float y, float z, float w) {
    p = push_command(p, NV097_SET_VIEWPORT_OFFSET, 4);
    p = push_float(p, x);
    p = push_float(p, y);
    p = push_float(p, z);
    p = push_float(p, w);
    return p;
}

inline
uint32_t* xgu_set_viewport_scale(uint32_t* p, float x, float y, float z, float w) {
    p = push_command(p, NV097_SET_VIEWPORT_SCALE, 4);
    p = push_float(p, x);
    p = push_float(p, y);
    p = push_float(p, z);
    p = push_float(p, w);
    return p;
}

inline
uint32_t* xgu_set_clip_min(uint32_t* p, float znear) {
    return push_command_float(p, NV097_SET_CLIP_MIN, znear);
}

inline
uint32_t* xgu_set_clip_max(uint32_t* p, float zfar) {
    return push_command_float(p, NV097_SET_CLIP_MAX, zfar);
}

inline
uint32_t* xgu_set_zstencil_clear_value(uint32_t* p, uint32_t value) {
    return push_command_parameter(p, NV097_SET_ZSTENCIL_CLEAR_VALUE, value);
}

inline
uint32_t* xgu_set_color_clear_value(uint32_t* p, uint32_t color) {
    return push_command_parameter(p, NV097_SET_COLOR_CLEAR_VALUE, color);
}

inline
uint32_t* xgu_clear_surface(uint32_t* p, XguClearSurface flags) {
    /*
     * flags should probably be a combination of values -
     * not sure if using an enum allows such magic and/or
     * if it hinders out-of-bounds values.
     */
    return push_command_parameter(p, NV097_CLEAR_SURFACE, flags);
}

inline
uint32_t* xgu_set_clear_rect_horizontal(uint32_t* p, uint32_t x1, uint32_t x2) {
    return push_command_parameter(p, NV097_SET_CLEAR_RECT_HORIZONTAL,
                                  ((x2-1)<<16)|x1);
}

inline
uint32_t* xgu_set_clear_rect_vertical(uint32_t* p, uint32_t y1, uint32_t y2) {
    return push_command_parameter(p, NV097_SET_CLEAR_RECT_VERTICAL,
                                  ((y2-1)<<16)|y1);
}

inline
uint32_t* xgu_set_object(uint32_t* p, uint32_t instance) {
    return push_command_parameter(p, NV097_SET_OBJECT, instance);
}

inline
uint32_t* xgu_set_texgen_s(uint32_t* p, uint32_t texture_index, XguTexgen tg) {
    assert(texture_index == 0);
    return push_command_parameter(p, NV097_SET_TEXGEN_S, tg);
}

inline
uint32_t* xgu_set_texgen_t(uint32_t* p, uint32_t texture_index, XguTexgen tg) {
    assert(texture_index == 0);
    return push_command_parameter(p, NV097_SET_TEXGEN_T, tg);
}

inline
uint32_t* xgu_set_texgen_r(uint32_t* p, uint32_t texture_index, XguTexgen tg) {
    assert(texture_index == 0);
    return push_command_parameter(p, NV097_SET_TEXGEN_R, tg);
}

inline
uint32_t* xgu_set_texgen_q(uint32_t* p, uint32_t texture_index, XguTexgen tg) {
    assert(texture_index == 0);
    return push_command_parameter(p, NV097_SET_TEXGEN_Q, tg);
}

inline
uint32_t* xgu_set_texture_matrix_enable(uint32_t* p, uint32_t texture_index, bool enabled) {
    assert(texture_index == 0);
    return push_command_boolean(p, NV097_SET_TEXTURE_MATRIX_ENABLE, enabled);
}

inline
uint32_t* xgu_set_projection_matrix(uint32_t* p, float m[4*4]) {
    return push_command_matrix4x4(p, NV097_SET_PROJECTION_MATRIX, m);
}

inline
uint32_t* xgu_set_model_view_matrix(uint32_t* p, uint32_t bone_index, float m[4*4]) {
    return push_command_matrix4x4(p, NV097_SET_MODEL_VIEW_MATRIX + bone_index*(4*4)*4, m);
}

inline
uint32_t* xgu_set_inverse_model_view_matrix(uint32_t* p, uint32_t bone_index, float m[4*4]) {
    return push_command_matrix4x4(p, NV097_SET_INVERSE_MODEL_VIEW_MATRIX + bone_index*(4*4)*4, m);
}

inline
uint32_t* xgu_set_composite_matrix(uint32_t* p, float m[4*4]) {
    return push_command_matrix4x4(p, NV097_SET_COMPOSITE_MATRIX, m);
}

inline
uint32_t* xgu_set_texture_matrix(uint32_t* p, uint32_t slot, float m[4*4]) {
    assert(slot == 0);
    return push_command_matrix4x4(p, NV097_SET_TEXTURE_MATRIX, m);
}

/* ==== Stencil OP ==== */

inline
uint32_t* xgu_set_stencil_op_fail(uint32_t* p, XguStencilOp so) {
    return push_command_parameter(p, NV097_SET_STENCIL_OP_FAIL, so);
}

inline
uint32_t* xgu_set_stencil_op_zfail(uint32_t* p, XguStencilOp so) {
    return push_command_parameter(p, NV097_SET_STENCIL_OP_ZFAIL, so);
}

inline
uint32_t* xgu_set_stencil_op_zpass(uint32_t* p, XguStencilOp so) {
    return push_command_parameter(p, NV097_SET_STENCIL_OP_ZPASS, so);
}

/* ==== Vertex Data Array ==== */

inline
uint32_t* xgu_set_vertex_data_array_format(uint32_t* p, XguVertexArray index, XguVertexArrayType format,
                                        uint32_t size, uint32_t stride) {
    return push_command_parameter(p, NV097_SET_VERTEX_DATA_ARRAY_FORMAT + index*4,
                                  XGU_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE, format) |
                                  XGU_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_SIZE, size) |
                                  XGU_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_STRIDE, stride));
}

inline
uint32_t* xgu_set_vertex_data_array_offset(uint32_t* p, XguVertexArray index, const void* data) {
    return push_command_parameter(p, NV097_SET_VERTEX_DATA_ARRAY_OFFSET + index*4,
                                  (uint32_t)data/* & 0x03fffff*/);
}

inline
uint32_t* xgu_element16(uint32_t* p, const uint16_t* elements, unsigned int count) {
    assert(count % 2 == 0);
    p = push_command(p, 0x40000000|NV097_ARRAY_ELEMENT16, count/2);
    memcpy(p, elements, count * sizeof(uint16_t));
    p += count / 2;
    return p;
}

inline
uint32_t* xgu_element32(uint32_t* p, const uint32_t* elements, unsigned int count) {
    p = push_command(p, 0x40000000|NV097_ARRAY_ELEMENT32, count);
    memcpy(p, elements, count * sizeof(uint32_t));
    p += count;
    return p;
}

inline
uint32_t* xgu_draw_arrays(uint32_t* p, unsigned int start, unsigned int count) {
    assert(count>=1);
    assert(count<=256);
    return push_command_parameter(p, 0x40000000|NV097_DRAW_ARRAYS,
                                  XGU_MASK(NV097_DRAW_ARRAYS_COUNT, (count-1)) |
                                  XGU_MASK(NV097_DRAW_ARRAYS_START_INDEX, start));
}

/* ==== Alpha/Blend/Cull ==== */
inline
uint32_t* xgu_set_alpha_test_enable(uint32_t* p, bool enabled) {
    return push_command_boolean(p, NV097_SET_ALPHA_TEST_ENABLE, enabled);
}

inline
uint32_t* xgu_set_blend_enable(uint32_t* p, bool enabled) {
    return push_command_boolean(p, NV097_SET_BLEND_ENABLE, enabled);
}

inline
uint32_t* xgu_set_cull_face_enable(uint32_t* p, bool enabled) {
    return push_command_boolean(p, NV097_SET_CULL_FACE_ENABLE, enabled);
}

inline
uint32_t* xgu_set_depth_test_enable(uint32_t* p, bool enabled) {
    return push_command_boolean(p, NV097_SET_DEPTH_TEST_ENABLE, enabled);
}

inline
uint32_t* xgu_set_dither_enable(uint32_t* p, bool enabled) {
    return push_command_boolean(p, NV097_SET_DITHER_ENABLE, enabled);
}

inline
uint32_t* xgu_set_lighting_enable(uint32_t* p, bool enabled) {
    return push_command_boolean(p, NV097_SET_LIGHTING_ENABLE, enabled);
}

inline
uint32_t* xgu_set_stencil_test_enable(uint32_t* p, bool enabled) {
    return push_command_boolean(p, NV097_SET_STENCIL_TEST_ENABLE, enabled);
}

inline
uint32_t* xgu_set_alpha_func(uint32_t* p, uint8_t func) {
    return push_command_parameter(p, NV097_SET_ALPHA_FUNC, func);
}

inline
uint32_t* xgu_set_alpha_ref(uint32_t* p, uint32_t ref) {
    return push_command_parameter(p, NV097_SET_ALPHA_REF, ref);
}

inline
uint32_t* xgu_set_blend_func_sfactor(uint32_t* p, XguBlendFactor sf) {
    return push_command_parameter(p, NV097_SET_BLEND_FUNC_SFACTOR, sf);
}

inline
uint32_t* xgu_set_blend_func_dfactor(uint32_t* p, XguBlendFactor df) {
    return push_command_parameter(p, NV097_SET_BLEND_FUNC_DFACTOR, df);
}

inline
uint32_t* xgu_set_color_mask(uint32_t* p, XguColorMask cm) {
    return push_command_parameter(p, NV097_SET_COLOR_MASK, cm);
}

inline
uint32_t* xgu_set_front_polygon_mode(uint32_t* p, XguPolygonMode pm) {
    return push_command_parameter(p, NV097_SET_FRONT_POLYGON_MODE, pm);
}

inline
uint32_t* xgu_set_cull_face(uint32_t* p, XguCullFace cf) {
    return push_command_parameter(p, NV097_SET_CULL_FACE, cf);
}

inline
uint32_t* xgu_set_front_face(uint32_t* p, XguFrontFace ff) {
    return push_command_parameter(p, NV097_SET_FRONT_FACE, ff);
}

/* ==== Transform ==== */
inline
uint32_t* xgu_set_transform_execution_mode(uint32_t* p, XguExecMode mode, XguExecRange range) {
    return push_command_parameter(p,
                                  NV097_SET_TRANSFORM_EXECUTION_MODE,
                                  XGU_MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_MODE, mode) |
                                  XGU_MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE, range));
}

inline
uint32_t* xgu_set_transform_constant(uint32_t* p, XguVec4 *v, unsigned int count) {
    p = push_command(p, NV097_SET_TRANSFORM_CONSTANT, count*4);
    for (uint32_t i = 0; i < count; ++i) {
        p = push_floats(p, v[i].f, 4);
    }
    return p;
}

inline
uint32_t* xgu_set_transform_constant_load(uint32_t* p, uint32_t offset) {
    return push_command_parameter(p, NV097_SET_TRANSFORM_CONSTANT_LOAD, offset);
}

inline
uint32_t* xgu_set_transform_program(uint32_t* p, XguVec4 *v, unsigned int count) {
    p = push_command(p, NV097_SET_TRANSFORM_PROGRAM, count*4);
    for (uint32_t i = 0; i < count; ++i) {
        p = push_floats(p, v[i].f, 4);
    }
    return p;
}

inline
uint32_t* xgu_set_transform_program_start(uint32_t* p, uint32_t offset) {
    return push_command_parameter(p, NV097_SET_TRANSFORM_PROGRAM_START, offset);
}

inline
uint32_t* xgu_set_transform_program_load(uint32_t* p, uint32_t offset) {
    return push_command_parameter(p, NV097_SET_TRANSFORM_PROGRAM_LOAD, offset);
}

inline
uint32_t* xgu_set_transform_program_cxt_write_enable(uint32_t* p, bool enabled) {
    return push_command_boolean(p, NV097_SET_TRANSFORM_PROGRAM_CXT_WRITE_EN, enabled);
}

/* ==== Lights ==== */
inline
uint32_t* xgu_set_light_enable_mask(uint32_t* p, unsigned int light_index, XguLightMask lm) {
    assert(light_index >= 0);
    assert(light_index < XGU_LIGHT_COUNT);
    return push_command_parameter(p, NV097_SET_LIGHT_ENABLE_MASK, lm);
}

inline
uint32_t* xgu_set_back_light_ambient_color(uint32_t* p, unsigned int light_index, uint32_t color) {
    assert(light_index >= 0);
    assert(light_index < XGU_LIGHT_COUNT);
    return push_command_parameter(p, NV097_SET_BACK_LIGHT_AMBIENT_COLOR + light_index*4, color);
}

inline
uint32_t* xgu_set_back_light_diffuse_color(uint32_t* p, unsigned int light_index, uint32_t color) {
    assert(light_index >= 0);
    assert(light_index < XGU_LIGHT_COUNT);
    return push_command_parameter(p, NV097_SET_BACK_LIGHT_DIFFUSE_COLOR + light_index*4, color);
}

inline
uint32_t* xgu_set_back_light_specular_color(uint32_t* p, unsigned int light_index, uint32_t color) {
    assert(light_index >= 0);
    assert(light_index < XGU_LIGHT_COUNT);
    return push_command_parameter(p, NV097_SET_BACK_LIGHT_SPECULAR_COLOR + light_index*4, color);
}

inline
uint32_t* xgu_set_light_ambient_color(uint32_t* p, unsigned int light_index, uint32_t color) {
    assert(light_index >= 0);
    assert(light_index < XGU_LIGHT_COUNT);
    return push_command_parameter(p, NV097_SET_LIGHT_AMBIENT_COLOR + light_index*4, color);
}

inline
uint32_t* xgu_set_light_diffuse_color(uint32_t* p, unsigned int light_index, uint32_t color) {
    assert(light_index >= 0);
    assert(light_index < XGU_LIGHT_COUNT);
    return push_command_parameter(p, NV097_SET_LIGHT_DIFFUSE_COLOR + light_index*4, color);
}

inline
uint32_t* xgu_set_light_specular_color(uint32_t* p, unsigned int light_index, uint32_t color) {
    assert(light_index >= 0);
    assert(light_index < XGU_LIGHT_COUNT);
    return push_command_parameter(p, NV097_SET_LIGHT_SPECULAR_COLOR + light_index*4, color);
}

inline
uint32_t* xgu_set_light_local_range(uint32_t* p, unsigned int light_index, float range) {
    assert(light_index >= 0);
    assert(light_index < XGU_LIGHT_COUNT);
    return push_command_float(p, NV097_SET_LIGHT_LOCAL_RANGE + light_index*4, range);
}

inline
uint32_t* xgu_set_light_infinite_half_vector(uint32_t* p, unsigned int light_index, XguVec3 v) {
    assert(light_index >= 0);
    assert(light_index < XGU_LIGHT_COUNT);
    p = push_command(p, NV097_SET_LIGHT_INFINITE_HALF_VECTOR + light_index*3*4, 3);
    return push_floats(p, v.f, 3);
}

inline
uint32_t* xgu_set_light_infinite_direction(uint32_t* p, unsigned int light_index, XguVec3 v) {
    assert(light_index >= 0);
    assert(light_index < XGU_LIGHT_COUNT);
    p = push_command(p, NV097_SET_LIGHT_INFINITE_DIRECTION + light_index*3*4, 3);
    return push_floats(p, v.f, 3);
}

inline
uint32_t* xgu_set_light_spot_falloff(uint32_t* p, unsigned int light_index, XguVec3 v) {
    assert(light_index >= 0);
    assert(light_index < XGU_LIGHT_COUNT);
    p = push_command(p, NV097_SET_LIGHT_SPOT_FALLOFF + light_index*3*4, 3);
    return push_floats(p, v.f, 3);
}

inline
uint32_t* xgu_set_light_spot_direction(uint32_t* p, unsigned int light_index, XguVec4 v) {
    assert(light_index >= 0);
    assert(light_index < XGU_LIGHT_COUNT);
    p = push_command(p, NV097_SET_LIGHT_SPOT_DIRECTION + light_index*4*4, 4);
    return push_floats(p, v.f, 4);
}

inline
uint32_t* xgu_set_light_local_position(uint32_t* p, unsigned int light_index, XguVec3 v) {
    assert(light_index >= 0);
    assert(light_index < XGU_LIGHT_COUNT);
    p = push_command(p, NV097_SET_LIGHT_LOCAL_POSITION + light_index*3*4, 3);
    return push_floats(p, v.f, 3);
}

inline
uint32_t* xgu_set_light_local_attenuation(uint32_t* p, unsigned int light_index, XguVec3 v) {
    assert(light_index >= 0);
    assert(light_index < XGU_LIGHT_COUNT);
    p = push_command(p, NV097_SET_LIGHT_LOCAL_ATTENUATION + light_index*3*4, 3);
    return push_floats(p, v.f, 3);
}

/* ==== Direct Mode stuff ==== */

inline
uint32_t* xgu_set_vertex3f(uint32_t* p, XguVec3 v) {
    p = push_command(p, NV097_SET_VERTEX3F, 3);
    return push_floats(p, v.f, 3);
}

inline
uint32_t* xgu_set_vertex4f(uint32_t* p, XguVec4 v) {
    p = push_command(p, NV097_SET_VERTEX4F, 4);
    return push_floats(p, v.f, 4);
}



#endif

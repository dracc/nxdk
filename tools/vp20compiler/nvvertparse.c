/*
 * Mesa 3-D graphics library
 * Version:  6.5.2
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file nvvertparse.c
 * NVIDIA vertex program parser.
 * \author Brian Paul
 */

/*
 * Regarding GL_NV_vertex_program, GL_NV_vertex_program1_1:
 *
 * Portions of this software may use or implement intellectual
 * property owned and licensed by NVIDIA Corporation. NVIDIA disclaims
 * any and all warranties with respect to such intellectual property,
 * including any use thereof or modifications thereto.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// #include "main/glheader.h"
// #include "main/context.h"
// #include "main/imports.h"
// #include "main/nvprogram.h"
#include "nvvertparse.h"
#include "prog_instruction.h"
// #include "prog_parameter.h"
// #include "prog_print.h"
// #include "program.h"

#include "mtypes.h"
#include "config.h"

/**
 * Current parsing state.  This structure is passed among the parsing
 * functions and keeps track of the current parser position and various
 * program attributes.
 */
struct parse_state {
   // struct gl_context *ctx;
   const unsigned char *start;
   const unsigned char *pos;
   const unsigned char *curLine;
   bool isStateProgram;
   bool isPositionInvariant;
   bool isVersion1_1;
   unsigned int inputsRead;
   unsigned int outputsWritten;
   bool anyProgRegsWritten;
   bool indirectRegisterFiles;
   unsigned int numInst;                 /* number of instructions parsed */
};

/**
 * Find the line number and column for 'pos' within 'string'.
 * Return a copy of the line which contains 'pos'.  Free the line with
 * free().
 * \param string  the program string
 * \param pos     the position within the string
 * \param line    returns the line number corresponding to 'pos'.
 * \param col     returns the column number corresponding to 'pos'.
 * \return copy of the line containing 'pos'.
 */
const unsigned char *
_mesa_find_line_column(const unsigned char *string, const unsigned char *pos,
                       int *line, int *col)
{
   const unsigned char *lineStart = string;
   const unsigned char *p = string;
   unsigned char *s;
   int len;
   *line = 1;
   while (p != pos) {
      if (*p == (unsigned char) '\n') {
         (*line)++;
         lineStart = p + 1;
      }
      p++;
   }
   *col = (pos - lineStart) + 1;
   /* return copy of this line */
   while (*p != 0 && *p != '\n')
      p++;
   len = p - lineStart;
   s = (unsigned char *) malloc(len + 1);
   memcpy(s, lineStart, len);
   s[len] = 0;
   return s;
}


/*
 * Called whenever we find an error during parsing.
 */
static void
record_error(struct parse_state *parseState, const char *msg, const char* file,
             int lineNo, bool onlyWarn)
{
   int line, column;
   const unsigned char *lineStr;
   lineStr = _mesa_find_line_column(parseState->start,
                                    parseState->pos, &line, &column);
   // _mesa_debug(parseState->ctx,
   fprintf(stderr,
               "%s: line %d, column %d: %s (%s)\n",
               onlyWarn ? "warning" : "error",
               line, column, (char *) lineStr, msg);
   free((void *) lineStr);

   /* Check that no error was already recorded.  Only record the first one. */
   // if (parseState->ctx->Program.ErrorString[0] == 0) {
   //    _mesa_set_program_error(parseState->ctx,
   //                            parseState->pos - parseState->start,
   //                            msg);
   // }
   if (!onlyWarn) {
      exit(1);
   }
}

#define WARNING1(msg)                                                        \
do {                                                                         \
   record_error(parseState, msg, __FILE__, __LINE__, TRUE);                  \
} while(0)

#define RETURN_ERROR                                                         \
do {                                                                         \
   record_error(parseState, "Unexpected end of input", __FILE__, __LINE__,   \
                FALSE);                                                      \
   return FALSE;                                                             \
} while(0)

#define RETURN_ERROR1(msg)                                                   \
do {                                                                         \
   record_error(parseState, msg, __FILE__, __LINE__, FALSE);                 \
   return FALSE;                                                             \
} while(0)

#define RETURN_ERROR2(msg1, msg2)                                            \
do {                                                                         \
   char err[1000];                                                           \
   sprintf(err, "%s %s", msg1, msg2);                                        \
   record_error(parseState, err, __FILE__, __LINE__, FALSE);                 \
   return FALSE;                                                             \
} while(0)

#define EXPECT(string)                                                       \
do {                                                                         \
   if (!Parse_String(parseState, string))                                    \
      RETURN_ERROR2("Expected", string);                                     \
} while(0)


static bool IsLetter(unsigned char b)
{
   return (b >= 'a' && b <= 'z') || (b >= 'A' && b <= 'Z');
}


static bool IsDigit(unsigned char b)
{
   return b >= '0' && b <= '9';
}


static bool IsWhitespace(unsigned char b)
{
   return b == ' ' || b == '\t' || b == '\n' || b == '\r';
}


/**
 * Starting at 'str' find the next token.  A token can be an integer,
 * an identifier or punctuation symbol.
 * \return <= 0 we found an error, else, return number of characters parsed.
 */
static int
GetToken(struct parse_state *parseState, unsigned char *token)
{
   const unsigned char *str = parseState->pos;
   int i = 0, j = 0;

   token[0] = 0;

   /* skip whitespace and comments */
   while (str[i] && (IsWhitespace(str[i]) || str[i] == '#')) {
      if (str[i] == '#') {
         /* skip comment */
         while (str[i] && (str[i] != '\n' && str[i] != '\r')) {
            i++;
         }
         if (str[i] == '\n' || str[i] == '\r')
            parseState->curLine = str + i + 1;
      }
      else {
         /* skip whitespace */
         if (str[i] == '\n' || str[i] == '\r')
            parseState->curLine = str + i + 1;
         i++;
      }
   }

   if (str[i] == 0)
      return -i;

   /* try matching an integer */
   while (str[i] && IsDigit(str[i])) {
      token[j++] = str[i++];
   }
   if (j > 0 || !str[i]) {
      token[j] = 0;
      return i;
   }

   /* try matching an identifier */
   if (IsLetter(str[i])) {
      while (str[i] && (IsLetter(str[i]) || IsDigit(str[i]))) {
         token[j++] = str[i++];
      }
      token[j] = 0;
      return i;
   }

   /* punctuation character */
   if (str[i]) {
      token[0] = str[i++];
      token[1] = 0;
      return i;
   }

   /* end of input */
   token[0] = 0;
   return i;
}


/**
 * Get next token from input stream and increment stream pointer past token.
 */
static bool
Parse_Token(struct parse_state *parseState, unsigned char *token)
{
   int i;
   i = GetToken(parseState, token);
   if (i <= 0) {
      parseState->pos += (-i);
      return FALSE;
   }
   parseState->pos += i;
   return TRUE;
}


/**
 * Get next token from input stream but don't increment stream pointer.
 */
static bool
Peek_Token(struct parse_state *parseState, unsigned char *token)
{
   int i, len;
   i = GetToken(parseState, token);
   if (i <= 0) {
      parseState->pos += (-i);
      return FALSE;
   }
   len = (int) strlen((const char *) token);
   parseState->pos += (i - len);
   return TRUE;
}


/**
 * Try to match 'pattern' as the next token after any whitespace/comments.
 * Advance the current parsing position only if we match the pattern.
 * \return TRUE if pattern is matched, FALSE otherwise.
 */
static bool
Parse_String(struct parse_state *parseState, const char *pattern)
{
   const unsigned char *m;
   int i;

   /* skip whitespace and comments */
   while (IsWhitespace(*parseState->pos) || *parseState->pos == '#') {
      if (*parseState->pos == '#') {
         while (*parseState->pos && (*parseState->pos != '\n' && *parseState->pos != '\r')) {
            parseState->pos += 1;
         }
         if (*parseState->pos == '\n' || *parseState->pos == '\r')
            parseState->curLine = parseState->pos + 1;
      }
      else {
         /* skip whitespace */
         if (*parseState->pos == '\n' || *parseState->pos == '\r')
            parseState->curLine = parseState->pos + 1;
         parseState->pos += 1;
      }
   }

   /* Try to match the pattern */
   m = parseState->pos;
   for (i = 0; pattern[i]; i++) {
      if (*m != (unsigned char) pattern[i])
         return FALSE;
      m += 1;
   }
   parseState->pos = m;

   return TRUE; /* success */
}


/**********************************************************************/

/* as defined in NV_vertex_program */
static const char *InputRegisters[MAX_NV_VERTEX_PROGRAM_INPUTS + 1] = {
   "OPOS", "WGHT", "NRML", "COL0", "COL1", "FOGC", "6", "7",
   "TEX0", "TEX1", "TEX2", "TEX3", "TEX4", "TEX5", "TEX6", "TEX7", NULL
};

/* as implemented in NV2A */
static const char *HardwareInputRegisters[MAX_HARDWARE_INPUTS + 1] = {
   "OPOS", "WGHT", "NRML", "COL0", "COL1", "FOGC",
   "PSIZ", "BFC0", "BFC1",
   "TEX0", "TEX1", "TEX2", "TEX3", "13", "14", "15", NULL
};

/* as defined in NV_vertex_program */
static const char *OutputRegisters[MAX_NV_VERTEX_PROGRAM_OUTPUTS + 1] = {
   "HPOS", "COL0", "COL1", "FOGC", 
   "TEX0", "TEX1", "TEX2", "TEX3", "TEX4", "TEX5", "TEX6", "TEX7", 
   "PSIZ", "BFC0", "BFC1", NULL
};

/* as implemented in NV2A */
static const char *HardwareOutputRegisters[MAX_HARDWARE_OUTPUTS + 1] = {
   "HPOS", "1", "2", "COL0", "COL1", "FOGC",
   "PSIZ", "BFC0", "BFC1",
   "TEX0", "TEX1", "TEX2", "TEX3",
   "13", "14", NULL
};



/**
 * Parse a temporary register: Rnn
 */
static bool
Parse_TempReg(struct parse_state *parseState, int *tempRegNum)
{
   unsigned char token[100];

   /* Should be 'R##' */
   if (!Parse_Token(parseState, token))
      RETURN_ERROR;
   if (token[0] != 'R')
      RETURN_ERROR1("Expected R##");

   if (IsDigit(token[1])) {
      int reg = atoi((char *) (token + 1));
      if (reg >= MAX_NV_VERTEX_PROGRAM_TEMPS)
         RETURN_ERROR1("Bad temporary register name");
      *tempRegNum = reg;
   }
   else {
      RETURN_ERROR1("Bad temporary register name");
   }

   return TRUE;
}


/**
 * Parse address register "A0.x"
 */
static bool
Parse_AddrReg(struct parse_state *parseState)
{
   /* match 'A0' */
   if (!Parse_String(parseState, "A0"))
      RETURN_ERROR;

   /* match '.' */
   EXPECT(".");

   /* match 'x' */
   if (!Parse_String(parseState, "x"))
      RETURN_ERROR;

   return TRUE;
}


/**
 * Parse absolute program parameter register "c[##]"
 */
static bool
Parse_AbsParamReg(struct parse_state *parseState, int *regNum)
{
   unsigned char token[100];

   EXPECT("c");

   EXPECT("[");

   if (!Parse_Token(parseState, token))
      RETURN_ERROR;

   if (IsDigit(token[0])) {
      /* a numbered program parameter register */
      int reg = atoi((char *) token);
      if (reg >= MAX_NV_VERTEX_PROGRAM_PARAMS)
         RETURN_ERROR1("Bad program parameter number");
      *regNum = reg;
   }
   else {
      RETURN_ERROR;
   }

   EXPECT("]");

   return TRUE;
}


static bool
Parse_ParamReg(struct parse_state *parseState, struct prog_src_register *srcReg)
{
   unsigned char token[100];

   EXPECT("c");

   EXPECT("[");

   if (!Peek_Token(parseState, token))
      RETURN_ERROR;

   if (IsDigit(token[0])) {
      /* a numbered program parameter register */
      int reg;
      (void) Parse_Token(parseState, token);
      reg = atoi((char *) token);
      if (reg >= MAX_NV_VERTEX_PROGRAM_PARAMS)
         RETURN_ERROR1("Bad program parameter number");
      srcReg->File = PROGRAM_ENV_PARAM;
      srcReg->Index = reg;
   }
   else if (strcmp((const char *) token, "A0") == 0) {
      /* address register "A0.x" */
      if (!Parse_AddrReg(parseState))
         RETURN_ERROR;

      srcReg->RelAddr = TRUE;
      srcReg->File = PROGRAM_ENV_PARAM;
      parseState->indirectRegisterFiles |= (1 << srcReg->File);
      /* Look for +/-N offset */
      if (!Peek_Token(parseState, token))
         RETURN_ERROR;

      if (token[0] == '-' || token[0] == '+') {
         const unsigned char sign = token[0];
         (void) Parse_Token(parseState, token); /* consume +/- */

         /* an integer should be next */
         if (!Parse_Token(parseState, token))
            RETURN_ERROR;

         if (IsDigit(token[0])) {
            const int k = atoi((char *) token);
            if (sign == '-') {
               if (k > 64)
                  RETURN_ERROR1("Bad address offset");
               srcReg->Index = -k;
            }
            else {
               if (k > 63)
                  RETURN_ERROR1("Bad address offset");
               srcReg->Index = k;
            }
         }
         else {
            RETURN_ERROR;
         }
      }
      else {
         /* probably got a ']', catch it below */
      }
   }
   else {
      RETURN_ERROR;
   }

   /* Match closing ']' */
   EXPECT("]");

   return TRUE;
}


/**
 * Parse v[#] or v[<name>]
 */
static bool
Parse_AttribReg(struct parse_state *parseState, int *tempRegNum)
{
   unsigned char token[100];
   int j;

   /* Match 'v' */
   EXPECT("v");

   /* Match '[' */
   EXPECT("[");

   /* match number or named register */
   if (!Parse_Token(parseState, token))
      RETURN_ERROR;

   if (IsDigit(token[0])) {
      int reg = atoi((char *) token);
      if (reg >= MAX_NV_VERTEX_PROGRAM_INPUTS)
         RETURN_ERROR1("Bad vertex attribute register name");

      if (parseState->isStateProgram && reg != 0)
         RETURN_ERROR1("Vertex state programs can only access vertex attribute register v[0]");

      *tempRegNum = reg;
   }
   else {

      if (parseState->isStateProgram)
         RETURN_ERROR1("Vertex state programs can only access vertex attribute registers by index");

      for (j = 0; InputRegisters[j]; j++) {
         if (strcmp((const char *) token, InputRegisters[j]) == 0) {
            *tempRegNum = j;
            break;
         }
      }
      if (!InputRegisters[j]) {
         /* unknown input register label */
         RETURN_ERROR2("Bad register name", token);
      }
   }

   /* Match '[' */
   EXPECT("]");

   /* make sure this register is available on hardware */
   const char *name = InputRegisters[*tempRegNum];
   for (j = 0; HardwareInputRegisters[j]; j++) {
      if (strcmp(name, HardwareInputRegisters[j]) == 0) {
         break;
      }
   }
   if (!HardwareInputRegisters[j]) {
      char msg[1000];
      sprintf(msg, "Vertex attribute register v[%d] (%s) not available on hardware",
              *tempRegNum, name);
      RETURN_ERROR1(msg);
   }
   else if (j != *tempRegNum) {
      char msg[1000];
      sprintf(msg, "Vertex attribute register v[%d] (%s) will be mapped to hardware register v[%d]",
              *tempRegNum, name, j);
      WARNING1(msg);
   }
   return TRUE;
}


static bool
Parse_OutputReg(struct parse_state *parseState, int *outputRegNum)
{
   unsigned char token[100];
   int start, j;

   /* Match 'o' */
   EXPECT("o");

   /* Match '[' */
   EXPECT("[");

   /* Get output reg name */
   if (!Parse_Token(parseState, token))
      RETURN_ERROR;

   if (parseState->isPositionInvariant)
      start = 1; /* skip HPOS register name */
   else
      start = 0;

   /* try to match an output register name */
   for (j = start; OutputRegisters[j]; j++) {
      if (strcmp((const char *) token, OutputRegisters[j]) == 0) {
         *outputRegNum = j;
         break;
      }
   }
   if (!OutputRegisters[j])
      RETURN_ERROR1("Unrecognized output register name");

   /* Match ']' */
   EXPECT("]");

   /* make sure this register is available on hardware */
   for (j = 0; HardwareOutputRegisters[j]; j++) {
      if (strcmp((const char *) token, HardwareOutputRegisters[j]) == 0) {
         break;
      }
   }
   if (!HardwareOutputRegisters[j]) {
      char msg[1000];
      sprintf(msg, "Output register o[%s] not available in hardware", token);
      WARNING1(msg);
   }

   return TRUE;
}


static bool
Parse_MaskedDstReg(struct parse_state *parseState, struct prog_dst_register *dstReg)
{
   unsigned char token[100];
   int idx;

   /* Dst reg can be R<n> or o[n] */
   if (!Peek_Token(parseState, token))
      RETURN_ERROR;

   if (token[0] == 'R') {
      /* a temporary register */
      dstReg->File = PROGRAM_TEMPORARY;
      if (!Parse_TempReg(parseState, &idx))
         RETURN_ERROR;
      dstReg->Index = idx;
   }
   else if (!parseState->isStateProgram && token[0] == 'o') {
      /* an output register */
      dstReg->File = PROGRAM_OUTPUT;
      if (!Parse_OutputReg(parseState, &idx))
         RETURN_ERROR;
      dstReg->Index = idx;
   }
   else if (parseState->isStateProgram && token[0] == 'c' &&
            parseState->isStateProgram) {
      /* absolute program parameter register */
      /* Only valid for vertex state programs */
      dstReg->File = PROGRAM_ENV_PARAM;
      if (!Parse_AbsParamReg(parseState, &idx))
         RETURN_ERROR;
      dstReg->Index = idx;
   }
   else {
      RETURN_ERROR1("Bad destination register name");
   }

   /* Parse optional write mask */
   if (!Peek_Token(parseState, token))
      RETURN_ERROR;

   if (token[0] == '.') {
      /* got a mask */
      int k = 0;

      EXPECT(".");

      if (!Parse_Token(parseState, token))
         RETURN_ERROR;

      dstReg->WriteMask = 0;

      if (token[k] == 'x') {
         dstReg->WriteMask |= WRITEMASK_X;
         k++;
      }
      if (token[k] == 'y') {
         dstReg->WriteMask |= WRITEMASK_Y;
         k++;
      }
      if (token[k] == 'z') {
         dstReg->WriteMask |= WRITEMASK_Z;
         k++;
      }
      if (token[k] == 'w') {
         dstReg->WriteMask |= WRITEMASK_W;
         k++;
      }
      if (k == 0) {
         RETURN_ERROR1("Bad writemask character");
      }
      return TRUE;
   }
   else {
      dstReg->WriteMask = WRITEMASK_XYZW;
      return TRUE;
   }
}


static bool
Parse_SwizzleSrcReg(struct parse_state *parseState, struct prog_src_register *srcReg)
{
   unsigned char token[100];
   int idx;

   srcReg->RelAddr = FALSE;

   /* check for '-' */
   if (!Peek_Token(parseState, token))
      RETURN_ERROR;
   if (token[0] == '-') {
      (void) Parse_String(parseState, "-");
      srcReg->Negate = NEGATE_XYZW;
      if (!Peek_Token(parseState, token))
         RETURN_ERROR;
   }
   else {
      srcReg->Negate = NEGATE_NONE;
   }

   /* Src reg can be R<n>, c[n], c[n +/- offset], or a named vertex attrib */
   if (token[0] == 'R') {
      srcReg->File = PROGRAM_TEMPORARY;
      if (!Parse_TempReg(parseState, &idx))
         RETURN_ERROR;
      srcReg->Index = idx;
   }
   else if (token[0] == 'c') {
      if (!Parse_ParamReg(parseState, srcReg))
         RETURN_ERROR;
   }
   else if (token[0] == 'v') {
      srcReg->File = PROGRAM_INPUT;
      if (!Parse_AttribReg(parseState, &idx))
         RETURN_ERROR;
      srcReg->Index = idx;
   }
   else {
      RETURN_ERROR2("Bad source register name", token);
   }

   /* init swizzle fields */
   srcReg->Swizzle = SWIZZLE_NOOP;

   /* Look for optional swizzle suffix */
   if (!Peek_Token(parseState, token))
      RETURN_ERROR;
   if (token[0] == '.') {
      (void) Parse_String(parseState, ".");  /* consume . */

      if (!Parse_Token(parseState, token))
         RETURN_ERROR;

      if (token[1] == 0) {
         /* single letter swizzle */
         if (token[0] == 'x')
            srcReg->Swizzle = SWIZZLE_XXXX;
         else if (token[0] == 'y')
            srcReg->Swizzle = SWIZZLE_YYYY;
         else if (token[0] == 'z')
            srcReg->Swizzle = SWIZZLE_ZZZZ;
         else if (token[0] == 'w')
            srcReg->Swizzle = SWIZZLE_WWWW;
         else
            RETURN_ERROR1("Expected x, y, z, or w");
      }
      else {
         /* 2, 3 or 4-component swizzle */
         int k;

         srcReg->Swizzle = 0;

         for (k = 0; token[k] && k < 5; k++) {
            if (token[k] == 'x')
               srcReg->Swizzle |= 0 << (k*3);
            else if (token[k] == 'y')
               srcReg->Swizzle |= 1 << (k*3);
            else if (token[k] == 'z')
               srcReg->Swizzle |= 2 << (k*3);
            else if (token[k] == 'w')
               srcReg->Swizzle |= 3 << (k*3);
            else
               RETURN_ERROR;
         }
         if (k >= 5)
            RETURN_ERROR;
      }
   }

   return TRUE;
}


static bool
Parse_ScalarSrcReg(struct parse_state *parseState, struct prog_src_register *srcReg)
{
   unsigned char token[100];
   int idx;

   srcReg->RelAddr = FALSE;

   /* check for '-' */
   if (!Peek_Token(parseState, token))
      RETURN_ERROR;
   if (token[0] == '-') {
      srcReg->Negate = NEGATE_XYZW;
      (void) Parse_String(parseState, "-"); /* consume '-' */
      if (!Peek_Token(parseState, token))
         RETURN_ERROR;
   }
   else {
      srcReg->Negate = NEGATE_NONE;
   }

   /* Src reg can be R<n>, c[n], c[n +/- offset], or a named vertex attrib */
   if (token[0] == 'R') {
      srcReg->File = PROGRAM_TEMPORARY;
      if (!Parse_TempReg(parseState, &idx))
         RETURN_ERROR;
      srcReg->Index = idx;
   }
   else if (token[0] == 'c') {
      if (!Parse_ParamReg(parseState, srcReg))
         RETURN_ERROR;
   }
   else if (token[0] == 'v') {
      srcReg->File = PROGRAM_INPUT;
      if (!Parse_AttribReg(parseState, &idx))
         RETURN_ERROR;
      srcReg->Index = idx;
   }
   else {
      RETURN_ERROR2("Bad source register name", token);
   }

   /* Look for .[xyzw] suffix */
   EXPECT(".");

   if (!Parse_Token(parseState, token))
      RETURN_ERROR;

   if (token[0] == 'x' && token[1] == 0) {
      srcReg->Swizzle = 0;
   }
   else if (token[0] == 'y' && token[1] == 0) {
      srcReg->Swizzle = 1;
   }
   else if (token[0] == 'z' && token[1] == 0) {
      srcReg->Swizzle = 2;
   }
   else if (token[0] == 'w' && token[1] == 0) {
      srcReg->Swizzle = 3;
   }
   else {
      RETURN_ERROR1("Bad scalar source suffix");
   }

   return TRUE;
}


static int
Parse_UnaryOpInstruction(struct parse_state *parseState,
                         struct prog_instruction *inst,
                         enum prog_opcode opcode)
{
   if (opcode == OPCODE_ABS && !parseState->isVersion1_1)
      RETURN_ERROR1("ABS requires vertex program 1.1");

   inst->Opcode = opcode;

   /* dest reg */
   if (!Parse_MaskedDstReg(parseState, &inst->DstReg))
      RETURN_ERROR;

   /* comma */
   EXPECT(",");

   /* src arg */
   if (!Parse_SwizzleSrcReg(parseState, &inst->SrcReg[0]))
      RETURN_ERROR;

   /* semicolon */
   EXPECT(";");

   return TRUE;
}


static bool
Parse_BiOpInstruction(struct parse_state *parseState,
                      struct prog_instruction *inst,
                      enum prog_opcode opcode)
{
   if (opcode == OPCODE_DPH && !parseState->isVersion1_1)
      RETURN_ERROR1("DPH requires vertex program 1.1");
   if (opcode == OPCODE_SUB && !parseState->isVersion1_1)
      RETURN_ERROR1("SUB requires vertex program 1.1");

   inst->Opcode = opcode;

   /* dest reg */
   if (!Parse_MaskedDstReg(parseState, &inst->DstReg))
      RETURN_ERROR;

   /* comma */
   EXPECT(",");

   /* first src arg */
   if (!Parse_SwizzleSrcReg(parseState, &inst->SrcReg[0]))
      RETURN_ERROR;

   /* comma */
   EXPECT(",");

   /* second src arg */
   if (!Parse_SwizzleSrcReg(parseState, &inst->SrcReg[1]))
      RETURN_ERROR;

   /* semicolon */
   EXPECT(";");

   /* make sure we don't reference more than one program parameter register */
   if (inst->SrcReg[0].File == PROGRAM_ENV_PARAM &&
       inst->SrcReg[1].File == PROGRAM_ENV_PARAM &&
       inst->SrcReg[0].Index != inst->SrcReg[1].Index)
      RETURN_ERROR1("Can't reference two program parameter registers");

   /* make sure we don't reference more than one vertex attribute register */
   if (inst->SrcReg[0].File == PROGRAM_INPUT &&
       inst->SrcReg[1].File == PROGRAM_INPUT &&
       inst->SrcReg[0].Index != inst->SrcReg[1].Index)
      RETURN_ERROR1("Can't reference two vertex attribute registers");

   return TRUE;
}


static bool
Parse_TriOpInstruction(struct parse_state *parseState,
                       struct prog_instruction *inst,
                       enum prog_opcode opcode)
{
   inst->Opcode = opcode;

   /* dest reg */
   if (!Parse_MaskedDstReg(parseState, &inst->DstReg))
      RETURN_ERROR;

   /* comma */
   EXPECT(",");

   /* first src arg */
   if (!Parse_SwizzleSrcReg(parseState, &inst->SrcReg[0]))
      RETURN_ERROR;

   /* comma */
   EXPECT(",");

   /* second src arg */
   if (!Parse_SwizzleSrcReg(parseState, &inst->SrcReg[1]))
      RETURN_ERROR;

   /* comma */
   EXPECT(",");

   /* third src arg */
   if (!Parse_SwizzleSrcReg(parseState, &inst->SrcReg[2]))
      RETURN_ERROR;

   /* semicolon */
   EXPECT(";");

   /* make sure we don't reference more than one program parameter register */
   if ((inst->SrcReg[0].File == PROGRAM_ENV_PARAM &&
        inst->SrcReg[1].File == PROGRAM_ENV_PARAM &&
        inst->SrcReg[0].Index != inst->SrcReg[1].Index) ||
       (inst->SrcReg[0].File == PROGRAM_ENV_PARAM &&
        inst->SrcReg[2].File == PROGRAM_ENV_PARAM &&
        inst->SrcReg[0].Index != inst->SrcReg[2].Index) ||
       (inst->SrcReg[1].File == PROGRAM_ENV_PARAM &&
        inst->SrcReg[2].File == PROGRAM_ENV_PARAM &&
        inst->SrcReg[1].Index != inst->SrcReg[2].Index))
      RETURN_ERROR1("Can only reference one program register");

   /* make sure we don't reference more than one vertex attribute register */
   if ((inst->SrcReg[0].File == PROGRAM_INPUT &&
        inst->SrcReg[1].File == PROGRAM_INPUT &&
        inst->SrcReg[0].Index != inst->SrcReg[1].Index) ||
       (inst->SrcReg[0].File == PROGRAM_INPUT &&
        inst->SrcReg[2].File == PROGRAM_INPUT &&
        inst->SrcReg[0].Index != inst->SrcReg[2].Index) ||
       (inst->SrcReg[1].File == PROGRAM_INPUT &&
        inst->SrcReg[2].File == PROGRAM_INPUT &&
        inst->SrcReg[1].Index != inst->SrcReg[2].Index))
      RETURN_ERROR1("Can only reference one input register");

   return TRUE;
}


static bool
Parse_ScalarInstruction(struct parse_state *parseState,
                        struct prog_instruction *inst,
                        enum prog_opcode opcode)
{
   if (opcode == OPCODE_RCC && !parseState->isVersion1_1)
      RETURN_ERROR1("RCC requires vertex program 1.1");

   inst->Opcode = opcode;

   /* dest reg */
   if (!Parse_MaskedDstReg(parseState, &inst->DstReg))
      RETURN_ERROR;

   /* comma */
   EXPECT(",");

   /* first src arg */
   if (!Parse_ScalarSrcReg(parseState, &inst->SrcReg[0]))
      RETURN_ERROR;

   /* semicolon */
   EXPECT(";");

   return TRUE;
}


static bool
Parse_AddressInstruction(struct parse_state *parseState, struct prog_instruction *inst)
{
   inst->Opcode = OPCODE_ARL;

   /* Make ARB_vp backends happy */
   inst->DstReg.File = PROGRAM_ADDRESS;
   inst->DstReg.WriteMask = WRITEMASK_X;
   inst->DstReg.Index = 0;

   /* dest A0 reg */
   if (!Parse_AddrReg(parseState))
      RETURN_ERROR;

   /* comma */
   EXPECT(",");

   /* parse src reg */
   if (!Parse_ScalarSrcReg(parseState, &inst->SrcReg[0]))
      RETURN_ERROR;

   /* semicolon */
   EXPECT(";");

   return TRUE;
}


static bool
Parse_EndInstruction(struct parse_state *parseState, struct prog_instruction *inst)
{
   unsigned char token[100];

   inst->Opcode = OPCODE_END;

   /* this should fail! */
   if (Parse_Token(parseState, token))
      RETURN_ERROR2("Unexpected token after END:", token);
   else
      return TRUE;
}


/**
 * The PRINT instruction is Mesa-specific and is meant as a debugging aid for
 * the vertex program developer.
 * The NV_vertex_program extension grammar is modified as follows:
 *
 *  <instruction>        ::= <ARL-instruction>
 *                         | ...
 *                         | <PRINT-instruction>
 *
 *  <PRINT-instruction>  ::= "PRINT" <string literal>
 *                         | "PRINT" <string literal> "," <srcReg>
 *                         | "PRINT" <string literal> "," <dstReg>
 */
static bool
Parse_PrintInstruction(struct parse_state *parseState, struct prog_instruction *inst)
{
   const unsigned char *str;
   unsigned char *msg;
   unsigned int len;
   unsigned char token[100];
   struct prog_src_register *srcReg = &inst->SrcReg[0];
   int idx;

   inst->Opcode = OPCODE_PRINT;

   /* The first argument is a literal string 'just like this' */
   EXPECT("'");

   str = parseState->pos;
   for (len = 0; str[len] != '\''; len++) /* find closing quote */
      ;
   parseState->pos += len + 1;
   msg = malloc(len + 1);

   memcpy(msg, str, len);
   msg[len] = 0;
   inst->Data = msg;

   /* comma */
   if (Parse_String(parseState, ",")) {

      /* The second argument is a register name */
      if (!Peek_Token(parseState, token))
         RETURN_ERROR;

      srcReg->RelAddr = FALSE;
      srcReg->Negate = NEGATE_NONE;
      srcReg->Swizzle = SWIZZLE_NOOP;

      /* Register can be R<n>, c[n], c[n +/- offset], a named vertex attrib,
       * or an o[n] output register.
       */
      if (token[0] == 'R') {
         srcReg->File = PROGRAM_TEMPORARY;
         if (!Parse_TempReg(parseState, &idx))
            RETURN_ERROR;
         srcReg->Index = idx;
      }
      else if (token[0] == 'c') {
         srcReg->File = PROGRAM_ENV_PARAM;
         if (!Parse_ParamReg(parseState, srcReg))
            RETURN_ERROR;
      }
      else if (token[0] == 'v') {
         srcReg->File = PROGRAM_INPUT;
         if (!Parse_AttribReg(parseState, &idx))
            RETURN_ERROR;
         srcReg->Index = idx;
      }
      else if (token[0] == 'o') {
         srcReg->File = PROGRAM_OUTPUT;
         if (!Parse_OutputReg(parseState, &idx))
            RETURN_ERROR;
         srcReg->Index = idx;
      }
      else {
         RETURN_ERROR2("Bad source register name", token);
      }
   }
   else {
      srcReg->File = PROGRAM_UNDEFINED;
   }

   /* semicolon */
   EXPECT(";");

   return TRUE;
}


static bool
Parse_OptionSequence(struct parse_state *parseState,
                     struct prog_instruction program[])
{
   (void) program;
   while (1) {
      if (!Parse_String(parseState, "OPTION"))
         return TRUE;  /* ok, not an OPTION statement */
      if (Parse_String(parseState, "NV_position_invariant")) {
         parseState->isPositionInvariant = TRUE;
      }
      else {
         RETURN_ERROR1("unexpected OPTION statement");
      }
      EXPECT(";");
   }
}


static bool
Parse_InstructionSequence(struct parse_state *parseState,
                          struct prog_instruction program[])
{
   while (1) {
      struct prog_instruction *inst = program + parseState->numInst;

      /* Initialize the instruction */
      _mesa_init_instructions(inst, 1);

      if (Parse_String(parseState, "MOV")) {
         if (!Parse_UnaryOpInstruction(parseState, inst, OPCODE_MOV))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "LIT")) {
         if (!Parse_UnaryOpInstruction(parseState, inst, OPCODE_LIT))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "ABS")) {
         if (!Parse_UnaryOpInstruction(parseState, inst, OPCODE_ABS))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "MUL")) {
         if (!Parse_BiOpInstruction(parseState, inst, OPCODE_MUL))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "ADD")) {
         if (!Parse_BiOpInstruction(parseState, inst, OPCODE_ADD))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "DP3")) {
         if (!Parse_BiOpInstruction(parseState, inst, OPCODE_DP3))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "DP4")) {
         if (!Parse_BiOpInstruction(parseState, inst, OPCODE_DP4))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "DST")) {
         if (!Parse_BiOpInstruction(parseState, inst, OPCODE_DST))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "MIN")) {
         if (!Parse_BiOpInstruction(parseState, inst, OPCODE_MIN))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "MAX")) {
         if (!Parse_BiOpInstruction(parseState, inst, OPCODE_MAX))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "SLT")) {
         if (!Parse_BiOpInstruction(parseState, inst, OPCODE_SLT))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "SGE")) {
         if (!Parse_BiOpInstruction(parseState, inst, OPCODE_SGE))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "DPH")) {
         if (!Parse_BiOpInstruction(parseState, inst, OPCODE_DPH))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "SUB")) {
         if (!Parse_BiOpInstruction(parseState, inst, OPCODE_SUB))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "MAD")) {
         if (!Parse_TriOpInstruction(parseState, inst, OPCODE_MAD))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "RCP")) {
         if (!Parse_ScalarInstruction(parseState, inst, OPCODE_RCP))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "RSQ")) {
         if (!Parse_ScalarInstruction(parseState, inst, OPCODE_RSQ))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "EXP")) {
         if (!Parse_ScalarInstruction(parseState, inst, OPCODE_EXP))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "LOG")) {
         if (!Parse_ScalarInstruction(parseState, inst, OPCODE_LOG))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "RCC")) {
         if (!Parse_ScalarInstruction(parseState, inst, OPCODE_RCC))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "ARL")) {
         if (!Parse_AddressInstruction(parseState, inst))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "PRINT")) {
         if (!Parse_PrintInstruction(parseState, inst))
            RETURN_ERROR;
      }
      else if (Parse_String(parseState, "END")) {
         if (!Parse_EndInstruction(parseState, inst))
            RETURN_ERROR;
         else {
            parseState->numInst++;
            return TRUE;  /* all done */
         }
      }
      else {
         /* bad instruction name */
         RETURN_ERROR1("Unexpected token");
      }

      /* examine input/output registers */
      if (inst->DstReg.File == PROGRAM_OUTPUT)
         parseState->outputsWritten |= (1 << inst->DstReg.Index);
      else if (inst->DstReg.File == PROGRAM_ENV_PARAM)
         parseState->anyProgRegsWritten = TRUE;

      if (inst->SrcReg[0].File == PROGRAM_INPUT)
         parseState->inputsRead |= (1 << inst->SrcReg[0].Index);
      if (inst->SrcReg[1].File == PROGRAM_INPUT)
         parseState->inputsRead |= (1 << inst->SrcReg[1].Index);
      if (inst->SrcReg[2].File == PROGRAM_INPUT)
         parseState->inputsRead |= (1 << inst->SrcReg[2].Index);

      parseState->numInst++;

      if (parseState->numInst >= MAX_NV_VERTEX_PROGRAM_INSTRUCTIONS)
         RETURN_ERROR1("Program too long");
   }

   RETURN_ERROR;
}


static bool
Parse_Program(struct parse_state *parseState,
              struct prog_instruction instBuffer[])
{
   if (parseState->isVersion1_1) {
      if (!Parse_OptionSequence(parseState, instBuffer)) {
         return FALSE;
      }
   }
   return Parse_InstructionSequence(parseState, instBuffer);
}


int parse_nv_vertex_program(const char *str,
                            struct prog_instruction **out_instructions,
                            unsigned int *out_num_instructions)
{
   struct parse_state parseState;
   struct prog_instruction instBuffer[MAX_NV_VERTEX_PROGRAM_INSTRUCTIONS];
   struct prog_instruction *newInst;
   // GLenum target;
   
   unsigned char* programString = (unsigned char*)strdup(str);

   /* Get ready to parse */
   // parseState.ctx = ctx;
   parseState.start = programString;
   parseState.isPositionInvariant = FALSE;
   parseState.isVersion1_1 = FALSE;
   parseState.numInst = 0;
   parseState.inputsRead = 0;
   parseState.outputsWritten = 0;
   parseState.anyProgRegsWritten = FALSE;
   parseState.indirectRegisterFiles = 0x0;

   /* Reset error state */
   // _mesa_set_program_error(ctx, -1, NULL);

   /* check the program header */
   if (strncmp((const char *) programString, "!!VP1.0", 7) == 0) {
      // target = GL_VERTEX_PROGRAM_NV;
      parseState.pos = programString + 7;
      parseState.isStateProgram = FALSE;
   }
   else if (strncmp((const char *) programString, "!!VP1.1", 7) == 0) {
      // target = GL_VERTEX_PROGRAM_NV;
      parseState.pos = programString + 7;
      parseState.isStateProgram = FALSE;
      parseState.isVersion1_1 = TRUE;
   }
   else if (strncmp((const char *) programString, "!!VSP1.0", 8) == 0) {
      // target = GL_VERTEX_STATE_PROGRAM_NV;
      parseState.pos = programString + 8;
      parseState.isStateProgram = TRUE;
   }
   else {
      /* invalid header */
      fprintf(stderr, "bad header\n");
      return 1;
   }

   if (Parse_Program(&parseState, instBuffer)) {
      // gl_state_index state_tokens[STATE_LENGTH] = {0, 0, 0, 0, 0};
      int i;

      /* successful parse! */

      if (parseState.isStateProgram) {
         if (!parseState.anyProgRegsWritten) {
            // _mesa_error(ctx, GL_INVALID_OPERATION,
            //             "glLoadProgramNV(c[#] not written)");
            fprintf(stderr, "c[#] not written\n");
            return 1;
         }
      }
      else {
         if (!parseState.isPositionInvariant &&
             !(parseState.outputsWritten & (1 << VERT_RESULT_HPOS))) {
            /* bit 1 = HPOS register */
            fprintf(stderr, "HPOS not written\n");
            return 1;
         }
      }

      /* copy the compiled instructions */
      assert(parseState.numInst <= MAX_NV_VERTEX_PROGRAM_INSTRUCTIONS);
      newInst = _mesa_alloc_instructions(parseState.numInst);
      assert(newInst);
      _mesa_copy_instructions(newInst, instBuffer, parseState.numInst);

      *out_instructions = newInst;
      *out_num_instructions = parseState.numInst;

      return 0;
   }
   else {
      /* Error! */
      return 1;
   }
}

/**
 * Parse/compile the 'str' returning the compiled 'program'.
 * ctx->Program.ErrorPos will be -1 if successful.  Otherwise, ErrorPos
 * indicates the position of the error in 'str'.
 */
// void
// _mesa_parse_nv_vertex_program(struct gl_context *ctx, GLenum dstTarget,
//                               const unsigned char *str, GLsizei len,
//                               struct gl_vertex_program *program)
// {
//    struct parse_state parseState;
//    struct prog_instruction instBuffer[MAX_NV_VERTEX_PROGRAM_INSTRUCTIONS];
//    struct prog_instruction *newInst;
//    GLenum target;
//    unsigned char *programString;

//    /* Make a null-terminated copy of the program string */
//    programString = malloc(len + 1);
//    if (!programString) {
//       _mesa_error(ctx, GL_OUT_OF_MEMORY, "glLoadProgramNV");
//       return;
//    }
//    memcpy(programString, str, len);
//    programString[len] = 0;

//    /* Get ready to parse */
//    parseState.ctx = ctx;
//    parseState.start = programString;
//    parseState.isPositionInvariant = FALSE;
//    parseState.isVersion1_1 = FALSE;
//    parseState.numInst = 0;
//    parseState.inputsRead = 0;
//    parseState.outputsWritten = 0;
//    parseState.anyProgRegsWritten = FALSE;
//    parseState.indirectRegisterFiles = 0x0;

//    /* Reset error state */
//    _mesa_set_program_error(ctx, -1, NULL);

//    /* check the program header */
//    if (strncmp((const char *) programString, "!!VP1.0", 7) == 0) {
//       target = GL_VERTEX_PROGRAM_NV;
//       parseState.pos = programString + 7;
//       parseState.isStateProgram = FALSE;
//    }
//    else if (strncmp((const char *) programString, "!!VP1.1", 7) == 0) {
//       target = GL_VERTEX_PROGRAM_NV;
//       parseState.pos = programString + 7;
//       parseState.isStateProgram = FALSE;
//       parseState.isVersion1_1 = TRUE;
//    }
//    else if (strncmp((const char *) programString, "!!VSP1.0", 8) == 0) {
//       target = GL_VERTEX_STATE_PROGRAM_NV;
//       parseState.pos = programString + 8;
//       parseState.isStateProgram = TRUE;
//    }
//    else {
//       /* invalid header */
//       ctx->Program.ErrorPos = 0;
//       _mesa_error(ctx, GL_INVALID_OPERATION, "glLoadProgramNV(bad header)");
//       return;
//    }

//    /* make sure target and header match */
//    if (target != dstTarget) {
//       _mesa_error(ctx, GL_INVALID_OPERATION,
//                   "glLoadProgramNV(target mismatch)");
//       return;
//    }


//    if (Parse_Program(&parseState, instBuffer)) {
//       gl_state_index state_tokens[STATE_LENGTH] = {0, 0, 0, 0, 0};
//       int i;

//       /* successful parse! */

//       if (parseState.isStateProgram) {
//          if (!parseState.anyProgRegsWritten) {
//             _mesa_error(ctx, GL_INVALID_OPERATION,
//                         "glLoadProgramNV(c[#] not written)");
//             return;
//          }
//       }
//       else {
//          if (!parseState.isPositionInvariant &&
//              !(parseState.outputsWritten & (1 << VERT_RESULT_HPOS))) {
//             /* bit 1 = HPOS register */
//             _mesa_error(ctx, GL_INVALID_OPERATION,
//                         "glLoadProgramNV(HPOS not written)");
//             return;
//          }
//       }

//       /* copy the compiled instructions */
//       assert(parseState.numInst <= MAX_NV_VERTEX_PROGRAM_INSTRUCTIONS);
//       newInst = _mesa_alloc_instructions(parseState.numInst);
//       if (!newInst) {
//          _mesa_error(ctx, GL_OUT_OF_MEMORY, "glLoadProgramNV");
//          free(programString);
//          return;  /* out of memory */
//       }
//       _mesa_copy_instructions(newInst, instBuffer, parseState.numInst);

//       /* install the program */
//       program->Base.Target = target;
//       free(program->Base.String);
//       program->Base.String = programString;
//       program->Base.Format = GL_PROGRAM_FORMAT_ASCII_ARB;
//       free(program->Base.Instructions);
//       program->Base.Instructions = newInst;
//       program->Base.InputsRead = parseState.inputsRead;
//       if (parseState.isPositionInvariant)
//          program->Base.InputsRead |= VERT_BIT_POS;
//       program->Base.NumInstructions = parseState.numInst;
//       program->Base.OutputsWritten = parseState.outputsWritten;
//       program->IsPositionInvariant = parseState.isPositionInvariant;
//       program->IsNVProgram = TRUE;

// #ifdef DEBUG_foo
//       printf("--- glLoadProgramNV result ---\n");
//       _mesa_fprint_program_opt(stdout, &program->Base, PROG_PRINT_NV, 0);
//       printf("------------------------------\n");
// #endif

//       if (program->Base.Parameters)
//          _mesa_free_parameter_list(program->Base.Parameters);

//       program->Base.Parameters = _mesa_new_parameter_list ();
//       program->Base.NumParameters = 0;

//       program->Base.IndirectRegisterFiles = parseState.indirectRegisterFiles;

//       state_tokens[0] = STATE_VERTEX_PROGRAM;
//       state_tokens[1] = STATE_ENV;
//       /* Add refs to all of the potential params, in order.  If we want to not
//        * upload everything, _mesa_layout_parameters is the answer.
//        */
//       for (i = 0; i < MAX_NV_VERTEX_PROGRAM_PARAMS; i++) {
//          int index;
//          state_tokens[2] = i;
//          index = _mesa_add_state_reference(program->Base.Parameters,
//                                            state_tokens);
//          assert(index == i);
//          (void)index;
//       }
//       program->Base.NumParameters = program->Base.Parameters->NumParameters;

//       _mesa_setup_nv_temporary_count(&program->Base);
//       _mesa_emit_nv_temp_initialization(ctx, &program->Base);
//    }
//    else {
//       /* Error! */
//       _mesa_error(ctx, GL_INVALID_OPERATION, "glLoadProgramNV");
//       /* NOTE: _mesa_set_program_error would have been called already */
//       /* GL_NV_vertex_program isn't supposed to set the error string
//        * so we reset it here.
//        */
//       _mesa_set_program_error(ctx, ctx->Program.ErrorPos, NULL);
//    }
// }


const char *
_mesa_nv_vertex_input_register_name(unsigned int i)
{
   assert(i < MAX_NV_VERTEX_PROGRAM_INPUTS);
   return InputRegisters[i];
}

const char *
_mesa_nv_vertex_hw_input_register_name(unsigned int i)
{
   assert(i < MAX_HARDWARE_INPUTS);
   return HardwareInputRegisters[i];
}

const char *
_mesa_nv_vertex_output_register_name(unsigned int i)
{
   assert(i < MAX_NV_VERTEX_PROGRAM_OUTPUTS);
   return OutputRegisters[i];
}

const char *
_mesa_nv_vertex_hw_output_register_name(unsigned int i)
{
   assert(i < MAX_HARDWARE_OUTPUTS);
   return HardwareOutputRegisters[i];
}

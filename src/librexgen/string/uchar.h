/*
    rexgen - a tool to create words based on regular expressions
    Copyright (C) 2012-2013  Jan Starke <jan.starke@outofbed.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
    more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin St, Fifth Floor, Boston, MA 02110, USA
*/


#ifndef __uchar_h__
#define __uchar_h__

#include <stdint.h>
#include <stddef.h>
#include <librexgen/string/unicode.h>
#include <librexgen/osdepend.h>

typedef uint8_t charset;
#define CHARSET_ANSI    1
#define CHARSET_UTF8    2
#define CHARSET_UTF16BE 3
#define CHARSET_UTF32BE 4
#define CHARSET_UTF16LE 5
#define CHARSET_UTF32LE 6

#if ! defined(__cplusplus) || defined(_WIN32) || __cplusplus==1 || __cplusplus==199711L
typedef uint16_t char16_t;
typedef uint32_t char32_t;
#endif

typedef uint8_t uchar_flags_t;
typedef uint8_t uchar_info_t;

#define UCHAR_FLAGS_CHANGE_CASE         0x01
#define UCHAR_FLAGS_PRESERVE_CASE       0x02
#define UCHAR_FLAGS_USE_CASEFOLDED      0x04

#define UCHAR_CAN_CHANGE_CASE(a) (!((a).flags&UCHAR_FLAGS_PRESERVE_CASE))
#define UCHAR_MUST_CHANGE_CASE(a) ((a).flags&UCHAR_CHANGE_CASE && !((a).flags&UCHAR_PRESERVE_CASE))
#define UCHAR_MUST_PRESERVE_CASE(a) ((a).flags&UCHAR_FLAGS_PRESERVE_CASE)
#define UCHAR_SET_CHANGE_CASE(a) do {            \
  (a).flags |= UCHAR_FLAGS_CHANGE_CASE;          \
  (a).flags &= (! UCHAR_FLAGS_PRESERVE_CASE );   \
} while (0)
#define UCHAR_SET_PRESERVE_CASE(a) do {          \
  (a).flags &= (! UCHAR_FLAGS_CHANGE_CASE );     \
  (a).flags |= UCHAR_FLAGS_PRESERVE_CASE;        \
} while (0)

typedef enum {
  BMP   =  0,               /* Basic Multilingual Plane            */
  SMP   =  1,               /* Supplementary Multilingual Plane    */
  SIP   =  2,               /* Supplementary Ideographic Plane     */
  SSP   = 14,               /* Supplementary Special-purpose Plane */
  PUA_A = 15,               /* Supplementary Private Use Area-A    */
  PUA_B = 16                /* Supplementary Private Use Area-B    */
} unicode_plane_t;

/*
 * this datastructure is used to cache the binary representation 
 * of unicode characters. Because we use this cache, it is not
 * necessary to create the binary value during the output
 */
typedef union {
  byte    bytes[4];
  struct {
    uint8_t  pad[3];
    char     value;
  } ansi;
  struct {
    char16_t high;
    char16_t low;
  } ucs2;
  struct {
    char32_t value;
  } ucs4;
} binary_character_t;

const char32_t UCHAR_UNASSIGNED = 0xffffffff;

struct __uchar_t {
  charset         variant;    /* 1 byte */
  uint8_t         char_length;/* 1 byte */
  uchar_flags_t   flags;      /* 1 byte */
  unicode_plane_t plane;      /* 1 byte */

  /* this one is needed for simple iteration in in UTF8 and UTF16 */
  uint32_t codepoint;         /* 4 byte */

  binary_character_t character; /* 4 byte */
  binary_character_t casefolded[3]; /* 12 byte */
};
typedef struct __uchar_t uchar_t;

#ifdef __cplusplus
extern "C" {
EXPORT
bool uchar_isascii(const uchar_t& uch);

EXPORT
void uchar_toggle_case(uchar_t& uch);

}
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT
const byte* firstByteAddressOf(const uchar_t* c);

EXPORT
uchar_t char_to_uchar(char ch);

EXPORT
void codepoint_to_uchar(uchar_t* dst, uint32_t codepoint, charset cs);

EXPORT
uint8_t encode_codepoint(uint32_t codepoint, charset cs, binary_character_t* bch);

EXPORT
uint8_t uchar_to_ansi(const binary_character_t*, byte* ansi_dst);

EXPORT
uint8_t uchar_to_utf8(const binary_character_t*, byte* utf8_dst);

EXPORT
uint8_t uchar_to_utf16(const binary_character_t*, byte* utf16_dst);

EXPORT
uint8_t uchar_to_utf32(const binary_character_t*, byte* utf32_dst);

EXPORT
uint8_t uchar_to_binary(const uchar_t* uch, byte* dst);

EXPORT
uchar_t create_BOM(charset cs);

#ifdef __cplusplus
}
#endif

#endif

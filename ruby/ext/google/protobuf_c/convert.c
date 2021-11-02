// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// -----------------------------------------------------------------------------
// Ruby <-> upb data conversion functions.
//
// This file Also contains a few other assorted algorithms on upb_msgval.
//
// None of the algorithms in this file require any access to the internal
// representation of Ruby or upb objects.
// -----------------------------------------------------------------------------

#include "convert.h"

#include "message.h"
#include "protobuf.h"

/* Adapted from ABSL's wyhash. */

static uint64_t UnalignedLoad64(const void *p) {
  uint64_t val;
  memcpy(&val, p, 8);
  return val;
}

static uint32_t UnalignedLoad32(const void *p) {
  uint32_t val;
  memcpy(&val, p, 4);
  return val;
}

#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#endif

/* Computes a * b, returning the low 64 bits of the result and storing the high
 * 64 bits in |*high|. */
static uint64_t upb_umul128(uint64_t v0, uint64_t v1, uint64_t* out_high) {
#ifdef __SIZEOF_INT128__
  __uint128_t p = v0;
  p *= v1;
  *out_high = (uint64_t)(p >> 64);
  return (uint64_t)p;
#elif defined(_MSC_VER) && defined(_M_X64)
  return _umul128(v0, v1, out_high);
#else
  uint64_t a32 = v0 >> 32;
  uint64_t a00 = v0 & 0xffffffff;
  uint64_t b32 = v1 >> 32;
  uint64_t b00 = v1 & 0xffffffff;
  uint64_t high = a32 * b32;
  uint64_t low = a00 * b00;
  uint64_t mid1 = a32 * b00;
  uint64_t mid2 = a00 * b32;
  low += (mid1 << 32) + (mid2 << 32);
  // Omit carry bit, for mixing we do not care about exact numerical precision.
  high += (mid1 >> 32) + (mid2 >> 32);
  *out_high = high;
  return low;
#endif
}

static uint64_t WyhashMix(uint64_t v0, uint64_t v1) {
  uint64_t high;
  uint64_t low = upb_umul128(v0, v1, &high);
  return low ^ high;
}

uint64_t RubyWyhash(const void *data, size_t len, uint64_t seed,
                const uint64_t salt[]) {
  const uint8_t* ptr = (const uint8_t*)data;
  uint64_t starting_length = (uint64_t)len;
  uint64_t current_state = seed ^ salt[0];

  if (len > 64) {
    // If we have more than 64 bytes, we're going to handle chunks of 64
    // bytes at a time. We're going to build up two separate hash states
    // which we will then hash together.
    uint64_t duplicated_state = current_state;

    do {
      uint64_t a = UnalignedLoad64(ptr);
      uint64_t b = UnalignedLoad64(ptr + 8);
      uint64_t c = UnalignedLoad64(ptr + 16);
      uint64_t d = UnalignedLoad64(ptr + 24);
      uint64_t e = UnalignedLoad64(ptr + 32);
      uint64_t f = UnalignedLoad64(ptr + 40);
      uint64_t g = UnalignedLoad64(ptr + 48);
      uint64_t h = UnalignedLoad64(ptr + 56);

      uint64_t cs0 = WyhashMix(a ^ salt[1], b ^ current_state);
      uint64_t cs1 = WyhashMix(c ^ salt[2], d ^ current_state);
      current_state = (cs0 ^ cs1);

      uint64_t ds0 = WyhashMix(e ^ salt[3], f ^ duplicated_state);
      uint64_t ds1 = WyhashMix(g ^ salt[4], h ^ duplicated_state);
      duplicated_state = (ds0 ^ ds1);

      ptr += 64;
      len -= 64;
    } while (len > 64);

    current_state = current_state ^ duplicated_state;
  }

  // We now have a data `ptr` with at most 64 bytes and the current state
  // of the hashing state machine stored in current_state.
  while (len > 16) {
    uint64_t a = UnalignedLoad64(ptr);
    uint64_t b = UnalignedLoad64(ptr + 8);

    current_state = WyhashMix(a ^ salt[1], b ^ current_state);

    ptr += 16;
    len -= 16;
  }

  // We now have a data `ptr` with at most 16 bytes.
  uint64_t a = 0;
  uint64_t b = 0;
  if (len > 8) {
    // When we have at least 9 and at most 16 bytes, set A to the first 64
    // bits of the input and B to the last 64 bits of the input. Yes, they will
    // overlap in the middle if we are working with less than the full 16
    // bytes.
    a = UnalignedLoad64(ptr);
    b = UnalignedLoad64(ptr + len - 8);
  } else if (len > 3) {
    // If we have at least 4 and at most 8 bytes, set A to the first 32
    // bits and B to the last 32 bits.
    a = UnalignedLoad32(ptr);
    b = UnalignedLoad32(ptr + len - 4);
  } else if (len > 0) {
    // If we have at least 1 and at most 3 bytes, read all of the provided
    // bits into A, with some adjustments.
    a = ((ptr[0] << 16) | (ptr[len >> 1] << 8) | ptr[len - 1]);
    b = 0;
  } else {
    a = 0;
    b = 0;
  }

  uint64_t w = WyhashMix(a ^ salt[1], b ^ current_state);
  uint64_t z = salt[1] ^ starting_length;
  return WyhashMix(w, z);
}

const uint64_t kRubyWyhashSalt[5] = {
    0x243F6A8885A308D3ULL, 0x13198A2E03707344ULL, 0xA4093822299F31D0ULL,
    0x082EFA98EC4E6C89ULL, 0x452821E638D01377ULL,
};

static upb_strview Convert_StringData(VALUE str, upb_arena *arena) {
  upb_strview ret;
  if (arena) {
    char *ptr = upb_arena_malloc(arena, RSTRING_LEN(str));
    memcpy(ptr, RSTRING_PTR(str), RSTRING_LEN(str));
    ret.data = ptr;
  } else {
    // Data is only needed temporarily (within map lookup).
    ret.data = RSTRING_PTR(str);
  }
  ret.size = RSTRING_LEN(str);
  return ret;
}

static bool is_ruby_num(VALUE value) {
  return (TYPE(value) == T_FLOAT ||
          TYPE(value) == T_FIXNUM ||
          TYPE(value) == T_BIGNUM);
}

static void Convert_CheckInt(const char* name, upb_fieldtype_t type,
                             VALUE val) {
  if (!is_ruby_num(val)) {
    rb_raise(cTypeError,
             "Expected number type for integral field '%s' (given %s).", name,
             rb_class2name(CLASS_OF(val)));
  }

  // NUM2{INT,UINT,LL,ULL} macros do the appropriate range checks on upper
  // bound; we just need to do precision checks (i.e., disallow rounding) and
  // check for < 0 on unsigned types.
  if (TYPE(val) == T_FLOAT) {
    double dbl_val = NUM2DBL(val);
    if (floor(dbl_val) != dbl_val) {
      rb_raise(rb_eRangeError,
               "Non-integral floating point value assigned to integer field "
               "'%s' (given %s).",
               name, rb_class2name(CLASS_OF(val)));
    }
  }
  if (type == UPB_TYPE_UINT32 || type == UPB_TYPE_UINT64) {
    if (NUM2DBL(val) < 0) {
      rb_raise(
          rb_eRangeError,
          "Assigning negative value to unsigned integer field '%s' (given %s).",
          name, rb_class2name(CLASS_OF(val)));
    }
  }
}

static int32_t Convert_ToEnum(VALUE value, const char* name,
                              const upb_enumdef* e) {
  int32_t val;

  switch (TYPE(value)) {
    case T_FLOAT:
    case T_FIXNUM:
    case T_BIGNUM:
      Convert_CheckInt(name, UPB_TYPE_INT32, value);
      val = NUM2INT(value);
      break;
    case T_STRING: {
      const upb_enumvaldef *ev =
          upb_enumdef_lookupname(e, RSTRING_PTR(value), RSTRING_LEN(value));
      if (!ev) goto unknownval;
      val = upb_enumvaldef_number(ev);
      break;
    }
    case T_SYMBOL: {
      const upb_enumvaldef *ev =
          upb_enumdef_lookupnamez(e, rb_id2name(SYM2ID(value)));
      if (!ev) goto unknownval;
      val = upb_enumvaldef_number(ev);
      break;
    }
    default:
      rb_raise(cTypeError,
               "Expected number or symbol type for enum field '%s'.", name);
  }

  return val;

unknownval:
  rb_raise(rb_eRangeError, "Unknown symbol value for enum field '%s'.", name);
}

upb_msgval Convert_RubyToUpb(VALUE value, const char* name, TypeInfo type_info,
                             upb_arena* arena) {
  upb_msgval ret;

  switch (type_info.type) {
    case UPB_TYPE_FLOAT:
      if (!is_ruby_num(value)) {
        rb_raise(cTypeError, "Expected number type for float field '%s' (given %s).",
                 name, rb_class2name(CLASS_OF(value)));
      }
      ret.float_val = NUM2DBL(value);
      break;
    case UPB_TYPE_DOUBLE:
      if (!is_ruby_num(value)) {
        rb_raise(cTypeError, "Expected number type for double field '%s' (given %s).",
                 name, rb_class2name(CLASS_OF(value)));
      }
      ret.double_val = NUM2DBL(value);
      break;
    case UPB_TYPE_BOOL: {
      if (value == Qtrue) {
        ret.bool_val = 1;
      } else if (value == Qfalse) {
        ret.bool_val = 0;
      } else {
        rb_raise(cTypeError, "Invalid argument for boolean field '%s' (given %s).",
                 name, rb_class2name(CLASS_OF(value)));
      }
      break;
    }
    case UPB_TYPE_STRING: {
      VALUE utf8 = rb_enc_from_encoding(rb_utf8_encoding());
      if (CLASS_OF(value) == rb_cSymbol) {
        value = rb_funcall(value, rb_intern("to_s"), 0);
      } else if (CLASS_OF(value) != rb_cString) {
        rb_raise(cTypeError, "Invalid argument for string field '%s' (given %s).",
                 name, rb_class2name(CLASS_OF(value)));
      }

      if (rb_obj_encoding(value) != utf8) {
        // Note: this will not duplicate underlying string data unless necessary.
        value = rb_str_encode(value, utf8, 0, Qnil);

        if (rb_enc_str_coderange(value) == ENC_CODERANGE_BROKEN) {
          rb_raise(rb_eEncodingError, "String is invalid UTF-8");
        }
      }

      ret.str_val = Convert_StringData(value, arena);
      break;
    }
    case UPB_TYPE_BYTES: {
      VALUE bytes = rb_enc_from_encoding(rb_ascii8bit_encoding());
      if (CLASS_OF(value) != rb_cString) {
        rb_raise(cTypeError, "Invalid argument for bytes field '%s' (given %s).",
                 name, rb_class2name(CLASS_OF(value)));
      }

      if (rb_obj_encoding(value) != bytes) {
        // Note: this will not duplicate underlying string data unless necessary.
        // TODO(haberman): is this really necessary to get raw bytes?
        value = rb_str_encode(value, bytes, 0, Qnil);
      }

      ret.str_val = Convert_StringData(value, arena);
      break;
    }
    case UPB_TYPE_MESSAGE:
      ret.msg_val =
          Message_GetUpbMessage(value, type_info.def.msgdef, name, arena);
      break;
    case UPB_TYPE_ENUM:
      ret.int32_val = Convert_ToEnum(value, name, type_info.def.enumdef);
      break;
    case UPB_TYPE_INT32:
    case UPB_TYPE_INT64:
    case UPB_TYPE_UINT32:
    case UPB_TYPE_UINT64:
      Convert_CheckInt(name, type_info.type, value);
      switch (type_info.type) {
      case UPB_TYPE_INT32:
        ret.int32_val = NUM2INT(value);
        break;
      case UPB_TYPE_INT64:
        ret.int64_val = NUM2LL(value);
        break;
      case UPB_TYPE_UINT32:
        ret.uint32_val = NUM2UINT(value);
        break;
      case UPB_TYPE_UINT64:
        ret.uint64_val = NUM2ULL(value);
        break;
      default:
        break;
      }
      break;
    default:
      break;
  }

  return ret;
}

VALUE Convert_UpbToRuby(upb_msgval upb_val, TypeInfo type_info, VALUE arena) {
  switch (type_info.type) {
    case UPB_TYPE_FLOAT:
      return DBL2NUM(upb_val.float_val);
    case UPB_TYPE_DOUBLE:
      return DBL2NUM(upb_val.double_val);
    case UPB_TYPE_BOOL:
      return upb_val.bool_val ? Qtrue : Qfalse;
    case UPB_TYPE_INT32:
      return INT2NUM(upb_val.int32_val);
    case UPB_TYPE_INT64:
      return LL2NUM(upb_val.int64_val);
    case UPB_TYPE_UINT32:
      return UINT2NUM(upb_val.uint32_val);
    case UPB_TYPE_UINT64:
      return ULL2NUM(upb_val.int64_val);
    case UPB_TYPE_ENUM: {
      const upb_enumvaldef* ev =
          upb_enumdef_lookupnum(type_info.def.enumdef, upb_val.int32_val);
      if (ev) {
        return ID2SYM(rb_intern(upb_enumvaldef_name(ev)));
      } else {
        return INT2NUM(upb_val.int32_val);
      }
    }
    case UPB_TYPE_STRING: {
      VALUE str_rb = rb_str_new(upb_val.str_val.data, upb_val.str_val.size);
      rb_enc_associate(str_rb, rb_utf8_encoding());
      rb_obj_freeze(str_rb);
      return str_rb;
    }
    case UPB_TYPE_BYTES: {
      VALUE str_rb = rb_str_new(upb_val.str_val.data, upb_val.str_val.size);
      rb_enc_associate(str_rb, rb_ascii8bit_encoding());
      rb_obj_freeze(str_rb);
      return str_rb;
    }
    case UPB_TYPE_MESSAGE:
      return Message_GetRubyWrapper((upb_msg*)upb_val.msg_val,
                                    type_info.def.msgdef, arena);
    default:
      rb_raise(rb_eRuntimeError, "Convert_UpbToRuby(): Unexpected type %d",
               (int)type_info.type);
  }
}

upb_msgval Msgval_DeepCopy(upb_msgval msgval, TypeInfo type_info,
                           upb_arena* arena) {
  upb_msgval new_msgval;

  switch (type_info.type) {
    default:
      memcpy(&new_msgval, &msgval, sizeof(msgval));
      break;
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES: {
      size_t n = msgval.str_val.size;
      char *mem = upb_arena_malloc(arena, n);
      new_msgval.str_val.data = mem;
      new_msgval.str_val.size = n;
      memcpy(mem, msgval.str_val.data, n);
      break;
    }
    case UPB_TYPE_MESSAGE:
      new_msgval.msg_val =
          Message_deep_copy(msgval.msg_val, type_info.def.msgdef, arena);
      break;
  }

  return new_msgval;
}

bool Msgval_IsEqual(upb_msgval val1, upb_msgval val2, TypeInfo type_info) {
  switch (type_info.type) {
    case UPB_TYPE_BOOL:
      return memcmp(&val1, &val2, 1) == 0;
    case UPB_TYPE_FLOAT:
    case UPB_TYPE_INT32:
    case UPB_TYPE_UINT32:
    case UPB_TYPE_ENUM:
      return memcmp(&val1, &val2, 4) == 0;
    case UPB_TYPE_DOUBLE:
    case UPB_TYPE_INT64:
    case UPB_TYPE_UINT64:
      return memcmp(&val1, &val2, 8) == 0;
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES:
      return val1.str_val.size == val2.str_val.size &&
             memcmp(val1.str_val.data, val2.str_val.data,
                    val1.str_val.size) == 0;
    case UPB_TYPE_MESSAGE:
      return Message_Equal(val1.msg_val, val2.msg_val, type_info.def.msgdef);
    default:
      rb_raise(rb_eRuntimeError, "Internal error, unexpected type");
  }
}

uint64_t Msgval_GetHash(upb_msgval val, TypeInfo type_info, uint64_t seed) {
  switch (type_info.type) {
    case UPB_TYPE_BOOL:
      return RubyWyhash(&val, 1, seed, kRubyWyhashSalt);
    case UPB_TYPE_FLOAT:
    case UPB_TYPE_INT32:
    case UPB_TYPE_UINT32:
    case UPB_TYPE_ENUM:
      return RubyWyhash(&val, 4, seed, kRubyWyhashSalt);
    case UPB_TYPE_DOUBLE:
    case UPB_TYPE_INT64:
    case UPB_TYPE_UINT64:
      return RubyWyhash(&val, 8, seed, kRubyWyhashSalt);
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES:
      return RubyWyhash(val.str_val.data, val.str_val.size, seed,
                        kRubyWyhashSalt);
    case UPB_TYPE_MESSAGE:
      return Message_Hash(val.msg_val, type_info.def.msgdef, seed);
    default:
      rb_raise(rb_eRuntimeError, "Internal error, unexpected type");
  }
}

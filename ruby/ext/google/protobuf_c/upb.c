/* Amalgamated source file */
#include "lite-upb.h"
/*
* This is where we define macros used across upb.
*
* All of these macros are undef'd in port_undef.inc to avoid leaking them to
* users.
*
* The correct usage is:
*
*   #include "upb/foobar.h"
*   #include "upb/baz.h"
*
*   // MUST be last included header.
*   #include "upb/port_def.inc"
*
*   // Code for this file.
*   // <...>
*
*   // Can be omitted for .c files, required for .h.
*   #include "upb/port_undef.inc"
*
* This file is private and must not be included by users!
*/
#include <stdint.h>

#if UINTPTR_MAX == 0xffffffff
#define UPB_SIZE(size32, size64) size32
#else
#define UPB_SIZE(size32, size64) size64
#endif

/* These macros aren't really "port", they are helper macros that we don't want
 * to leak.
 */
#define UPB_FIELD_AT(msg, fieldtype, offset) \
  *(fieldtype*)((const char*)(msg) + offset)

#define UPB_READ_ONEOF(msg, fieldtype, offset, case_offset, case_val, default) \
  UPB_FIELD_AT(msg, int, case_offset) == case_val                              \
      ? UPB_FIELD_AT(msg, fieldtype, offset)                                   \
      : default

#define UPB_WRITE_ONEOF(msg, fieldtype, offset, value, case_offset, case_val) \
  UPB_FIELD_AT(msg, int, case_offset) = case_val;                             \
  UPB_FIELD_AT(msg, fieldtype, offset) = value;

#define UPB_MAPTYPE_STRING 0

/* UPB_INLINE: inline if possible, emit standalone code if required. */
#ifdef __cplusplus
#define UPB_INLINE inline
#elif defined (__GNUC__) || defined(__clang__)
#define UPB_INLINE static __inline__
#else
#define UPB_INLINE static
#endif

/* Hints to the compiler about likely/unlikely branches. */
#if defined (__GNUC__) || defined(__clang__)
#define UPB_LIKELY(x) __builtin_expect((x),1)
#define UPB_UNLIKELY(x) __builtin_expect((x),0)
#else
#define UPB_LIKELY(x) (x)
#define UPB_UNLIKELY(x) (x)
#endif

/* Define UPB_BIG_ENDIAN manually if you're on big endian and your compiler
 * doesn't provide these preprocessor symbols. */
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define UPB_BIG_ENDIAN
#endif

/* Macros for function attributes on compilers that support them. */
#ifdef __GNUC__
#define UPB_FORCEINLINE __inline__ __attribute__((always_inline))
#define UPB_NOINLINE __attribute__((noinline))
#define UPB_NORETURN __attribute__((__noreturn__))
#else  /* !defined(__GNUC__) */
#define UPB_FORCEINLINE
#define UPB_NOINLINE
#define UPB_NORETURN
#endif

#if __STDC_VERSION__ >= 199901L || __cplusplus >= 201103L
/* C99/C++11 versions. */
#include <stdio.h>
#define _upb_snprintf snprintf
#define _upb_vsnprintf vsnprintf
#define _upb_va_copy(a, b) va_copy(a, b)
#elif defined(_MSC_VER)
/* Microsoft C/C++ versions. */
#include <stdarg.h>
#include <stdio.h>
#if _MSC_VER < 1900
int msvc_snprintf(char* s, size_t n, const char* format, ...);
int msvc_vsnprintf(char* s, size_t n, const char* format, va_list arg);
#define UPB_MSVC_VSNPRINTF
#define _upb_snprintf msvc_snprintf
#define _upb_vsnprintf msvc_vsnprintf
#else
#define _upb_snprintf snprintf
#define _upb_vsnprintf vsnprintf
#endif
#define _upb_va_copy(a, b) va_copy(a, b)
#elif defined __GNUC__
/* A few hacky workarounds for functions not in C89.
 * For internal use only!
 * TODO(haberman): fix these by including our own implementations, or finding
 * another workaround.
 */
#define _upb_snprintf __builtin_snprintf
#define _upb_vsnprintf __builtin_vsnprintf
#define _upb_va_copy(a, b) __va_copy(a, b)
#else
#error Need implementations of [v]snprintf and va_copy
#endif

#ifdef __cplusplus
#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__) || \
    (defined(_MSC_VER) && _MSC_VER >= 1900)
/* C++11 is present */
#else
#error upb requires C++11 for C++ support
#endif
#endif

#define UPB_MAX(x, y) ((x) > (y) ? (x) : (y))
#define UPB_MIN(x, y) ((x) < (y) ? (x) : (y))

#define UPB_UNUSED(var) (void)var

/* UPB_ASSUME(): in release mode, we tell the compiler to assume this is true.
 */
#ifdef NDEBUG
#ifdef __GNUC__
#define UPB_ASSUME(expr) if (!(expr)) __builtin_unreachable()
#else
#define UPB_ASSUME(expr) do {} if (false && (expr))
#endif
#else
#define UPB_ASSUME(expr) assert(expr)
#endif

/* UPB_ASSERT(): in release mode, we use the expression without letting it be
 * evaluated.  This prevents "unused variable" warnings. */
#ifdef NDEBUG
#define UPB_ASSERT(expr) do {} while (false && (expr))
#else
#define UPB_ASSERT(expr) assert(expr)
#endif

/* UPB_ASSERT_DEBUGVAR(): assert that uses functions or variables that only
 * exist in debug mode.  This turns into regular assert. */
#define UPB_ASSERT_DEBUGVAR(expr) assert(expr)

#if defined(__GNUC__) || defined(__clang__)
#define UPB_UNREACHABLE() do { assert(0); __builtin_unreachable(); } while(0)
#else
#define UPB_UNREACHABLE() do { assert(0); } while(0)
#endif

/* UPB_INFINITY representing floating-point positive infinity. */
#include <math.h>
#ifdef INFINITY
#define UPB_INFINITY INFINITY
#else
#define UPB_INFINITY (1.0 / 0.0)
#endif

#include <string.h>


/* Maps descriptor type -> upb field type.  */
static const uint8_t desctype_to_fieldtype[] = {
  -1,  /* invalid descriptor type */
  UPB_TYPE_DOUBLE,          /* DOUBLE */
  UPB_TYPE_FLOAT,           /* FLOAT */
  UPB_TYPE_INT64,           /* INT64 */
  UPB_TYPE_UINT64,          /* UINT64 */
  UPB_TYPE_INT32,           /* INT32 */
  UPB_TYPE_UINT64,          /* FIXED64 */
  UPB_TYPE_UINT32,          /* FIXED32 */
  UPB_TYPE_BOOL,            /* BOOL */
  UPB_TYPE_STRING,          /* STRING */
  UPB_TYPE_MESSAGE,         /* GROUP */
  UPB_TYPE_MESSAGE,         /* MESSAGE */
  UPB_TYPE_BYTES,           /* BYTES */
  UPB_TYPE_UINT32,          /* UINT32 */
  UPB_TYPE_ENUM,            /* ENUM */
  UPB_TYPE_INT32,           /* SFIXED32 */
  UPB_TYPE_INT64,           /* SFIXED64 */
  UPB_TYPE_INT32,           /* SINT32 */
  UPB_TYPE_INT64,           /* SINT64 */
};

/* Maps descriptor type -> upb map size.  */
static const uint8_t desctype_to_mapsize[] = {
  -1,  /* invalid descriptor type */
  8,          /* DOUBLE */
  4,          /* FLOAT */
  8,          /* INT64 */
  8,          /* UINT64 */
  4,          /* INT32 */
  8,          /* FIXED64 */
  4,          /* FIXED32 */
  1,          /* BOOL */
  UPB_MAPTYPE_STRING,    /* STRING */
  sizeof(void*),         /* GROUP */
  sizeof(void*),         /* MESSAGE */
  UPB_MAPTYPE_STRING,    /* BYTES */
  4,          /* UINT32 */
  4,          /* ENUM */
  4,          /* SFIXED32 */
  8,          /* SFIXED64 */
  4,          /* SINT32 */
  8,          /* SINT64 */
};

/* Data pertaining to the parse. */
typedef struct {
  const char *field_start;   /* Start of this field. */
  const char *limit;         /* End of delimited region or end of buffer. */
  upb_arena *arena;
  int depth;
  uint32_t end_group;  /* Set to field number of END_GROUP tag, if any. */
} upb_decstate;

#define CHK(x) if (!(x)) { return 0; }
#define PTR_AT(msg, ofs, type) (type*)((const char*)msg + ofs)

static const char *upb_decode_message(const char *ptr, const upb_msglayout *l,
                                      upb_msg *msg, upb_decstate *d);

static const char *upb_decode_varint(const char *ptr, const char *limit,
                                     uint64_t *val) {
  uint8_t byte;
  int bitpos = 0;
  *val = 0;

  do {
    CHK(bitpos < 70 && ptr < limit);
    byte = *ptr;
    *val |= (uint64_t)(byte & 0x7F) << bitpos;
    ptr++;
    bitpos += 7;
  } while (byte & 0x80);

  return ptr;
}

static const char *upb_decode_varint32(const char *ptr, const char *limit,
                                       uint32_t *val) {
  uint64_t u64;
  CHK(ptr = upb_decode_varint(ptr, limit, &u64))
  CHK(u64 <= UINT32_MAX);
  *val = (uint32_t)u64;
  return ptr;
}

static const char *upb_decode_64bit(const char *ptr, const char *limit,
                                    uint64_t *val) {
  CHK(limit - ptr >= 8);
  memcpy(val, ptr, 8);
  return ptr + 8;
}

static const char *upb_decode_32bit(const char *ptr, const char *limit,
                                    uint32_t *val) {
  CHK(limit - ptr >= 4);
  memcpy(val, ptr, 4);
  return ptr + 4;
}

static int32_t upb_zzdecode_32(uint32_t n) {
  return (n >> 1) ^ -(int32_t)(n & 1);
}

static int64_t upb_zzdecode_64(uint64_t n) {
  return (n >> 1) ^ -(int64_t)(n & 1);
}

static const char *upb_decode_string(const char *ptr, const char *limit,
                                     int *outlen) {
  uint32_t len;

  CHK(ptr = upb_decode_varint32(ptr, limit, &len));
  CHK(len < INT32_MAX);
  CHK(limit - ptr >= (int32_t)len);

  *outlen = len;
  return ptr;
}

static void upb_set32(void *msg, size_t ofs, uint32_t val) {
  memcpy((char*)msg + ofs, &val, sizeof(val));
}

static const char *upb_append_unknown(const char *ptr, upb_msg *msg,
                                      upb_decstate *d) {
  upb_msg_addunknown(msg, d->field_start, ptr - d->field_start, d->arena);
  return ptr;
}

static const char *upb_skip_unknownfielddata(const char *ptr, upb_decstate *d,
                                             uint32_t tag) {
  switch (tag & 7) {
    case UPB_WIRE_TYPE_VARINT: {
      uint64_t val;
      return upb_decode_varint(ptr, d->limit, &val);
    }
    case UPB_WIRE_TYPE_32BIT: {
      uint32_t val;
      return upb_decode_32bit(ptr, d->limit, &val);
    }
    case UPB_WIRE_TYPE_64BIT: {
      uint64_t val;
      return upb_decode_64bit(ptr, d->limit, &val);
    }
    case UPB_WIRE_TYPE_DELIMITED: {
      int len;
      CHK(ptr = upb_decode_string(ptr, d->limit, &len));
      return ptr + len;
    }
    case UPB_WIRE_TYPE_START_GROUP: {
      uint32_t field_number = tag >> 3;
      while (ptr < d->limit && d->end_group == 0) {
        uint32_t tag = 0;
        CHK(ptr = upb_decode_varint32(ptr, d->limit, &tag));
        CHK(ptr = upb_skip_unknownfielddata(ptr, d, tag));
      }
      CHK(d->end_group == field_number);
      d->end_group = 0;
      return ptr;
    }
    case UPB_WIRE_TYPE_END_GROUP:
      d->end_group = tag >> 3;
      return ptr;
  }
  return false;
}

static void *upb_array_reserve(upb_array *arr, size_t elements,
                               size_t elem_size, upb_arena *arena) {
  if (arr->size - arr->len < elements) {
    CHK(_upb_array_realloc(arr, arr->len + elements, arena));
  }
  return (char*)_upb_array_ptr(arr) + (arr->len * elem_size);
}

bool upb_array_add(upb_array *arr, size_t elements, size_t elem_size,
                   const void *data, upb_arena *arena) {
  void *dest = upb_array_reserve(arr, elements, elem_size, arena);

  CHK(dest);
  arr->len += elements;
  memcpy(dest, data, elements * elem_size);

  return true;
}

static upb_array *upb_getarr(upb_msg *msg, const upb_msglayout_field *field) {
  UPB_ASSERT(field->label == UPB_LABEL_REPEATED);
  return *PTR_AT(msg, field->offset, upb_array*);
}

static upb_array *upb_getorcreatearr(upb_msg *msg,
                                     const upb_msglayout_field *field,
                                     upb_decstate *d) {
  upb_array *arr = upb_getarr(msg, field);

  if (!arr) {
    upb_fieldtype_t type = desctype_to_fieldtype[field->descriptortype];
    arr = _upb_array_new(d->arena, type);
    CHK(arr);
    *PTR_AT(msg, field->offset, upb_array*) = arr;
  }

  return arr;
}

static upb_msg *upb_getorcreatemsg(upb_msg *msg,
                                   const upb_msglayout_field *field,
                                   const upb_msglayout *layout,
                                   upb_decstate *d) {
  upb_msg **submsg = PTR_AT(msg, field->offset, upb_msg*);

  UPB_ASSERT(field->label != UPB_LABEL_REPEATED);

  if (!*submsg) {
    *submsg = _upb_msg_new(layout, d->arena);
    CHK(*submsg);
  }

  return *submsg;
}

static upb_msg *upb_addmsg(upb_msg *msg,
                           const upb_msglayout_field *field,
                           const upb_msglayout *layout,
                           upb_decstate *d) {
  upb_msg *submsg;
  upb_array *arr = upb_getorcreatearr(msg, field, d);

  UPB_ASSERT(field->label == UPB_LABEL_REPEATED);
  UPB_ASSERT(field->descriptortype == UPB_DESCRIPTOR_TYPE_MESSAGE ||
             field->descriptortype == UPB_DESCRIPTOR_TYPE_GROUP);

  submsg = _upb_msg_new(layout, d->arena);
  CHK(submsg);
  upb_array_add(arr, 1, sizeof(submsg), &submsg, d->arena);

  return submsg;
}

static void upb_sethasbit(upb_msg *msg, const upb_msglayout_field *field) {
  int32_t hasbit = field->presence;
  UPB_ASSERT(field->presence > 0);
  *PTR_AT(msg, hasbit / 8, char) |= (1 << (hasbit % 8));
}

static void upb_setoneofcase(upb_msg *msg, const upb_msglayout_field *field) {
  UPB_ASSERT(field->presence < 0);
  upb_set32(msg, ~field->presence, field->number);
}

static bool upb_decode_addval(upb_msg *msg, const upb_msglayout_field *field,
                              void *val, size_t size, upb_decstate *d) {
  char *field_mem = PTR_AT(msg, field->offset, char);
  upb_array *arr;

  if (field->label == UPB_LABEL_REPEATED) {
    arr = upb_getorcreatearr(msg, field, d);
    CHK(arr);
    field_mem = upb_array_reserve(arr, 1, size, d->arena);
    CHK(field_mem);
  }

  memcpy(field_mem, val, size);
  return true;
}

static void upb_decode_setpresent(upb_msg *msg,
                                  const upb_msglayout_field *field) {
  if (field->label == UPB_LABEL_REPEATED) {
   upb_array *arr = upb_getarr(msg, field);
   UPB_ASSERT(arr->len < arr->size);
   arr->len++;
  } else if (field->presence < 0) {
    upb_setoneofcase(msg, field);
  } else if (field->presence > 0) {
    upb_sethasbit(msg, field);
  }
}

static const char *upb_decode_msgfield(const char *ptr,
                                       const upb_msglayout *layout, int limit,
                                       upb_msg *msg, upb_decstate *d) {
  const char* saved_limit = d->limit;
  d->limit = ptr + limit;
  CHK(--d->depth >= 0);
  ptr = upb_decode_message(ptr, layout, msg, d);
  d->depth++;
  d->limit = saved_limit;
  CHK(d->end_group == 0);
  return ptr;
}

static const char *upb_decode_groupfield(const char *ptr,
                                         const upb_msglayout *layout,
                                         int field_number, upb_msg *msg,
                                         upb_decstate *d) {
  CHK(--d->depth >= 0);
  ptr = upb_decode_message(ptr, layout, msg, d);
  d->depth++;
  CHK(d->end_group == field_number);
  d->end_group = 0;
  return ptr;
}

static const char *upb_decode_varintfield(const char *ptr, upb_msg *msg,
                                          const upb_msglayout_field *field,
                                          upb_decstate *d) {
  uint64_t val;
  CHK(ptr = upb_decode_varint(ptr, d->limit, &val));

  switch (field->descriptortype) {
    case UPB_DESCRIPTOR_TYPE_INT64:
    case UPB_DESCRIPTOR_TYPE_UINT64:
      CHK(upb_decode_addval(msg, field, &val, sizeof(val), d));
      break;
    case UPB_DESCRIPTOR_TYPE_INT32:
    case UPB_DESCRIPTOR_TYPE_UINT32:
    case UPB_DESCRIPTOR_TYPE_ENUM: {
      uint32_t val32 = (uint32_t)val;
      CHK(upb_decode_addval(msg, field, &val32, sizeof(val32), d));
      break;
    }
    case UPB_DESCRIPTOR_TYPE_BOOL: {
      bool valbool = val != 0;
      CHK(upb_decode_addval(msg, field, &valbool, sizeof(valbool), d));
      break;
    }
    case UPB_DESCRIPTOR_TYPE_SINT32: {
      int32_t decoded = upb_zzdecode_32((uint32_t)val);
      CHK(upb_decode_addval(msg, field, &decoded, sizeof(decoded), d));
      break;
    }
    case UPB_DESCRIPTOR_TYPE_SINT64: {
      int64_t decoded = upb_zzdecode_64(val);
      CHK(upb_decode_addval(msg, field, &decoded, sizeof(decoded), d));
      break;
    }
    default:
      return upb_append_unknown(ptr, msg, d);
  }

  upb_decode_setpresent(msg, field);
  return ptr;
}

static const char *upb_decode_64bitfield(const char *ptr,
                                         const upb_msglayout_field *field,
                                         upb_msg *msg, upb_decstate *d) {
  uint64_t val;
  CHK(ptr = upb_decode_64bit(ptr, d->limit, &val));

  switch (field->descriptortype) {
    case UPB_DESCRIPTOR_TYPE_DOUBLE:
    case UPB_DESCRIPTOR_TYPE_FIXED64:
    case UPB_DESCRIPTOR_TYPE_SFIXED64:
      CHK(upb_decode_addval(msg, field, &val, sizeof(val), d));
      break;
    default:
      return upb_append_unknown(ptr, msg, d);
  }

  upb_decode_setpresent(msg, field);
  return ptr;
}

static const char *upb_decode_32bitfield(const char *ptr,
                                         const upb_msglayout_field *field,
                                         upb_msg *msg, upb_decstate *d) {
  uint32_t val;
  CHK(ptr = upb_decode_32bit(ptr, d->limit, &val));

  switch (field->descriptortype) {
    case UPB_DESCRIPTOR_TYPE_FLOAT:
    case UPB_DESCRIPTOR_TYPE_FIXED32:
    case UPB_DESCRIPTOR_TYPE_SFIXED32:
      CHK(upb_decode_addval(msg, field, &val, sizeof(val), d));
      break;
    default:
      return upb_append_unknown(ptr, msg, d);
  }

  upb_decode_setpresent(msg, field);
  return ptr;
}

static const char *upb_decode_fixedpacked(const char *ptr, upb_decstate *d,
                                          upb_array *arr, uint32_t len,
                                          int elem_size) {
  size_t elements = len / elem_size;

  CHK((size_t)(elements * elem_size) == len);
  CHK(upb_array_add(arr, elements, elem_size, ptr, d->arena));
  return ptr + len;
}

static const char *upb_decode_strfield(const char *ptr, upb_decstate *d,
                                       uint32_t len, upb_strview *str) {
  str->data = ptr;
  str->size = len;
  return ptr + len;
}

static const char *upb_decode_toarray(const char *ptr,
                                      const upb_msglayout *layout,
                                      const upb_msglayout_field *field, int len,
                                      upb_msg *msg, upb_decstate *d) {
  upb_array *arr = upb_getorcreatearr(msg, field, d);
  CHK(arr);

#define VARINT_CASE(ctype, decode) \
  VARINT_CASE_EX(ctype, decode, decode)

#define VARINT_CASE_EX(ctype, decode, dtype)                           \
  {                                                                    \
    const char *limit = ptr + len;                                     \
    while (ptr < limit) {                                              \
      uint64_t val;                                                    \
      ctype decoded;                                                   \
      CHK(ptr = upb_decode_varint(ptr, limit, &val));                  \
      decoded = (decode)((dtype)val);                                  \
      CHK(upb_array_add(arr, 1, sizeof(decoded), &decoded, d->arena)); \
    }                                                                  \
    return ptr;                                                        \
  }

  switch (field->descriptortype) {
    case UPB_DESCRIPTOR_TYPE_STRING:
    case UPB_DESCRIPTOR_TYPE_BYTES: {
      upb_strview str;
      ptr = upb_decode_strfield(ptr, d, len, &str);
      CHK(upb_array_add(arr, 1, sizeof(str), &str, d->arena));
      return ptr;
    }
    case UPB_DESCRIPTOR_TYPE_FLOAT:
    case UPB_DESCRIPTOR_TYPE_FIXED32:
    case UPB_DESCRIPTOR_TYPE_SFIXED32:
      return upb_decode_fixedpacked(ptr, d, arr, len, sizeof(int32_t));
    case UPB_DESCRIPTOR_TYPE_DOUBLE:
    case UPB_DESCRIPTOR_TYPE_FIXED64:
    case UPB_DESCRIPTOR_TYPE_SFIXED64:
      return upb_decode_fixedpacked(ptr, d, arr, len, sizeof(int64_t));
    case UPB_DESCRIPTOR_TYPE_INT32:
    case UPB_DESCRIPTOR_TYPE_UINT32:
    case UPB_DESCRIPTOR_TYPE_ENUM:
      VARINT_CASE(uint32_t, uint32_t);
    case UPB_DESCRIPTOR_TYPE_INT64:
    case UPB_DESCRIPTOR_TYPE_UINT64:
      VARINT_CASE(uint64_t, uint64_t);
    case UPB_DESCRIPTOR_TYPE_BOOL:
      VARINT_CASE(bool, bool);
    case UPB_DESCRIPTOR_TYPE_SINT32:
      VARINT_CASE_EX(int32_t, upb_zzdecode_32, uint32_t);
    case UPB_DESCRIPTOR_TYPE_SINT64:
      VARINT_CASE_EX(int64_t, upb_zzdecode_64, uint64_t);
    case UPB_DESCRIPTOR_TYPE_MESSAGE: {
      const upb_msglayout *subl = layout->submsgs[field->submsg_index];
      upb_msg *submsg = upb_addmsg(msg, field, subl, d);
      CHK(submsg);
      return upb_decode_msgfield(ptr, subl, len, submsg, d);
    }
    case UPB_DESCRIPTOR_TYPE_GROUP:
      return upb_append_unknown(ptr, msg, d);
  }
#undef VARINT_CASE
  UPB_UNREACHABLE();
}

static const char *upb_decode_mapfield(const char *ptr,
                                       const upb_msglayout *layout,
                                       const upb_msglayout_field *field,
                                       int len, upb_msg *msg, upb_decstate *d) {
  upb_map *map = *PTR_AT(msg, field->offset, upb_map*);
  const upb_msglayout *entry = layout->submsgs[field->submsg_index];
  upb_map_entry ent;

  if (!map) {
    /* Lazily create map. */
    const upb_msglayout_field *key_field = &entry->fields[0];
    const upb_msglayout_field *val_field = &entry->fields[1];
    char key_size = desctype_to_mapsize[key_field->descriptortype];
    char val_size = desctype_to_mapsize[val_field->descriptortype];
    UPB_ASSERT(key_field->number == 1);
    UPB_ASSERT(val_field->number == 2);
    UPB_ASSERT(key_field->offset == 0);
    UPB_ASSERT(val_field->offset == sizeof(upb_strview));
    map = _upb_map_new(d->arena, key_size, val_size);
    *PTR_AT(msg, field->offset, upb_map*) = map;
  }

  /* Parse map entry. */
  memset(&ent, 0, sizeof(ent));
  CHK(ptr = upb_decode_msgfield(ptr, entry, len, &ent.k, d));

  /* Insert into map. */
  _upb_map_set(map, &ent.k, map->key_size, &ent.v, map->val_size, d->arena);
  return ptr;
}

static const char *upb_decode_delimitedfield(const char *ptr,
                                             const upb_msglayout *layout,
                                             const upb_msglayout_field *field,
                                             upb_msg *msg, upb_decstate *d) {
  int len;

  CHK(ptr = upb_decode_string(ptr, d->limit, &len));

  if (field->label == UPB_LABEL_REPEATED) {
    return upb_decode_toarray(ptr, layout, field, len, msg, d);
  } else if (field->label == UPB_LABEL_MAP) {
    return upb_decode_mapfield(ptr, layout, field, len, msg, d);
  } else {
    switch (field->descriptortype) {
      case UPB_DESCRIPTOR_TYPE_STRING:
      case UPB_DESCRIPTOR_TYPE_BYTES: {
        upb_strview str;
        ptr = upb_decode_strfield(ptr, d, len, &str);
        CHK(upb_decode_addval(msg, field, &str, sizeof(str), d));
        break;
      }
      case UPB_DESCRIPTOR_TYPE_MESSAGE: {
        const upb_msglayout *subl = layout->submsgs[field->submsg_index];
        upb_msg *submsg = upb_getorcreatemsg(msg, field, subl, d);
        CHK(submsg);
        CHK(ptr = upb_decode_msgfield(ptr, subl, len, submsg, d));
        break;
      }
      default:
        /* TODO(haberman): should we accept the last element of a packed? */
        return upb_append_unknown(ptr + len, msg, d);
    }
    upb_decode_setpresent(msg, field);
    return ptr;
  }
}

static const upb_msglayout_field *upb_find_field(const upb_msglayout *l,
                                                 uint32_t field_number) {
  /* Lots of optimization opportunities here. */
  int i;
  for (i = 0; i < l->field_count; i++) {
    if (l->fields[i].number == field_number) {
      return &l->fields[i];
    }
  }

  return NULL;  /* Unknown field. */
}

static const char *upb_decode_field(const char *ptr,
                                    const upb_msglayout *layout, upb_msg *msg,
                                    upb_decstate *d) {
  uint32_t tag;
  const upb_msglayout_field *field;
  int field_number;

  d->field_start = ptr;
  CHK(ptr = upb_decode_varint32(ptr, d->limit, &tag));
  field_number = tag >> 3;
  field = upb_find_field(layout, field_number);

  if (field) {
    switch (tag & 7) {
      case UPB_WIRE_TYPE_VARINT:
        return upb_decode_varintfield(ptr, msg, field, d);
      case UPB_WIRE_TYPE_32BIT:
        return upb_decode_32bitfield(ptr, field, msg, d);
      case UPB_WIRE_TYPE_64BIT:
        return upb_decode_64bitfield(ptr, field, msg, d);
      case UPB_WIRE_TYPE_DELIMITED:
        return upb_decode_delimitedfield(ptr, layout, field, msg, d);
      case UPB_WIRE_TYPE_START_GROUP: {
        const upb_msglayout *subl = layout->submsgs[field->submsg_index];
        upb_msg *group;

        if (field->label == UPB_LABEL_REPEATED) {
          group = upb_addmsg(msg, field, subl, d);
        } else {
          group = upb_getorcreatemsg(msg, field, subl, d);
        }

        return upb_decode_groupfield(ptr, subl, field_number, group, d);
      }
      case UPB_WIRE_TYPE_END_GROUP:
        d->end_group = field_number;
        return ptr;
      default:
        CHK(false);
    }
  } else {
    CHK(field_number != 0);
    CHK(ptr = upb_skip_unknownfielddata(ptr, d, tag));
    CHK(ptr = upb_append_unknown(ptr, msg, d));
    return ptr;
  }
  UPB_UNREACHABLE();
}

static const char *upb_decode_message(const char *ptr, const upb_msglayout *l,
                                      upb_msg *msg, upb_decstate *d) {
  while (ptr < d->limit) {
    CHK(ptr = upb_decode_field(ptr, l, msg, d));
  }

  return ptr;
}

bool upb_decode(const char *buf, size_t size, void *msg, const upb_msglayout *l,
                upb_arena *arena) {
  upb_decstate state;
  state.limit = buf + size;
  state.arena = arena;
  state.depth = 64;
  state.end_group = 0;

  /* Early exit required for buf==NULL case. */
  if (size == 0) return true;

  CHK(upb_decode_message(buf, l, msg, &state));
  return state.end_group == 0;
}

#undef CHK
#undef PTR_AT
/* We encode backwards, to avoid pre-computing lengths (one-pass encode). */


#include <string.h>



#define UPB_PB_VARINT_MAX_LEN 10
#define CHK(x) do { if (!(x)) { return false; } } while(0)

static size_t upb_encode_varint(uint64_t val, char *buf) {
  size_t i;
  if (val < 128) { buf[0] = val; return 1; }
  i = 0;
  while (val) {
    uint8_t byte = val & 0x7fU;
    val >>= 7;
    if (val) byte |= 0x80U;
    buf[i++] = byte;
  }
  return i;
}

static uint32_t upb_zzencode_32(int32_t n) { return ((uint32_t)n << 1) ^ (n >> 31); }
static uint64_t upb_zzencode_64(int64_t n) { return ((uint64_t)n << 1) ^ (n >> 63); }

typedef struct {
  upb_alloc *alloc;
  char *buf, *ptr, *limit;
} upb_encstate;

static size_t upb_roundup_pow2(size_t bytes) {
  size_t ret = 128;
  while (ret < bytes) {
    ret *= 2;
  }
  return ret;
}

static bool upb_encode_growbuffer(upb_encstate *e, size_t bytes) {
  size_t old_size = e->limit - e->buf;
  size_t new_size = upb_roundup_pow2(bytes + (e->limit - e->ptr));
  char *new_buf = upb_realloc(e->alloc, e->buf, old_size, new_size);
  CHK(new_buf);

  /* We want previous data at the end, realloc() put it at the beginning. */
  if (old_size > 0) {
    memmove(new_buf + new_size - old_size, e->buf, old_size);
  }

  e->ptr = new_buf + new_size - (e->limit - e->ptr);
  e->limit = new_buf + new_size;
  e->buf = new_buf;
  return true;
}

/* Call to ensure that at least "bytes" bytes are available for writing at
 * e->ptr.  Returns false if the bytes could not be allocated. */
static bool upb_encode_reserve(upb_encstate *e, size_t bytes) {
  CHK(UPB_LIKELY((size_t)(e->ptr - e->buf) >= bytes) ||
      upb_encode_growbuffer(e, bytes));

  e->ptr -= bytes;
  return true;
}

/* Writes the given bytes to the buffer, handling reserve/advance. */
static bool upb_put_bytes(upb_encstate *e, const void *data, size_t len) {
  if (len == 0) return true;
  CHK(upb_encode_reserve(e, len));
  memcpy(e->ptr, data, len);
  return true;
}

static bool upb_put_fixed64(upb_encstate *e, uint64_t val) {
  /* TODO(haberman): byte-swap for big endian. */
  return upb_put_bytes(e, &val, sizeof(uint64_t));
}

static bool upb_put_fixed32(upb_encstate *e, uint32_t val) {
  /* TODO(haberman): byte-swap for big endian. */
  return upb_put_bytes(e, &val, sizeof(uint32_t));
}

static bool upb_put_varint(upb_encstate *e, uint64_t val) {
  size_t len;
  char *start;
  CHK(upb_encode_reserve(e, UPB_PB_VARINT_MAX_LEN));
  len = upb_encode_varint(val, e->ptr);
  start = e->ptr + UPB_PB_VARINT_MAX_LEN - len;
  memmove(start, e->ptr, len);
  e->ptr = start;
  return true;
}

static bool upb_put_double(upb_encstate *e, double d) {
  uint64_t u64;
  UPB_ASSERT(sizeof(double) == sizeof(uint64_t));
  memcpy(&u64, &d, sizeof(uint64_t));
  return upb_put_fixed64(e, u64);
}

static bool upb_put_float(upb_encstate *e, float d) {
  uint32_t u32;
  UPB_ASSERT(sizeof(float) == sizeof(uint32_t));
  memcpy(&u32, &d, sizeof(uint32_t));
  return upb_put_fixed32(e, u32);
}

static uint32_t upb_readcase(const char *msg, const upb_msglayout_field *f) {
  uint32_t ret;
  uint32_t offset = ~f->presence;
  memcpy(&ret, msg + offset, sizeof(ret));
  return ret;
}

static bool upb_readhasbit(const char *msg, const upb_msglayout_field *f) {
  uint32_t hasbit = f->presence;
  UPB_ASSERT(f->presence > 0);
  return msg[hasbit / 8] & (1 << (hasbit % 8));
}

static bool upb_put_tag(upb_encstate *e, int field_number, int wire_type) {
  return upb_put_varint(e, (field_number << 3) | wire_type);
}

static bool upb_put_fixedarray(upb_encstate *e, const upb_array *arr,
                               size_t size) {
  size_t bytes = arr->len * size;
  const void* data = _upb_array_constptr(arr);
  return upb_put_bytes(e, data, bytes) && upb_put_varint(e, bytes);
}

bool upb_encode_message(upb_encstate *e, const char *msg,
                        const upb_msglayout *m, size_t *size);

static bool upb_encode_scalarfield(upb_encstate *e, const void *_field_mem,
                                   const upb_msglayout *m,
                                   const upb_msglayout_field *f,
                                   bool skip_zero_value) {
  const char *field_mem = _field_mem;
#define CASE(ctype, type, wire_type, encodeval) do { \
  ctype val = *(ctype*)field_mem; \
  if (skip_zero_value && val == 0) { \
    return true; \
  } \
  return upb_put_ ## type(e, encodeval) && \
      upb_put_tag(e, f->number, wire_type); \
} while(0)

  switch (f->descriptortype) {
    case UPB_DESCRIPTOR_TYPE_DOUBLE:
      CASE(double, double, UPB_WIRE_TYPE_64BIT, val);
    case UPB_DESCRIPTOR_TYPE_FLOAT:
      CASE(float, float, UPB_WIRE_TYPE_32BIT, val);
    case UPB_DESCRIPTOR_TYPE_INT64:
    case UPB_DESCRIPTOR_TYPE_UINT64:
      CASE(uint64_t, varint, UPB_WIRE_TYPE_VARINT, val);
    case UPB_DESCRIPTOR_TYPE_UINT32:
      CASE(uint32_t, varint, UPB_WIRE_TYPE_VARINT, val);
    case UPB_DESCRIPTOR_TYPE_INT32:
    case UPB_DESCRIPTOR_TYPE_ENUM:
      CASE(int32_t, varint, UPB_WIRE_TYPE_VARINT, (int64_t)val);
    case UPB_DESCRIPTOR_TYPE_SFIXED64:
    case UPB_DESCRIPTOR_TYPE_FIXED64:
      CASE(uint64_t, fixed64, UPB_WIRE_TYPE_64BIT, val);
    case UPB_DESCRIPTOR_TYPE_FIXED32:
    case UPB_DESCRIPTOR_TYPE_SFIXED32:
      CASE(uint32_t, fixed32, UPB_WIRE_TYPE_32BIT, val);
    case UPB_DESCRIPTOR_TYPE_BOOL:
      CASE(bool, varint, UPB_WIRE_TYPE_VARINT, val);
    case UPB_DESCRIPTOR_TYPE_SINT32:
      CASE(int32_t, varint, UPB_WIRE_TYPE_VARINT, upb_zzencode_32(val));
    case UPB_DESCRIPTOR_TYPE_SINT64:
      CASE(int64_t, varint, UPB_WIRE_TYPE_VARINT, upb_zzencode_64(val));
    case UPB_DESCRIPTOR_TYPE_STRING:
    case UPB_DESCRIPTOR_TYPE_BYTES: {
      upb_strview view = *(upb_strview*)field_mem;
      if (skip_zero_value && view.size == 0) {
        return true;
      }
      return upb_put_bytes(e, view.data, view.size) &&
          upb_put_varint(e, view.size) &&
          upb_put_tag(e, f->number, UPB_WIRE_TYPE_DELIMITED);
    }
    case UPB_DESCRIPTOR_TYPE_GROUP: {
      size_t size;
      void *submsg = *(void **)field_mem;
      const upb_msglayout *subm = m->submsgs[f->submsg_index];
      if (submsg == NULL) {
        return true;
      }
      return upb_put_tag(e, f->number, UPB_WIRE_TYPE_END_GROUP) &&
          upb_encode_message(e, submsg, subm, &size) &&
          upb_put_tag(e, f->number, UPB_WIRE_TYPE_START_GROUP);
    }
    case UPB_DESCRIPTOR_TYPE_MESSAGE: {
      size_t size;
      void *submsg = *(void **)field_mem;
      const upb_msglayout *subm = m->submsgs[f->submsg_index];
      if (submsg == NULL) {
        return true;
      }
      return upb_encode_message(e, submsg, subm, &size) &&
          upb_put_varint(e, size) &&
          upb_put_tag(e, f->number, UPB_WIRE_TYPE_DELIMITED);
    }
  }
#undef CASE
  UPB_UNREACHABLE();
}

static bool upb_encode_array(upb_encstate *e, const char *field_mem,
                             const upb_msglayout *m,
                             const upb_msglayout_field *f) {
  const upb_array *arr = *(const upb_array**)field_mem;

  if (arr == NULL || arr->len == 0) {
    return true;
  }

#define VARINT_CASE(ctype, encode) { \
  const ctype *start = _upb_array_constptr(arr); \
  const ctype *ptr = start + arr->len; \
  size_t pre_len = e->limit - e->ptr; \
  do { \
    ptr--; \
    CHK(upb_put_varint(e, encode)); \
  } while (ptr != start); \
  CHK(upb_put_varint(e, e->limit - e->ptr - pre_len)); \
} \
break; \
do { ; } while(0)

  switch (f->descriptortype) {
    case UPB_DESCRIPTOR_TYPE_DOUBLE:
      CHK(upb_put_fixedarray(e, arr, sizeof(double)));
      break;
    case UPB_DESCRIPTOR_TYPE_FLOAT:
      CHK(upb_put_fixedarray(e, arr, sizeof(float)));
      break;
    case UPB_DESCRIPTOR_TYPE_SFIXED64:
    case UPB_DESCRIPTOR_TYPE_FIXED64:
      CHK(upb_put_fixedarray(e, arr, sizeof(uint64_t)));
      break;
    case UPB_DESCRIPTOR_TYPE_FIXED32:
    case UPB_DESCRIPTOR_TYPE_SFIXED32:
      CHK(upb_put_fixedarray(e, arr, sizeof(uint32_t)));
      break;
    case UPB_DESCRIPTOR_TYPE_INT64:
    case UPB_DESCRIPTOR_TYPE_UINT64:
      VARINT_CASE(uint64_t, *ptr);
    case UPB_DESCRIPTOR_TYPE_UINT32:
      VARINT_CASE(uint32_t, *ptr);
    case UPB_DESCRIPTOR_TYPE_INT32:
    case UPB_DESCRIPTOR_TYPE_ENUM:
      VARINT_CASE(int32_t, (int64_t)*ptr);
    case UPB_DESCRIPTOR_TYPE_BOOL:
      VARINT_CASE(bool, *ptr);
    case UPB_DESCRIPTOR_TYPE_SINT32:
      VARINT_CASE(int32_t, upb_zzencode_32(*ptr));
    case UPB_DESCRIPTOR_TYPE_SINT64:
      VARINT_CASE(int64_t, upb_zzencode_64(*ptr));
    case UPB_DESCRIPTOR_TYPE_STRING:
    case UPB_DESCRIPTOR_TYPE_BYTES: {
      const upb_strview *start = _upb_array_constptr(arr);
      const upb_strview *ptr = start + arr->len;
      do {
        ptr--;
        CHK(upb_put_bytes(e, ptr->data, ptr->size) &&
            upb_put_varint(e, ptr->size) &&
            upb_put_tag(e, f->number, UPB_WIRE_TYPE_DELIMITED));
      } while (ptr != start);
      return true;
    }
    case UPB_DESCRIPTOR_TYPE_GROUP: {
      const void *const*start = _upb_array_constptr(arr);
      const void *const*ptr = start + arr->len;
      const upb_msglayout *subm = m->submsgs[f->submsg_index];
      do {
        size_t size;
        ptr--;
        CHK(upb_put_tag(e, f->number, UPB_WIRE_TYPE_END_GROUP) &&
            upb_encode_message(e, *ptr, subm, &size) &&
            upb_put_tag(e, f->number, UPB_WIRE_TYPE_START_GROUP));
      } while (ptr != start);
      return true;
    }
    case UPB_DESCRIPTOR_TYPE_MESSAGE: {
      const void *const*start = _upb_array_constptr(arr);
      const void *const*ptr = start + arr->len;
      const upb_msglayout *subm = m->submsgs[f->submsg_index];
      do {
        size_t size;
        ptr--;
        CHK(upb_encode_message(e, *ptr, subm, &size) &&
            upb_put_varint(e, size) &&
            upb_put_tag(e, f->number, UPB_WIRE_TYPE_DELIMITED));
      } while (ptr != start);
      return true;
    }
  }
#undef VARINT_CASE

  /* We encode all primitive arrays as packed, regardless of what was specified
   * in the .proto file.  Could special case 1-sized arrays. */
  CHK(upb_put_tag(e, f->number, UPB_WIRE_TYPE_DELIMITED));
  return true;
}

static bool upb_encode_map(upb_encstate *e, const char *field_mem,
                           const upb_msglayout *m,
                           const upb_msglayout_field *f) {
  const upb_map *map = *(const upb_map**)field_mem;
  const upb_msglayout *entry = m->submsgs[f->submsg_index];
  const upb_msglayout_field *key_field = &entry->fields[0];
  const upb_msglayout_field *val_field = &entry->fields[1];
  upb_strtable_iter i;
  if (map == NULL) {
    return true;
  }

  upb_strtable_begin(&i, &map->table);
  for(; !upb_strtable_done(&i); upb_strtable_next(&i)) {
    size_t pre_len = e->limit - e->ptr;
    size_t size;
    upb_strview key = upb_strtable_iter_key(&i);
    const upb_value val = upb_strtable_iter_value(&i);
    const void *keyp =
        map->key_size == UPB_MAPTYPE_STRING ? (void *)&key : key.data;
    const void *valp =
        map->val_size == UPB_MAPTYPE_STRING ? upb_value_getptr(val) : &val;

    CHK(upb_encode_scalarfield(e, valp, entry, val_field, false));
    CHK(upb_encode_scalarfield(e, keyp, entry, key_field, false));
    size = (e->limit - e->ptr) - pre_len;
    CHK(upb_put_varint(e, size));
    CHK(upb_put_tag(e, f->number, UPB_WIRE_TYPE_DELIMITED));
  }

  return true;
}


bool upb_encode_message(upb_encstate *e, const char *msg,
                        const upb_msglayout *m, size_t *size) {
  int i;
  size_t pre_len = e->limit - e->ptr;
  const char *unknown;
  size_t unknown_size;

  unknown = upb_msg_getunknown(msg, &unknown_size);

  if (unknown) {
    upb_put_bytes(e, unknown, unknown_size);
  }

  for (i = m->field_count - 1; i >= 0; i--) {
    const upb_msglayout_field *f = &m->fields[i];

    if (f->label == UPB_LABEL_REPEATED) {
      CHK(upb_encode_array(e, msg + f->offset, m, f));
    } else if (f->label == UPB_LABEL_MAP) {
      CHK(upb_encode_map(e, msg + f->offset, m, f));
    } else {
      bool skip_empty = false;
      if (f->presence == 0) {
        /* Proto3 presence. */
        skip_empty = true;
      } else if (f->presence > 0) {
        /* Proto2 presence: hasbit. */
        if (!upb_readhasbit(msg, f)) {
          continue;
        }
      } else {
        /* Field is in a oneof. */
        if (upb_readcase(msg, f) != f->number) {
          continue;
        }
      }
      CHK(upb_encode_scalarfield(e, msg + f->offset, m, f, skip_empty));
    }
  }

  *size = (e->limit - e->ptr) - pre_len;
  return true;
}

char *upb_encode(const void *msg, const upb_msglayout *m, upb_arena *arena,
                 size_t *size) {
  upb_encstate e;
  e.alloc = upb_arena_alloc(arena);
  e.buf = NULL;
  e.limit = NULL;
  e.ptr = NULL;

  if (!upb_encode_message(&e, msg, m, size)) {
    *size = 0;
    return NULL;
  }

  *size = e.limit - e.ptr;

  if (*size == 0) {
    static char ch;
    return &ch;
  } else {
    UPB_ASSERT(e.ptr);
    return e.ptr;
  }
}

#undef CHK




#define VOIDPTR_AT(msg, ofs) (void*)((char*)msg + (int)ofs)

/** upb_msg *******************************************************************/

static const char _upb_fieldtype_to_sizelg2[12] = {
  0,
  0,  /* UPB_TYPE_BOOL */
  2,  /* UPB_TYPE_FLOAT */
  2,  /* UPB_TYPE_INT32 */
  2,  /* UPB_TYPE_UINT32 */
  2,  /* UPB_TYPE_ENUM */
  UPB_SIZE(2, 3),  /* UPB_TYPE_MESSAGE */
  3,  /* UPB_TYPE_DOUBLE */
  3,  /* UPB_TYPE_INT64 */
  3,  /* UPB_TYPE_UINT64 */
  UPB_SIZE(3, 4),  /* UPB_TYPE_STRING */
  UPB_SIZE(3, 4),  /* UPB_TYPE_BYTES */
};

static uintptr_t tag_arrptr(void* ptr, int elem_size_lg2) {
  UPB_ASSERT(elem_size_lg2 <= 4);
  return (uintptr_t)ptr | elem_size_lg2;
}

static int upb_msg_internalsize(const upb_msglayout *l) {
  return sizeof(upb_msg_internal) - l->extendable * sizeof(void *);
}

static size_t upb_msg_sizeof(const upb_msglayout *l) {
  return l->size + upb_msg_internalsize(l);
}

static upb_msg_internal *upb_msg_getinternal(upb_msg *msg) {
  return VOIDPTR_AT(msg, -sizeof(upb_msg_internal));
}

static const upb_msg_internal *upb_msg_getinternal_const(const upb_msg *msg) {
  return VOIDPTR_AT(msg, -sizeof(upb_msg_internal));
}

static upb_msg_internal_withext *upb_msg_getinternalwithext(
    upb_msg *msg, const upb_msglayout *l) {
  UPB_ASSERT(l->extendable);
  return VOIDPTR_AT(msg, -sizeof(upb_msg_internal_withext));
}

upb_msg *_upb_msg_new(const upb_msglayout *l, upb_arena *a) {
  upb_alloc *alloc = upb_arena_alloc(a);
  void *mem = upb_malloc(alloc, upb_msg_sizeof(l));
  upb_msg_internal *in;
  upb_msg *msg;

  if (!mem) {
    return NULL;
  }

  msg = VOIDPTR_AT(mem, upb_msg_internalsize(l));

  /* Initialize normal members. */
  memset(msg, 0, l->size);

  /* Initialize internal members. */
  in = upb_msg_getinternal(msg);
  in->unknown = NULL;
  in->unknown_len = 0;
  in->unknown_size = 0;

  if (l->extendable) {
    upb_msg_getinternalwithext(msg, l)->extdict = NULL;
  }

  return msg;
}

void upb_msg_addunknown(upb_msg *msg, const char *data, size_t len,
                        upb_arena *arena) {
  upb_msg_internal *in = upb_msg_getinternal(msg);
  if (len > in->unknown_size - in->unknown_len) {
    upb_alloc *alloc = upb_arena_alloc(arena);
    size_t need = in->unknown_size + len;
    size_t newsize = UPB_MAX(in->unknown_size * 2, need);
    in->unknown = upb_realloc(alloc, in->unknown, in->unknown_size, newsize);
    in->unknown_size = newsize;
  }
  memcpy(in->unknown + in->unknown_len, data, len);
  in->unknown_len += len;
}

const char *upb_msg_getunknown(const upb_msg *msg, size_t *len) {
  const upb_msg_internal *in = upb_msg_getinternal_const(msg);
  *len = in->unknown_len;
  return in->unknown;
}

/** upb_array *****************************************************************/

upb_array *_upb_array_new(upb_arena *a, upb_fieldtype_t type) {
  upb_array *arr = upb_arena_malloc(a, sizeof(upb_array));

  if (!arr) {
    return NULL;
  }

  arr->data = tag_arrptr(NULL, _upb_fieldtype_to_sizelg2[type]);
  arr->len = 0;
  arr->size = 0;

  return arr;
}

bool _upb_array_realloc(upb_array *arr, size_t min_size, upb_arena *arena) {
  size_t new_size = UPB_MAX(arr->size, 4);
  int elem_size_lg2 = arr->data & 7;
  size_t old_bytes = arr->size << elem_size_lg2;
  size_t new_bytes;
  void* ptr = _upb_array_ptr(arr);

  /* Log2 ceiling of size. */
  while (new_size < min_size) new_size *= 2;

  new_bytes = new_size << elem_size_lg2;
  ptr = upb_arena_realloc(arena, ptr, old_bytes, new_bytes);

  if (!ptr) {
    return false;
  }

  arr->data = tag_arrptr(ptr, elem_size_lg2);
  arr->size = new_size;
  return true;
}

static upb_array *getorcreate_array(upb_array **arr_ptr, upb_fieldtype_t type,
                                    upb_arena *arena) {
  upb_array *arr = *arr_ptr;
  if (!arr) {
    arr = _upb_array_new(arena, type);
    if (!arr) return NULL;
    *arr_ptr = arr;
  }
  return arr;
}

static bool resize_array(upb_array *arr, size_t size, upb_arena *arena) {
  if (size > arr->size && !_upb_array_realloc(arr, size, arena)) {
    return false;
  }

  arr->len = size;
  return true;
}

void *_upb_array_resize_fallback(upb_array **arr_ptr, size_t size,
                                 upb_fieldtype_t type, upb_arena *arena) {
  upb_array *arr = getorcreate_array(arr_ptr, type, arena);
  return arr && resize_array(arr, size, arena) ? _upb_array_ptr(arr) : NULL;
}

bool _upb_array_append_fallback(upb_array **arr_ptr, const void *value,
                                upb_fieldtype_t type, upb_arena *arena) {
  upb_array *arr = getorcreate_array(arr_ptr, type, arena);
  size_t elem = arr->len;
  int lg2 = _upb_fieldtype_to_sizelg2[type];
  char *data;

  if (!arr || !resize_array(arr, elem + 1, arena)) return false;

  data = _upb_array_ptr(arr);
  memcpy(data + (elem << lg2), value, 1 << lg2);
  return true;
}

/** upb_map *******************************************************************/

upb_map *_upb_map_new(upb_arena *a, size_t key_size, size_t value_size) {
  upb_map *map = upb_arena_malloc(a, sizeof(upb_map));

  if (!map) {
    return NULL;
  }

  upb_strtable_init2(&map->table, UPB_CTYPE_INT32, upb_arena_alloc(a));
  map->key_size = key_size;
  map->val_size = value_size;

  return map;
}

#undef VOIDPTR_AT
/*
** upb_table Implementation
**
** Implementation is heavily inspired by Lua's ltable.c.
*/


#include <string.h>


#define UPB_MAXARRSIZE 16  /* 64k. */

/* From Chromium. */
#define ARRAY_SIZE(x) \
    ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

static const double MAX_LOAD = 0.85;

/* The minimum utilization of the array part of a mixed hash/array table.  This
 * is a speed/memory-usage tradeoff (though it's not straightforward because of
 * cache effects).  The lower this is, the more memory we'll use. */
static const double MIN_DENSITY = 0.1;

bool is_pow2(uint64_t v) { return v == 0 || (v & (v - 1)) == 0; }

int log2ceil(uint64_t v) {
  int ret = 0;
  bool pow2 = is_pow2(v);
  while (v >>= 1) ret++;
  ret = pow2 ? ret : ret + 1;  /* Ceiling. */
  return UPB_MIN(UPB_MAXARRSIZE, ret);
}

char *upb_strdup(const char *s, upb_alloc *a) {
  return upb_strdup2(s, strlen(s), a);
}

char *upb_strdup2(const char *s, size_t len, upb_alloc *a) {
  size_t n;
  char *p;

  /* Prevent overflow errors. */
  if (len == SIZE_MAX) return NULL;
  /* Always null-terminate, even if binary data; but don't rely on the input to
   * have a null-terminating byte since it may be a raw binary buffer. */
  n = len + 1;
  p = upb_malloc(a, n);
  if (p) {
    memcpy(p, s, len);
    p[len] = 0;
  }
  return p;
}

/* A type to represent the lookup key of either a strtable or an inttable. */
typedef union {
  uintptr_t num;
  struct {
    const char *str;
    size_t len;
  } str;
} lookupkey_t;

static lookupkey_t strkey2(const char *str, size_t len) {
  lookupkey_t k;
  k.str.str = str;
  k.str.len = len;
  return k;
}

static lookupkey_t intkey(uintptr_t key) {
  lookupkey_t k;
  k.num = key;
  return k;
}

typedef uint32_t hashfunc_t(upb_tabkey key);
typedef bool eqlfunc_t(upb_tabkey k1, lookupkey_t k2);

/* Base table (shared code) ***************************************************/

/* For when we need to cast away const. */
static upb_tabent *mutable_entries(upb_table *t) {
  return (upb_tabent*)t->entries;
}

static bool isfull(upb_table *t) {
  if (upb_table_size(t) == 0) {
    return true;
  } else {
    return ((double)(t->count + 1) / upb_table_size(t)) > MAX_LOAD;
  }
}

static bool init(upb_table *t, uint8_t size_lg2, upb_alloc *a) {
  size_t bytes;

  t->count = 0;
  t->size_lg2 = size_lg2;
  t->mask = upb_table_size(t) ? upb_table_size(t) - 1 : 0;
  bytes = upb_table_size(t) * sizeof(upb_tabent);
  if (bytes > 0) {
    t->entries = upb_malloc(a, bytes);
    if (!t->entries) return false;
    memset(mutable_entries(t), 0, bytes);
  } else {
    t->entries = NULL;
  }
  return true;
}

static void uninit(upb_table *t, upb_alloc *a) {
  upb_free(a, mutable_entries(t));
}

static upb_tabent *emptyent(upb_table *t) {
  upb_tabent *e = mutable_entries(t) + upb_table_size(t);
  while (1) { if (upb_tabent_isempty(--e)) return e; UPB_ASSERT(e > t->entries); }
}

static upb_tabent *getentry_mutable(upb_table *t, uint32_t hash) {
  return (upb_tabent*)upb_getentry(t, hash);
}

static const upb_tabent *findentry(const upb_table *t, lookupkey_t key,
                                   uint32_t hash, eqlfunc_t *eql) {
  const upb_tabent *e;

  if (t->size_lg2 == 0) return NULL;
  e = upb_getentry(t, hash);
  if (upb_tabent_isempty(e)) return NULL;
  while (1) {
    if (eql(e->key, key)) return e;
    if ((e = e->next) == NULL) return NULL;
  }
}

static upb_tabent *findentry_mutable(upb_table *t, lookupkey_t key,
                                     uint32_t hash, eqlfunc_t *eql) {
  return (upb_tabent*)findentry(t, key, hash, eql);
}

static bool lookup(const upb_table *t, lookupkey_t key, upb_value *v,
                   uint32_t hash, eqlfunc_t *eql) {
  const upb_tabent *e = findentry(t, key, hash, eql);
  if (e) {
    if (v) {
      _upb_value_setval(v, e->val.val);
    }
    return true;
  } else {
    return false;
  }
}

/* The given key must not already exist in the table. */
static void insert(upb_table *t, lookupkey_t key, upb_tabkey tabkey,
                   upb_value val, uint32_t hash,
                   hashfunc_t *hashfunc, eqlfunc_t *eql) {
  upb_tabent *mainpos_e;
  upb_tabent *our_e;

  UPB_ASSERT(findentry(t, key, hash, eql) == NULL);

  t->count++;
  mainpos_e = getentry_mutable(t, hash);
  our_e = mainpos_e;

  if (upb_tabent_isempty(mainpos_e)) {
    /* Our main position is empty; use it. */
    our_e->next = NULL;
  } else {
    /* Collision. */
    upb_tabent *new_e = emptyent(t);
    /* Head of collider's chain. */
    upb_tabent *chain = getentry_mutable(t, hashfunc(mainpos_e->key));
    if (chain == mainpos_e) {
      /* Existing ent is in its main posisiton (it has the same hash as us, and
       * is the head of our chain).  Insert to new ent and append to this chain. */
      new_e->next = mainpos_e->next;
      mainpos_e->next = new_e;
      our_e = new_e;
    } else {
      /* Existing ent is not in its main position (it is a node in some other
       * chain).  This implies that no existing ent in the table has our hash.
       * Evict it (updating its chain) and use its ent for head of our chain. */
      *new_e = *mainpos_e;  /* copies next. */
      while (chain->next != mainpos_e) {
        chain = (upb_tabent*)chain->next;
        UPB_ASSERT(chain);
      }
      chain->next = new_e;
      our_e = mainpos_e;
      our_e->next = NULL;
    }
  }
  our_e->key = tabkey;
  our_e->val.val = val.val;
  UPB_ASSERT(findentry(t, key, hash, eql) == our_e);
}

static bool rm(upb_table *t, lookupkey_t key, upb_value *val,
               upb_tabkey *removed, uint32_t hash, eqlfunc_t *eql) {
  upb_tabent *chain = getentry_mutable(t, hash);
  if (upb_tabent_isempty(chain)) return false;
  if (eql(chain->key, key)) {
    /* Element to remove is at the head of its chain. */
    t->count--;
    if (val) _upb_value_setval(val, chain->val.val);
    if (removed) *removed = chain->key;
    if (chain->next) {
      upb_tabent *move = (upb_tabent*)chain->next;
      *chain = *move;
      move->key = 0;  /* Make the slot empty. */
    } else {
      chain->key = 0;  /* Make the slot empty. */
    }
    return true;
  } else {
    /* Element to remove is either in a non-head position or not in the
     * table. */
    while (chain->next && !eql(chain->next->key, key)) {
      chain = (upb_tabent*)chain->next;
    }
    if (chain->next) {
      /* Found element to remove. */
      upb_tabent *rm = (upb_tabent*)chain->next;
      t->count--;
      if (val) _upb_value_setval(val, chain->next->val.val);
      if (removed) *removed = rm->key;
      rm->key = 0;  /* Make the slot empty. */
      chain->next = rm->next;
      return true;
    } else {
      /* Element to remove is not in the table. */
      return false;
    }
  }
}

static size_t next(const upb_table *t, size_t i) {
  do {
    if (++i >= upb_table_size(t))
      return SIZE_MAX;
  } while(upb_tabent_isempty(&t->entries[i]));

  return i;
}

static size_t begin(const upb_table *t) {
  return next(t, -1);
}


/* upb_strtable ***************************************************************/

/* A simple "subclass" of upb_table that only adds a hash function for strings. */

static upb_tabkey strcopy(lookupkey_t k2, upb_alloc *a) {
  uint32_t len = (uint32_t) k2.str.len;
  char *str = upb_malloc(a, k2.str.len + sizeof(uint32_t) + 1);
  if (str == NULL) return 0;
  memcpy(str, &len, sizeof(uint32_t));
  memcpy(str + sizeof(uint32_t), k2.str.str, k2.str.len);
  str[sizeof(uint32_t) + k2.str.len] = '\0';
  return (uintptr_t)str;
}

static uint32_t strhash(upb_tabkey key) {
  uint32_t len;
  char *str = upb_tabstr(key, &len);
  return upb_murmur_hash2(str, len, 0);
}

static bool streql(upb_tabkey k1, lookupkey_t k2) {
  uint32_t len;
  char *str = upb_tabstr(k1, &len);
  return len == k2.str.len && memcmp(str, k2.str.str, len) == 0;
}

bool upb_strtable_init2(upb_strtable *t, upb_ctype_t ctype, upb_alloc *a) {
  return init(&t->t, 2, a);
}

void upb_strtable_clear(upb_strtable *t) {
  size_t bytes = upb_table_size(&t->t) * sizeof(upb_tabent);
  t->t.count = 0;
  memset((char*)t->t.entries, 0, bytes);
}

void upb_strtable_uninit2(upb_strtable *t, upb_alloc *a) {
  size_t i;
  for (i = 0; i < upb_table_size(&t->t); i++)
    upb_free(a, (void*)t->t.entries[i].key);
  uninit(&t->t, a);
}

bool upb_strtable_resize(upb_strtable *t, size_t size_lg2, upb_alloc *a) {
  upb_strtable new_table;
  upb_strtable_iter i;

  if (!init(&new_table.t, size_lg2, a))
    return false;
  upb_strtable_begin(&i, t);
  for ( ; !upb_strtable_done(&i); upb_strtable_next(&i)) {
    upb_strview key = upb_strtable_iter_key(&i);
    upb_strtable_insert3(
        &new_table, key.data, key.size,
        upb_strtable_iter_value(&i), a);
  }
  upb_strtable_uninit2(t, a);
  *t = new_table;
  return true;
}

bool upb_strtable_insert3(upb_strtable *t, const char *k, size_t len,
                          upb_value v, upb_alloc *a) {
  lookupkey_t key;
  upb_tabkey tabkey;
  uint32_t hash;

  if (isfull(&t->t)) {
    /* Need to resize.  New table of double the size, add old elements to it. */
    if (!upb_strtable_resize(t, t->t.size_lg2 + 1, a)) {
      return false;
    }
  }

  key = strkey2(k, len);
  tabkey = strcopy(key, a);
  if (tabkey == 0) return false;

  hash = upb_murmur_hash2(key.str.str, key.str.len, 0);
  insert(&t->t, key, tabkey, v, hash, &strhash, &streql);
  return true;
}

bool upb_strtable_lookup2(const upb_strtable *t, const char *key, size_t len,
                          upb_value *v) {
  uint32_t hash = upb_murmur_hash2(key, len, 0);
  return lookup(&t->t, strkey2(key, len), v, hash, &streql);
}

bool upb_strtable_remove3(upb_strtable *t, const char *key, size_t len,
                         upb_value *val, upb_alloc *alloc) {
  uint32_t hash = upb_murmur_hash2(key, len, 0);
  upb_tabkey tabkey;
  if (rm(&t->t, strkey2(key, len), val, &tabkey, hash, &streql)) {
    if (alloc) {
      /* Arena-based allocs don't need to free and won't pass this. */
      upb_free(alloc, (void*)tabkey);
    }
    return true;
  } else {
    return false;
  }
}

/* Iteration */

void upb_strtable_begin(upb_strtable_iter *i, const upb_strtable *t) {
  i->t = t;
  i->index = begin(&t->t);
}

void upb_strtable_next(upb_strtable_iter *i) {
  i->index = next(&i->t->t, i->index);
}

bool upb_strtable_done(const upb_strtable_iter *i) {
  if (!i->t) return true;
  return i->index >= upb_table_size(&i->t->t) ||
         upb_tabent_isempty(str_tabent(i));
}

upb_strview upb_strtable_iter_key(const upb_strtable_iter *i) {
  upb_strview key;
  uint32_t len;
  UPB_ASSERT(!upb_strtable_done(i));
  key.data = upb_tabstr(str_tabent(i)->key, &len);
  key.size = len;
  return key;
}

upb_value upb_strtable_iter_value(const upb_strtable_iter *i) {
  UPB_ASSERT(!upb_strtable_done(i));
  return _upb_value_val(str_tabent(i)->val.val);
}

void upb_strtable_iter_setdone(upb_strtable_iter *i) {
  i->t = NULL;
  i->index = SIZE_MAX;
}

bool upb_strtable_iter_isequal(const upb_strtable_iter *i1,
                               const upb_strtable_iter *i2) {
  if (upb_strtable_done(i1) && upb_strtable_done(i2))
    return true;
  return i1->t == i2->t && i1->index == i2->index;
}


/* upb_inttable ***************************************************************/

/* For inttables we use a hybrid structure where small keys are kept in an
 * array and large keys are put in the hash table. */

static uint32_t inthash(upb_tabkey key) { return upb_inthash(key); }

static bool inteql(upb_tabkey k1, lookupkey_t k2) {
  return k1 == k2.num;
}

static upb_tabval *mutable_array(upb_inttable *t) {
  return (upb_tabval*)t->array;
}

static upb_tabval *inttable_val(upb_inttable *t, uintptr_t key) {
  if (key < t->array_size) {
    return upb_arrhas(t->array[key]) ? &(mutable_array(t)[key]) : NULL;
  } else {
    upb_tabent *e =
        findentry_mutable(&t->t, intkey(key), upb_inthash(key), &inteql);
    return e ? &e->val : NULL;
  }
}

static const upb_tabval *inttable_val_const(const upb_inttable *t,
                                            uintptr_t key) {
  return inttable_val((upb_inttable*)t, key);
}

size_t upb_inttable_count(const upb_inttable *t) {
  return t->t.count + t->array_count;
}

static void check(upb_inttable *t) {
  UPB_UNUSED(t);
#if defined(UPB_DEBUG_TABLE) && !defined(NDEBUG)
  {
    /* This check is very expensive (makes inserts/deletes O(N)). */
    size_t count = 0;
    upb_inttable_iter i;
    upb_inttable_begin(&i, t);
    for(; !upb_inttable_done(&i); upb_inttable_next(&i), count++) {
      UPB_ASSERT(upb_inttable_lookup(t, upb_inttable_iter_key(&i), NULL));
    }
    UPB_ASSERT(count == upb_inttable_count(t));
  }
#endif
}

bool upb_inttable_sizedinit(upb_inttable *t, size_t asize, int hsize_lg2,
                            upb_alloc *a) {
  size_t array_bytes;

  if (!init(&t->t, hsize_lg2, a)) return false;
  /* Always make the array part at least 1 long, so that we know key 0
   * won't be in the hash part, which simplifies things. */
  t->array_size = UPB_MAX(1, asize);
  t->array_count = 0;
  array_bytes = t->array_size * sizeof(upb_value);
  t->array = upb_malloc(a, array_bytes);
  if (!t->array) {
    uninit(&t->t, a);
    return false;
  }
  memset(mutable_array(t), 0xff, array_bytes);
  check(t);
  return true;
}

bool upb_inttable_init2(upb_inttable *t, upb_ctype_t ctype, upb_alloc *a) {
  return upb_inttable_sizedinit(t, 0, 4, a);
}

void upb_inttable_uninit2(upb_inttable *t, upb_alloc *a) {
  uninit(&t->t, a);
  upb_free(a, mutable_array(t));
}

bool upb_inttable_insert2(upb_inttable *t, uintptr_t key, upb_value val,
                          upb_alloc *a) {
  upb_tabval tabval;
  tabval.val = val.val;
  UPB_ASSERT(upb_arrhas(tabval));  /* This will reject (uint64_t)-1.  Fix this. */

  if (key < t->array_size) {
    UPB_ASSERT(!upb_arrhas(t->array[key]));
    t->array_count++;
    mutable_array(t)[key].val = val.val;
  } else {
    if (isfull(&t->t)) {
      /* Need to resize the hash part, but we re-use the array part. */
      size_t i;
      upb_table new_table;

      if (!init(&new_table, t->t.size_lg2 + 1, a)) {
        return false;
      }

      for (i = begin(&t->t); i < upb_table_size(&t->t); i = next(&t->t, i)) {
        const upb_tabent *e = &t->t.entries[i];
        uint32_t hash;
        upb_value v;

        _upb_value_setval(&v, e->val.val);
        hash = upb_inthash(e->key);
        insert(&new_table, intkey(e->key), e->key, v, hash, &inthash, &inteql);
      }

      UPB_ASSERT(t->t.count == new_table.count);

      uninit(&t->t, a);
      t->t = new_table;
    }
    insert(&t->t, intkey(key), key, val, upb_inthash(key), &inthash, &inteql);
  }
  check(t);
  return true;
}

bool upb_inttable_lookup(const upb_inttable *t, uintptr_t key, upb_value *v) {
  const upb_tabval *table_v = inttable_val_const(t, key);
  if (!table_v) return false;
  if (v) _upb_value_setval(v, table_v->val);
  return true;
}

bool upb_inttable_replace(upb_inttable *t, uintptr_t key, upb_value val) {
  upb_tabval *table_v = inttable_val(t, key);
  if (!table_v) return false;
  table_v->val = val.val;
  return true;
}

bool upb_inttable_remove(upb_inttable *t, uintptr_t key, upb_value *val) {
  bool success;
  if (key < t->array_size) {
    if (upb_arrhas(t->array[key])) {
      upb_tabval empty = UPB_TABVALUE_EMPTY_INIT;
      t->array_count--;
      if (val) {
        _upb_value_setval(val, t->array[key].val);
      }
      mutable_array(t)[key] = empty;
      success = true;
    } else {
      success = false;
    }
  } else {
    success = rm(&t->t, intkey(key), val, NULL, upb_inthash(key), &inteql);
  }
  check(t);
  return success;
}

bool upb_inttable_push2(upb_inttable *t, upb_value val, upb_alloc *a) {
  return upb_inttable_insert2(t, upb_inttable_count(t), val, a);
}

upb_value upb_inttable_pop(upb_inttable *t) {
  upb_value val;
  bool ok = upb_inttable_remove(t, upb_inttable_count(t) - 1, &val);
  UPB_ASSERT(ok);
  return val;
}

bool upb_inttable_insertptr2(upb_inttable *t, const void *key, upb_value val,
                             upb_alloc *a) {
  return upb_inttable_insert2(t, (uintptr_t)key, val, a);
}

bool upb_inttable_lookupptr(const upb_inttable *t, const void *key,
                            upb_value *v) {
  return upb_inttable_lookup(t, (uintptr_t)key, v);
}

bool upb_inttable_removeptr(upb_inttable *t, const void *key, upb_value *val) {
  return upb_inttable_remove(t, (uintptr_t)key, val);
}

void upb_inttable_compact2(upb_inttable *t, upb_alloc *a) {
  /* A power-of-two histogram of the table keys. */
  size_t counts[UPB_MAXARRSIZE + 1] = {0};

  /* The max key in each bucket. */
  uintptr_t max[UPB_MAXARRSIZE + 1] = {0};

  upb_inttable_iter i;
  size_t arr_count;
  int size_lg2;
  upb_inttable new_t;

  upb_inttable_begin(&i, t);
  for (; !upb_inttable_done(&i); upb_inttable_next(&i)) {
    uintptr_t key = upb_inttable_iter_key(&i);
    int bucket = log2ceil(key);
    max[bucket] = UPB_MAX(max[bucket], key);
    counts[bucket]++;
  }

  /* Find the largest power of two that satisfies the MIN_DENSITY
   * definition (while actually having some keys). */
  arr_count = upb_inttable_count(t);

  for (size_lg2 = ARRAY_SIZE(counts) - 1; size_lg2 > 0; size_lg2--) {
    if (counts[size_lg2] == 0) {
      /* We can halve again without losing any entries. */
      continue;
    } else if (arr_count >= (1 << size_lg2) * MIN_DENSITY) {
      break;
    }

    arr_count -= counts[size_lg2];
  }

  UPB_ASSERT(arr_count <= upb_inttable_count(t));

  {
    /* Insert all elements into new, perfectly-sized table. */
    size_t arr_size = max[size_lg2] + 1;  /* +1 so arr[max] will fit. */
    size_t hash_count = upb_inttable_count(t) - arr_count;
    size_t hash_size = hash_count ? (hash_count / MAX_LOAD) + 1 : 0;
    int hashsize_lg2 = log2ceil(hash_size);

    upb_inttable_sizedinit(&new_t, arr_size, hashsize_lg2, a);
    upb_inttable_begin(&i, t);
    for (; !upb_inttable_done(&i); upb_inttable_next(&i)) {
      uintptr_t k = upb_inttable_iter_key(&i);
      upb_inttable_insert2(&new_t, k, upb_inttable_iter_value(&i), a);
    }
    UPB_ASSERT(new_t.array_size == arr_size);
    UPB_ASSERT(new_t.t.size_lg2 == hashsize_lg2);
  }
  upb_inttable_uninit2(t, a);
  *t = new_t;
}

/* Iteration. */

static const upb_tabent *int_tabent(const upb_inttable_iter *i) {
  UPB_ASSERT(!i->array_part);
  return &i->t->t.entries[i->index];
}

static upb_tabval int_arrent(const upb_inttable_iter *i) {
  UPB_ASSERT(i->array_part);
  return i->t->array[i->index];
}

void upb_inttable_begin(upb_inttable_iter *i, const upb_inttable *t) {
  i->t = t;
  i->index = -1;
  i->array_part = true;
  upb_inttable_next(i);
}

void upb_inttable_next(upb_inttable_iter *iter) {
  const upb_inttable *t = iter->t;
  if (iter->array_part) {
    while (++iter->index < t->array_size) {
      if (upb_arrhas(int_arrent(iter))) {
        return;
      }
    }
    iter->array_part = false;
    iter->index = begin(&t->t);
  } else {
    iter->index = next(&t->t, iter->index);
  }
}

bool upb_inttable_done(const upb_inttable_iter *i) {
  if (!i->t) return true;
  if (i->array_part) {
    return i->index >= i->t->array_size ||
           !upb_arrhas(int_arrent(i));
  } else {
    return i->index >= upb_table_size(&i->t->t) ||
           upb_tabent_isempty(int_tabent(i));
  }
}

uintptr_t upb_inttable_iter_key(const upb_inttable_iter *i) {
  UPB_ASSERT(!upb_inttable_done(i));
  return i->array_part ? i->index : int_tabent(i)->key;
}

upb_value upb_inttable_iter_value(const upb_inttable_iter *i) {
  UPB_ASSERT(!upb_inttable_done(i));
  return _upb_value_val(
      i->array_part ? i->t->array[i->index].val : int_tabent(i)->val.val);
}

void upb_inttable_iter_setdone(upb_inttable_iter *i) {
  i->t = NULL;
  i->index = SIZE_MAX;
  i->array_part = false;
}

bool upb_inttable_iter_isequal(const upb_inttable_iter *i1,
                                          const upb_inttable_iter *i2) {
  if (upb_inttable_done(i1) && upb_inttable_done(i2))
    return true;
  return i1->t == i2->t && i1->index == i2->index &&
         i1->array_part == i2->array_part;
}

#if defined(UPB_UNALIGNED_READS_OK) || defined(__s390x__)
/* -----------------------------------------------------------------------------
 * MurmurHash2, by Austin Appleby (released as public domain).
 * Reformatted and C99-ified by Joshua Haberman.
 * Note - This code makes a few assumptions about how your machine behaves -
 *   1. We can read a 4-byte value from any address without crashing
 *   2. sizeof(int) == 4 (in upb this limitation is removed by using uint32_t
 * And it has a few limitations -
 *   1. It will not work incrementally.
 *   2. It will not produce the same results on little-endian and big-endian
 *      machines. */
uint32_t upb_murmur_hash2(const void *key, size_t len, uint32_t seed) {
  /* 'm' and 'r' are mixing constants generated offline.
   * They're not really 'magic', they just happen to work well. */
  const uint32_t m = 0x5bd1e995;
  const int32_t r = 24;

  /* Initialize the hash to a 'random' value */
  uint32_t h = seed ^ len;

  /* Mix 4 bytes at a time into the hash */
  const uint8_t * data = (const uint8_t *)key;
  while(len >= 4) {
    uint32_t k;
    memcpy(&k, data, sizeof(k));

    k *= m;
    k ^= k >> r;
    k *= m;

    h *= m;
    h ^= k;

    data += 4;
    len -= 4;
  }

  /* Handle the last few bytes of the input array */
  switch(len) {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0]; h *= m;
  };

  /* Do a few final mixes of the hash to ensure the last few
   * bytes are well-incorporated. */
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;

  return h;
}

#else /* !UPB_UNALIGNED_READS_OK */

/* -----------------------------------------------------------------------------
 * MurmurHashAligned2, by Austin Appleby
 * Same algorithm as MurmurHash2, but only does aligned reads - should be safer
 * on certain platforms.
 * Performance will be lower than MurmurHash2 */

#define MIX(h,k,m) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }

uint32_t upb_murmur_hash2(const void * key, size_t len, uint32_t seed) {
  const uint32_t m = 0x5bd1e995;
  const int32_t r = 24;
  const uint8_t * data = (const uint8_t *)key;
  uint32_t h = (uint32_t)(seed ^ len);
  uint8_t align = (uintptr_t)data & 3;

  if(align && (len >= 4)) {
    /* Pre-load the temp registers */
    uint32_t t = 0, d = 0;
    int32_t sl;
    int32_t sr;

    switch(align) {
      case 1: t |= data[2] << 16;
      case 2: t |= data[1] << 8;
      case 3: t |= data[0];
    }

    t <<= (8 * align);

    data += 4-align;
    len -= 4-align;

    sl = 8 * (4-align);
    sr = 8 * align;

    /* Mix */

    while(len >= 4) {
      uint32_t k;

      d = *(uint32_t *)data;
      t = (t >> sr) | (d << sl);

      k = t;

      MIX(h,k,m);

      t = d;

      data += 4;
      len -= 4;
    }

    /* Handle leftover data in temp registers */

    d = 0;

    if(len >= align) {
      uint32_t k;

      switch(align) {
        case 3: d |= data[2] << 16;
        case 2: d |= data[1] << 8;
        case 1: d |= data[0];
      }

      k = (t >> sr) | (d << sl);
      MIX(h,k,m);

      data += align;
      len -= align;

      /* ----------
       * Handle tail bytes */

      switch(len) {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0]; h *= m;
      };
    } else {
      switch(len) {
        case 3: d |= data[2] << 16;
        case 2: d |= data[1] << 8;
        case 1: d |= data[0];
        case 0: h ^= (t >> sr) | (d << sl); h *= m;
      }
    }

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
  } else {
    while(len >= 4) {
      uint32_t k = *(uint32_t *)data;

      MIX(h,k,m);

      data += 4;
      len -= 4;
    }

    /* ----------
     * Handle tail bytes */

    switch(len) {
      case 3: h ^= data[2] << 16;
      case 2: h ^= data[1] << 8;
      case 1: h ^= data[0]; h *= m;
    };

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
  }
}
#undef MIX

#endif /* UPB_UNALIGNED_READS_OK */


#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Guarantee null-termination and provide ellipsis truncation.
 * It may be tempting to "optimize" this by initializing these final
 * four bytes up-front and then being careful never to overwrite them,
 * this is safer and simpler. */
static void nullz(upb_status *status) {
  const char *ellipsis = "...";
  size_t len = strlen(ellipsis);
  UPB_ASSERT(sizeof(status->msg) > len);
  memcpy(status->msg + sizeof(status->msg) - len, ellipsis, len);
}

/* upb_status *****************************************************************/

void upb_status_clear(upb_status *status) {
  if (!status) return;
  status->ok = true;
  status->msg[0] = '\0';
}

bool upb_ok(const upb_status *status) { return status->ok; }

const char *upb_status_errmsg(const upb_status *status) { return status->msg; }

void upb_status_seterrmsg(upb_status *status, const char *msg) {
  if (!status) return;
  status->ok = false;
  strncpy(status->msg, msg, sizeof(status->msg));
  nullz(status);
}

void upb_status_seterrf(upb_status *status, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  upb_status_vseterrf(status, fmt, args);
  va_end(args);
}

void upb_status_vseterrf(upb_status *status, const char *fmt, va_list args) {
  if (!status) return;
  status->ok = false;
  _upb_vsnprintf(status->msg, sizeof(status->msg), fmt, args);
  nullz(status);
}

/* upb_alloc ******************************************************************/

static void *upb_global_allocfunc(upb_alloc *alloc, void *ptr, size_t oldsize,
                                  size_t size) {
  UPB_UNUSED(alloc);
  UPB_UNUSED(oldsize);
  if (size == 0) {
    free(ptr);
    return NULL;
  } else {
    return realloc(ptr, size);
  }
}

upb_alloc upb_alloc_global = {&upb_global_allocfunc};

/* upb_arena ******************************************************************/

/* Be conservative and choose 16 in case anyone is using SSE. */
static const size_t maxalign = 16;

static size_t align_up_max(size_t size) {
  return ((size + maxalign - 1) / maxalign) * maxalign;
}

struct upb_arena {
  /* We implement the allocator interface.
   * This must be the first member of upb_arena! */
  upb_alloc alloc;

  /* Allocator to allocate arena blocks.  We are responsible for freeing these
   * when we are destroyed. */
  upb_alloc *block_alloc;

  size_t bytes_allocated;
  size_t next_block_size;
  size_t max_block_size;

  /* Linked list of blocks.  Points to an arena_block, defined in env.c */
  void *block_head;

  /* Cleanup entries.  Pointer to a cleanup_ent, defined in env.c */
  void *cleanup_head;
};

typedef struct mem_block {
  struct mem_block *next;
  size_t size;
  size_t used;
  bool owned;
  /* Data follows. */
} mem_block;

typedef struct cleanup_ent {
  struct cleanup_ent *next;
  upb_cleanup_func *cleanup;
  void *ud;
} cleanup_ent;

static void upb_arena_addblock(upb_arena *a, void *ptr, size_t size,
                               bool owned) {
  mem_block *block = ptr;

  block->next = a->block_head;
  block->size = size;
  block->used = align_up_max(sizeof(mem_block));
  block->owned = owned;

  a->block_head = block;

  /* TODO(haberman): ASAN poison. */
}

static mem_block *upb_arena_allocblock(upb_arena *a, size_t size) {
  size_t block_size = UPB_MAX(size, a->next_block_size) + sizeof(mem_block);
  mem_block *block = upb_malloc(a->block_alloc, block_size);

  if (!block) {
    return NULL;
  }

  upb_arena_addblock(a, block, block_size, true);
  a->next_block_size = UPB_MIN(block_size * 2, a->max_block_size);

  return block;
}

static void *upb_arena_doalloc(upb_alloc *alloc, void *ptr, size_t oldsize,
                               size_t size) {
  upb_arena *a = (upb_arena*)alloc;  /* upb_alloc is initial member. */
  mem_block *block = a->block_head;
  void *ret;

  if (size == 0) {
    return NULL;  /* We are an arena, don't need individual frees. */
  }

  size = align_up_max(size);

  /* TODO(haberman): special-case if this is a realloc of the last alloc? */

  if (!block || block->size - block->used < size) {
    /* Slow path: have to allocate a new block. */
    block = upb_arena_allocblock(a, size);

    if (!block) {
      return NULL;  /* Out of memory. */
    }
  }

  ret = (char*)block + block->used;
  block->used += size;

  if (oldsize > 0) {
    memcpy(ret, ptr, oldsize);  /* Preserve existing data. */
  }

  /* TODO(haberman): ASAN unpoison. */

  a->bytes_allocated += size;
  return ret;
}

/* Public Arena API ***********************************************************/

#define upb_alignof(type) offsetof (struct { char c; type member; }, member)

upb_arena *upb_arena_init(void *mem, size_t n, upb_alloc *alloc) {
  const size_t first_block_overhead = sizeof(upb_arena) + sizeof(mem_block);
  upb_arena *a;
  bool owned = false;

  /* Round block size down to alignof(*a) since we will allocate the arena
   * itself at the end. */
  n &= ~(upb_alignof(upb_arena) - 1);

  if (n < first_block_overhead) {
    /* We need to malloc the initial block. */
    n = first_block_overhead + 256;
    owned = true;
    if (!alloc || !(mem = upb_malloc(alloc, n))) {
      return NULL;
    }
  }

  a = (void*)((char*)mem + n - sizeof(*a));
  n -= sizeof(*a);

  a->alloc.func = &upb_arena_doalloc;
  a->block_alloc = &upb_alloc_global;
  a->bytes_allocated = 0;
  a->next_block_size = 256;
  a->max_block_size = 16384;
  a->cleanup_head = NULL;
  a->block_head = NULL;
  a->block_alloc = alloc;

  upb_arena_addblock(a, mem, n, owned);

  return a;
}

#undef upb_alignof

void upb_arena_free(upb_arena *a) {
  cleanup_ent *ent = a->cleanup_head;
  mem_block *block = a->block_head;

  while (ent) {
    ent->cleanup(ent->ud);
    ent = ent->next;
  }

  /* Must do this after running cleanup functions, because this will delete
   * the memory we store our cleanup entries in! */
  while (block) {
    /* Load first since we are deleting block. */
    mem_block *next = block->next;

    if (block->owned) {
      upb_free(a->block_alloc, block);
    }

    block = next;
  }
}

bool upb_arena_addcleanup(upb_arena *a, void *ud, upb_cleanup_func *func) {
  cleanup_ent *ent = upb_malloc(&a->alloc, sizeof(cleanup_ent));
  if (!ent) {
    return false;  /* Out of memory. */
  }

  ent->cleanup = func;
  ent->ud = ud;
  ent->next = a->cleanup_head;
  a->cleanup_head = ent;

  return true;
}

size_t upb_arena_bytesallocated(const upb_arena *a) {
  return a->bytes_allocated;
}
/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     google/protobuf/descriptor.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include <stddef.h>


static const upb_msglayout *const google_protobuf_FileDescriptorSet_submsgs[1] = {
  &google_protobuf_FileDescriptorProto_msginit,
};

static const upb_msglayout_field google_protobuf_FileDescriptorSet__fields[1] = {
  {1, UPB_SIZE(0, 0), 0, 0, 11, 3},
};

const upb_msglayout google_protobuf_FileDescriptorSet_msginit = {
  &google_protobuf_FileDescriptorSet_submsgs[0],
  &google_protobuf_FileDescriptorSet__fields[0],
  UPB_SIZE(4, 8), 1, false,
};

static const upb_msglayout *const google_protobuf_FileDescriptorProto_submsgs[6] = {
  &google_protobuf_DescriptorProto_msginit,
  &google_protobuf_EnumDescriptorProto_msginit,
  &google_protobuf_FieldDescriptorProto_msginit,
  &google_protobuf_FileOptions_msginit,
  &google_protobuf_ServiceDescriptorProto_msginit,
  &google_protobuf_SourceCodeInfo_msginit,
};

static const upb_msglayout_field google_protobuf_FileDescriptorProto__fields[12] = {
  {1, UPB_SIZE(4, 8), 1, 0, 9, 1},
  {2, UPB_SIZE(12, 24), 2, 0, 9, 1},
  {3, UPB_SIZE(36, 72), 0, 0, 9, 3},
  {4, UPB_SIZE(40, 80), 0, 0, 11, 3},
  {5, UPB_SIZE(44, 88), 0, 1, 11, 3},
  {6, UPB_SIZE(48, 96), 0, 4, 11, 3},
  {7, UPB_SIZE(52, 104), 0, 2, 11, 3},
  {8, UPB_SIZE(28, 56), 4, 3, 11, 1},
  {9, UPB_SIZE(32, 64), 5, 5, 11, 1},
  {10, UPB_SIZE(56, 112), 0, 0, 5, 3},
  {11, UPB_SIZE(60, 120), 0, 0, 5, 3},
  {12, UPB_SIZE(20, 40), 3, 0, 9, 1},
};

const upb_msglayout google_protobuf_FileDescriptorProto_msginit = {
  &google_protobuf_FileDescriptorProto_submsgs[0],
  &google_protobuf_FileDescriptorProto__fields[0],
  UPB_SIZE(64, 128), 12, false,
};

static const upb_msglayout *const google_protobuf_DescriptorProto_submsgs[8] = {
  &google_protobuf_DescriptorProto_msginit,
  &google_protobuf_DescriptorProto_ExtensionRange_msginit,
  &google_protobuf_DescriptorProto_ReservedRange_msginit,
  &google_protobuf_EnumDescriptorProto_msginit,
  &google_protobuf_FieldDescriptorProto_msginit,
  &google_protobuf_MessageOptions_msginit,
  &google_protobuf_OneofDescriptorProto_msginit,
};

static const upb_msglayout_field google_protobuf_DescriptorProto__fields[10] = {
  {1, UPB_SIZE(4, 8), 1, 0, 9, 1},
  {2, UPB_SIZE(16, 32), 0, 4, 11, 3},
  {3, UPB_SIZE(20, 40), 0, 0, 11, 3},
  {4, UPB_SIZE(24, 48), 0, 3, 11, 3},
  {5, UPB_SIZE(28, 56), 0, 1, 11, 3},
  {6, UPB_SIZE(32, 64), 0, 4, 11, 3},
  {7, UPB_SIZE(12, 24), 2, 5, 11, 1},
  {8, UPB_SIZE(36, 72), 0, 6, 11, 3},
  {9, UPB_SIZE(40, 80), 0, 2, 11, 3},
  {10, UPB_SIZE(44, 88), 0, 0, 9, 3},
};

const upb_msglayout google_protobuf_DescriptorProto_msginit = {
  &google_protobuf_DescriptorProto_submsgs[0],
  &google_protobuf_DescriptorProto__fields[0],
  UPB_SIZE(48, 96), 10, false,
};

static const upb_msglayout *const google_protobuf_DescriptorProto_ExtensionRange_submsgs[1] = {
  &google_protobuf_ExtensionRangeOptions_msginit,
};

static const upb_msglayout_field google_protobuf_DescriptorProto_ExtensionRange__fields[3] = {
  {1, UPB_SIZE(4, 4), 1, 0, 5, 1},
  {2, UPB_SIZE(8, 8), 2, 0, 5, 1},
  {3, UPB_SIZE(12, 16), 3, 0, 11, 1},
};

const upb_msglayout google_protobuf_DescriptorProto_ExtensionRange_msginit = {
  &google_protobuf_DescriptorProto_ExtensionRange_submsgs[0],
  &google_protobuf_DescriptorProto_ExtensionRange__fields[0],
  UPB_SIZE(16, 24), 3, false,
};

static const upb_msglayout_field google_protobuf_DescriptorProto_ReservedRange__fields[2] = {
  {1, UPB_SIZE(4, 4), 1, 0, 5, 1},
  {2, UPB_SIZE(8, 8), 2, 0, 5, 1},
};

const upb_msglayout google_protobuf_DescriptorProto_ReservedRange_msginit = {
  NULL,
  &google_protobuf_DescriptorProto_ReservedRange__fields[0],
  UPB_SIZE(12, 12), 2, false,
};

static const upb_msglayout *const google_protobuf_ExtensionRangeOptions_submsgs[1] = {
  &google_protobuf_UninterpretedOption_msginit,
};

static const upb_msglayout_field google_protobuf_ExtensionRangeOptions__fields[1] = {
  {999, UPB_SIZE(0, 0), 0, 0, 11, 3},
};

const upb_msglayout google_protobuf_ExtensionRangeOptions_msginit = {
  &google_protobuf_ExtensionRangeOptions_submsgs[0],
  &google_protobuf_ExtensionRangeOptions__fields[0],
  UPB_SIZE(4, 8), 1, false,
};

static const upb_msglayout *const google_protobuf_FieldDescriptorProto_submsgs[1] = {
  &google_protobuf_FieldOptions_msginit,
};

static const upb_msglayout_field google_protobuf_FieldDescriptorProto__fields[10] = {
  {1, UPB_SIZE(32, 32), 5, 0, 9, 1},
  {2, UPB_SIZE(40, 48), 6, 0, 9, 1},
  {3, UPB_SIZE(24, 24), 3, 0, 5, 1},
  {4, UPB_SIZE(8, 8), 1, 0, 14, 1},
  {5, UPB_SIZE(16, 16), 2, 0, 14, 1},
  {6, UPB_SIZE(48, 64), 7, 0, 9, 1},
  {7, UPB_SIZE(56, 80), 8, 0, 9, 1},
  {8, UPB_SIZE(72, 112), 10, 0, 11, 1},
  {9, UPB_SIZE(28, 28), 4, 0, 5, 1},
  {10, UPB_SIZE(64, 96), 9, 0, 9, 1},
};

const upb_msglayout google_protobuf_FieldDescriptorProto_msginit = {
  &google_protobuf_FieldDescriptorProto_submsgs[0],
  &google_protobuf_FieldDescriptorProto__fields[0],
  UPB_SIZE(80, 128), 10, false,
};

static const upb_msglayout *const google_protobuf_OneofDescriptorProto_submsgs[1] = {
  &google_protobuf_OneofOptions_msginit,
};

static const upb_msglayout_field google_protobuf_OneofDescriptorProto__fields[2] = {
  {1, UPB_SIZE(4, 8), 1, 0, 9, 1},
  {2, UPB_SIZE(12, 24), 2, 0, 11, 1},
};

const upb_msglayout google_protobuf_OneofDescriptorProto_msginit = {
  &google_protobuf_OneofDescriptorProto_submsgs[0],
  &google_protobuf_OneofDescriptorProto__fields[0],
  UPB_SIZE(16, 32), 2, false,
};

static const upb_msglayout *const google_protobuf_EnumDescriptorProto_submsgs[3] = {
  &google_protobuf_EnumDescriptorProto_EnumReservedRange_msginit,
  &google_protobuf_EnumOptions_msginit,
  &google_protobuf_EnumValueDescriptorProto_msginit,
};

static const upb_msglayout_field google_protobuf_EnumDescriptorProto__fields[5] = {
  {1, UPB_SIZE(4, 8), 1, 0, 9, 1},
  {2, UPB_SIZE(16, 32), 0, 2, 11, 3},
  {3, UPB_SIZE(12, 24), 2, 1, 11, 1},
  {4, UPB_SIZE(20, 40), 0, 0, 11, 3},
  {5, UPB_SIZE(24, 48), 0, 0, 9, 3},
};

const upb_msglayout google_protobuf_EnumDescriptorProto_msginit = {
  &google_protobuf_EnumDescriptorProto_submsgs[0],
  &google_protobuf_EnumDescriptorProto__fields[0],
  UPB_SIZE(32, 64), 5, false,
};

static const upb_msglayout_field google_protobuf_EnumDescriptorProto_EnumReservedRange__fields[2] = {
  {1, UPB_SIZE(4, 4), 1, 0, 5, 1},
  {2, UPB_SIZE(8, 8), 2, 0, 5, 1},
};

const upb_msglayout google_protobuf_EnumDescriptorProto_EnumReservedRange_msginit = {
  NULL,
  &google_protobuf_EnumDescriptorProto_EnumReservedRange__fields[0],
  UPB_SIZE(12, 12), 2, false,
};

static const upb_msglayout *const google_protobuf_EnumValueDescriptorProto_submsgs[1] = {
  &google_protobuf_EnumValueOptions_msginit,
};

static const upb_msglayout_field google_protobuf_EnumValueDescriptorProto__fields[3] = {
  {1, UPB_SIZE(8, 8), 2, 0, 9, 1},
  {2, UPB_SIZE(4, 4), 1, 0, 5, 1},
  {3, UPB_SIZE(16, 24), 3, 0, 11, 1},
};

const upb_msglayout google_protobuf_EnumValueDescriptorProto_msginit = {
  &google_protobuf_EnumValueDescriptorProto_submsgs[0],
  &google_protobuf_EnumValueDescriptorProto__fields[0],
  UPB_SIZE(24, 32), 3, false,
};

static const upb_msglayout *const google_protobuf_ServiceDescriptorProto_submsgs[2] = {
  &google_protobuf_MethodDescriptorProto_msginit,
  &google_protobuf_ServiceOptions_msginit,
};

static const upb_msglayout_field google_protobuf_ServiceDescriptorProto__fields[3] = {
  {1, UPB_SIZE(4, 8), 1, 0, 9, 1},
  {2, UPB_SIZE(16, 32), 0, 0, 11, 3},
  {3, UPB_SIZE(12, 24), 2, 1, 11, 1},
};

const upb_msglayout google_protobuf_ServiceDescriptorProto_msginit = {
  &google_protobuf_ServiceDescriptorProto_submsgs[0],
  &google_protobuf_ServiceDescriptorProto__fields[0],
  UPB_SIZE(24, 48), 3, false,
};

static const upb_msglayout *const google_protobuf_MethodDescriptorProto_submsgs[1] = {
  &google_protobuf_MethodOptions_msginit,
};

static const upb_msglayout_field google_protobuf_MethodDescriptorProto__fields[6] = {
  {1, UPB_SIZE(4, 8), 3, 0, 9, 1},
  {2, UPB_SIZE(12, 24), 4, 0, 9, 1},
  {3, UPB_SIZE(20, 40), 5, 0, 9, 1},
  {4, UPB_SIZE(28, 56), 6, 0, 11, 1},
  {5, UPB_SIZE(1, 1), 1, 0, 8, 1},
  {6, UPB_SIZE(2, 2), 2, 0, 8, 1},
};

const upb_msglayout google_protobuf_MethodDescriptorProto_msginit = {
  &google_protobuf_MethodDescriptorProto_submsgs[0],
  &google_protobuf_MethodDescriptorProto__fields[0],
  UPB_SIZE(32, 64), 6, false,
};

static const upb_msglayout *const google_protobuf_FileOptions_submsgs[1] = {
  &google_protobuf_UninterpretedOption_msginit,
};

static const upb_msglayout_field google_protobuf_FileOptions__fields[21] = {
  {1, UPB_SIZE(28, 32), 11, 0, 9, 1},
  {8, UPB_SIZE(36, 48), 12, 0, 9, 1},
  {9, UPB_SIZE(8, 8), 1, 0, 14, 1},
  {10, UPB_SIZE(16, 16), 2, 0, 8, 1},
  {11, UPB_SIZE(44, 64), 13, 0, 9, 1},
  {16, UPB_SIZE(17, 17), 3, 0, 8, 1},
  {17, UPB_SIZE(18, 18), 4, 0, 8, 1},
  {18, UPB_SIZE(19, 19), 5, 0, 8, 1},
  {20, UPB_SIZE(20, 20), 6, 0, 8, 1},
  {23, UPB_SIZE(21, 21), 7, 0, 8, 1},
  {27, UPB_SIZE(22, 22), 8, 0, 8, 1},
  {31, UPB_SIZE(23, 23), 9, 0, 8, 1},
  {36, UPB_SIZE(52, 80), 14, 0, 9, 1},
  {37, UPB_SIZE(60, 96), 15, 0, 9, 1},
  {39, UPB_SIZE(68, 112), 16, 0, 9, 1},
  {40, UPB_SIZE(76, 128), 17, 0, 9, 1},
  {41, UPB_SIZE(84, 144), 18, 0, 9, 1},
  {42, UPB_SIZE(24, 24), 10, 0, 8, 1},
  {44, UPB_SIZE(92, 160), 19, 0, 9, 1},
  {45, UPB_SIZE(100, 176), 20, 0, 9, 1},
  {999, UPB_SIZE(108, 192), 0, 0, 11, 3},
};

const upb_msglayout google_protobuf_FileOptions_msginit = {
  &google_protobuf_FileOptions_submsgs[0],
  &google_protobuf_FileOptions__fields[0],
  UPB_SIZE(112, 208), 21, false,
};

static const upb_msglayout *const google_protobuf_MessageOptions_submsgs[1] = {
  &google_protobuf_UninterpretedOption_msginit,
};

static const upb_msglayout_field google_protobuf_MessageOptions__fields[5] = {
  {1, UPB_SIZE(1, 1), 1, 0, 8, 1},
  {2, UPB_SIZE(2, 2), 2, 0, 8, 1},
  {3, UPB_SIZE(3, 3), 3, 0, 8, 1},
  {7, UPB_SIZE(4, 4), 4, 0, 8, 1},
  {999, UPB_SIZE(8, 8), 0, 0, 11, 3},
};

const upb_msglayout google_protobuf_MessageOptions_msginit = {
  &google_protobuf_MessageOptions_submsgs[0],
  &google_protobuf_MessageOptions__fields[0],
  UPB_SIZE(12, 16), 5, false,
};

static const upb_msglayout *const google_protobuf_FieldOptions_submsgs[1] = {
  &google_protobuf_UninterpretedOption_msginit,
};

static const upb_msglayout_field google_protobuf_FieldOptions__fields[7] = {
  {1, UPB_SIZE(8, 8), 1, 0, 14, 1},
  {2, UPB_SIZE(24, 24), 3, 0, 8, 1},
  {3, UPB_SIZE(25, 25), 4, 0, 8, 1},
  {5, UPB_SIZE(26, 26), 5, 0, 8, 1},
  {6, UPB_SIZE(16, 16), 2, 0, 14, 1},
  {10, UPB_SIZE(27, 27), 6, 0, 8, 1},
  {999, UPB_SIZE(28, 32), 0, 0, 11, 3},
};

const upb_msglayout google_protobuf_FieldOptions_msginit = {
  &google_protobuf_FieldOptions_submsgs[0],
  &google_protobuf_FieldOptions__fields[0],
  UPB_SIZE(32, 40), 7, false,
};

static const upb_msglayout *const google_protobuf_OneofOptions_submsgs[1] = {
  &google_protobuf_UninterpretedOption_msginit,
};

static const upb_msglayout_field google_protobuf_OneofOptions__fields[1] = {
  {999, UPB_SIZE(0, 0), 0, 0, 11, 3},
};

const upb_msglayout google_protobuf_OneofOptions_msginit = {
  &google_protobuf_OneofOptions_submsgs[0],
  &google_protobuf_OneofOptions__fields[0],
  UPB_SIZE(4, 8), 1, false,
};

static const upb_msglayout *const google_protobuf_EnumOptions_submsgs[1] = {
  &google_protobuf_UninterpretedOption_msginit,
};

static const upb_msglayout_field google_protobuf_EnumOptions__fields[3] = {
  {2, UPB_SIZE(1, 1), 1, 0, 8, 1},
  {3, UPB_SIZE(2, 2), 2, 0, 8, 1},
  {999, UPB_SIZE(4, 8), 0, 0, 11, 3},
};

const upb_msglayout google_protobuf_EnumOptions_msginit = {
  &google_protobuf_EnumOptions_submsgs[0],
  &google_protobuf_EnumOptions__fields[0],
  UPB_SIZE(8, 16), 3, false,
};

static const upb_msglayout *const google_protobuf_EnumValueOptions_submsgs[1] = {
  &google_protobuf_UninterpretedOption_msginit,
};

static const upb_msglayout_field google_protobuf_EnumValueOptions__fields[2] = {
  {1, UPB_SIZE(1, 1), 1, 0, 8, 1},
  {999, UPB_SIZE(4, 8), 0, 0, 11, 3},
};

const upb_msglayout google_protobuf_EnumValueOptions_msginit = {
  &google_protobuf_EnumValueOptions_submsgs[0],
  &google_protobuf_EnumValueOptions__fields[0],
  UPB_SIZE(8, 16), 2, false,
};

static const upb_msglayout *const google_protobuf_ServiceOptions_submsgs[1] = {
  &google_protobuf_UninterpretedOption_msginit,
};

static const upb_msglayout_field google_protobuf_ServiceOptions__fields[2] = {
  {33, UPB_SIZE(1, 1), 1, 0, 8, 1},
  {999, UPB_SIZE(4, 8), 0, 0, 11, 3},
};

const upb_msglayout google_protobuf_ServiceOptions_msginit = {
  &google_protobuf_ServiceOptions_submsgs[0],
  &google_protobuf_ServiceOptions__fields[0],
  UPB_SIZE(8, 16), 2, false,
};

static const upb_msglayout *const google_protobuf_MethodOptions_submsgs[1] = {
  &google_protobuf_UninterpretedOption_msginit,
};

static const upb_msglayout_field google_protobuf_MethodOptions__fields[3] = {
  {33, UPB_SIZE(16, 16), 2, 0, 8, 1},
  {34, UPB_SIZE(8, 8), 1, 0, 14, 1},
  {999, UPB_SIZE(20, 24), 0, 0, 11, 3},
};

const upb_msglayout google_protobuf_MethodOptions_msginit = {
  &google_protobuf_MethodOptions_submsgs[0],
  &google_protobuf_MethodOptions__fields[0],
  UPB_SIZE(24, 32), 3, false,
};

static const upb_msglayout *const google_protobuf_UninterpretedOption_submsgs[1] = {
  &google_protobuf_UninterpretedOption_NamePart_msginit,
};

static const upb_msglayout_field google_protobuf_UninterpretedOption__fields[7] = {
  {2, UPB_SIZE(56, 80), 0, 0, 11, 3},
  {3, UPB_SIZE(32, 32), 4, 0, 9, 1},
  {4, UPB_SIZE(8, 8), 1, 0, 4, 1},
  {5, UPB_SIZE(16, 16), 2, 0, 3, 1},
  {6, UPB_SIZE(24, 24), 3, 0, 1, 1},
  {7, UPB_SIZE(40, 48), 5, 0, 12, 1},
  {8, UPB_SIZE(48, 64), 6, 0, 9, 1},
};

const upb_msglayout google_protobuf_UninterpretedOption_msginit = {
  &google_protobuf_UninterpretedOption_submsgs[0],
  &google_protobuf_UninterpretedOption__fields[0],
  UPB_SIZE(64, 96), 7, false,
};

static const upb_msglayout_field google_protobuf_UninterpretedOption_NamePart__fields[2] = {
  {1, UPB_SIZE(4, 8), 2, 0, 9, 2},
  {2, UPB_SIZE(1, 1), 1, 0, 8, 2},
};

const upb_msglayout google_protobuf_UninterpretedOption_NamePart_msginit = {
  NULL,
  &google_protobuf_UninterpretedOption_NamePart__fields[0],
  UPB_SIZE(16, 32), 2, false,
};

static const upb_msglayout *const google_protobuf_SourceCodeInfo_submsgs[1] = {
  &google_protobuf_SourceCodeInfo_Location_msginit,
};

static const upb_msglayout_field google_protobuf_SourceCodeInfo__fields[1] = {
  {1, UPB_SIZE(0, 0), 0, 0, 11, 3},
};

const upb_msglayout google_protobuf_SourceCodeInfo_msginit = {
  &google_protobuf_SourceCodeInfo_submsgs[0],
  &google_protobuf_SourceCodeInfo__fields[0],
  UPB_SIZE(4, 8), 1, false,
};

static const upb_msglayout_field google_protobuf_SourceCodeInfo_Location__fields[5] = {
  {1, UPB_SIZE(20, 40), 0, 0, 5, 3},
  {2, UPB_SIZE(24, 48), 0, 0, 5, 3},
  {3, UPB_SIZE(4, 8), 1, 0, 9, 1},
  {4, UPB_SIZE(12, 24), 2, 0, 9, 1},
  {6, UPB_SIZE(28, 56), 0, 0, 9, 3},
};

const upb_msglayout google_protobuf_SourceCodeInfo_Location_msginit = {
  NULL,
  &google_protobuf_SourceCodeInfo_Location__fields[0],
  UPB_SIZE(32, 64), 5, false,
};

static const upb_msglayout *const google_protobuf_GeneratedCodeInfo_submsgs[1] = {
  &google_protobuf_GeneratedCodeInfo_Annotation_msginit,
};

static const upb_msglayout_field google_protobuf_GeneratedCodeInfo__fields[1] = {
  {1, UPB_SIZE(0, 0), 0, 0, 11, 3},
};

const upb_msglayout google_protobuf_GeneratedCodeInfo_msginit = {
  &google_protobuf_GeneratedCodeInfo_submsgs[0],
  &google_protobuf_GeneratedCodeInfo__fields[0],
  UPB_SIZE(4, 8), 1, false,
};

static const upb_msglayout_field google_protobuf_GeneratedCodeInfo_Annotation__fields[4] = {
  {1, UPB_SIZE(20, 32), 0, 0, 5, 3},
  {2, UPB_SIZE(12, 16), 3, 0, 9, 1},
  {3, UPB_SIZE(4, 4), 1, 0, 5, 1},
  {4, UPB_SIZE(8, 8), 2, 0, 5, 1},
};

const upb_msglayout google_protobuf_GeneratedCodeInfo_Annotation_msginit = {
  NULL,
  &google_protobuf_GeneratedCodeInfo_Annotation__fields[0],
  UPB_SIZE(24, 48), 4, false,
};




#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
  size_t len;
  char str[1];  /* Null-terminated string data follows. */
} str_t;

static str_t *newstr(upb_alloc *alloc, const char *data, size_t len) {
  str_t *ret = upb_malloc(alloc, sizeof(*ret) + len);
  if (!ret) return NULL;
  ret->len = len;
  memcpy(ret->str, data, len);
  ret->str[len] = '\0';
  return ret;
}

struct upb_fielddef {
  const upb_filedef *file;
  const upb_msgdef *msgdef;
  const char *full_name;
  const char *json_name;
  union {
    int64_t sint;
    uint64_t uint;
    double dbl;
    float flt;
    bool boolean;
    str_t *str;
  } defaultval;
  const upb_oneofdef *oneof;
  union {
    const upb_msgdef *msgdef;
    const upb_enumdef *enumdef;
    const google_protobuf_FieldDescriptorProto *unresolved;
  } sub;
  uint32_t number_;
  uint16_t index_;
  uint16_t layout_index;
  uint32_t selector_base;  /* Used to index into a upb::Handlers table. */
  bool is_extension_;
  bool lazy_;
  bool packed_;
  upb_descriptortype_t type_;
  upb_label_t label_;
};

struct upb_msgdef {
  const upb_msglayout *layout;
  const upb_filedef *file;
  const char *full_name;
  uint32_t selector_count;
  uint32_t submsg_field_count;

  /* Tables for looking up fields by number and name. */
  upb_inttable itof;
  upb_strtable ntof;

  const upb_fielddef *fields;
  const upb_oneofdef *oneofs;
  int field_count;
  int oneof_count;

  /* Is this a map-entry message? */
  bool map_entry;
  upb_wellknowntype_t well_known_type;

  /* TODO(haberman): proper extension ranges (there can be multiple). */
};

struct upb_enumdef {
  const upb_filedef *file;
  const char *full_name;
  upb_strtable ntoi;
  upb_inttable iton;
  int32_t defaultval;
};

struct upb_oneofdef {
  const upb_msgdef *parent;
  const char *full_name;
  uint32_t index;
  upb_strtable ntof;
  upb_inttable itof;
};

struct upb_filedef {
  const char *name;
  const char *package;
  const char *phpprefix;
  const char *phpnamespace;
  upb_syntax_t syntax;

  const upb_filedef **deps;
  const upb_msgdef *msgs;
  const upb_enumdef *enums;
  const upb_fielddef *exts;

  int dep_count;
  int msg_count;
  int enum_count;
  int ext_count;
};

struct upb_symtab {
  upb_arena *arena;
  upb_strtable syms;  /* full_name -> packed def ptr */
  upb_strtable files;  /* file_name -> upb_filedef* */
};

/* Inside a symtab we store tagged pointers to specific def types. */
typedef enum {
  UPB_DEFTYPE_FIELD = 0,

  /* Only inside symtab table. */
  UPB_DEFTYPE_MSG = 1,
  UPB_DEFTYPE_ENUM = 2,

  /* Only inside message table. */
  UPB_DEFTYPE_ONEOF = 1,
  UPB_DEFTYPE_FIELD_JSONNAME = 2
} upb_deftype_t;

static const void *unpack_def(upb_value v, upb_deftype_t type) {
  uintptr_t num = (uintptr_t)upb_value_getconstptr(v);
  return (num & 3) == type ? (const void*)(num & ~3) : NULL;
}

static upb_value pack_def(const void *ptr, upb_deftype_t type) {
  uintptr_t num = (uintptr_t)ptr | type;
  return upb_value_constptr((const void*)num);
}

/* isalpha() etc. from <ctype.h> are locale-dependent, which we don't want. */
static bool upb_isbetween(char c, char low, char high) {
  return c >= low && c <= high;
}

static bool upb_isletter(char c) {
  return upb_isbetween(c, 'A', 'Z') || upb_isbetween(c, 'a', 'z') || c == '_';
}

static bool upb_isalphanum(char c) {
  return upb_isletter(c) || upb_isbetween(c, '0', '9');
}

static bool upb_isident(upb_strview name, bool full, upb_status *s) {
  const char *str = name.data;
  size_t len = name.size;
  bool start = true;
  size_t i;
  for (i = 0; i < len; i++) {
    char c = str[i];
    if (c == '.') {
      if (start || !full) {
        upb_status_seterrf(s, "invalid name: unexpected '.' (%s)", str);
        return false;
      }
      start = true;
    } else if (start) {
      if (!upb_isletter(c)) {
        upb_status_seterrf(
            s, "invalid name: path components must start with a letter (%s)",
            str);
        return false;
      }
      start = false;
    } else {
      if (!upb_isalphanum(c)) {
        upb_status_seterrf(s, "invalid name: non-alphanumeric character (%s)",
                           str);
        return false;
      }
    }
  }
  return !start;
}

static const char *shortdefname(const char *fullname) {
  const char *p;

  if (fullname == NULL) {
    return NULL;
  } else if ((p = strrchr(fullname, '.')) == NULL) {
    /* No '.' in the name, return the full string. */
    return fullname;
  } else {
    /* Return one past the last '.'. */
    return p + 1;
  }
}

/* All submessage fields are lower than all other fields.
 * Secondly, fields are increasing in order. */
uint32_t field_rank(const upb_fielddef *f) {
  uint32_t ret = upb_fielddef_number(f);
  const uint32_t high_bit = 1 << 30;
  UPB_ASSERT(ret < high_bit);
  if (!upb_fielddef_issubmsg(f))
    ret |= high_bit;
  return ret;
}

int cmp_fields(const void *p1, const void *p2) {
  const upb_fielddef *f1 = *(upb_fielddef*const*)p1;
  const upb_fielddef *f2 = *(upb_fielddef*const*)p2;
  return field_rank(f1) - field_rank(f2);
}

/* A few implementation details of handlers.  We put these here to avoid
 * a def -> handlers dependency. */

#define UPB_STATIC_SELECTOR_COUNT 3  /* Warning: also in upb/handlers.h. */

static uint32_t upb_handlers_selectorbaseoffset(const upb_fielddef *f) {
  return upb_fielddef_isseq(f) ? 2 : 0;
}

static uint32_t upb_handlers_selectorcount(const upb_fielddef *f) {
  uint32_t ret = 1;
  if (upb_fielddef_isseq(f)) ret += 2;    /* STARTSEQ/ENDSEQ */
  if (upb_fielddef_isstring(f)) ret += 2; /* [STRING]/STARTSTR/ENDSTR */
  if (upb_fielddef_issubmsg(f)) {
    /* ENDSUBMSG (STARTSUBMSG is at table beginning) */
    ret += 0;
    if (upb_fielddef_lazy(f)) {
      /* STARTSTR/ENDSTR/STRING (for lazy) */
      ret += 3;
    }
  }
  return ret;
}

static bool assign_msg_indices(upb_msgdef *m, upb_status *s) {
  /* Sort fields.  upb internally relies on UPB_TYPE_MESSAGE fields having the
   * lowest indexes, but we do not publicly guarantee this. */
  upb_msg_field_iter j;
  upb_msg_oneof_iter k;
  int i;
  uint32_t selector;
  int n = upb_msgdef_numfields(m);
  upb_fielddef **fields;

  if (n == 0) {
    m->selector_count = UPB_STATIC_SELECTOR_COUNT;
    m->submsg_field_count = 0;
    return true;
  }

  fields = upb_gmalloc(n * sizeof(*fields));
  if (!fields) {
    upb_status_setoom(s);
    return false;
  }

  m->submsg_field_count = 0;
  for(i = 0, upb_msg_field_begin(&j, m);
      !upb_msg_field_done(&j);
      upb_msg_field_next(&j), i++) {
    upb_fielddef *f = upb_msg_iter_field(&j);
    UPB_ASSERT(f->msgdef == m);
    if (upb_fielddef_issubmsg(f)) {
      m->submsg_field_count++;
    }
    fields[i] = f;
  }

  qsort(fields, n, sizeof(*fields), cmp_fields);

  selector = UPB_STATIC_SELECTOR_COUNT + m->submsg_field_count;
  for (i = 0; i < n; i++) {
    upb_fielddef *f = fields[i];
    f->index_ = i;
    f->selector_base = selector + upb_handlers_selectorbaseoffset(f);
    selector += upb_handlers_selectorcount(f);
  }
  m->selector_count = selector;

  for(upb_msg_oneof_begin(&k, m), i = 0;
      !upb_msg_oneof_done(&k);
      upb_msg_oneof_next(&k), i++) {
    upb_oneofdef *o = (upb_oneofdef*)upb_msg_iter_oneof(&k);
    o->index = i;
  }

  upb_gfree(fields);
  return true;
}

static void assign_msg_wellknowntype(upb_msgdef *m) {
  const char *name = upb_msgdef_fullname(m);
  if (name == NULL) {
    m->well_known_type = UPB_WELLKNOWN_UNSPECIFIED;
    return;
  }
  if (!strcmp(name, "google.protobuf.Any")) {
    m->well_known_type = UPB_WELLKNOWN_ANY;
  } else if (!strcmp(name, "google.protobuf.FieldMask")) {
    m->well_known_type = UPB_WELLKNOWN_FIELDMASK;
  } else if (!strcmp(name, "google.protobuf.Duration")) {
    m->well_known_type = UPB_WELLKNOWN_DURATION;
  } else if (!strcmp(name, "google.protobuf.Timestamp")) {
    m->well_known_type = UPB_WELLKNOWN_TIMESTAMP;
  } else if (!strcmp(name, "google.protobuf.DoubleValue")) {
    m->well_known_type = UPB_WELLKNOWN_DOUBLEVALUE;
  } else if (!strcmp(name, "google.protobuf.FloatValue")) {
    m->well_known_type = UPB_WELLKNOWN_FLOATVALUE;
  } else if (!strcmp(name, "google.protobuf.Int64Value")) {
    m->well_known_type = UPB_WELLKNOWN_INT64VALUE;
  } else if (!strcmp(name, "google.protobuf.UInt64Value")) {
    m->well_known_type = UPB_WELLKNOWN_UINT64VALUE;
  } else if (!strcmp(name, "google.protobuf.Int32Value")) {
    m->well_known_type = UPB_WELLKNOWN_INT32VALUE;
  } else if (!strcmp(name, "google.protobuf.UInt32Value")) {
    m->well_known_type = UPB_WELLKNOWN_UINT32VALUE;
  } else if (!strcmp(name, "google.protobuf.BoolValue")) {
    m->well_known_type = UPB_WELLKNOWN_BOOLVALUE;
  } else if (!strcmp(name, "google.protobuf.StringValue")) {
    m->well_known_type = UPB_WELLKNOWN_STRINGVALUE;
  } else if (!strcmp(name, "google.protobuf.BytesValue")) {
    m->well_known_type = UPB_WELLKNOWN_BYTESVALUE;
  } else if (!strcmp(name, "google.protobuf.Value")) {
    m->well_known_type = UPB_WELLKNOWN_VALUE;
  } else if (!strcmp(name, "google.protobuf.ListValue")) {
    m->well_known_type = UPB_WELLKNOWN_LISTVALUE;
  } else if (!strcmp(name, "google.protobuf.Struct")) {
    m->well_known_type = UPB_WELLKNOWN_STRUCT;
  } else {
    m->well_known_type = UPB_WELLKNOWN_UNSPECIFIED;
  }
}


/* upb_enumdef ****************************************************************/

const char *upb_enumdef_fullname(const upb_enumdef *e) {
  return e->full_name;
}

const char *upb_enumdef_name(const upb_enumdef *e) {
  return shortdefname(e->full_name);
}

const upb_filedef *upb_enumdef_file(const upb_enumdef *e) {
  return e->file;
}

int32_t upb_enumdef_default(const upb_enumdef *e) {
  UPB_ASSERT(upb_enumdef_iton(e, e->defaultval));
  return e->defaultval;
}

int upb_enumdef_numvals(const upb_enumdef *e) {
  return (int)upb_strtable_count(&e->ntoi);
}

void upb_enum_begin(upb_enum_iter *i, const upb_enumdef *e) {
  /* We iterate over the ntoi table, to account for duplicate numbers. */
  upb_strtable_begin(i, &e->ntoi);
}

void upb_enum_next(upb_enum_iter *iter) { upb_strtable_next(iter); }
bool upb_enum_done(upb_enum_iter *iter) { return upb_strtable_done(iter); }

bool upb_enumdef_ntoi(const upb_enumdef *def, const char *name,
                      size_t len, int32_t *num) {
  upb_value v;
  if (!upb_strtable_lookup2(&def->ntoi, name, len, &v)) {
    return false;
  }
  if (num) *num = upb_value_getint32(v);
  return true;
}

const char *upb_enumdef_iton(const upb_enumdef *def, int32_t num) {
  upb_value v;
  return upb_inttable_lookup32(&def->iton, num, &v) ?
      upb_value_getcstr(v) : NULL;
}

const char *upb_enum_iter_name(upb_enum_iter *iter) {
  return upb_strtable_iter_key(iter).data;
}

int32_t upb_enum_iter_number(upb_enum_iter *iter) {
  return upb_value_getint32(upb_strtable_iter_value(iter));
}


/* upb_fielddef ***************************************************************/

const char *upb_fielddef_fullname(const upb_fielddef *f) {
  return f->full_name;
}

upb_fieldtype_t upb_fielddef_type(const upb_fielddef *f) {
  switch (f->type_) {
    case UPB_DESCRIPTOR_TYPE_DOUBLE:
      return UPB_TYPE_DOUBLE;
    case UPB_DESCRIPTOR_TYPE_FLOAT:
      return UPB_TYPE_FLOAT;
    case UPB_DESCRIPTOR_TYPE_INT64:
    case UPB_DESCRIPTOR_TYPE_SINT64:
    case UPB_DESCRIPTOR_TYPE_SFIXED64:
      return UPB_TYPE_INT64;
    case UPB_DESCRIPTOR_TYPE_INT32:
    case UPB_DESCRIPTOR_TYPE_SFIXED32:
    case UPB_DESCRIPTOR_TYPE_SINT32:
      return UPB_TYPE_INT32;
    case UPB_DESCRIPTOR_TYPE_UINT64:
    case UPB_DESCRIPTOR_TYPE_FIXED64:
      return UPB_TYPE_UINT64;
    case UPB_DESCRIPTOR_TYPE_UINT32:
    case UPB_DESCRIPTOR_TYPE_FIXED32:
      return UPB_TYPE_UINT32;
    case UPB_DESCRIPTOR_TYPE_ENUM:
      return UPB_TYPE_ENUM;
    case UPB_DESCRIPTOR_TYPE_BOOL:
      return UPB_TYPE_BOOL;
    case UPB_DESCRIPTOR_TYPE_STRING:
      return UPB_TYPE_STRING;
    case UPB_DESCRIPTOR_TYPE_BYTES:
      return UPB_TYPE_BYTES;
    case UPB_DESCRIPTOR_TYPE_GROUP:
    case UPB_DESCRIPTOR_TYPE_MESSAGE:
      return UPB_TYPE_MESSAGE;
  }
  UPB_UNREACHABLE();
}

upb_descriptortype_t upb_fielddef_descriptortype(const upb_fielddef *f) {
  return f->type_;
}

uint32_t upb_fielddef_index(const upb_fielddef *f) {
  return f->index_;
}

upb_label_t upb_fielddef_label(const upb_fielddef *f) {
  return f->label_;
}

uint32_t upb_fielddef_number(const upb_fielddef *f) {
  return f->number_;
}

bool upb_fielddef_isextension(const upb_fielddef *f) {
  return f->is_extension_;
}

bool upb_fielddef_lazy(const upb_fielddef *f) {
  return f->lazy_;
}

bool upb_fielddef_packed(const upb_fielddef *f) {
  return f->packed_;
}

const char *upb_fielddef_name(const upb_fielddef *f) {
  return shortdefname(f->full_name);
}

const char *upb_fielddef_jsonname(const upb_fielddef *f) {
  return f->json_name;
}

uint32_t upb_fielddef_selectorbase(const upb_fielddef *f) {
  return f->selector_base;
}

const upb_msgdef *upb_fielddef_containingtype(const upb_fielddef *f) {
  return f->msgdef;
}

const upb_oneofdef *upb_fielddef_containingoneof(const upb_fielddef *f) {
  return f->oneof;
}

static void chkdefaulttype(const upb_fielddef *f, int ctype) {
  UPB_UNUSED(f);
  UPB_UNUSED(ctype);
}

int64_t upb_fielddef_defaultint64(const upb_fielddef *f) {
  chkdefaulttype(f, UPB_TYPE_INT64);
  return f->defaultval.sint;
}

int32_t upb_fielddef_defaultint32(const upb_fielddef *f) {
  chkdefaulttype(f, UPB_TYPE_INT32);
  return (int32_t)f->defaultval.sint;
}

uint64_t upb_fielddef_defaultuint64(const upb_fielddef *f) {
  chkdefaulttype(f, UPB_TYPE_UINT64);
  return f->defaultval.uint;
}

uint32_t upb_fielddef_defaultuint32(const upb_fielddef *f) {
  chkdefaulttype(f, UPB_TYPE_UINT32);
  return (uint32_t)f->defaultval.uint;
}

bool upb_fielddef_defaultbool(const upb_fielddef *f) {
  chkdefaulttype(f, UPB_TYPE_BOOL);
  return f->defaultval.boolean;
}

float upb_fielddef_defaultfloat(const upb_fielddef *f) {
  chkdefaulttype(f, UPB_TYPE_FLOAT);
  return f->defaultval.flt;
}

double upb_fielddef_defaultdouble(const upb_fielddef *f) {
  chkdefaulttype(f, UPB_TYPE_DOUBLE);
  return f->defaultval.dbl;
}

const char *upb_fielddef_defaultstr(const upb_fielddef *f, size_t *len) {
  str_t *str = f->defaultval.str;
  UPB_ASSERT(upb_fielddef_type(f) == UPB_TYPE_STRING ||
         upb_fielddef_type(f) == UPB_TYPE_BYTES ||
         upb_fielddef_type(f) == UPB_TYPE_ENUM);
  if (str) {
    if (len) *len = str->len;
    return str->str;
  } else {
    if (len) *len = 0;
    return NULL;
  }
}

const upb_msgdef *upb_fielddef_msgsubdef(const upb_fielddef *f) {
  UPB_ASSERT(upb_fielddef_type(f) == UPB_TYPE_MESSAGE);
  return f->sub.msgdef;
}

const upb_enumdef *upb_fielddef_enumsubdef(const upb_fielddef *f) {
  UPB_ASSERT(upb_fielddef_type(f) == UPB_TYPE_ENUM);
  return f->sub.enumdef;
}

const upb_msglayout_field *upb_fielddef_layout(const upb_fielddef *f) {
  return &f->msgdef->layout->fields[f->layout_index];
}

bool upb_fielddef_issubmsg(const upb_fielddef *f) {
  return upb_fielddef_type(f) == UPB_TYPE_MESSAGE;
}

bool upb_fielddef_isstring(const upb_fielddef *f) {
  return upb_fielddef_type(f) == UPB_TYPE_STRING ||
         upb_fielddef_type(f) == UPB_TYPE_BYTES;
}

bool upb_fielddef_isseq(const upb_fielddef *f) {
  return upb_fielddef_label(f) == UPB_LABEL_REPEATED;
}

bool upb_fielddef_isprimitive(const upb_fielddef *f) {
  return !upb_fielddef_isstring(f) && !upb_fielddef_issubmsg(f);
}

bool upb_fielddef_ismap(const upb_fielddef *f) {
  return upb_fielddef_isseq(f) && upb_fielddef_issubmsg(f) &&
         upb_msgdef_mapentry(upb_fielddef_msgsubdef(f));
}

bool upb_fielddef_hassubdef(const upb_fielddef *f) {
  return upb_fielddef_issubmsg(f) || upb_fielddef_type(f) == UPB_TYPE_ENUM;
}

bool upb_fielddef_haspresence(const upb_fielddef *f) {
  if (upb_fielddef_isseq(f)) return false;
  if (upb_fielddef_issubmsg(f)) return true;
  if (upb_fielddef_containingoneof(f)) return true;
  return f->file->syntax == UPB_SYNTAX_PROTO2;
}

static bool between(int32_t x, int32_t low, int32_t high) {
  return x >= low && x <= high;
}

bool upb_fielddef_checklabel(int32_t label) { return between(label, 1, 3); }
bool upb_fielddef_checktype(int32_t type) { return between(type, 1, 11); }
bool upb_fielddef_checkintfmt(int32_t fmt) { return between(fmt, 1, 3); }

bool upb_fielddef_checkdescriptortype(int32_t type) {
  return between(type, 1, 18);
}

/* upb_msgdef *****************************************************************/

const char *upb_msgdef_fullname(const upb_msgdef *m) {
  return m->full_name;
}

const upb_filedef *upb_msgdef_file(const upb_msgdef *m) {
  return m->file;
}

const char *upb_msgdef_name(const upb_msgdef *m) {
  return shortdefname(m->full_name);
}

upb_syntax_t upb_msgdef_syntax(const upb_msgdef *m) {
  return m->file->syntax;
}

size_t upb_msgdef_selectorcount(const upb_msgdef *m) {
  return m->selector_count;
}

uint32_t upb_msgdef_submsgfieldcount(const upb_msgdef *m) {
  return m->submsg_field_count;
}

const upb_fielddef *upb_msgdef_itof(const upb_msgdef *m, uint32_t i) {
  upb_value val;
  return upb_inttable_lookup32(&m->itof, i, &val) ?
      upb_value_getconstptr(val) : NULL;
}

const upb_fielddef *upb_msgdef_ntof(const upb_msgdef *m, const char *name,
                                    size_t len) {
  upb_value val;

  if (!upb_strtable_lookup2(&m->ntof, name, len, &val)) {
    return NULL;
  }

  return unpack_def(val, UPB_DEFTYPE_FIELD);
}

const upb_oneofdef *upb_msgdef_ntoo(const upb_msgdef *m, const char *name,
                                    size_t len) {
  upb_value val;

  if (!upb_strtable_lookup2(&m->ntof, name, len, &val)) {
    return NULL;
  }

  return unpack_def(val, UPB_DEFTYPE_ONEOF);
}

bool upb_msgdef_lookupname(const upb_msgdef *m, const char *name, size_t len,
                           const upb_fielddef **f, const upb_oneofdef **o) {
  upb_value val;

  if (!upb_strtable_lookup2(&m->ntof, name, len, &val)) {
    return false;
  }

  *o = unpack_def(val, UPB_DEFTYPE_ONEOF);
  *f = unpack_def(val, UPB_DEFTYPE_FIELD);
  return *o || *f;  /* False if this was a JSON name. */
}

const upb_fielddef *upb_msgdef_lookupjsonname(const upb_msgdef *m,
                                              const char *name, size_t len) {
  upb_value val;
  const upb_fielddef* f;

  if (!upb_strtable_lookup2(&m->ntof, name, len, &val)) {
    return NULL;
  }

  f = unpack_def(val, UPB_DEFTYPE_FIELD);
  if (!f) f = unpack_def(val, UPB_DEFTYPE_FIELD_JSONNAME);

  return f;
}

int upb_msgdef_numfields(const upb_msgdef *m) {
  return m->field_count;
}

int upb_msgdef_numoneofs(const upb_msgdef *m) {
  return m->oneof_count;
}

const upb_msglayout *upb_msgdef_layout(const upb_msgdef *m) {
  return m->layout;
}

const upb_fielddef *_upb_msgdef_field(const upb_msgdef *m, int i) {
  if (i >= m->field_count) return NULL;
  return &m->fields[i];
}

bool upb_msgdef_mapentry(const upb_msgdef *m) {
  return m->map_entry;
}

upb_wellknowntype_t upb_msgdef_wellknowntype(const upb_msgdef *m) {
  return m->well_known_type;
}

bool upb_msgdef_isnumberwrapper(const upb_msgdef *m) {
  upb_wellknowntype_t type = upb_msgdef_wellknowntype(m);
  return type >= UPB_WELLKNOWN_DOUBLEVALUE &&
         type <= UPB_WELLKNOWN_UINT32VALUE;
}

void upb_msg_field_begin(upb_msg_field_iter *iter, const upb_msgdef *m) {
  upb_inttable_begin(iter, &m->itof);
}

void upb_msg_field_next(upb_msg_field_iter *iter) { upb_inttable_next(iter); }

bool upb_msg_field_done(const upb_msg_field_iter *iter) {
  return upb_inttable_done(iter);
}

upb_fielddef *upb_msg_iter_field(const upb_msg_field_iter *iter) {
  return (upb_fielddef *)upb_value_getconstptr(upb_inttable_iter_value(iter));
}

void upb_msg_field_iter_setdone(upb_msg_field_iter *iter) {
  upb_inttable_iter_setdone(iter);
}

bool upb_msg_field_iter_isequal(const upb_msg_field_iter * iter1,
                                const upb_msg_field_iter * iter2) {
  return upb_inttable_iter_isequal(iter1, iter2);
}

void upb_msg_oneof_begin(upb_msg_oneof_iter *iter, const upb_msgdef *m) {
  upb_strtable_begin(iter, &m->ntof);
  /* We need to skip past any initial fields. */
  while (!upb_strtable_done(iter) &&
         !unpack_def(upb_strtable_iter_value(iter), UPB_DEFTYPE_ONEOF)) {
    upb_strtable_next(iter);
  }
}

void upb_msg_oneof_next(upb_msg_oneof_iter *iter) {
  /* We need to skip past fields to return only oneofs. */
  do {
    upb_strtable_next(iter);
  } while (!upb_strtable_done(iter) &&
           !unpack_def(upb_strtable_iter_value(iter), UPB_DEFTYPE_ONEOF));
}

bool upb_msg_oneof_done(const upb_msg_oneof_iter *iter) {
  return upb_strtable_done(iter);
}

const upb_oneofdef *upb_msg_iter_oneof(const upb_msg_oneof_iter *iter) {
  return unpack_def(upb_strtable_iter_value(iter), UPB_DEFTYPE_ONEOF);
}

void upb_msg_oneof_iter_setdone(upb_msg_oneof_iter *iter) {
  upb_strtable_iter_setdone(iter);
}

bool upb_msg_oneof_iter_isequal(const upb_msg_oneof_iter *iter1,
                                const upb_msg_oneof_iter *iter2) {
  return upb_strtable_iter_isequal(iter1, iter2);
}

/* upb_oneofdef ***************************************************************/

const char *upb_oneofdef_name(const upb_oneofdef *o) {
  return shortdefname(o->full_name);
}

const upb_msgdef *upb_oneofdef_containingtype(const upb_oneofdef *o) {
  return o->parent;
}

int upb_oneofdef_numfields(const upb_oneofdef *o) {
  return (int)upb_strtable_count(&o->ntof);
}

uint32_t upb_oneofdef_index(const upb_oneofdef *o) {
  return o->index;
}

const upb_fielddef *upb_oneofdef_ntof(const upb_oneofdef *o,
                                      const char *name, size_t length) {
  upb_value val;
  return upb_strtable_lookup2(&o->ntof, name, length, &val) ?
      upb_value_getptr(val) : NULL;
}

const upb_fielddef *upb_oneofdef_itof(const upb_oneofdef *o, uint32_t num) {
  upb_value val;
  return upb_inttable_lookup32(&o->itof, num, &val) ?
      upb_value_getptr(val) : NULL;
}

void upb_oneof_begin(upb_oneof_iter *iter, const upb_oneofdef *o) {
  upb_inttable_begin(iter, &o->itof);
}

void upb_oneof_next(upb_oneof_iter *iter) {
  upb_inttable_next(iter);
}

bool upb_oneof_done(upb_oneof_iter *iter) {
  return upb_inttable_done(iter);
}

upb_fielddef *upb_oneof_iter_field(const upb_oneof_iter *iter) {
  return (upb_fielddef *)upb_value_getconstptr(upb_inttable_iter_value(iter));
}

void upb_oneof_iter_setdone(upb_oneof_iter *iter) {
  upb_inttable_iter_setdone(iter);
}

/* Dynamic Layout Generation. *************************************************/

static bool is_power_of_two(size_t val) {
  return (val & (val - 1)) == 0;
}

/* Align up to the given power of 2. */
static size_t align_up(size_t val, size_t align) {
  UPB_ASSERT(is_power_of_two(align));
  return (val + align - 1) & ~(align - 1);
}

static size_t div_round_up(size_t n, size_t d) {
  return (n + d - 1) / d;
}

static size_t upb_msgval_sizeof(upb_fieldtype_t type) {
  switch (type) {
    case UPB_TYPE_DOUBLE:
    case UPB_TYPE_INT64:
    case UPB_TYPE_UINT64:
      return 8;
    case UPB_TYPE_ENUM:
    case UPB_TYPE_INT32:
    case UPB_TYPE_UINT32:
    case UPB_TYPE_FLOAT:
      return 4;
    case UPB_TYPE_BOOL:
      return 1;
    case UPB_TYPE_MESSAGE:
      return sizeof(void*);
    case UPB_TYPE_BYTES:
    case UPB_TYPE_STRING:
      return sizeof(upb_strview);
  }
  UPB_UNREACHABLE();
}

static uint8_t upb_msg_fielddefsize(const upb_fielddef *f) {
  if (upb_msgdef_mapentry(upb_fielddef_containingtype(f))) {
    upb_map_entry ent;
    UPB_ASSERT(sizeof(ent.k) == sizeof(ent.v));
    return sizeof(ent.k);
  } else if (upb_fielddef_isseq(f)) {
    return sizeof(void*);
  } else {
    return upb_msgval_sizeof(upb_fielddef_type(f));
  }
}

static uint32_t upb_msglayout_place(upb_msglayout *l, size_t size) {
  uint32_t ret;

  l->size = align_up(l->size, size);
  ret = l->size;
  l->size += size;
  return ret;
}

/* This function is the dynamic equivalent of message_layout.{cc,h} in upbc.
 * It computes a dynamic layout for all of the fields in |m|. */
static bool make_layout(const upb_symtab *symtab, const upb_msgdef *m) {
  upb_msglayout *l = (upb_msglayout*)m->layout;
  upb_msg_field_iter it;
  upb_msg_oneof_iter oit;
  size_t hasbit;
  size_t submsg_count = m->submsg_field_count;
  const upb_msglayout **submsgs;
  upb_msglayout_field *fields;
  upb_alloc *alloc = upb_arena_alloc(symtab->arena);

  memset(l, 0, sizeof(*l));

  fields = upb_malloc(alloc, upb_msgdef_numfields(m) * sizeof(*fields));
  submsgs = upb_malloc(alloc, submsg_count * sizeof(*submsgs));

  if ((!fields && upb_msgdef_numfields(m)) ||
      (!submsgs && submsg_count)) {
    /* OOM. */
    return false;
  }

  l->field_count = upb_msgdef_numfields(m);
  l->fields = fields;
  l->submsgs = submsgs;

  if (upb_msgdef_mapentry(m)) {
    /* TODO(haberman): refactor this method so this special case is more
     * elegant. */
    const upb_fielddef *key = upb_msgdef_itof(m, 1);
    const upb_fielddef *val = upb_msgdef_itof(m, 2);
    fields[0].number = 1;
    fields[1].number = 2;
    fields[0].label = UPB_LABEL_OPTIONAL;
    fields[1].label = UPB_LABEL_OPTIONAL;
    fields[0].presence = 0;
    fields[1].presence = 0;
    fields[0].descriptortype = upb_fielddef_descriptortype(key);
    fields[1].descriptortype = upb_fielddef_descriptortype(val);
    fields[0].offset = 0;
    fields[1].offset = sizeof(upb_strview);
    fields[1].submsg_index = 0;

    if (upb_fielddef_type(val) == UPB_TYPE_MESSAGE) {
      submsgs[0] = upb_fielddef_msgsubdef(val)->layout;
    }

    l->field_count = 2;
    l->size = 2 * sizeof(upb_strview);align_up(l->size, 8);
    return true;
  }

  /* Allocate data offsets in three stages:
   *
   * 1. hasbits.
   * 2. regular fields.
   * 3. oneof fields.
   *
   * OPT: There is a lot of room for optimization here to minimize the size.
   */

  /* Allocate hasbits and set basic field attributes. */
  submsg_count = 0;
  for (upb_msg_field_begin(&it, m), hasbit = 0;
       !upb_msg_field_done(&it);
       upb_msg_field_next(&it)) {
    upb_fielddef* f = upb_msg_iter_field(&it);
    upb_msglayout_field *field = &fields[upb_fielddef_index(f)];

    field->number = upb_fielddef_number(f);
    field->descriptortype = upb_fielddef_descriptortype(f);
    field->label = upb_fielddef_label(f);

    if (upb_fielddef_ismap(f)) {
      field->label = UPB_LABEL_MAP;
    }

    /* TODO: we probably should sort the fields by field number to match the
     * output of upbc, and to improve search speed for the table parser. */
    f->layout_index = f->index_;

    if (upb_fielddef_issubmsg(f)) {
      const upb_msgdef *subm = upb_fielddef_msgsubdef(f);
      field->submsg_index = submsg_count++;
      submsgs[field->submsg_index] = subm->layout;
    }

    if (upb_fielddef_haspresence(f) && !upb_fielddef_containingoneof(f)) {
      /* We don't use hasbit 0, so that 0 can indicate "no presence" in the
       * table. This wastes one hasbit, but we don't worry about it for now. */
      field->presence = ++hasbit;
    } else {
      field->presence = 0;
    }
  }

  /* Account for space used by hasbits. */
  l->size = div_round_up(hasbit, 8);

  /* Allocate non-oneof fields. */
  for (upb_msg_field_begin(&it, m); !upb_msg_field_done(&it);
       upb_msg_field_next(&it)) {
    const upb_fielddef* f = upb_msg_iter_field(&it);
    size_t field_size = upb_msg_fielddefsize(f);
    size_t index = upb_fielddef_index(f);

    if (upb_fielddef_containingoneof(f)) {
      /* Oneofs are handled separately below. */
      continue;
    }

    fields[index].offset = upb_msglayout_place(l, field_size);
  }

  /* Allocate oneof fields.  Each oneof field consists of a uint32 for the case
   * and space for the actual data. */
  for (upb_msg_oneof_begin(&oit, m); !upb_msg_oneof_done(&oit);
       upb_msg_oneof_next(&oit)) {
    const upb_oneofdef* o = upb_msg_iter_oneof(&oit);
    upb_oneof_iter fit;

    size_t case_size = sizeof(uint32_t);  /* Could potentially optimize this. */
    size_t field_size = 0;
    uint32_t case_offset;
    uint32_t data_offset;

    /* Calculate field size: the max of all field sizes. */
    for (upb_oneof_begin(&fit, o);
         !upb_oneof_done(&fit);
         upb_oneof_next(&fit)) {
      const upb_fielddef* f = upb_oneof_iter_field(&fit);
      field_size = UPB_MAX(field_size, upb_msg_fielddefsize(f));
    }

    /* Align and allocate case offset. */
    case_offset = upb_msglayout_place(l, case_size);
    data_offset = upb_msglayout_place(l, field_size);

    for (upb_oneof_begin(&fit, o);
         !upb_oneof_done(&fit);
         upb_oneof_next(&fit)) {
      const upb_fielddef* f = upb_oneof_iter_field(&fit);
      fields[upb_fielddef_index(f)].offset = data_offset;
      fields[upb_fielddef_index(f)].presence = ~case_offset;
    }
  }

  /* Size of the entire structure should be a multiple of its greatest
   * alignment.  TODO: track overall alignment for real? */
  l->size = align_up(l->size, 8);

  return true;
}

/* Code to build defs from descriptor protos. *********************************/

/* There is a question of how much validation to do here.  It will be difficult
 * to perfectly match the amount of validation performed by proto2.  But since
 * this code is used to directly build defs from Ruby (for example) we do need
 * to validate important constraints like uniqueness of names and numbers. */

#define CHK(x) if (!(x)) { return false; }
#define CHK_OOM(x) if (!(x)) { upb_status_setoom(ctx->status); return false; }

typedef struct {
  const upb_symtab *symtab;
  upb_filedef *file;              /* File we are building. */
  upb_alloc *alloc;               /* Allocate defs here. */
  upb_alloc *tmp;                 /* Alloc for addtab and any other tmp data. */
  upb_strtable *addtab;           /* full_name -> packed def ptr for new defs */
  const upb_msglayout **layouts;  /* NULL if we should build layouts. */
  upb_status *status;             /* Record errors here. */
} symtab_addctx;

static char* strviewdup(const symtab_addctx *ctx, upb_strview view) {
  return upb_strdup2(view.data, view.size, ctx->alloc);
}

static bool streql2(const char *a, size_t n, const char *b) {
  return n == strlen(b) && memcmp(a, b, n) == 0;
}

static bool streql_view(upb_strview view, const char *b) {
  return streql2(view.data, view.size, b);
}

static const char *makefullname(const symtab_addctx *ctx, const char *prefix,
                                upb_strview name) {
  if (prefix) {
    /* ret = prefix + '.' + name; */
    size_t n = strlen(prefix);
    char *ret = upb_malloc(ctx->alloc, n + name.size + 2);
    CHK_OOM(ret);
    strcpy(ret, prefix);
    ret[n] = '.';
    memcpy(&ret[n + 1], name.data, name.size);
    ret[n + 1 + name.size] = '\0';
    return ret;
  } else {
    return strviewdup(ctx, name);
  }
}

size_t getjsonname(const char *name, char *buf, size_t len) {
  size_t src, dst = 0;
  bool ucase_next = false;

#define WRITE(byte) \
  ++dst; \
  if (dst < len) buf[dst - 1] = byte; \
  else if (dst == len) buf[dst - 1] = '\0'

  if (!name) {
    WRITE('\0');
    return 0;
  }

  /* Implement the transformation as described in the spec:
   *   1. upper case all letters after an underscore.
   *   2. remove all underscores.
   */
  for (src = 0; name[src]; src++) {
    if (name[src] == '_') {
      ucase_next = true;
      continue;
    }

    if (ucase_next) {
      WRITE(toupper(name[src]));
      ucase_next = false;
    } else {
      WRITE(name[src]);
    }
  }

  WRITE('\0');
  return dst;

#undef WRITE
}

static char* makejsonname(const char* name, upb_alloc *alloc) {
  size_t size = getjsonname(name, NULL, 0);
  char* json_name = upb_malloc(alloc, size);
  getjsonname(name, json_name, size);
  return json_name;
}

static bool symtab_add(const symtab_addctx *ctx, const char *name,
                       upb_value v) {
  upb_value tmp;
  if (upb_strtable_lookup(ctx->addtab, name, &tmp) ||
      upb_strtable_lookup(&ctx->symtab->syms, name, &tmp)) {
    upb_status_seterrf(ctx->status, "duplicate symbol '%s'", name);
    return false;
  }

  CHK_OOM(upb_strtable_insert3(ctx->addtab, name, strlen(name), v, ctx->tmp));
  return true;
}

/* Given a symbol and the base symbol inside which it is defined, find the
 * symbol's definition in t. */
static bool resolvename(const upb_strtable *t, const upb_fielddef *f,
                        const char *base, upb_strview sym,
                        upb_deftype_t type, upb_status *status,
                        const void **def) {
  if(sym.size == 0) return NULL;
  if(sym.data[0] == '.') {
    /* Symbols starting with '.' are absolute, so we do a single lookup.
     * Slice to omit the leading '.' */
    upb_value v;
    if (!upb_strtable_lookup2(t, sym.data + 1, sym.size - 1, &v)) {
      return false;
    }

    *def = unpack_def(v, type);

    if (!*def) {
      upb_status_seterrf(status,
                         "type mismatch when resolving field %s, name %s",
                         f->full_name, sym.data);
      return false;
    }

    return true;
  } else {
    /* Remove components from base until we find an entry or run out.
     * TODO: This branch is totally broken, but currently not used. */
    (void)base;
    UPB_ASSERT(false);
    return false;
  }
}

const void *symtab_resolve(const symtab_addctx *ctx, const upb_fielddef *f,
                           const char *base, upb_strview sym,
                           upb_deftype_t type) {
  const void *ret;
  if (!resolvename(ctx->addtab, f, base, sym, type, ctx->status, &ret) &&
      !resolvename(&ctx->symtab->syms, f, base, sym, type, ctx->status, &ret)) {
    if (upb_ok(ctx->status)) {
      upb_status_seterrf(ctx->status, "couldn't resolve name '%s'", sym.data);
    }
    return false;
  }
  return ret;
}

static bool create_oneofdef(
    const symtab_addctx *ctx, upb_msgdef *m,
    const google_protobuf_OneofDescriptorProto *oneof_proto) {
  upb_oneofdef *o;
  upb_strview name = google_protobuf_OneofDescriptorProto_name(oneof_proto);
  upb_value v;

  o = (upb_oneofdef*)&m->oneofs[m->oneof_count++];
  o->parent = m;
  o->full_name = makefullname(ctx, m->full_name, name);

  v = pack_def(o, UPB_DEFTYPE_ONEOF);
  CHK_OOM(symtab_add(ctx, o->full_name, v));
  CHK_OOM(upb_strtable_insert3(&m->ntof, name.data, name.size, v, ctx->alloc));

  CHK_OOM(upb_inttable_init2(&o->itof, UPB_CTYPE_CONSTPTR, ctx->alloc));
  CHK_OOM(upb_strtable_init2(&o->ntof, UPB_CTYPE_CONSTPTR, ctx->alloc));

  return true;
}

static bool parse_default(const symtab_addctx *ctx, const char *str, size_t len,
                          upb_fielddef *f) {
  char *end;
  char nullz[64];
  errno = 0;

  switch (upb_fielddef_type(f)) {
    case UPB_TYPE_INT32:
    case UPB_TYPE_INT64:
    case UPB_TYPE_UINT32:
    case UPB_TYPE_UINT64:
    case UPB_TYPE_DOUBLE:
    case UPB_TYPE_FLOAT:
      /* Standard C number parsing functions expect null-terminated strings. */
      if (len >= sizeof(nullz) - 1) {
        return false;
      }
      memcpy(nullz, str, len);
      nullz[len] = '\0';
      str = nullz;
      break;
    default:
      break;
  }

  switch (upb_fielddef_type(f)) {
    case UPB_TYPE_INT32: {
      long val = strtol(str, &end, 0);
      CHK(val <= INT32_MAX && val >= INT32_MIN && errno != ERANGE && !*end);
      f->defaultval.sint = val;
      break;
    }
    case UPB_TYPE_ENUM: {
      const upb_enumdef *e = f->sub.enumdef;
      int32_t val;
      CHK(upb_enumdef_ntoi(e, str, len, &val));
      f->defaultval.sint = val;
      break;
    }
    case UPB_TYPE_INT64: {
      /* XXX: Need to write our own strtoll, since it's not available in c89. */
      int64_t val = strtol(str, &end, 0);
      CHK(val <= INT64_MAX && val >= INT64_MIN && errno != ERANGE && !*end);
      f->defaultval.sint = val;
      break;
    }
    case UPB_TYPE_UINT32: {
      unsigned long val = strtoul(str, &end, 0);
      CHK(val <= UINT32_MAX && errno != ERANGE && !*end);
      f->defaultval.uint = val;
      break;
    }
    case UPB_TYPE_UINT64: {
      /* XXX: Need to write our own strtoull, since it's not available in c89. */
      uint64_t val = strtoul(str, &end, 0);
      CHK(val <= UINT64_MAX && errno != ERANGE && !*end);
      f->defaultval.uint = val;
      break;
    }
    case UPB_TYPE_DOUBLE: {
      double val = strtod(str, &end);
      CHK(errno != ERANGE && !*end);
      f->defaultval.dbl = val;
      break;
    }
    case UPB_TYPE_FLOAT: {
      /* XXX: Need to write our own strtof, since it's not available in c89. */
      float val = strtod(str, &end);
      CHK(errno != ERANGE && !*end);
      f->defaultval.flt = val;
      break;
    }
    case UPB_TYPE_BOOL: {
      if (streql2(str, len, "false")) {
        f->defaultval.boolean = false;
      } else if (streql2(str, len, "true")) {
        f->defaultval.boolean = true;
      } else {
        return false;
      }
      break;
    }
    case UPB_TYPE_STRING:
      f->defaultval.str = newstr(ctx->alloc, str, len);
      break;
    case UPB_TYPE_BYTES:
      /* XXX: need to interpret the C-escaped value. */
      f->defaultval.str = newstr(ctx->alloc, str, len);
      break;
    case UPB_TYPE_MESSAGE:
      /* Should not have a default value. */
      return false;
  }
  return true;
}

static void set_default_default(const symtab_addctx *ctx, upb_fielddef *f) {
  switch (upb_fielddef_type(f)) {
    case UPB_TYPE_INT32:
    case UPB_TYPE_INT64:
    case UPB_TYPE_ENUM:
      f->defaultval.sint = 0;
      break;
    case UPB_TYPE_UINT64:
    case UPB_TYPE_UINT32:
      f->defaultval.uint = 0;
      break;
    case UPB_TYPE_DOUBLE:
    case UPB_TYPE_FLOAT:
      f->defaultval.dbl = 0;
      break;
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES:
      f->defaultval.str = newstr(ctx->alloc, NULL, 0);
      break;
    case UPB_TYPE_BOOL:
      f->defaultval.boolean = false;
      break;
    case UPB_TYPE_MESSAGE:
      break;
  }
}

static bool create_fielddef(
    const symtab_addctx *ctx, const char *prefix, upb_msgdef *m,
    const google_protobuf_FieldDescriptorProto *field_proto) {
  upb_alloc *alloc = ctx->alloc;
  upb_fielddef *f;
  const google_protobuf_FieldOptions *options;
  upb_strview name;
  const char *full_name;
  const char *json_name;
  const char *shortname;
  uint32_t field_number;

  if (!google_protobuf_FieldDescriptorProto_has_name(field_proto)) {
    upb_status_seterrmsg(ctx->status, "field has no name");
    return false;
  }

  name = google_protobuf_FieldDescriptorProto_name(field_proto);
  CHK(upb_isident(name, false, ctx->status));
  full_name = makefullname(ctx, prefix, name);
  shortname = shortdefname(full_name);

  if (google_protobuf_FieldDescriptorProto_has_json_name(field_proto)) {
    json_name = strviewdup(
        ctx, google_protobuf_FieldDescriptorProto_json_name(field_proto));
  } else {
    json_name = makejsonname(shortname, ctx->alloc);
  }

  field_number = google_protobuf_FieldDescriptorProto_number(field_proto);

  if (field_number == 0 || field_number > UPB_MAX_FIELDNUMBER) {
    upb_status_seterrf(ctx->status, "invalid field number (%u)", field_number);
    return false;
  }

  if (m) {
    /* direct message field. */
    upb_value v, field_v, json_v;
    size_t json_size;

    f = (upb_fielddef*)&m->fields[m->field_count++];
    f->msgdef = m;
    f->is_extension_ = false;

    if (upb_strtable_lookup(&m->ntof, shortname, NULL)) {
      upb_status_seterrf(ctx->status, "duplicate field name (%s)", shortname);
      return false;
    }

    if (upb_strtable_lookup(&m->ntof, json_name, NULL)) {
      upb_status_seterrf(ctx->status, "duplicate json_name (%s)", json_name);
      return false;
    }

    if (upb_inttable_lookup(&m->itof, field_number, NULL)) {
      upb_status_seterrf(ctx->status, "duplicate field number (%u)",
                         field_number);
      return false;
    }

    field_v = pack_def(f, UPB_DEFTYPE_FIELD);
    json_v = pack_def(f, UPB_DEFTYPE_FIELD_JSONNAME);
    v = upb_value_constptr(f);
    json_size = strlen(json_name);

    CHK_OOM(
        upb_strtable_insert3(&m->ntof, name.data, name.size, field_v, alloc));
    CHK_OOM(upb_inttable_insert2(&m->itof, field_number, v, alloc));

    if (strcmp(shortname, json_name) != 0) {
      upb_strtable_insert3(&m->ntof, json_name, json_size, json_v, alloc);
    }

    if (ctx->layouts) {
      const upb_msglayout_field *fields = m->layout->fields;
      int count = m->layout->field_count;
      bool found = false;
      int i;
      for (i = 0; i < count; i++) {
        if (fields[i].number == field_number) {
          f->layout_index = i;
          found = true;
          break;
        }
      }
      UPB_ASSERT(found);
    }
  } else {
    /* extension field. */
    f = (upb_fielddef*)&ctx->file->exts[ctx->file->ext_count++];
    f->is_extension_ = true;
    CHK_OOM(symtab_add(ctx, full_name, pack_def(f, UPB_DEFTYPE_FIELD)));
  }

  f->full_name = full_name;
  f->json_name = json_name;
  f->file = ctx->file;
  f->type_ = (int)google_protobuf_FieldDescriptorProto_type(field_proto);
  f->label_ = (int)google_protobuf_FieldDescriptorProto_label(field_proto);
  f->number_ = field_number;
  f->oneof = NULL;

  /* We can't resolve the subdef or (in the case of extensions) the containing
   * message yet, because it may not have been defined yet.  We stash a pointer
   * to the field_proto until later when we can properly resolve it. */
  f->sub.unresolved = field_proto;

  if (f->label_ == UPB_LABEL_REQUIRED && f->file->syntax == UPB_SYNTAX_PROTO3) {
    upb_status_seterrf(ctx->status, "proto3 fields cannot be required (%s)",
                       f->full_name);
    return false;
  }

  if (google_protobuf_FieldDescriptorProto_has_oneof_index(field_proto)) {
    int oneof_index =
        google_protobuf_FieldDescriptorProto_oneof_index(field_proto);
    upb_oneofdef *oneof;
    upb_value v = upb_value_constptr(f);

    if (upb_fielddef_label(f) != UPB_LABEL_OPTIONAL) {
      upb_status_seterrf(ctx->status,
                         "fields in oneof must have OPTIONAL label (%s)",
                         f->full_name);
      return false;
    }

    if (!m) {
      upb_status_seterrf(ctx->status,
                         "oneof_index provided for extension field (%s)",
                         f->full_name);
      return false;
    }

    if (oneof_index >= m->oneof_count) {
      upb_status_seterrf(ctx->status, "oneof_index out of range (%s)",
                         f->full_name);
      return false;
    }

    oneof = (upb_oneofdef*)&m->oneofs[oneof_index];
    f->oneof = oneof;

    CHK(upb_inttable_insert2(&oneof->itof, f->number_, v, alloc));
    CHK(upb_strtable_insert3(&oneof->ntof, name.data, name.size, v, alloc));
  } else {
    f->oneof = NULL;
  }

  if (google_protobuf_FieldDescriptorProto_has_options(field_proto)) {
    options = google_protobuf_FieldDescriptorProto_options(field_proto);
    f->lazy_ = google_protobuf_FieldOptions_lazy(options);
    f->packed_ = google_protobuf_FieldOptions_packed(options);
  } else {
    f->lazy_ = false;
    f->packed_ = false;
  }

  return true;
}

static bool create_enumdef(
    const symtab_addctx *ctx, const char *prefix,
    const google_protobuf_EnumDescriptorProto *enum_proto) {
  upb_enumdef *e;
  const google_protobuf_EnumValueDescriptorProto *const *values;
  upb_strview name;
  size_t i, n;

  name = google_protobuf_EnumDescriptorProto_name(enum_proto);
  CHK(upb_isident(name, false, ctx->status));

  e = (upb_enumdef*)&ctx->file->enums[ctx->file->enum_count++];
  e->full_name = makefullname(ctx, prefix, name);
  CHK_OOM(symtab_add(ctx, e->full_name, pack_def(e, UPB_DEFTYPE_ENUM)));

  CHK_OOM(upb_strtable_init2(&e->ntoi, UPB_CTYPE_INT32, ctx->alloc));
  CHK_OOM(upb_inttable_init2(&e->iton, UPB_CTYPE_CSTR, ctx->alloc));

  e->file = ctx->file;
  e->defaultval = 0;

  values = google_protobuf_EnumDescriptorProto_value(enum_proto, &n);

  if (n == 0) {
    upb_status_seterrf(ctx->status,
                       "enums must contain at least one value (%s)",
                       e->full_name);
    return false;
  }

  for (i = 0; i < n; i++) {
    const google_protobuf_EnumValueDescriptorProto *value = values[i];
    upb_strview name = google_protobuf_EnumValueDescriptorProto_name(value);
    char *name2 = strviewdup(ctx, name);
    int32_t num = google_protobuf_EnumValueDescriptorProto_number(value);
    upb_value v = upb_value_int32(num);

    if (i == 0 && e->file->syntax == UPB_SYNTAX_PROTO3 && num != 0) {
      upb_status_seterrf(ctx->status,
                         "for proto3, the first enum value must be zero (%s)",
                         e->full_name);
      return false;
    }

    if (upb_strtable_lookup(&e->ntoi, name2, NULL)) {
      upb_status_seterrf(ctx->status, "duplicate enum label '%s'", name2);
      return false;
    }

    CHK_OOM(name2)
    CHK_OOM(
        upb_strtable_insert3(&e->ntoi, name2, strlen(name2), v, ctx->alloc));

    if (!upb_inttable_lookup(&e->iton, num, NULL)) {
      upb_value v = upb_value_cstr(name2);
      CHK_OOM(upb_inttable_insert2(&e->iton, num, v, ctx->alloc));
    }
  }

  upb_inttable_compact2(&e->iton, ctx->alloc);

  return true;
}

static bool create_msgdef(symtab_addctx *ctx, const char *prefix,
                          const google_protobuf_DescriptorProto *msg_proto) {
  upb_msgdef *m;
  const google_protobuf_MessageOptions *options;
  const google_protobuf_OneofDescriptorProto *const *oneofs;
  const google_protobuf_FieldDescriptorProto *const *fields;
  const google_protobuf_EnumDescriptorProto *const *enums;
  const google_protobuf_DescriptorProto *const *msgs;
  size_t i, n;
  upb_strview name;

  name = google_protobuf_DescriptorProto_name(msg_proto);
  CHK(upb_isident(name, false, ctx->status));

  m = (upb_msgdef*)&ctx->file->msgs[ctx->file->msg_count++];
  m->full_name = makefullname(ctx, prefix, name);
  CHK_OOM(symtab_add(ctx, m->full_name, pack_def(m, UPB_DEFTYPE_MSG)));

  CHK_OOM(upb_inttable_init2(&m->itof, UPB_CTYPE_CONSTPTR, ctx->alloc));
  CHK_OOM(upb_strtable_init2(&m->ntof, UPB_CTYPE_CONSTPTR, ctx->alloc));

  m->file = ctx->file;
  m->map_entry = false;

  options = google_protobuf_DescriptorProto_options(msg_proto);

  if (options) {
    m->map_entry = google_protobuf_MessageOptions_map_entry(options);
  }

  if (ctx->layouts) {
    m->layout = *ctx->layouts;
    ctx->layouts++;
  } else {
    /* Allocate now (to allow cross-linking), populate later. */
    m->layout = upb_malloc(ctx->alloc, sizeof(*m->layout));
  }

  oneofs = google_protobuf_DescriptorProto_oneof_decl(msg_proto, &n);
  m->oneof_count = 0;
  m->oneofs = upb_malloc(ctx->alloc, sizeof(*m->oneofs) * n);
  for (i = 0; i < n; i++) {
    CHK(create_oneofdef(ctx, m, oneofs[i]));
  }

  fields = google_protobuf_DescriptorProto_field(msg_proto, &n);
  m->field_count = 0;
  m->fields = upb_malloc(ctx->alloc, sizeof(*m->fields) * n);
  for (i = 0; i < n; i++) {
    CHK(create_fielddef(ctx, m->full_name, m, fields[i]));
  }

  CHK(assign_msg_indices(m, ctx->status));
  assign_msg_wellknowntype(m);
  upb_inttable_compact2(&m->itof, ctx->alloc);

  /* This message is built.  Now build nested messages and enums. */

  enums = google_protobuf_DescriptorProto_enum_type(msg_proto, &n);
  for (i = 0; i < n; i++) {
    CHK(create_enumdef(ctx, m->full_name, enums[i]));
  }

  msgs = google_protobuf_DescriptorProto_nested_type(msg_proto, &n);
  for (i = 0; i < n; i++) {
    CHK(create_msgdef(ctx, m->full_name, msgs[i]));
  }

  return true;
}

typedef struct {
  int msg_count;
  int enum_count;
  int ext_count;
} decl_counts;

static void count_types_in_msg(const google_protobuf_DescriptorProto *msg_proto,
                               decl_counts *counts) {
  const google_protobuf_DescriptorProto *const *msgs;
  size_t i, n;

  counts->msg_count++;

  msgs = google_protobuf_DescriptorProto_nested_type(msg_proto, &n);
  for (i = 0; i < n; i++) {
    count_types_in_msg(msgs[i], counts);
  }

  google_protobuf_DescriptorProto_enum_type(msg_proto, &n);
  counts->enum_count += n;

  google_protobuf_DescriptorProto_extension(msg_proto, &n);
  counts->ext_count += n;
}

static void count_types_in_file(
    const google_protobuf_FileDescriptorProto *file_proto,
    decl_counts *counts) {
  const google_protobuf_DescriptorProto *const *msgs;
  size_t i, n;

  msgs = google_protobuf_FileDescriptorProto_message_type(file_proto, &n);
  for (i = 0; i < n; i++) {
    count_types_in_msg(msgs[i], counts);
  }

  google_protobuf_FileDescriptorProto_enum_type(file_proto, &n);
  counts->enum_count += n;

  google_protobuf_FileDescriptorProto_extension(file_proto, &n);
  counts->ext_count += n;
}

static bool resolve_fielddef(const symtab_addctx *ctx, const char *prefix,
                             upb_fielddef *f) {
  upb_strview name;
  const google_protobuf_FieldDescriptorProto *field_proto = f->sub.unresolved;

  if (f->is_extension_) {
    if (!google_protobuf_FieldDescriptorProto_has_extendee(field_proto)) {
      upb_status_seterrf(ctx->status,
                         "extension for field '%s' had no extendee",
                         f->full_name);
      return false;
    }

    name = google_protobuf_FieldDescriptorProto_extendee(field_proto);
    f->msgdef = symtab_resolve(ctx, f, prefix, name, UPB_DEFTYPE_MSG);
    CHK(f->msgdef);
  }

  if ((upb_fielddef_issubmsg(f) || f->type_ == UPB_DESCRIPTOR_TYPE_ENUM) &&
      !google_protobuf_FieldDescriptorProto_has_type_name(field_proto)) {
    upb_status_seterrf(ctx->status, "field '%s' is missing type name",
                       f->full_name);
    return false;
  }

  name = google_protobuf_FieldDescriptorProto_type_name(field_proto);

  if (upb_fielddef_issubmsg(f)) {
    f->sub.msgdef = symtab_resolve(ctx, f, prefix, name, UPB_DEFTYPE_MSG);
    CHK(f->sub.msgdef);
  } else if (f->type_ == UPB_DESCRIPTOR_TYPE_ENUM) {
    f->sub.enumdef = symtab_resolve(ctx, f, prefix, name, UPB_DEFTYPE_ENUM);
    CHK(f->sub.enumdef);
  }

  /* Have to delay resolving of the default value until now because of the enum
   * case, since enum defaults are specified with a label. */
  if (google_protobuf_FieldDescriptorProto_has_default_value(field_proto)) {
    upb_strview defaultval =
        google_protobuf_FieldDescriptorProto_default_value(field_proto);

    if (f->file->syntax == UPB_SYNTAX_PROTO3) {
      upb_status_seterrf(ctx->status,
                         "proto3 fields cannot have explicit defaults (%s)",
                         f->full_name);
      return false;
    }

    if (upb_fielddef_issubmsg(f)) {
      upb_status_seterrf(ctx->status,
                         "message fields cannot have explicit defaults (%s)",
                         f->full_name);
      return false;
    }

    if (!parse_default(ctx, defaultval.data, defaultval.size, f)) {
      upb_status_seterrf(ctx->status,
                         "couldn't parse default '" UPB_STRVIEW_FORMAT
                         "' for field (%s)",
                         UPB_STRVIEW_ARGS(defaultval), f->full_name);
      return false;
    }
  } else {
    set_default_default(ctx, f);
  }

  return true;
}

static bool build_filedef(
    symtab_addctx *ctx, upb_filedef *file,
    const google_protobuf_FileDescriptorProto *file_proto) {
  upb_alloc *alloc = ctx->alloc;
  const google_protobuf_FileOptions *file_options_proto;
  const google_protobuf_DescriptorProto *const *msgs;
  const google_protobuf_EnumDescriptorProto *const *enums;
  const google_protobuf_FieldDescriptorProto *const *exts;
  const upb_strview* strs;
  size_t i, n;
  decl_counts counts = {0};

  count_types_in_file(file_proto, &counts);

  file->msgs = upb_malloc(alloc, sizeof(*file->msgs) * counts.msg_count);
  file->enums = upb_malloc(alloc, sizeof(*file->enums) * counts.enum_count);
  file->exts = upb_malloc(alloc, sizeof(*file->exts) * counts.ext_count);

  CHK_OOM(counts.msg_count == 0 || file->msgs);
  CHK_OOM(counts.enum_count == 0 || file->enums);
  CHK_OOM(counts.ext_count == 0 || file->exts);

  /* We increment these as defs are added. */
  file->msg_count = 0;
  file->enum_count = 0;
  file->ext_count = 0;

  if (!google_protobuf_FileDescriptorProto_has_name(file_proto)) {
    upb_status_seterrmsg(ctx->status, "File has no name");
    return false;
  }

  file->name =
      strviewdup(ctx, google_protobuf_FileDescriptorProto_name(file_proto));
  file->phpprefix = NULL;
  file->phpnamespace = NULL;

  if (google_protobuf_FileDescriptorProto_has_package(file_proto)) {
    upb_strview package =
        google_protobuf_FileDescriptorProto_package(file_proto);
    CHK(upb_isident(package, true, ctx->status));
    file->package = strviewdup(ctx, package);
  } else {
    file->package = NULL;
  }

  if (google_protobuf_FileDescriptorProto_has_syntax(file_proto)) {
    upb_strview syntax =
        google_protobuf_FileDescriptorProto_syntax(file_proto);

    if (streql_view(syntax, "proto2")) {
      file->syntax = UPB_SYNTAX_PROTO2;
    } else if (streql_view(syntax, "proto3")) {
      file->syntax = UPB_SYNTAX_PROTO3;
    } else {
      upb_status_seterrf(ctx->status, "Invalid syntax '" UPB_STRVIEW_FORMAT "'",
                         UPB_STRVIEW_ARGS(syntax));
      return false;
    }
  } else {
    file->syntax = UPB_SYNTAX_PROTO2;
  }

  /* Read options. */
  file_options_proto = google_protobuf_FileDescriptorProto_options(file_proto);
  if (file_options_proto) {
    if (google_protobuf_FileOptions_has_php_class_prefix(file_options_proto)) {
      file->phpprefix = strviewdup(
          ctx,
          google_protobuf_FileOptions_php_class_prefix(file_options_proto));
    }
    if (google_protobuf_FileOptions_has_php_namespace(file_options_proto)) {
      file->phpnamespace = strviewdup(
          ctx, google_protobuf_FileOptions_php_namespace(file_options_proto));
    }
  }

  /* Verify dependencies. */
  strs = google_protobuf_FileDescriptorProto_dependency(file_proto, &n);
  file->deps = upb_malloc(alloc, sizeof(*file->deps) * n) ;
  CHK_OOM(n == 0 || file->deps);

  for (i = 0; i < n; i++) {
    upb_strview dep_name = strs[i];
    upb_value v;
    if (!upb_strtable_lookup2(&ctx->symtab->files, dep_name.data,
                              dep_name.size, &v)) {
      upb_status_seterrf(ctx->status,
                         "Depends on file '" UPB_STRVIEW_FORMAT
                         "', but it has not been loaded",
                         UPB_STRVIEW_ARGS(dep_name));
      return false;
    }
    file->deps[i] = upb_value_getconstptr(v);
  }

  /* Create messages. */
  msgs = google_protobuf_FileDescriptorProto_message_type(file_proto, &n);
  for (i = 0; i < n; i++) {
    CHK(create_msgdef(ctx, file->package, msgs[i]));
  }

  /* Create enums. */
  enums = google_protobuf_FileDescriptorProto_enum_type(file_proto, &n);
  for (i = 0; i < n; i++) {
    CHK(create_enumdef(ctx, file->package, enums[i]));
  }

  /* Create extensions. */
  exts = google_protobuf_FileDescriptorProto_extension(file_proto, &n);
  file->exts = upb_malloc(alloc, sizeof(*file->exts) * n);
  CHK_OOM(n == 0 || file->exts);
  for (i = 0; i < n; i++) {
    CHK(create_fielddef(ctx, file->package, NULL, exts[i]));
  }

  /* Now that all names are in the table, build layouts and resolve refs. */
  for (i = 0; i < file->ext_count; i++) {
    CHK(resolve_fielddef(ctx, file->package, (upb_fielddef*)&file->exts[i]));
  }

  for (i = 0; i < file->msg_count; i++) {
    const upb_msgdef *m = &file->msgs[i];
    int j;
    for (j = 0; j < m->field_count; j++) {
      CHK(resolve_fielddef(ctx, m->full_name, (upb_fielddef*)&m->fields[j]));
    }
  }

  if (!ctx->layouts) {
    for (i = 0; i < file->msg_count; i++) {
      const upb_msgdef *m = &file->msgs[i];
      make_layout(ctx->symtab, m);
    }
  }

  return true;
 }

static bool upb_symtab_addtotabs(upb_symtab *s, symtab_addctx *ctx,
                                 upb_status *status) {
  const upb_filedef *file = ctx->file;
  upb_alloc *alloc = upb_arena_alloc(s->arena);
  upb_strtable_iter iter;

  CHK_OOM(upb_strtable_insert3(&s->files, file->name, strlen(file->name),
                               upb_value_constptr(file), alloc));

  upb_strtable_begin(&iter, ctx->addtab);
  for (; !upb_strtable_done(&iter); upb_strtable_next(&iter)) {
    upb_strview key = upb_strtable_iter_key(&iter);
    upb_value value = upb_strtable_iter_value(&iter);
    CHK_OOM(upb_strtable_insert3(&s->syms, key.data, key.size, value, alloc));
  }

  return true;
}

/* upb_filedef ****************************************************************/

const char *upb_filedef_name(const upb_filedef *f) {
  return f->name;
}

const char *upb_filedef_package(const upb_filedef *f) {
  return f->package;
}

const char *upb_filedef_phpprefix(const upb_filedef *f) {
  return f->phpprefix;
}

const char *upb_filedef_phpnamespace(const upb_filedef *f) {
  return f->phpnamespace;
}

upb_syntax_t upb_filedef_syntax(const upb_filedef *f) {
  return f->syntax;
}

int upb_filedef_msgcount(const upb_filedef *f) {
  return f->msg_count;
}

int upb_filedef_depcount(const upb_filedef *f) {
  return f->dep_count;
}

int upb_filedef_enumcount(const upb_filedef *f) {
  return f->enum_count;
}

const upb_filedef *upb_filedef_dep(const upb_filedef *f, int i) {
  return i < 0 || i >= f->dep_count ? NULL : f->deps[i];
}

const upb_msgdef *upb_filedef_msg(const upb_filedef *f, int i) {
  return i < 0 || i >= f->msg_count ? NULL : &f->msgs[i];
}

const upb_enumdef *upb_filedef_enum(const upb_filedef *f, int i) {
  return i < 0 || i >= f->enum_count ? NULL : &f->enums[i];
}

void upb_symtab_free(upb_symtab *s) {
  upb_arena_free(s->arena);
  upb_gfree(s);
}

upb_symtab *upb_symtab_new(void) {
  upb_symtab *s = upb_gmalloc(sizeof(*s));
  upb_alloc *alloc;

  if (!s) {
    return NULL;
  }

  s->arena = upb_arena_new();
  alloc = upb_arena_alloc(s->arena);

  if (!upb_strtable_init2(&s->syms, UPB_CTYPE_CONSTPTR, alloc) ||
      !upb_strtable_init2(&s->files, UPB_CTYPE_CONSTPTR, alloc)) {
    upb_arena_free(s->arena);
    upb_gfree(s);
    s = NULL;
  }
  return s;
}

const upb_msgdef *upb_symtab_lookupmsg(const upb_symtab *s, const char *sym) {
  upb_value v;
  return upb_strtable_lookup(&s->syms, sym, &v) ?
      unpack_def(v, UPB_DEFTYPE_MSG) : NULL;
}

const upb_msgdef *upb_symtab_lookupmsg2(const upb_symtab *s, const char *sym,
                                        size_t len) {
  upb_value v;
  return upb_strtable_lookup2(&s->syms, sym, len, &v) ?
      unpack_def(v, UPB_DEFTYPE_MSG) : NULL;
}

const upb_enumdef *upb_symtab_lookupenum(const upb_symtab *s, const char *sym) {
  upb_value v;
  return upb_strtable_lookup(&s->syms, sym, &v) ?
      unpack_def(v, UPB_DEFTYPE_ENUM) : NULL;
}

const upb_filedef *upb_symtab_lookupfile(const upb_symtab *s, const char *name) {
  upb_value v;
  return upb_strtable_lookup(&s->files, name, &v) ? upb_value_getconstptr(v)
                                                  : NULL;
}

int upb_symtab_filecount(const upb_symtab *s) {
  return (int)upb_strtable_count(&s->files);
}

static const upb_filedef *_upb_symtab_addfile(
    upb_symtab *s, const google_protobuf_FileDescriptorProto *file_proto,
    const upb_msglayout **layouts, upb_status *status) {
  upb_arena *tmparena = upb_arena_new();
  upb_strtable addtab;
  upb_alloc *alloc = upb_arena_alloc(s->arena);
  upb_filedef *file = upb_malloc(alloc, sizeof(*file));
  bool ok;
  symtab_addctx ctx;

  ctx.file = file;
  ctx.symtab = s;
  ctx.alloc = alloc;
  ctx.tmp = upb_arena_alloc(tmparena);
  ctx.addtab = &addtab;
  ctx.layouts = layouts;
  ctx.status = status;

  ok = file &&
      upb_strtable_init2(&addtab, UPB_CTYPE_CONSTPTR, ctx.tmp) &&
      build_filedef(&ctx, file, file_proto) &&
      upb_symtab_addtotabs(s, &ctx, status);

  upb_arena_free(tmparena);
  return ok ? file : NULL;
}

const upb_filedef *upb_symtab_addfile(
    upb_symtab *s, const google_protobuf_FileDescriptorProto *file_proto,
    upb_status *status) {
  return _upb_symtab_addfile(s, file_proto, NULL, status);
}

/* Include here since we want most of this file to be stdio-free. */
#include <stdio.h>

bool _upb_symtab_loaddefinit(upb_symtab *s, const upb_def_init *init) {
  /* Since this function should never fail (it would indicate a bug in upb) we
   * print errors to stderr instead of returning error status to the user. */
  upb_def_init **deps = init->deps;
  google_protobuf_FileDescriptorProto *file;
  upb_arena *arena;
  upb_status status;

  upb_status_clear(&status);

  if (upb_strtable_lookup(&s->files, init->filename, NULL)) {
    return true;
  }

  arena = upb_arena_new();

  for (; *deps; deps++) {
    if (!_upb_symtab_loaddefinit(s, *deps)) goto err;
  }

  file = google_protobuf_FileDescriptorProto_parse(
      init->descriptor.data, init->descriptor.size, arena);

  if (!file) {
    upb_status_seterrf(
        &status,
        "Failed to parse compiled-in descriptor for file '%s'. This should "
        "never happen.",
        init->filename);
    goto err;
  }

  if (!_upb_symtab_addfile(s, file, init->layouts, &status)) goto err;

  upb_arena_free(arena);
  return true;

err:
  fprintf(stderr, "Error loading compiled-in descriptor: %s\n",
          upb_status_errmsg(&status));
  upb_arena_free(arena);
  return false;
}

#undef CHK
#undef CHK_OOM


#include <string.h>


static char field_size[] = {
  0,/* 0 */
  8, /* UPB_DESCRIPTOR_TYPE_DOUBLE */
  4, /* UPB_DESCRIPTOR_TYPE_FLOAT */
  8, /* UPB_DESCRIPTOR_TYPE_INT64 */
  8, /* UPB_DESCRIPTOR_TYPE_UINT64 */
  4, /* UPB_DESCRIPTOR_TYPE_INT32 */
  8, /* UPB_DESCRIPTOR_TYPE_FIXED64 */
  4, /* UPB_DESCRIPTOR_TYPE_FIXED32 */
  1, /* UPB_DESCRIPTOR_TYPE_BOOL */
  sizeof(upb_strview), /* UPB_DESCRIPTOR_TYPE_STRING */
  sizeof(void*), /* UPB_DESCRIPTOR_TYPE_GROUP */
  sizeof(void*), /* UPB_DESCRIPTOR_TYPE_MESSAGE */
  sizeof(upb_strview), /* UPB_DESCRIPTOR_TYPE_BYTES */
  4, /* UPB_DESCRIPTOR_TYPE_UINT32 */
  4, /* UPB_DESCRIPTOR_TYPE_ENUM */
  4, /* UPB_DESCRIPTOR_TYPE_SFIXED32 */
  8, /* UPB_DESCRIPTOR_TYPE_SFIXED64 */
  4, /* UPB_DESCRIPTOR_TYPE_SINT32 */
  8, /* UPB_DESCRIPTOR_TYPE_SINT64 */
};

/* Strings/bytes are special-cased in maps. */
static char _upb_fieldtype_to_mapsize[12] = {
  0,
  1,  /* UPB_TYPE_BOOL */
  4,  /* UPB_TYPE_FLOAT */
  4,  /* UPB_TYPE_INT32 */
  4,  /* UPB_TYPE_UINT32 */
  4,  /* UPB_TYPE_ENUM */
  sizeof(void*),  /* UPB_TYPE_MESSAGE */
  8,  /* UPB_TYPE_DOUBLE */
  8,  /* UPB_TYPE_INT64 */
  8,  /* UPB_TYPE_UINT64 */
  0,  /* UPB_TYPE_STRING */
  0,  /* UPB_TYPE_BYTES */
};

/** upb_msg *******************************************************************/

/* If we always read/write as a consistent type to each address, this shouldn't
 * violate aliasing.
 */
#define PTR_AT(msg, ofs, type) (type*)((char*)msg + ofs)

upb_msg *upb_msg_new(const upb_msgdef *m, upb_arena *a) {
  return _upb_msg_new(upb_msgdef_layout(m), a);
}

static bool in_oneof(const upb_msglayout_field *field) {
  return field->presence < 0;
}

static uint32_t *oneofcase(const upb_msg *msg,
                           const upb_msglayout_field *field) {
  UPB_ASSERT(in_oneof(field));
  return PTR_AT(msg, ~field->presence, uint32_t);
}

static upb_msgval _upb_msg_getraw(const upb_msg *msg, const upb_fielddef *f) {
  const upb_msglayout_field *field = upb_fielddef_layout(f);
  const char *mem = PTR_AT(msg, field->offset, char);
  upb_msgval val = {0};
  int size = upb_fielddef_isseq(f) ? sizeof(void *)
                                   : field_size[field->descriptortype];
  memcpy(&val, mem, size);
  return val;
}

bool upb_msg_has(const upb_msg *msg, const upb_fielddef *f) {
  const upb_msglayout_field *field = upb_fielddef_layout(f);
  if (in_oneof(field)) {
    return *oneofcase(msg, field) == field->number;
  } else if (field->presence > 0) {
    uint32_t hasbit = field->presence;
    return *PTR_AT(msg, hasbit / 8, char) & (1 << (hasbit % 8));
  } else {
    UPB_ASSERT(field->descriptortype == UPB_DESCRIPTOR_TYPE_MESSAGE ||
               field->descriptortype == UPB_DESCRIPTOR_TYPE_GROUP);
    return _upb_msg_getraw(msg, f).msg_val != NULL;
  }
}

bool upb_msg_hasoneof(const upb_msg *msg, const upb_oneofdef *o) {
  upb_oneof_iter i;
  const upb_fielddef *f;
  const upb_msglayout_field *field;

  upb_oneof_begin(&i, o);
  if (upb_oneof_done(&i)) return false;
  f = upb_oneof_iter_field(&i);
  field = upb_fielddef_layout(f);
  return *oneofcase(msg, field) != 0;
}

upb_msgval upb_msg_get(const upb_msg *msg, const upb_fielddef *f) {
  if (!upb_fielddef_haspresence(f) || upb_msg_has(msg, f)) {
    return _upb_msg_getraw(msg, f);
  } else {
    /* TODO(haberman): change upb_fielddef to not require this switch(). */
    upb_msgval val = {0};
    switch (upb_fielddef_type(f)) {
      case UPB_TYPE_INT32:
      case UPB_TYPE_ENUM:
        val.int32_val = upb_fielddef_defaultint32(f);
        break;
      case UPB_TYPE_INT64:
        val.int64_val = upb_fielddef_defaultint64(f);
        break;
      case UPB_TYPE_UINT32:
        val.uint32_val = upb_fielddef_defaultuint32(f);
        break;
      case UPB_TYPE_UINT64:
        val.uint64_val = upb_fielddef_defaultuint64(f);
        break;
      case UPB_TYPE_FLOAT:
        val.float_val = upb_fielddef_defaultfloat(f);
        break;
      case UPB_TYPE_DOUBLE:
        val.double_val = upb_fielddef_defaultdouble(f);
        break;
      case UPB_TYPE_BOOL:
        val.double_val = upb_fielddef_defaultbool(f);
        break;
      case UPB_TYPE_STRING:
      case UPB_TYPE_BYTES:
        val.str_val.data = upb_fielddef_defaultstr(f, &val.str_val.size);
        break;
      case UPB_TYPE_MESSAGE:
        val.msg_val = NULL;
        break;
    }
    return val;
  }
}

upb_mutmsgval upb_msg_mutable(upb_msg *msg, const upb_fielddef *f,
                              upb_arena *a) {
  const upb_msglayout_field *field = upb_fielddef_layout(f);
  upb_mutmsgval ret;
  char *mem = PTR_AT(msg, field->offset, char);
  bool wrong_oneof = in_oneof(field) && *oneofcase(msg, field) != field->number;

  memcpy(&ret, mem, sizeof(void*));

  if (a && (!ret.msg || wrong_oneof)) {
    if (upb_fielddef_ismap(f)) {
      const upb_msgdef *entry = upb_fielddef_msgsubdef(f);
      const upb_fielddef *key = upb_msgdef_itof(entry, UPB_MAPENTRY_KEY);
      const upb_fielddef *value = upb_msgdef_itof(entry, UPB_MAPENTRY_VALUE);
      ret.map = upb_map_new(a, upb_fielddef_type(key), upb_fielddef_type(value));
    } else if (upb_fielddef_isseq(f)) {
      ret.array = upb_array_new(a, upb_fielddef_type(f));
    } else {
      UPB_ASSERT(upb_fielddef_issubmsg(f));
      ret.msg = upb_msg_new(upb_fielddef_msgsubdef(f), a);
    }

    memcpy(mem, &ret, sizeof(void*));

    if (wrong_oneof) {
      *oneofcase(msg, field) = field->number;
    }
  }
  return ret;
}

void upb_msg_set(upb_msg *msg, const upb_fielddef *f, upb_msgval val,
                 upb_arena *a) {
  const upb_msglayout_field *field = upb_fielddef_layout(f);
  char *mem = PTR_AT(msg, field->offset, char);
  int size = upb_fielddef_isseq(f) ? sizeof(void *)
                                   : field_size[field->descriptortype];
  memcpy(mem, &val, size);
  if (in_oneof(field)) {
    *oneofcase(msg, field) = field->number;
  }
}

bool upb_msg_next(const upb_msg *msg, const upb_msgdef *m,
                  const upb_symtab *ext_pool, const upb_fielddef **out_f,
                  upb_msgval *out_val, size_t *iter) {
  size_t i = *iter;
  const upb_msgval zero = {0};
  const upb_fielddef *f;
  while ((f = _upb_msgdef_field(m, (int)++i)) != NULL) {
    upb_msgval val = _upb_msg_getraw(msg, f);

    /* Skip field if unset or empty. */
    if (upb_fielddef_haspresence(f)) {
      if (!upb_msg_has(msg, f)) continue;
    } else {
      upb_msgval test = val;
      if (upb_fielddef_isstring(f) && !upb_fielddef_isseq(f)) {
        /* Clear string pointer, only size matters (ptr could be non-NULL). */
        test.str_val.data = NULL;
      }
      /* Continue if NULL or 0. */
      if (memcmp(&test, &zero, sizeof(test)) == 0) continue;

      /* Continue on empty array or map. */
      if (upb_fielddef_ismap(f)) {
        if (upb_map_size(test.map_val) == 0) continue;
      } else if (upb_fielddef_isseq(f)) {
        if (upb_array_size(test.array_val) == 0) continue;
      }
    }

    *out_val = val;
    *out_f = f;
    *iter = i;
    return true;
  }
  *iter = i;
  return false;
}

/** upb_array *****************************************************************/

upb_array *upb_array_new(upb_arena *a, upb_fieldtype_t type) {
  return _upb_array_new(a, type);
}

size_t upb_array_size(const upb_array *arr) {
  return arr->len;
}

upb_msgval upb_array_get(const upb_array *arr, size_t i) {
  upb_msgval ret;
  const char* data = _upb_array_constptr(arr);
  int lg2 = arr->data & 7;
  UPB_ASSERT(i < arr->len);
  memcpy(&ret, data + (i << lg2), 1 << lg2);
  return ret;
}

void upb_array_set(upb_array *arr, size_t i, upb_msgval val) {
  char* data = _upb_array_ptr(arr);
  int lg2 = arr->data & 7;
  UPB_ASSERT(i < arr->len);
  memcpy(data + (i << lg2), &val, 1 << lg2);
}

bool upb_array_append(upb_array *arr, upb_msgval val, upb_arena *arena) {
  if (!_upb_array_realloc(arr, arr->len + 1, arena)) {
    return false;
  }
  arr->len++;
  upb_array_set(arr, arr->len - 1, val);
  return true;
}

/* Resizes the array to the given size, reallocating if necessary, and returns a
 * pointer to the new array elements. */
bool upb_array_resize(upb_array *arr, size_t size, upb_arena *arena) {
  return _upb_array_realloc(arr, size, arena);
}

/** upb_map *******************************************************************/

upb_map *upb_map_new(upb_arena *a, upb_fieldtype_t key_type,
                     upb_fieldtype_t value_type) {
  return _upb_map_new(a, _upb_fieldtype_to_mapsize[key_type],
                      _upb_fieldtype_to_mapsize[value_type]);
}

size_t upb_map_size(const upb_map *map) {
  return _upb_map_size(map);
}

bool upb_map_get(const upb_map *map, upb_msgval key, upb_msgval *val) {
  return _upb_map_get(map, &key, map->key_size, val, map->val_size);
}

bool upb_map_set(upb_map *map, upb_msgval key, upb_msgval val,
                 upb_arena *arena) {
  return _upb_map_set(map, &key, map->key_size, &val, map->val_size, arena);
}

bool upb_map_delete(upb_map *map, upb_msgval key) {
  return _upb_map_delete(map, &key, map->key_size);
}

bool upb_mapiter_next(const upb_map *map, size_t *iter) {
  return _upb_map_next(map, iter);
}

/* Returns the key and value for this entry of the map. */
upb_msgval upb_mapiter_key(const upb_map *map, size_t iter) {
  upb_strtable_iter i;
  upb_msgval ret;
  i.t = &map->table;
  i.index = iter;
  _upb_map_fromkey(upb_strtable_iter_key(&i), &ret, map->key_size);
  return ret;
}

upb_msgval upb_mapiter_value(const upb_map *map, size_t iter) {
  upb_strtable_iter i;
  upb_msgval ret;
  i.t = &map->table;
  i.index = iter;
  _upb_map_fromvalue(upb_strtable_iter_value(&i), &ret, map->val_size);
  return ret;
}

/* void upb_mapiter_setvalue(upb_map *map, size_t iter, upb_msgval value); */


#ifdef UPB_MSVC_VSNPRINTF
/* Visual C++ earlier than 2015 doesn't have standard C99 snprintf and
 * vsnprintf. To support them, missing functions are manually implemented
 * using the existing secure functions. */
int msvc_vsnprintf(char* s, size_t n, const char* format, va_list arg) {
  if (!s) {
    return _vscprintf(format, arg);
  }
  int ret = _vsnprintf_s(s, n, _TRUNCATE, format, arg);
  if (ret < 0) {
	ret = _vscprintf(format, arg);
  }
  return ret;
}

int msvc_snprintf(char* s, size_t n, const char* format, ...) {
  va_list arg;
  va_start(arg, format);
  int ret = msvc_vsnprintf(s, n, format, arg);
  va_end(arg);
  return ret;
}
#endif


#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>


/* Special header, must be included last. */

typedef struct {
  const char *ptr, *end;
  upb_arena *arena;  /* TODO: should we have a tmp arena for tmp data? */
  const upb_symtab *any_pool;
  int depth;
  upb_status *status;
  jmp_buf err;
  int line;
  const char *line_begin;
  bool is_first;
  int options;
  const upb_fielddef *debug_field;
} jsondec;

enum { JD_OBJECT, JD_ARRAY, JD_STRING, JD_NUMBER, JD_TRUE, JD_FALSE, JD_NULL };

/* Forward declarations of mutually-recursive functions. */
static void jsondec_wellknown(jsondec *d, upb_msg *msg, const upb_msgdef *m);
static upb_msgval jsondec_value(jsondec *d, const upb_fielddef *f);
static void jsondec_wellknownvalue(jsondec *d, upb_msg *msg,
                                   const upb_msgdef *m);
static void jsondec_object(jsondec *d, upb_msg *msg, const upb_msgdef *m);

static bool jsondec_streql(upb_strview str, const char *lit) {
  return str.size == strlen(lit) && memcmp(str.data, lit, str.size) == 0;
}

UPB_NORETURN static void jsondec_err(jsondec *d, const char *msg) {
  upb_status_seterrmsg(d->status, msg);
  longjmp(d->err, 1);
}

UPB_NORETURN static void jsondec_errf(jsondec *d, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  upb_status_vseterrf(d->status, fmt, argp);
  va_end(argp);
  longjmp(d->err, 1);
}

static void jsondec_skipws(jsondec *d) {
  while (d->ptr != d->end) {
    switch (*d->ptr) {
      case '\n':
        d->line++;
        d->line_begin = d->ptr;
        /* Fallthrough. */
      case '\r':
      case '\t':
      case ' ':
        d->ptr++;
        break;
      default:
        return;
    }
  }
  jsondec_err(d, "Unexpected EOF");
}

static bool jsondec_tryparsech(jsondec *d, char ch) {
  if (d->ptr == d->end || *d->ptr != ch) return false;
  d->ptr++;
  return true;
}

static void jsondec_parselit(jsondec *d, const char *lit) {
  size_t len = strlen(lit);
  if (d->end - d->ptr < len || memcmp(d->ptr, lit, len) != 0) {
    jsondec_errf(d, "Expected: '%s'", lit);
  }
  d->ptr += len;
}

static void jsondec_wsch(jsondec *d, char ch) {
  jsondec_skipws(d);
  if (!jsondec_tryparsech(d, ch)) {
    jsondec_errf(d, "Expected: '%c'", ch);
  }
}

static void jsondec_true(jsondec *d) { jsondec_parselit(d, "true"); }
static void jsondec_false(jsondec *d) { jsondec_parselit(d, "false"); }
static void jsondec_null(jsondec *d) { jsondec_parselit(d, "null"); }

static void jsondec_entrysep(jsondec *d) {
  jsondec_skipws(d);
  jsondec_parselit(d, ":");
}

static int jsondec_rawpeek(jsondec *d) {
  switch (*d->ptr) {
    case '{':
      return JD_OBJECT;
    case '[':
      return JD_ARRAY;
    case '"':
      return JD_STRING;
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return JD_NUMBER;
    case 't':
      return JD_TRUE;
    case 'f':
      return JD_FALSE;
    case 'n':
      return JD_NULL;
    default:
      jsondec_errf(d, "Unexpected character: '%c'", *d->ptr);
  }
}

/* JSON object/array **********************************************************/

/* These are used like so:
 *
 * jsondec_objstart(d);
 * while (jsondec_objnext(d)) {
 *   ...
 * }
 * jsondec_objend(d) */

static int jsondec_peek(jsondec *d) {
  jsondec_skipws(d);
  return jsondec_rawpeek(d);
}

static void jsondec_push(jsondec *d) {
  if (--d->depth < 0) {
    jsondec_err(d, "Recursion limit exceeded");
  }
  d->is_first = true;
}

static bool jsondec_seqnext(jsondec *d, char end_ch) {
  jsondec_skipws(d);
  if (*d->ptr == end_ch) return false;

  if (d->is_first) {
    d->is_first = false;
  } else {
    jsondec_parselit(d, ",");
  }

  return true;
}

static void jsondec_arrstart(jsondec *d) {
  jsondec_push(d);
  jsondec_wsch(d, '[');
}

static void jsondec_arrend(jsondec *d) {
  d->depth++;
  jsondec_wsch(d, ']');
}

static bool jsondec_arrnext(jsondec *d) {
  return jsondec_seqnext(d, ']');
}

static void jsondec_objstart(jsondec *d) {
  jsondec_push(d);
  jsondec_wsch(d, '{');
}

static void jsondec_objend(jsondec *d) {
  d->depth++;
  jsondec_wsch(d, '}');
}

static bool jsondec_objnext(jsondec *d) {
  if (!jsondec_seqnext(d, '}')) return false;
  if (jsondec_peek(d) != JD_STRING) {
    jsondec_err(d, "Object must start with string");
  }
  return true;
}

/* JSON number ****************************************************************/

static bool jsondec_tryskipdigits(jsondec *d) {
  const char *start = d->ptr;

  while (d->ptr < d->end) {
    if (*d->ptr < '0' || *d->ptr > '9') {
      break;
    }
    d->ptr++;
  }

  return d->ptr != start;
}

static void jsondec_skipdigits(jsondec *d) {
  if (!jsondec_tryskipdigits(d)) {
    jsondec_err(d, "Expected one or more digits");
  }
}

static double jsondec_number(jsondec *d) {
  const char *start = d->ptr;

  assert(jsondec_rawpeek(d) == JD_NUMBER);

  /* Skip over the syntax of a number, as specified by JSON. */
  if (*d->ptr == '-') d->ptr++;

  if (jsondec_tryparsech(d, '0')) {
    if (jsondec_tryskipdigits(d)) {
      jsondec_err(d, "number cannot have leading zero");
    }
  } else {
    jsondec_skipdigits(d);
  }

  if (d->ptr == d->end) goto parse;
  if (jsondec_tryparsech(d, '.')) {
    jsondec_skipdigits(d);
  }
  if (d->ptr == d->end) goto parse;

  if (*d->ptr == 'e' || *d->ptr == 'E') {
    d->ptr++;
    if (d->ptr == d->end) {
      jsondec_err(d, "Unexpected EOF in number");
    }
    if (*d->ptr == '+' || *d->ptr == '-') {
      d->ptr++;
    }
    jsondec_skipdigits(d);
  }

parse:
  /* Having verified the syntax of a JSON number, use strtod() to parse
   * (strtod() accepts a superset of JSON syntax). */
  errno = 0;
  {
    char* end;
    double val = strtod(start, &end);
    assert(end == d->ptr);

    /* Currently the min/max-val conformance tests fail if we check this.  Does
     * this mean the conformance tests are wrong or strtod() is wrong, or
     * something else?  Investigate further. */
    /*
    if (errno == ERANGE) {
      jsondec_err(d, "Number out of range");
    }
    */

    if (val > DBL_MAX || val < -DBL_MAX) {
      jsondec_err(d, "Number out of range");
    }

    return val;
  }
}

/* JSON string ****************************************************************/

static char jsondec_escape(jsondec *d) {
  switch (*d->ptr++) {
    case '"':
      return '\"';
    case '\\':
      return '\\';
    case '/':
      return '/';
    case 'b':
      return '\b';
    case 'f':
      return '\f';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 't':
      return '\t';
    default:
      jsondec_err(d, "Invalid escape char");
  }
}

static uint32_t jsondec_codepoint(jsondec *d) {
  uint32_t cp = 0;
  const char *end;

  if (d->end - d->ptr < 4) {
    jsondec_err(d, "EOF inside string");
  }

  end = d->ptr + 4;
  while (d->ptr < end) {
    char ch = *d->ptr++;
    if (ch >= '0' && ch <= '9') {
      ch -= '0';
    } else if (ch >= 'a' && ch <= 'f') {
      ch = ch - 'a' + 10;
    } else if (ch >= 'A' && ch <= 'F') {
      ch = ch - 'A' + 10;
    } else {
      jsondec_err(d, "Invalid hex digit");
    }
    cp = (cp << 4) | ch;
  }

  return cp;
}

/* Parses a \uXXXX unicode escape (possibly a surrogate pair). */
static size_t jsondec_unicode(jsondec *d, char* out) {
  uint32_t cp = jsondec_codepoint(d);
  if (cp >= 0xd800 && cp <= 0xdbff) {
    /* Surrogate pair: two 16-bit codepoints become a 32-bit codepoint. */
    uint32_t high = cp;
    uint32_t low;
    jsondec_parselit(d, "\\u");
    low = jsondec_codepoint(d);
    if (low < 0xdc00 || low > 0xdfff) {
      jsondec_err(d, "Invalid low surrogate");
    }
    cp = (high & 0x3ff) << 10;
    cp |= (low & 0x3ff);
    cp += 0x10000;
  } else if (cp >= 0xdc00 && cp <= 0xdfff) {
    jsondec_err(d, "Unpaired low surrogate");
  }

  /* Write to UTF-8 */
  if (cp <= 0x7f) {
    out[0] = cp;
    return 1;
  } else if (cp <= 0x07FF) {
    out[0] = ((cp >> 6) & 0x1F) | 0xC0;
    out[1] = ((cp >> 0) & 0x3F) | 0x80;
    return 2;
  } else if (cp <= 0xFFFF) {
    out[0] = ((cp >> 12) & 0x0F) | 0xE0;
    out[1] = ((cp >> 6) & 0x3F) | 0x80;
    out[2] = ((cp >> 0) & 0x3F) | 0x80;
    return 3;
  } else if (cp < 0x10FFFF) {
    out[0] = ((cp >> 18) & 0x07) | 0xF0;
    out[1] = ((cp >> 12) & 0x3f) | 0x80;
    out[2] = ((cp >> 6) & 0x3f) | 0x80;
    out[3] = ((cp >> 0) & 0x3f) | 0x80;
    return 4;
  } else {
    jsondec_err(d, "Invalid codepoint");
  }
}

static void jsondec_resize(jsondec *d, char **buf, char **end, char **buf_end) {
  size_t oldsize = *buf_end - *buf;
  size_t len = *end - *buf;
  size_t size = UPB_MAX(8, 2 * oldsize);

  *buf = upb_arena_realloc(d->arena, *buf, len, size);
  *end = *buf + len;
  *buf_end = *buf + size;
}

static upb_strview jsondec_string(jsondec *d) {
  char *buf = NULL;
  char *end = NULL;
  char *buf_end = NULL;

  jsondec_skipws(d);

  if (*d->ptr++ != '"') {
    jsondec_err(d, "Expected string");
  }

  while (d->ptr < d->end) {
    char ch = *d->ptr++;

    if (end == buf_end) {
      jsondec_resize(d, &buf, &end, &buf_end);
    }

    switch (ch) {
      case '"': {
        upb_strview ret = {buf, end - buf};
        return ret;
      }
      case '\\':
        if (d->ptr == d->end) goto eof;
        if (*d->ptr == 'u') {
          d->ptr++;
          if (buf_end - end < 4) {
            // Allow space for maximum-sized code point (4 bytes).
            jsondec_resize(d, &buf, &end, &buf_end);
          }
          end += jsondec_unicode(d, end);
        } else {
          *end++ = jsondec_escape(d);
        }
        break;
      default:
        if ((unsigned char)*d->ptr < 0x20) {
          jsondec_err(d, "Invalid char in JSON string");
        }
        *end++ = ch;
        break;
    }
  }

eof:
  jsondec_err(d, "EOF inside string");
}

static void jsondec_skipval(jsondec *d) {
  switch (jsondec_peek(d)) {
    case JD_OBJECT:
      jsondec_objstart(d);
      while (jsondec_objnext(d)) {
        jsondec_string(d);
        jsondec_entrysep(d);
        jsondec_skipval(d);
      }
      jsondec_objend(d);
      break;
    case JD_ARRAY:
      jsondec_arrstart(d);
      while (jsondec_arrnext(d)) {
        jsondec_skipval(d);
      }
      jsondec_arrend(d);
      break;
    case JD_TRUE:
      jsondec_true(d);
      break;
    case JD_FALSE:
      jsondec_false(d);
      break;
    case JD_NULL:
      jsondec_null(d);
      break;
    case JD_STRING:
      jsondec_string(d);
      break;
    case JD_NUMBER:
      jsondec_number(d);
      break;
  }
}

/* Base64 decoding for bytes fields. ******************************************/

static int jsondec_base64_tablelookup(const char ch) {
  /* Table includes the normal base64 chars plus the URL-safe variant. */
  const signed char table[256] = {
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       62 /*+*/, -1,       62 /*-*/, -1,       63 /*/ */, 52 /*0*/,
      53 /*1*/, 54 /*2*/, 55 /*3*/, 56 /*4*/, 57 /*5*/, 58 /*6*/,  59 /*7*/,
      60 /*8*/, 61 /*9*/, -1,       -1,       -1,       -1,        -1,
      -1,       -1,       0 /*A*/,  1 /*B*/,  2 /*C*/,  3 /*D*/,   4 /*E*/,
      5 /*F*/,  6 /*G*/,  07 /*H*/, 8 /*I*/,  9 /*J*/,  10 /*K*/,  11 /*L*/,
      12 /*M*/, 13 /*N*/, 14 /*O*/, 15 /*P*/, 16 /*Q*/, 17 /*R*/,  18 /*S*/,
      19 /*T*/, 20 /*U*/, 21 /*V*/, 22 /*W*/, 23 /*X*/, 24 /*Y*/,  25 /*Z*/,
      -1,       -1,       -1,       -1,       63 /*_*/, -1,        26 /*a*/,
      27 /*b*/, 28 /*c*/, 29 /*d*/, 30 /*e*/, 31 /*f*/, 32 /*g*/,  33 /*h*/,
      34 /*i*/, 35 /*j*/, 36 /*k*/, 37 /*l*/, 38 /*m*/, 39 /*n*/,  40 /*o*/,
      41 /*p*/, 42 /*q*/, 43 /*r*/, 44 /*s*/, 45 /*t*/, 46 /*u*/,  47 /*v*/,
      48 /*w*/, 49 /*x*/, 50 /*y*/, 51 /*z*/, -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1,       -1,       -1,        -1,
      -1,       -1,       -1,       -1};

  /* Sign-extend return value so high bit will be set on any unexpected char. */
  return table[(unsigned)ch];
}

static char *jsondec_partialbase64(jsondec *d, const char *ptr, const char *end,
                                   char *out) {
  int32_t val = -1;

  switch (end - ptr) {
    case 2:
      val = jsondec_base64_tablelookup(ptr[0]) << 18 |
            jsondec_base64_tablelookup(ptr[1]) << 12;
      out[0] = val >> 16;
      out += 1;
      break;
    case 3:
      val = jsondec_base64_tablelookup(ptr[0]) << 18 |
            jsondec_base64_tablelookup(ptr[1]) << 12 |
            jsondec_base64_tablelookup(ptr[2]) << 6;
      out[0] = val >> 16;
      out[1] = (val >> 8) & 0xff;
      out += 2;
      break;
  }

  if (val < 0) {
    jsondec_err(d, "Corrupt base64");
  }

  return out;
}

static size_t jsondec_base64(jsondec *d, upb_strview str) {
  /* We decode in place. This is safe because this is a new buffer (not
   * aliasing the input) and because base64 decoding shrinks 4 bytes into 3. */
  char *out = (char*)str.data;
  const char *ptr = str.data;
  const char *end = ptr + str.size;
  const char *end4 = ptr + (str.size & -4);  /* Round down to multiple of 4. */

  for (; ptr < end4; ptr += 4, out += 3) {
    int val = jsondec_base64_tablelookup(ptr[0]) << 18 |
              jsondec_base64_tablelookup(ptr[1]) << 12 |
              jsondec_base64_tablelookup(ptr[2]) << 6 |
              jsondec_base64_tablelookup(ptr[3]) << 0;

    if (val < 0) {
      /* Junk chars or padding. Remove trailing padding, if any. */
      if (end - ptr == 4 && ptr[3] == '=') {
        if (ptr[2] == '=') {
          end -= 2;
        } else {
          end -= 1;
        }
      }
      break;
    }

    out[0] = val >> 16;
    out[1] = (val >> 8) & 0xff;
    out[2] = val & 0xff;
  }

  if (ptr < end) {
    /* Process remaining chars. We do not require padding. */
    out = jsondec_partialbase64(d, ptr, end, out);
  }

  return out - str.data;
}

/* Low-level integer parsing **************************************************/

/* We use these hand-written routines instead of strto[u]l() because the "long
 * long" variants aren't in c89. Also our version allows setting a ptr limit. */

static const char *jsondec_buftouint64(jsondec *d, const char *ptr,
                                       const char *end, uint64_t *val) {
  uint64_t u64 = 0;
  while (ptr < end) {
    unsigned ch = *ptr - '0';
    if (ch >= 10) break;
    if (u64 > UINT64_MAX / 10 || u64 * 10 > UINT64_MAX - ch) {
      jsondec_err(d, "Integer overflow");
    }
    u64 *= 10;
    u64 += ch;
    ptr++;
  }

  *val = u64;
  return ptr;
}

static const char *jsondec_buftoint64(jsondec *d, const char *ptr,
                                      const char *end, int64_t *val) {
  bool neg = false;
  uint64_t u64;

  if (ptr != end && *ptr == '-') {
    ptr++;
    neg = true;
  }

  ptr = jsondec_buftouint64(d, ptr, end, &u64);
  if (u64 > (uint64_t)INT64_MAX + neg) {
    jsondec_err(d, "Integer overflow");
  }

  *val = neg ? -u64 : u64;
  return ptr;
}

static uint64_t jsondec_strtouint64(jsondec *d, upb_strview str) {
  const char *end = str.data + str.size;
  uint64_t ret;
  if (jsondec_buftouint64(d, str.data, end, &ret) != end) {
    jsondec_err(d, "Non-number characters in quoted integer");
  }
  return ret;
}

static int64_t jsondec_strtoint64(jsondec *d, upb_strview str) {
  const char *end = str.data + str.size;
  int64_t ret;
  if (jsondec_buftoint64(d, str.data, end, &ret) != end) {
    jsondec_err(d, "Non-number characters in quoted integer");
  }
  return ret;
}

/* Primitive value types ******************************************************/

/* Parse INT32 or INT64 value. */
static upb_msgval jsondec_int(jsondec *d, const upb_fielddef *f) {
  upb_msgval val;

  switch (jsondec_peek(d)) {
    case JD_NUMBER: {
      double dbl = jsondec_number(d);
      if (dbl > 9223372036854774784.0 || dbl < -9223372036854775808.0) {
        jsondec_err(d, "JSON number is out of range.");
      }
      val.int64_val = dbl;  /* must be guarded, overflow here is UB */
      if (val.int64_val != dbl) {
        jsondec_errf(d, "JSON number was not integral (%d != %" PRId64 ")", dbl,
                     val.int64_val);
      }
      break;
    }
    case JD_STRING: {
      upb_strview str = jsondec_string(d);
      val.int64_val = jsondec_strtoint64(d, str);
      break;
    }
    default:
      jsondec_err(d, "Expected number or string");
  }

  if (upb_fielddef_type(f) == UPB_TYPE_INT32) {
    if (val.int64_val > INT32_MAX || val.int64_val < INT32_MIN) {
      jsondec_err(d, "Integer out of range.");
    }
    val.int32_val = val.int64_val;
  }

  return val;
}

/* Parse UINT32 or UINT64 value. */
static upb_msgval jsondec_uint(jsondec *d, const upb_fielddef *f) {
  upb_msgval val;

  switch (jsondec_peek(d)) {
    case JD_NUMBER: {
      double dbl = jsondec_number(d);
      if (dbl > 18446744073709549568.0 || dbl < 0) {
        jsondec_err(d, "JSON number is out of range.");
      }
      val.uint64_val = dbl;  /* must be guarded, overflow here is UB */
      if (val.uint64_val != dbl) {
        jsondec_errf(d, "JSON number was not integral (%d != %" PRIu64 ")", dbl,
                     val.uint64_val);
      }
      break;
    }
    case JD_STRING: {
      upb_strview str = jsondec_string(d);
      val.uint64_val = jsondec_strtouint64(d, str);
      break;
    }
    default:
      jsondec_err(d, "Expected number or string");
  }

  if (upb_fielddef_type(f) == UPB_TYPE_UINT32) {
    if (val.uint64_val > UINT32_MAX) {
      jsondec_err(d, "Integer out of range.");
    }
    val.uint32_val = val.uint64_val;
  }

  return val;
}

/* Parse DOUBLE or FLOAT value. */
static upb_msgval jsondec_double(jsondec *d, const upb_fielddef *f) {
  upb_strview str;
  upb_msgval val;

  switch (jsondec_peek(d)) {
    case JD_NUMBER:
      val.double_val = jsondec_number(d);
      break;
    case JD_STRING:
      str = jsondec_string(d);
      if (jsondec_streql(str, "NaN")) {
        val.double_val = 0.0 / 0.0;
      } else if (jsondec_streql(str, "Infinity")) {
        val.double_val = UPB_INFINITY;
      } else if (jsondec_streql(str, "-Infinity")) {
        val.double_val = -UPB_INFINITY;
      } else {
        val.double_val = strtod(str.data, NULL);
      }
      break;
    default:
      jsondec_err(d, "Expected number or string");
  }

  if (upb_fielddef_type(f) == UPB_TYPE_FLOAT) {
    if (val.double_val != UPB_INFINITY && val.double_val != -UPB_INFINITY &&
        (val.double_val > FLT_MAX || val.double_val < -FLT_MAX)) {
      jsondec_err(d, "Float out of range");
    }
    val.float_val = val.double_val;
  }

  return val;
}

/* Parse STRING or BYTES value. */
static upb_msgval jsondec_strfield(jsondec *d, const upb_fielddef *f) {
  upb_msgval val;
  val.str_val = jsondec_string(d);
  if (upb_fielddef_type(f) == UPB_TYPE_BYTES) {
    val.str_val.size = jsondec_base64(d, val.str_val);
  }
  return val;
}

static upb_msgval jsondec_enum(jsondec *d, const upb_fielddef *f) {
  if (jsondec_peek(d) == JD_STRING) {
    const upb_enumdef *e = upb_fielddef_enumsubdef(f);
    upb_strview str = jsondec_string(d);
    upb_msgval val;
    if (!upb_enumdef_ntoi(e, str.data, str.size, &val.int32_val)) {
      jsondec_err(d, "Unknown enumerator");
    }
    return val;
  } else {
    return jsondec_int(d, f);
  }
}

static upb_msgval jsondec_bool(jsondec *d, const upb_fielddef *f) {
  bool is_map_key = upb_fielddef_number(f) == 1 &&
                    upb_msgdef_mapentry(upb_fielddef_containingtype(f));
  upb_msgval val;

  if (is_map_key) {
    upb_strview str = jsondec_string(d);
    if (jsondec_streql(str, "true")) {
      val.bool_val = true;
    } else if (jsondec_streql(str, "false")) {
      val.bool_val = false;
    } else {
      jsondec_err(d, "Invalid boolean map key");
    }
  } else {
    switch (jsondec_peek(d)) {
      case JD_TRUE:
        val.bool_val = true;
        jsondec_true(d);
        break;
      case JD_FALSE:
        val.bool_val = false;
        jsondec_false(d);
        break;
      default:
        jsondec_err(d, "Expected true or false");
    }
  }

  return val;
}

/* Composite types (array/message/map) ****************************************/

static void jsondec_array(jsondec *d, upb_msg *msg, const upb_fielddef *f) {
  upb_array *arr = upb_msg_mutable(msg, f, d->arena).array;

  jsondec_arrstart(d);
  while (jsondec_arrnext(d)) {
    upb_msgval elem = jsondec_value(d, f);
    upb_array_append(arr, elem, d->arena);
  }
  jsondec_arrend(d);
}

static void jsondec_map(jsondec *d, upb_msg *msg, const upb_fielddef *f) {
  upb_map *map = upb_msg_mutable(msg, f, d->arena).map;
  const upb_msgdef *entry = upb_fielddef_msgsubdef(f);
  const upb_fielddef *key_f = upb_msgdef_itof(entry, 1);
  const upb_fielddef *val_f = upb_msgdef_itof(entry, 2);

  jsondec_objstart(d);
  while (jsondec_objnext(d)) {
    upb_msgval key, val;
    key = jsondec_value(d, key_f);
    jsondec_entrysep(d);
    val = jsondec_value(d, val_f);
    upb_map_set(map, key, val, d->arena);
  }
  jsondec_objend(d);
}

static void jsondec_tomsg(jsondec *d, upb_msg *msg, const upb_msgdef *m) {
  if (upb_msgdef_wellknowntype(m) == UPB_WELLKNOWN_UNSPECIFIED) {
    jsondec_object(d, msg, m);
  } else {
    jsondec_wellknown(d, msg, m);
  }
}

static upb_msgval jsondec_msg(jsondec *d, const upb_fielddef *f) {
  const upb_msgdef *m = upb_fielddef_msgsubdef(f);
  upb_msg *msg = upb_msg_new(m, d->arena);
  upb_msgval val;

  jsondec_tomsg(d, msg, m);
  val.msg_val = msg;
  return val;
}

static bool jsondec_isvalue(const upb_fielddef *f) {
  return upb_fielddef_type(f) == UPB_TYPE_MESSAGE &&
         upb_msgdef_wellknowntype(upb_fielddef_msgsubdef(f)) ==
             UPB_WELLKNOWN_VALUE;
}

static void jsondec_field(jsondec *d, upb_msg *msg, const upb_msgdef *m) {
  upb_strview name;
  const upb_fielddef *f;
  const upb_fielddef *preserved;

  name = jsondec_string(d);
  jsondec_entrysep(d);
  f = upb_msgdef_lookupjsonname(m, name.data, name.size);

  if (!f) {
    if ((d->options & UPB_JSONDEC_IGNOREUNKNOWN) == 0) {
      jsondec_err(d, "Unknown field");
    }
    jsondec_skipval(d);
    return;
  }

  if (upb_fielddef_containingoneof(f) &&
      upb_msg_hasoneof(msg, upb_fielddef_containingoneof(f))) {
    jsondec_err(d, "More than one field for this oneof.");
  }

  if (jsondec_peek(d) == JD_NULL && !jsondec_isvalue(f)) {
    /* JSON "null" indicates a default value, so no need to set anything. */
    jsondec_null(d);
    return;
  }

  preserved = d->debug_field;
  d->debug_field = f;

  if (upb_fielddef_ismap(f)) {
    jsondec_map(d, msg, f);
  } else if (upb_fielddef_isseq(f)) {
    jsondec_array(d, msg, f);
  } else if (upb_fielddef_issubmsg(f)) {
    upb_msg *submsg = upb_msg_mutable(msg, f, d->arena).msg;
    const upb_msgdef *subm = upb_fielddef_msgsubdef(f);
    jsondec_tomsg(d, submsg, subm);
  } else {
    upb_msgval val = jsondec_value(d, f);
    upb_msg_set(msg, f, val, d->arena);
  }

  d->debug_field = preserved;
}

static void jsondec_object(jsondec *d, upb_msg *msg, const upb_msgdef *m) {
  jsondec_objstart(d);
  while (jsondec_objnext(d)) jsondec_field(d, msg, m);
  jsondec_objend(d);
}

static upb_msgval jsondec_value(jsondec *d, const upb_fielddef *f) {
  switch (upb_fielddef_type(f)) {
    case UPB_TYPE_BOOL:
      return jsondec_bool(d, f);
    case UPB_TYPE_FLOAT:
    case UPB_TYPE_DOUBLE:
      return jsondec_double(d, f);
    case UPB_TYPE_UINT32:
    case UPB_TYPE_UINT64:
      return jsondec_uint(d, f);
    case UPB_TYPE_INT32:
    case UPB_TYPE_INT64:
      return jsondec_int(d, f);
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES:
      return jsondec_strfield(d, f);
    case UPB_TYPE_ENUM:
      return jsondec_enum(d, f);
    case UPB_TYPE_MESSAGE:
      return jsondec_msg(d, f);
    default:
      UPB_UNREACHABLE();
  }
}

/* Well-known types ***********************************************************/

static int jsondec_tsdigits(jsondec *d, const char **ptr, size_t digits,
                            const char *after) {
  uint64_t val;
  const char *p = *ptr;
  const char *end = p + digits;
  size_t after_len = after ? strlen(after) : 0;

  assert(digits <= 9);  /* int can't overflow. */

  if (jsondec_buftouint64(d, p, end, &val) != end ||
      (after_len && memcmp(end, after, after_len) != 0)) {
    jsondec_err(d, "Malformed timestamp");
  }

  *ptr = end + after_len;
  return val;
}

static int jsondec_nanos(jsondec *d, const char **ptr, const char *end) {
  uint64_t nanos = 0;
  const char *p = *ptr;

  if (p != end && *p == '.') {
    const char *nano_end = jsondec_buftouint64(d, p + 1, end, &nanos);
    int digits = nano_end - p - 1;
    int exp_lg10 = 9 - digits;
    if (digits > 9) {
      jsondec_err(d, "Too many digits for partial seconds");
    }
    while (exp_lg10--) nanos *= 10;
    *ptr = nano_end;
  }

  return nanos;
}

// jsondec_epochdays(1970, 1, 1) == 1970-01-01 == 0
static int jsondec_epochdays(int y, int m, int d) {
  unsigned year_base = 4800;  /* Before minimum year, divisible by 100 & 400 */
  unsigned epoch = 2472632;   /* Days between year_base and 1970 (Unix epoch) */
  unsigned carry = (unsigned)m - 3 > m;
  unsigned m_adj = m - 3 + (carry ? 12 : 0);   /* Month, counting from March */
  unsigned y_adj = y + year_base - carry;  /* Year, positive and March-based */
  unsigned base_days = (365 * 4 + 1) * y_adj / 4;    /* Approx days for year */
  unsigned centuries = y_adj / 100;
  unsigned extra_leap_days = (3 * centuries + 3) / 4; /* base_days correction */
  unsigned year_days = (367 * (m_adj + 1)) / 12 - 30;  /* Counting from March */
  return base_days - extra_leap_days + year_days + (d - 1) - epoch;
}

static int64_t jsondec_unixtime(int y, int m, int d, int h, int min, int s) {
  return (int64_t)jsondec_epochdays(y, m, d) * 86400 + h * 3600 + min * 60 + s;
}

static void jsondec_timestamp(jsondec *d, upb_msg *msg, const upb_msgdef *m) {
  upb_msgval seconds;
  upb_msgval nanos;
  upb_strview str = jsondec_string(d);
  const char *ptr = str.data;
  const char *end = ptr + str.size;

  if (str.size < 20) goto malformed;

  {
    // 1972-01-01T01:00:00
    int year = jsondec_tsdigits(d, &ptr, 4, "-");
    int mon = jsondec_tsdigits(d, &ptr, 2, "-");
    int day = jsondec_tsdigits(d, &ptr, 2, "T");
    int hour = jsondec_tsdigits(d, &ptr, 2, ":");
    int min = jsondec_tsdigits(d, &ptr, 2, ":");
    int sec = jsondec_tsdigits(d, &ptr, 2, NULL);

    seconds.int64_val = jsondec_unixtime(year, mon, day, hour, min, sec);
  }

  nanos.int32_val = jsondec_nanos(d, &ptr, end);

  {
    // [+-]08:00 or Z
    int ofs = 0;
    bool neg = false;

    if (ptr == end) goto malformed;

    switch (*ptr++) {
      case '-':
        neg = true;
        /* Fallthrough intended. */
      case '+':
        if ((end - ptr) != 5) goto malformed;
        ofs = jsondec_tsdigits(d, &ptr, 2, ":00");
        ofs *= 60 * 60;
        seconds.int64_val += (neg ? ofs : -ofs);
        break;
      case 'Z':
        if (ptr != end) goto malformed;
        break;
      default:
        goto malformed;
    }
  }

  if (seconds.int64_val < -62135596800) {
    jsondec_err(d, "Timestamp out of range");
  }

  upb_msg_set(msg, upb_msgdef_itof(m, 1), seconds, d->arena);
  upb_msg_set(msg, upb_msgdef_itof(m, 2), nanos, d->arena);
  return;

malformed:
  jsondec_err(d, "Malformed timestamp");
}

static void jsondec_duration(jsondec *d, upb_msg *msg, const upb_msgdef *m) {
  upb_msgval seconds;
  upb_msgval nanos;
  upb_strview str = jsondec_string(d);
  const char *ptr = str.data;
  const char *end = ptr + str.size;

  // "3.000000001s", "3s", etc.
  ptr = jsondec_buftoint64(d, ptr, end, &seconds.int64_val);
  nanos.int32_val = jsondec_nanos(d, &ptr, end);

  if (end - ptr != 1 || *ptr != 's') {
    jsondec_err(d, "Malformed duration");
  }

  if (seconds.int64_val < -315576000000LL || seconds.int64_val > 315576000000LL) {
    jsondec_err(d, "Duration out of range");
  }

  if (seconds.int64_val < 0) {
    nanos.int32_val = - nanos.int32_val;
  }

  upb_msg_set(msg, upb_msgdef_itof(m, 1), seconds, d->arena);
  upb_msg_set(msg, upb_msgdef_itof(m, 2), nanos, d->arena);
}

static void jsondec_listvalue(jsondec *d, upb_msg *msg, const upb_msgdef *m) {
  const upb_fielddef *values_f = upb_msgdef_itof(m, 1);
  const upb_msgdef *value_m = upb_fielddef_msgsubdef(values_f);
  upb_array *values = upb_msg_mutable(msg, values_f, d->arena).array;

  jsondec_arrstart(d);
  while (jsondec_arrnext(d)) {
    upb_msg *value_msg = upb_msg_new(value_m, d->arena);
    upb_msgval value;
    value.msg_val = value_msg;
    upb_array_append(values, value, d->arena);
    jsondec_wellknownvalue(d, value_msg, value_m);
  }
  jsondec_arrend(d);
}

static void jsondec_struct(jsondec *d, upb_msg *msg, const upb_msgdef *m) {
  const upb_fielddef *fields_f = upb_msgdef_itof(m, 1);
  const upb_msgdef *entry_m = upb_fielddef_msgsubdef(fields_f);
  const upb_fielddef *value_f = upb_msgdef_itof(entry_m, 2);
  const upb_msgdef *value_m = upb_fielddef_msgsubdef(value_f);
  upb_map *fields = upb_msg_mutable(msg, fields_f, d->arena).map;

  jsondec_objstart(d);
  while (jsondec_objnext(d)) {
    upb_msgval key, value;
    upb_msg *value_msg = upb_msg_new(value_m, d->arena);
    key.str_val = jsondec_string(d);
    value.msg_val = value_msg;
    upb_map_set(fields, key, value, d->arena);
    jsondec_entrysep(d);
    jsondec_wellknownvalue(d, value_msg, value_m);
  }
  jsondec_objend(d);
}

static void jsondec_wellknownvalue(jsondec *d, upb_msg *msg,
                                   const upb_msgdef *m) {
  upb_msgval val;
  const upb_fielddef *f;
  upb_msg *submsg;

  switch (jsondec_peek(d)) {
    case JD_NUMBER:
      /* double number_value = 2; */
      f = upb_msgdef_itof(m, 2);
      val.double_val = jsondec_number(d);
      break;
    case JD_STRING:
      /* string string_value = 3; */
      f = upb_msgdef_itof(m, 3);
      val.str_val = jsondec_string(d);
      break;
    case JD_FALSE:
      /* bool bool_value = 4; */
      f = upb_msgdef_itof(m, 4);
      val.bool_val = false;
      jsondec_false(d);
      break;
    case JD_TRUE:
      /* bool bool_value = 4; */
      f = upb_msgdef_itof(m, 4);
      val.bool_val = true;
      jsondec_true(d);
      break;
    case JD_NULL:
      /* NullValue null_value = 1; */
      f = upb_msgdef_itof(m, 1);
      val.int32_val = 0;
      jsondec_null(d);
      break;
    /* Note: these cases return, because upb_msg_mutable() is enough. */
    case JD_OBJECT:
      /* Struct struct_value = 5; */
      f = upb_msgdef_itof(m, 5);
      submsg = upb_msg_mutable(msg, f, d->arena).msg;
      jsondec_struct(d, submsg, upb_fielddef_msgsubdef(f));
      return;
    case JD_ARRAY:
      /* ListValue list_value = 6; */
      f = upb_msgdef_itof(m, 6);
      submsg = upb_msg_mutable(msg, f, d->arena).msg;
      jsondec_listvalue(d, submsg, upb_fielddef_msgsubdef(f));
      return;
    default:
      UPB_UNREACHABLE();
  }

  upb_msg_set(msg, f, val, d->arena);
}

static upb_strview jsondec_mask(jsondec *d, const char *buf, const char *end) {
  /* FieldMask fields grow due to inserted '_' characters, so we can't do the
   * transform in place. */
  const char *ptr = buf;
  upb_strview ret;
  char *out;

  ret.size = end - ptr;
  while (ptr < end) {
    ret.size += (*ptr >= 'A' && *ptr <= 'Z');
    ptr++;
  }

  out = upb_arena_malloc(d->arena, ret.size);
  ptr = buf;
  ret.data = out;

  while (ptr < end) {
    char ch = *ptr++;
    if (ch >= 'A' && ch <= 'Z') {
      *out++ = '_';
      *out++ = ch + 32;
    } else if (ch == '_') {
      jsondec_err(d, "field mask may not contain '_'");
    } else {
      *out++ = ch;
    }
  }

  return ret;
}

static void jsondec_fieldmask(jsondec *d, upb_msg *msg, const upb_msgdef *m) {
  /* repeated string paths = 1; */
  const upb_fielddef *paths_f = upb_msgdef_itof(m, 1);
  upb_array *arr = upb_msg_mutable(msg, paths_f, d->arena).array;
  upb_strview str = jsondec_string(d);
  const char *ptr = str.data;
  const char *end = ptr + str.size;
  upb_msgval val;

  while (ptr < end) {
    const char *elem_end = memchr(ptr, ',', end - ptr);
    if (elem_end) {
      val.str_val = jsondec_mask(d, ptr, elem_end);
      ptr = elem_end + 1;
    } else {
      val.str_val = jsondec_mask(d, ptr, end);
      ptr = end;
    }
    upb_array_append(arr, val, d->arena);
  }
}

static void jsondec_anyfield(jsondec *d, upb_msg *msg, const upb_msgdef *m) {
  if (upb_msgdef_wellknowntype(m) == UPB_WELLKNOWN_UNSPECIFIED) {
    /* For regular types: {"@type": "[user type]", "f1": <V1>, "f2": <V2>}
     * where f1, f2, etc. are the normal fields of this type. */
    jsondec_field(d, msg, m);
  } else {
    /* For well-known types: {"@type": "[well-known type]", "value": <X>}
     * where <X> is whatever encoding the WKT normally uses. */
    upb_strview str = jsondec_string(d);
    jsondec_entrysep(d);
    if (!jsondec_streql(str, "value")) {
      jsondec_err(d, "Key for well-known type must be 'value'");
    }
    jsondec_wellknown(d, msg, m);
  }
}

static const upb_msgdef *jsondec_typeurl(jsondec *d, upb_msg *msg,
                                         const upb_msgdef *m) {
  const upb_fielddef *type_url_f = upb_msgdef_itof(m, 1);
  const upb_msgdef *type_m;
  upb_strview type_url = jsondec_string(d);
  const char *end = type_url.data + type_url.size;
  const char *ptr = end;
  upb_msgval val;

  val.str_val = type_url;
  upb_msg_set(msg, type_url_f, val, d->arena);

  /* Find message name after the last '/' */
  while (ptr > type_url.data && *--ptr != '/') {}

  if (ptr == type_url.data || ptr == end) {
    jsondec_err(d, "Type url must have at least one '/' and non-empty host");
  }

  ptr++;
  type_m = upb_symtab_lookupmsg2(d->any_pool, ptr, end - ptr);

  if (!type_m) {
    jsondec_err(d, "Type was not found");
  }

  return type_m;
}

static void jsondec_any(jsondec *d, upb_msg *msg, const upb_msgdef *m) {
  /* string type_url = 1;
   * bytes value = 2; */
  const upb_fielddef *value_f = upb_msgdef_itof(m, 2);
  upb_msg *any_msg;
  const upb_msgdef *any_m = NULL;
  const char *pre_type_data = NULL;
  const char *pre_type_end = NULL;
  upb_msgval encoded;

  jsondec_objstart(d);

  /* Scan looking for "@type", which is not necessarily first. */
  while (!any_m && jsondec_objnext(d)) {
    const char *start = d->ptr;
    upb_strview name = jsondec_string(d);
    jsondec_entrysep(d);
    if (jsondec_streql(name, "@type")) {
      any_m = jsondec_typeurl(d, msg, m);
      if (pre_type_data) {
        pre_type_end = start;
        while (*pre_type_end != ',') pre_type_end--;
      }
    } else {
      if (!pre_type_data) pre_type_data = start;
      jsondec_skipval(d);
    }
  }

  if (!any_m) {
    jsondec_err(d, "Any object didn't contain a '@type' field");
  }

  any_msg = upb_msg_new(any_m, d->arena);

  if (pre_type_data) {
    size_t len = pre_type_end - pre_type_data + 1;
    char *tmp = upb_arena_malloc(d->arena, len);
    memcpy(tmp, pre_type_data, len - 1);
    tmp[len - 1] = '}';
    const char *saved_ptr = d->ptr;
    const char *saved_end = d->end;
    d->ptr = tmp;
    d->end = tmp + len;
    d->is_first = true;
    while (jsondec_objnext(d)) {
      jsondec_anyfield(d, any_msg, any_m);
    }
    d->ptr = saved_ptr;
    d->end = saved_end;
  }

  while (jsondec_objnext(d)) {
    jsondec_anyfield(d, any_msg, any_m);
  }

  jsondec_objend(d);

  encoded.str_val.data = upb_encode(any_msg, upb_msgdef_layout(any_m), d->arena,
                                    &encoded.str_val.size);
  upb_msg_set(msg, value_f, encoded, d->arena);
}

static void jsondec_wrapper(jsondec *d, upb_msg *msg, const upb_msgdef *m) {
  const upb_fielddef *value_f = upb_msgdef_itof(m, 1);
  upb_msgval val = jsondec_value(d, value_f);
  upb_msg_set(msg, value_f, val, d->arena);
}

static void jsondec_wellknown(jsondec *d, upb_msg *msg, const upb_msgdef *m) {
  switch (upb_msgdef_wellknowntype(m)) {
    case UPB_WELLKNOWN_ANY:
      jsondec_any(d, msg, m);
      break;
    case UPB_WELLKNOWN_FIELDMASK:
      jsondec_fieldmask(d, msg, m);
      break;
    case UPB_WELLKNOWN_DURATION:
      jsondec_duration(d, msg, m);
      break;
    case UPB_WELLKNOWN_TIMESTAMP:
      jsondec_timestamp(d, msg, m);
      break;
    case UPB_WELLKNOWN_VALUE:
      jsondec_wellknownvalue(d, msg, m);
      break;
    case UPB_WELLKNOWN_LISTVALUE:
      jsondec_listvalue(d, msg, m);
      break;
    case UPB_WELLKNOWN_STRUCT:
      jsondec_struct(d, msg, m);
      break;
    case UPB_WELLKNOWN_DOUBLEVALUE:
    case UPB_WELLKNOWN_FLOATVALUE:
    case UPB_WELLKNOWN_INT64VALUE:
    case UPB_WELLKNOWN_UINT64VALUE:
    case UPB_WELLKNOWN_INT32VALUE:
    case UPB_WELLKNOWN_UINT32VALUE:
    case UPB_WELLKNOWN_STRINGVALUE:
    case UPB_WELLKNOWN_BYTESVALUE:
    case UPB_WELLKNOWN_BOOLVALUE:
      jsondec_wrapper(d, msg, m);
      break;
    default:
      UPB_UNREACHABLE();
  }
}

bool upb_json_decode(const char *buf, size_t size, upb_msg *msg,
                     const upb_msgdef *m, const upb_symtab *any_pool,
                     int options, upb_arena *arena, upb_status *status) {
  jsondec d;
  d.ptr = buf;
  d.end = buf + size;
  d.arena = arena;
  d.any_pool = any_pool;
  d.status = status;
  d.options = options;
  d.depth = 64;
  d.line = 1;
  d.debug_field = NULL;
  d.is_first = false;

  if (setjmp(d.err)) return false;

  jsondec_object(&d, msg, m);
  return true;
}


#include <ctype.h>
#include <float.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>



typedef struct {
  char *buf, *ptr, *end;
  size_t overflow;
  int indent_depth;
  int options;
  const upb_symtab *ext_pool;
  jmp_buf err;
  upb_status *status;
  upb_arena *arena;
} jsonenc;

static void jsonenc_msg(jsonenc *e, const upb_msg *msg, const upb_msgdef *m);
static void jsonenc_scalar(jsonenc *e, upb_msgval val, const upb_fielddef *f);
static void jsonenc_msgfield(jsonenc *e, const upb_msg *msg,
                             const upb_msgdef *m);
static void jsonenc_msgfields(jsonenc *e, const upb_msg *msg,
                              const upb_msgdef *m);
static void jsonenc_value(jsonenc *e, const upb_msg *msg, const upb_msgdef *m);

UPB_NORETURN static void jsonenc_err(jsonenc *e, const char *msg) {
  upb_status_seterrmsg(e->status, msg);
  longjmp(e->err, 1);
}

static upb_arena *jsonenc_arena(jsonenc *e) {
  /* Create lazily, since it's only needed for Any */
  if (!e->arena) {
    e->arena = upb_arena_new();
  }
  return e->arena;
}

static void jsonenc_putbytes(jsonenc *e, const void *data, size_t len) {
  size_t have = e->end - e->ptr;
  if (UPB_LIKELY(have >= len)) {
    memcpy(e->ptr, data, len);
    e->ptr += len;
  } else {
    memcpy(e->ptr, data, have);
    e->ptr += have;
    e->overflow += (len - have);
  }
}

static void jsonenc_putstr(jsonenc *e, const char *str) {
  jsonenc_putbytes(e, str, strlen(str));
}

static void jsonenc_printf(jsonenc *e, const char *fmt, ...) {
  size_t n;
  size_t have = e->end - e->ptr;
  va_list args;

  va_start(args, fmt);
  n = _upb_vsnprintf(e->ptr, have, fmt, args);
  va_end(args);

  if (UPB_LIKELY(have > n)) {
    e->ptr += n;
  } else {
    e->ptr += have;
    e->overflow += (n - have);
  }
}

static void jsonenc_nanos(jsonenc *e, int32_t nanos) {
  int digits = 9;

  if (nanos == 0) return;
  if (nanos < 0 || nanos >= 1000000000) {
    jsonenc_err(e, "error formatting timestamp as JSON: invalid nanos");
  }

  while (nanos % 1000 == 0) {
    nanos /= 1000;
    digits -= 3;
  }

  jsonenc_printf(e, ".%0.*" PRId32, digits, nanos);
}

static void jsonenc_timestamp(jsonenc *e, const upb_msg *msg,
                              const upb_msgdef *m) {
  const upb_fielddef *seconds_f = upb_msgdef_itof(m, 1);
  const upb_fielddef *nanos_f = upb_msgdef_itof(m, 2);
  int64_t seconds = upb_msg_get(msg, seconds_f).int64_val;
  int32_t nanos = upb_msg_get(msg, nanos_f).int32_val;
  int L, N, I, J, K, hour, min, sec;

  if (seconds < -62135596800) {
    jsonenc_err(e,
                "error formatting timestamp as JSON: minimum acceptable value "
                "is 0001-01-01T00:00:00Z");
  } else if (seconds > 253402300799) {
    jsonenc_err(e,
                "error formatting timestamp as JSON: maximum acceptable value "
                "is 9999-12-31T23:59:59Z");
  }

  /* Julian Day -> Y/M/D, Algorithm from:
   * Fliegel, H. F., and Van Flandern, T. C., "A Machine Algorithm for
   *   Processing Calendar Dates," Communications of the Association of
   *   Computing Machines, vol. 11 (1968), p. 657.  */
  L = (seconds / 86400) + 68569 + 2440588;
  N = 4 * L / 146097;
  L = L - (146097 * N + 3) / 4;
  I = 4000 * (L + 1) / 1461001;
  L = L - 1461 * I / 4 + 31;
  J = 80 * L / 2447;
  K = L - 2447 * J / 80;
  L = J / 11;
  J = J + 2 - 12 * L;
  I = 100 * (N - 49) + I + L;

  sec = seconds % 60;
  min = (seconds / 60) % 60;
  hour = (seconds / 3600) % 24;

  jsonenc_printf(e, "\"%04d-%02d-%02dT%02d:%02d:%02d", I, J, K, hour, min, sec);
  jsonenc_nanos(e, nanos);
  jsonenc_putstr(e, "Z\"");
}

static void jsonenc_duration(jsonenc *e, const upb_msg *msg, const upb_msgdef *m) {
  const upb_fielddef *seconds_f = upb_msgdef_itof(m, 1);
  const upb_fielddef *nanos_f = upb_msgdef_itof(m, 2);
  int64_t seconds = upb_msg_get(msg, seconds_f).int64_val;
  int32_t nanos = upb_msg_get(msg, nanos_f).int32_val;

  if (seconds > 315576000000 || seconds < -315576000000 ||
      (seconds < 0) != (nanos < 0)) {
    jsonenc_err(e, "bad duration");
  }

  if (nanos < 0) {
    nanos = -nanos;
  }

  jsonenc_printf(e, "\"%" PRId64, seconds);
  jsonenc_nanos(e, nanos);
  jsonenc_putstr(e, "s\"");
}

static void jsonenc_enum(int32_t val, const upb_fielddef *f, jsonenc *e) {
  const upb_enumdef *e_def = upb_fielddef_enumsubdef(f);
  const char *name = upb_enumdef_iton(e_def, val);

  if (name) {
    jsonenc_printf(e, "\"%s\"", name);
  } else {
    jsonenc_printf(e, "%" PRId32, val);
  }
}

static void jsonenc_bytes(jsonenc *e, upb_strview str) {
  /* This is the regular base64, not the "web-safe" version. */
  static const char base64[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  const unsigned char *ptr = (unsigned char*)str.data;
  const unsigned char *end = ptr + str.size;
  char buf[4];

  jsonenc_putstr(e, "\"");

  while (end - ptr >= 3) {
    buf[0] = base64[ptr[0] >> 2];
    buf[1] = base64[((ptr[0] & 0x3) << 4) | (ptr[1] >> 4)];
    buf[2] = base64[((ptr[1] & 0xf) << 2) | (ptr[2] >> 6)];
    buf[3] = base64[ptr[2] & 0x3f];
    jsonenc_putbytes(e, buf, 4);
    ptr += 3;
  }

  switch (end - ptr) {
    case 2:
      buf[0] = base64[ptr[0] >> 2];
      buf[1] = base64[((ptr[0] & 0x3) << 4) | (ptr[1] >> 4)];
      buf[2] = base64[(ptr[1] & 0xf) << 2];
      buf[3] = '=';
      jsonenc_putbytes(e, buf, 4);
      break;
    case 1:
      buf[0] = base64[ptr[0] >> 2];
      buf[1] = base64[((ptr[0] & 0x3) << 4)];
      buf[2] = '=';
      buf[3] = '=';
      jsonenc_putbytes(e, buf, 4);
      break;
  }

  jsonenc_putstr(e, "\"");
}

static void jsonenc_stringbody(jsonenc *e, upb_strview str) {
  const char *ptr = str.data;
  const char *end = ptr + str.size;

  while (ptr < end) {
    switch (*ptr) {
      case '\n':
        jsonenc_putstr(e, "\\n");
        break;
      case '\r':
        jsonenc_putstr(e, "\\r");
        break;
      case '\t':
        jsonenc_putstr(e, "\\t");
        break;
      case '\"':
        jsonenc_putstr(e, "\\\"");
        break;
      case '\f':
        jsonenc_putstr(e, "\\f");
        break;
      case '\b':
        jsonenc_putstr(e, "\\b");
        break;
      case '\\':
        jsonenc_putstr(e, "\\\\");
        break;
      default:
        if ((uint8_t)*ptr < 0x20) {
          jsonenc_printf(e, "\\u%04x", (int)(uint8_t)*ptr);
        } else {
          /* This could be a non-ASCII byte.  We rely on the string being valid
           * UTF-8. */
          jsonenc_putbytes(e, ptr, 1);
        }
        break;
    }
    ptr++;
  }
}

static void jsonenc_string(jsonenc *e, upb_strview str) {
  jsonenc_putstr(e, "\"");
  jsonenc_stringbody(e, str);
  jsonenc_putstr(e, "\"");
}

static void jsonenc_double(jsonenc *e, const char *fmt, double val) {
  if (val == UPB_INFINITY) {
    jsonenc_putstr(e, "\"Infinity\"");
  } else if (val == -UPB_INFINITY) {
    jsonenc_putstr(e, "\"-Infinity\"");
  } else if (val != val) {
    jsonenc_putstr(e, "\"NaN\"");
  } else {
    jsonenc_printf(e, fmt, val);
  }
}

static void jsonenc_wrapper(jsonenc *e, const upb_msg *msg,
                            const upb_msgdef *m) {
  const upb_fielddef *val_f = upb_msgdef_itof(m, 1);
  upb_msgval val = upb_msg_get(msg, val_f);
  jsonenc_scalar(e, val, val_f);
}

static const upb_msgdef *jsonenc_getanymsg(jsonenc *e, upb_strview type_url) {
  /* Find last '/', if any. */
  const char *end = type_url.data + type_url.size;
  const char *ptr = end;
  const upb_msgdef *ret;

  if (!e->ext_pool || type_url.size == 0) goto badurl;

  while (true) {
    if (--ptr == type_url.data) {
      /* Type URL must contain at least one '/', with host before. */
      goto badurl;
    }
    if (*ptr == '/') {
      ptr++;
      break;
    }
  }

  ret = upb_symtab_lookupmsg2(e->ext_pool, ptr, end - ptr);

  if (!ret) {
    jsonenc_err(e, "Couldn't find Any type");
  }

  return ret;

badurl:
  jsonenc_err(e, "Bad type URL");
}

static void jsonenc_any(jsonenc *e, const upb_msg *msg, const upb_msgdef *m) {
  const upb_fielddef *type_url_f = upb_msgdef_itof(m, 1);
  const upb_fielddef *value_f = upb_msgdef_itof(m, 2);
  upb_strview type_url = upb_msg_get(msg, type_url_f).str_val;
  upb_strview value = upb_msg_get(msg, value_f).str_val;
  const upb_msgdef *any_m = jsonenc_getanymsg(e, type_url);
  const upb_msglayout *any_layout = upb_msgdef_layout(any_m);
  upb_arena *arena = jsonenc_arena(e);
  upb_msg *any = upb_msg_new(any_m, arena);

  if (!upb_decode(value.data, value.size, any, any_layout, arena)) {
    jsonenc_err(e, "Error decoding message in Any");
  }

  jsonenc_putstr(e, "{\"@type\": ");
  jsonenc_string(e, type_url);
  jsonenc_putstr(e, ", ");

  if (upb_msgdef_wellknowntype(any_m) == UPB_WELLKNOWN_UNSPECIFIED) {
    /* Regular messages: {"@type": "...", "foo": 1, "bar": 2} */
    jsonenc_msgfields(e, any, any_m);
  } else {
    /* Well-known type: {"@type": "...", "value": <well-known encoding>} */
    jsonenc_putstr(e, "value: ");
    jsonenc_msgfield(e, any, any_m);
  }

  jsonenc_putstr(e, "}");
}

static void jsonenc_putsep(jsonenc *e, const char *str, bool *first) {
  if (*first) {
    *first = false;
  } else {
    jsonenc_putstr(e, str);
  }
}

static void jsonenc_fieldpath(jsonenc *e, upb_strview path) {
  const char *ptr = path.data;
  const char *end = ptr + path.size;

  while (ptr < end) {
    char ch = *ptr;

    if (ch >= 'A' && ch <= 'Z') {
      jsonenc_err(e, "Field mask element may not have upper-case letter.");
    } else if (ch == '_') {
      if (ptr == end - 1 || *(ptr + 1) < 'a' || *(ptr + 1) > 'z') {
        jsonenc_err(e, "Underscore must be followed by a lowercase letter.");
      }
      ch = *++ptr - 32;
    }

    jsonenc_putbytes(e, &ch, 1);
    ptr++;
  }
}

static void jsonenc_fieldmask(jsonenc *e, const upb_msg *msg,
                              const upb_msgdef *m) {
  const upb_fielddef *paths_f = upb_msgdef_itof(m, 1);
  const upb_array *paths = upb_msg_get(msg, paths_f).array_val;
  bool first = true;
  size_t i, n = 0;

  if (paths) n = upb_array_size(paths);

  jsonenc_putstr(e, "\"");

  for (i = 0; i < n; i++) {
    jsonenc_putsep(e, ",", &first);
    jsonenc_fieldpath(e, upb_array_get(paths, i).str_val);
  }

  jsonenc_putstr(e, "\"");
}

static void jsonenc_struct(jsonenc *e, const upb_msg *msg,
                           const upb_msgdef *m) {
  const upb_fielddef *fields_f = upb_msgdef_itof(m, 1);
  const upb_map *fields = upb_msg_get(msg, fields_f).map_val;
  const upb_msgdef *entry_m = upb_fielddef_msgsubdef(fields_f);
  const upb_fielddef *value_f = upb_msgdef_itof(entry_m, 2);
  size_t iter = UPB_MAP_BEGIN;
  bool first = true;

  jsonenc_putstr(e, "{");

  while (upb_mapiter_next(fields, &iter)) {
    upb_msgval key = upb_mapiter_key(fields, iter);
    upb_msgval val = upb_mapiter_value(fields, iter);

    jsonenc_putsep(e, ", ", &first);
    jsonenc_string(e, key.str_val);
    jsonenc_putstr(e, ": ");
    jsonenc_value(e, val.msg_val, upb_fielddef_msgsubdef(value_f));
  }

  jsonenc_putstr(e, "}");
}

static void jsonenc_listvalue(jsonenc *e, const upb_msg *msg,
                              const upb_msgdef *m) {
  const upb_fielddef *values_f = upb_msgdef_itof(m, 1);
  const upb_msgdef *values_m = upb_fielddef_msgsubdef(values_f);
  const upb_array *values = upb_msg_get(msg, values_f).array_val;
  const size_t size = upb_array_size(values);
  size_t i;
  bool first = true;

  jsonenc_putstr(e, "[");

  for (i = 0; i < size; i++) {
    upb_msgval elem = upb_array_get(values, i);

    jsonenc_putsep(e, ", ", &first);
    jsonenc_value(e, elem.msg_val, values_m);
  }

  jsonenc_putstr(e, "]");
}

static void jsonenc_value(jsonenc *e, const upb_msg *msg, const upb_msgdef *m) {
  /* TODO(haberman): do we want a reflection method to get oneof case? */
  size_t iter = UPB_MSG_BEGIN;
  const upb_fielddef *f;
  upb_msgval val;

  if (!upb_msg_next(msg, m, NULL,  &f, &val, &iter)) {
    jsonenc_err(e, "No value set in Value proto");
  }

  switch (upb_fielddef_number(f)) {
    case 1:
      jsonenc_putstr(e, "null");
      break;
    case 2:
      jsonenc_double(e, "%.17g", val.double_val);
      break;
    case 3:
      jsonenc_string(e, val.str_val);
      break;
    case 4:
      jsonenc_putstr(e, val.bool_val ? "true" : "false");
      break;
    case 5:
      jsonenc_struct(e, val.msg_val, upb_fielddef_msgsubdef(f));
      break;
    case 6:
      jsonenc_listvalue(e, val.msg_val, upb_fielddef_msgsubdef(f));
      break;
  }
}

static void jsonenc_msgfield(jsonenc *e, const upb_msg *msg,
                             const upb_msgdef *m) {
  switch (upb_msgdef_wellknowntype(m)) {
    case UPB_WELLKNOWN_UNSPECIFIED:
      jsonenc_msg(e, msg, m);
      break;
    case UPB_WELLKNOWN_ANY:
      jsonenc_any(e, msg, m);
      break;
    case UPB_WELLKNOWN_FIELDMASK:
      jsonenc_fieldmask(e, msg, m);
      break;
    case UPB_WELLKNOWN_DURATION:
      jsonenc_duration(e, msg, m);
      break;
    case UPB_WELLKNOWN_TIMESTAMP:
      jsonenc_timestamp(e, msg, m);
      break;
    case UPB_WELLKNOWN_DOUBLEVALUE:
    case UPB_WELLKNOWN_FLOATVALUE:
    case UPB_WELLKNOWN_INT64VALUE:
    case UPB_WELLKNOWN_UINT64VALUE:
    case UPB_WELLKNOWN_INT32VALUE:
    case UPB_WELLKNOWN_UINT32VALUE:
    case UPB_WELLKNOWN_STRINGVALUE:
    case UPB_WELLKNOWN_BYTESVALUE:
    case UPB_WELLKNOWN_BOOLVALUE:
      jsonenc_wrapper(e, msg, m);
      break;
    case UPB_WELLKNOWN_VALUE:
      jsonenc_value(e, msg, m);
      break;
    case UPB_WELLKNOWN_LISTVALUE:
      jsonenc_listvalue(e, msg, m);
      break;
    case UPB_WELLKNOWN_STRUCT:
      jsonenc_struct(e, msg, m);
      break;
  }
}

static void jsonenc_scalar(jsonenc *e, upb_msgval val, const upb_fielddef *f) {
  switch (upb_fielddef_type(f)) {
    case UPB_TYPE_BOOL:
      jsonenc_putstr(e, val.bool_val ? "true" : "false");
      break;
    case UPB_TYPE_FLOAT:
      jsonenc_double(e, "%.9g", val.float_val);
      break;
    case UPB_TYPE_DOUBLE:
      jsonenc_double(e, "%.17g", val.double_val);
      break;
    case UPB_TYPE_INT32:
      jsonenc_printf(e, "%" PRId32, val.int32_val);
      break;
    case UPB_TYPE_UINT32:
      jsonenc_printf(e, "%" PRIu32, val.uint32_val);
      break;
    case UPB_TYPE_INT64:
      jsonenc_printf(e, "\"%" PRId64 "\"", val.int64_val);
      break;
    case UPB_TYPE_UINT64:
      jsonenc_printf(e, "\"%" PRIu64 "\"", val.uint64_val);
      break;
    case UPB_TYPE_STRING:
      jsonenc_string(e, val.str_val);
      break;
    case UPB_TYPE_BYTES:
      jsonenc_bytes(e, val.str_val);
      break;
    case UPB_TYPE_ENUM:
      jsonenc_enum(val.int32_val, f, e);
      break;
    case UPB_TYPE_MESSAGE:
      jsonenc_msgfield(e, val.msg_val, upb_fielddef_msgsubdef(f));
      break;
  }
}

static void jsonenc_mapkey(jsonenc *e, upb_msgval val, const upb_fielddef *f) {
  jsonenc_putstr(e, "\"");

  switch (upb_fielddef_type(f)) {
    case UPB_TYPE_BOOL:
      jsonenc_putstr(e, val.bool_val ? "true" : "false");
      break;
    case UPB_TYPE_INT32:
      jsonenc_printf(e, "%" PRId32, val.int32_val);
      break;
    case UPB_TYPE_UINT32:
      jsonenc_printf(e, "%" PRIu32, val.uint32_val);
      break;
    case UPB_TYPE_INT64:
      jsonenc_printf(e, "%" PRId64, val.int64_val);
      break;
    case UPB_TYPE_UINT64:
      jsonenc_printf(e, "%" PRIu64, val.uint64_val);
      break;
    case UPB_TYPE_STRING:
      jsonenc_stringbody(e, val.str_val);
      break;
    default:
      UPB_UNREACHABLE();
  }

  jsonenc_putstr(e, "\": ");
}

static void jsonenc_array(jsonenc *e, const upb_array *arr,
                         const upb_fielddef *f) {
  size_t i;
  size_t size = upb_array_size(arr);
  bool first = true;

  jsonenc_putstr(e, "[");

  for (i = 0; i < size; i++) {
    jsonenc_putsep(e, ", ", &first);
    jsonenc_scalar(e, upb_array_get(arr, i), f);
  }

  jsonenc_putstr(e, "]");
}

static void jsonenc_map(jsonenc *e, const upb_map *map, const upb_fielddef *f) {
  const upb_msgdef *entry = upb_fielddef_msgsubdef(f);
  const upb_fielddef *key_f = upb_msgdef_itof(entry, 1);
  const upb_fielddef *val_f = upb_msgdef_itof(entry, 2);
  size_t iter = UPB_MAP_BEGIN;
  bool first = true;

  jsonenc_putstr(e, "{");

  while (upb_mapiter_next(map, &iter)) {
    jsonenc_putsep(e, ", ", &first);
    jsonenc_mapkey(e, upb_mapiter_key(map, iter), key_f);
    jsonenc_scalar(e, upb_mapiter_value(map, iter), val_f);
  }

  jsonenc_putstr(e, "}");
}

static void jsonenc_fieldval(jsonenc *e, const upb_fielddef *f,
                             upb_msgval val, bool *first) {
  const char *name;

  if (e->options & UPB_JSONENC_PROTONAMES) {
    name = upb_fielddef_name(f);
  } else {
    name = upb_fielddef_jsonname(f);
  }

  jsonenc_putsep(e, ", ", first);
  jsonenc_printf(e, "\"%s\": ", name);

  if (upb_fielddef_ismap(f)) {
    jsonenc_map(e, val.map_val, f);
  } else if (upb_fielddef_isseq(f)) {
    jsonenc_array(e, val.array_val, f);
  } else {
    jsonenc_scalar(e, val, f);
  }
}

static void jsonenc_msgfields(jsonenc *e, const upb_msg *msg,
                              const upb_msgdef *m) {
  upb_msgval val;
  const upb_fielddef *f;
  bool first = true;

  if (e->options & UPB_JSONENC_EMITDEFAULTS) {
    /* Iterate over all fields. */
    upb_msg_field_iter i;
    for (upb_msg_field_begin(&i, m); !upb_msg_field_done(&i);
         upb_msg_field_next(&i)) {
      f = upb_msg_iter_field(&i);
      jsonenc_fieldval(e, f, upb_msg_get(msg, f), &first);
    }
  } else {
    /* Iterate over non-empty fields. */
    size_t iter = UPB_MSG_BEGIN;
    while (upb_msg_next(msg, m, e->ext_pool, &f, &val, &iter)) {
      jsonenc_fieldval(e, f, val, &first);
    }
  }
}

static void jsonenc_msg(jsonenc *e, const upb_msg *msg, const upb_msgdef *m) {
  jsonenc_putstr(e, "{");
  jsonenc_msgfields(e, msg, m);
  jsonenc_putstr(e, "}");
}

static size_t jsonenc_nullz(jsonenc *e, size_t size) {
  size_t ret = e->ptr - e->buf + e->overflow;

  if (size > 0) {
    if (e->ptr == e->end) e->ptr--;
    *e->ptr = '\0';
  }

  return ret;
}

size_t upb_json_encode(const upb_msg *msg, const upb_msgdef *m,
                       const upb_symtab *ext_pool, int options, char *buf,
                       size_t size, upb_status *status) {
  jsonenc e;

  e.buf = buf;
  e.ptr = buf;
  e.end = buf + size;
  e.overflow = 0;
  e.options = options;
  e.ext_pool = ext_pool;
  e.status = status;
  e.arena = NULL;

  if (setjmp(e.err)) return -1;

  jsonenc_msg(&e, msg, m);
  if (e.arena) upb_arena_free(e.arena);
  return jsonenc_nullz(&e, size);
}
/* See port_def.inc.  This should #undef all macros #defined there. */

#undef UPB_MAPTYPE_STRING
#undef UPB_SIZE
#undef UPB_FIELD_AT
#undef UPB_READ_ONEOF
#undef UPB_WRITE_ONEOF
#undef UPB_INLINE
#undef UPB_FORCEINLINE
#undef UPB_NOINLINE
#undef UPB_NORETURN
#undef UPB_MAX
#undef UPB_MIN
#undef UPB_UNUSED
#undef UPB_ASSUME
#undef UPB_ASSERT
#undef UPB_ASSERT_DEBUGVAR
#undef UPB_UNREACHABLE
#undef UPB_INFINITY
#undef UPB_MSVC_VSNPRINTF
#undef _upb_snprintf
#undef _upb_vsnprintf
#undef _upb_va_copy

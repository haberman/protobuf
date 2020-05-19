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

#include "convert.h"

#include "message.h"
#include "php-upb.h"
#include "protobuf.h"

// -----------------------------------------------------------------------------
// GPBUtil
// -----------------------------------------------------------------------------

static zend_class_entry* gpb_util_ce;

// The implementation of type checking for primitive fields is empty. This is
// because type checking is done when direct assigning message fields (e.g.,
// foo->a = 1). Functions defined here are place holders in generated code for
// pure PHP implementation (c extension and pure PHP share the same generated
// code).

PHP_METHOD(Util, checkInt32) {}
PHP_METHOD(Util, checkUint32) {}
PHP_METHOD(Util, checkInt64) {}
PHP_METHOD(Util, checkUint64) {}
PHP_METHOD(Util, checkEnum) {}
PHP_METHOD(Util, checkFloat) {}
PHP_METHOD(Util, checkDouble) {}
PHP_METHOD(Util, checkBool) {}
PHP_METHOD(Util, checkString) {}
PHP_METHOD(Util, checkBytes) {}
PHP_METHOD(Util, checkMessage) {}

PHP_METHOD(Util, checkMapField) {
  zval *val, *key_type, *val_type, *klass;
  if (zend_parse_parameters(ZEND_NUM_ARGS(), "zzz|z", &val, &key_type,
                            &val_type, &klass) == FAILURE) {
    return;
  }
  RETURN_ZVAL(val, 1, 0);
}

PHP_METHOD(Util, checkRepeatedField) {
  zval *val, *type, *klass;
  if (zend_parse_parameters(ZEND_NUM_ARGS(), "zz|z", &val, &type, &klass) ==
      FAILURE) {
    return;
  }
  RETURN_ZVAL(val, 1, 0);
}

static zend_function_entry util_methods[] = {
  PHP_ME(Util, checkInt32,  NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(Util, checkUint32, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(Util, checkInt64,  NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(Util, checkUint64, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(Util, checkEnum,   NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(Util, checkFloat,  NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(Util, checkDouble, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(Util, checkBool,   NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(Util, checkString, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(Util, checkBytes,  NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(Util, checkMessage, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(Util, checkMapField, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(Util, checkRepeatedField, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  ZEND_FE_END
};

void util_init() {
  zend_class_entry class_type;
  INIT_CLASS_ENTRY(class_type, "Google\\Protobuf\\Internal\\GPBUtil",
                   util_methods);
  gpb_util_ce = zend_register_internal_class(&class_type);
}

// -----------------------------------------------------------------------------
// Conversion functions used from C
// -----------------------------------------------------------------------------

upb_fieldtype_t pbphp_dtype_to_type(upb_descriptortype_t type) {
  switch (type) {
#define CASE(descriptor_type, type)           \
  case UPB_DESCRIPTOR_TYPE_##descriptor_type: \
    return UPB_TYPE_##type;

  CASE(FLOAT,    FLOAT);
  CASE(DOUBLE,   DOUBLE);
  CASE(BOOL,     BOOL);
  CASE(STRING,   STRING);
  CASE(BYTES,    BYTES);
  CASE(MESSAGE,  MESSAGE);
  CASE(GROUP,    MESSAGE);
  CASE(ENUM,     ENUM);
  CASE(INT32,    INT32);
  CASE(INT64,    INT64);
  CASE(UINT32,   UINT32);
  CASE(UINT64,   UINT64);
  CASE(SINT32,   INT32);
  CASE(SINT64,   INT64);
  CASE(FIXED32,  UINT32);
  CASE(FIXED64,  UINT64);
  CASE(SFIXED32, INT32);
  CASE(SFIXED64, INT64);

#undef CONVERT

  }

  zend_error(E_ERROR, "Unknown field type.");
  return 0;
}

bool pbphp_toi64(zval *php_val, int64_t *i64) {
  convert_scalar_to_number(php_val);

  switch (Z_TYPE_P(php_val)) {
    case IS_LONG:
      *i64 = Z_LVAL_P(php_val);
      return true;
    case IS_DOUBLE: {
      double dbl = Z_DVAL_P(php_val);
      if (dbl > 9223372036854774784.0 || dbl < -9223372036854775808.0) {
        php_error_docref(NULL, E_USER_ERROR, "Out of range");
        return false;
      }
      *i64 = dbl; /* must be guarded, overflow here is UB */
      return true;
    }
    default:
      php_error_docref(NULL, E_USER_ERROR, "Cannot convert to integer");
      return false;
  }
}

static bool to_double(zval *php_val, double *dbl) {
  convert_scalar_to_number(php_val);

  switch (Z_TYPE_P(php_val)) {
    case IS_LONG:
      *dbl = Z_LVAL_P(php_val);
      return true;
    case IS_DOUBLE:
      *dbl = Z_DVAL_P(php_val);
      return true;
    default:
      php_error_docref(NULL, E_USER_ERROR, "Cannot convert to double");
      return false;
  }
}

static bool to_bool(zval* from, bool* to) {
  switch (Z_TYPE_P(from)) {
    case IS_TRUE:
      *to = true;
      return true;
    case IS_FALSE:
      *to = false;
      return true;
    case IS_LONG:
      *to = (Z_LVAL_P(from) != 0);
      return true;
    case IS_DOUBLE:
      *to = (Z_LVAL_P(from) != 0);
      return true;
    case IS_STRING:
      if (Z_STRLEN_P(from) == 0 ||
          (Z_STRLEN_P(from) == 1 && Z_STRVAL_P(from)[0] == '0')) {
        *to = false;
      } else {
        *to = true;
      }
      return true;
    default:
      php_error_docref(NULL, E_USER_ERROR,
                       "Given value cannot be converted to bool.");
      return false;
  }
}

static bool to_string(zval* from) {
  if (Z_ISREF_P(from)) {
    ZVAL_DEREF(from);
  }

  switch (Z_TYPE_P(from)) {
    case IS_STRING:
      return true;
    case IS_TRUE:
    case IS_FALSE:
    case IS_LONG:
    case IS_DOUBLE: {
      zval tmp;
      zend_make_printable_zval(from, &tmp);
      ZVAL_COPY_VALUE(from, &tmp);
      return true;
    }
    default:
      php_error_docref(NULL, E_USER_ERROR,
                       "Given value cannot be converted to string.");
      return false;
  }
}

bool pbphp_tomsgval(zval *php_val, upb_msgval *upb_val, upb_fieldtype_t type,
                    const Descriptor *desc, upb_arena *arena) {
  int64_t i64;

  if (Z_ISREF_P(php_val)) {
    ZVAL_DEREF(php_val);
  }

  switch (type) {
    case UPB_TYPE_INT64:
      return pbphp_toi64(php_val, &upb_val->int64_val);
    case UPB_TYPE_INT32:
    case UPB_TYPE_ENUM:
      if (!pbphp_toi64(php_val, &i64)) {
        return false;
      }
      upb_val->int32_val = i64;
      return true;
    case UPB_TYPE_UINT64:
      if (!pbphp_toi64(php_val, &i64)) {
        return false;
      }
      upb_val->uint64_val = i64;
      return true;
    case UPB_TYPE_UINT32:
      if (!pbphp_toi64(php_val, &i64)) {
        return false;
      }
      upb_val->uint32_val = i64;
      return true;
    case UPB_TYPE_DOUBLE:
      return to_double(php_val, &upb_val->double_val);
    case UPB_TYPE_FLOAT:
      if (!to_double(php_val, &upb_val->double_val)) return false;
      upb_val->float_val = upb_val->double_val;
      return true;
    case UPB_TYPE_BOOL:
      return to_bool(php_val, &upb_val->bool_val);
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES: {
      char *ptr;
      size_t size;

      if (!to_string(php_val)) return false;

      size = Z_STRLEN_P(php_val);

      // If arena is NULL we reference the input zval.
      // The resulting upb_strview will only be value while the zval is alive.
      if (arena) {
        ptr = upb_arena_malloc(arena, size);
        memcpy(ptr, Z_STRVAL_P(php_val), size);
      } else {
        ptr = Z_STRVAL_P(php_val);
      }

      upb_val->str_val = upb_strview_make(ptr, size);
      return true;
    }
    case UPB_TYPE_MESSAGE:
      PBPHP_ASSERT(desc);
      return pbphp_tomsg(php_val, desc, arena, (upb_msg**)&upb_val->msg_val);
  }

  return false;
}

void pbphp_tozval(upb_msgval upb_val, zval *php_val, upb_fieldtype_t type,
                  const Descriptor *desc, zval *arena) {
  switch (type) {
    case UPB_TYPE_INT64:
      ZVAL_LONG(php_val, upb_val.int64_val);
      break;
    case UPB_TYPE_INT32:
    case UPB_TYPE_ENUM:
      ZVAL_LONG(php_val, upb_val.int32_val);
      break;
    case UPB_TYPE_UINT64:
      ZVAL_LONG(php_val, upb_val.uint64_val);
      break;
    case UPB_TYPE_UINT32: {
      // Sign-extend for consistency between 32/64-bit builds.
      zend_long val = (int32_t)upb_val.uint32_val;
      ZVAL_LONG(php_val, val);
      break;
    }
    case UPB_TYPE_DOUBLE:
      ZVAL_DOUBLE(php_val, upb_val.double_val);
      break;
    case UPB_TYPE_FLOAT:
      ZVAL_DOUBLE(php_val, upb_val.float_val);
      break;
    case UPB_TYPE_BOOL:
      ZVAL_BOOL(php_val, upb_val.bool_val);
      break;
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES: {
      upb_strview str = upb_val.str_val;
      ZVAL_NEW_STR(php_val, zend_string_init(str.data, str.size, 0));
      break;
    }
    case UPB_TYPE_MESSAGE:
      PBPHP_ASSERT(desc);
      pbphp_getmsg(php_val, desc, (upb_msg*)upb_val.msg_val, arena);
      break;
  }
}

void convert_module_init(void) {
  util_init();
}

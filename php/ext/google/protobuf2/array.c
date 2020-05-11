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

#include "array.h"

#include <Zend/zend_API.h>
#include <Zend/zend_interfaces.h>

#include <ext/spl/spl_iterators.h>

#include "arena.h"
#include "convert.h"
#include "php-upb.h"

static void RepeatedFieldIter_make(zval *val, zval *repeated_field);

// -----------------------------------------------------------------------------
// RepeatedField
// -----------------------------------------------------------------------------

typedef struct {
  zend_object std;
  zval arena;
  upb_array *array;
  upb_fieldtype_t type;
  const zend_class_entry* klass;  // class entry for value message
} RepeatedField;

zend_class_entry *repeated_field_ce;
static zend_object_handlers repeated_field_object_handlers;

/**
 * Object handler to create a RepeatedField.
 */
static zend_object* repeated_field_create(zend_class_entry *class_type) {
  RepeatedField *intern = emalloc(sizeof(RepeatedField));
  zend_object_std_init(&intern->std, class_type);
  intern->std.handlers = &repeated_field_object_handlers;
  arena_init(&intern->arena);
  // Skip object_properties_init(), we don't allow derived classes.
  return &intern->std;
}

/**
 * Object handler to free a RepeatedField.
 */
static void repeated_field_dtor(zend_object* obj) {
  RepeatedField* intern = (RepeatedField*)obj;
  zval_ptr_dtor(&intern->arena);
  zend_object_std_dtor(&intern->std);
}

/**
 * Object handler to get GC references.
 */
static HashTable *repeated_field_get_gc(zval *object, zval **table, int *n) {
  RepeatedField *intern = (RepeatedField*)Z_OBJ_P(object);
  *table = &intern->arena;
  *n = 1;
  return NULL;
}

/**
 * Constructs an instance of RepeatedField.
 * @param long Type of the stored element.
 * @param string Message/Enum class.
 */
PHP_METHOD(RepeatedField, __construct) {
  RepeatedField *intern = (RepeatedField*)Z_OBJ_P(getThis());
  zend_long type;
  zend_class_entry* klass = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|C", &type, &klass) != SUCCESS) {
    return;
  }

  intern->type = pbphp_dtype_to_type(type);
  intern->klass = klass;
  intern->array = upb_array_new(arena_get(&intern->arena), intern->type);

  if (intern->type == UPB_TYPE_MESSAGE && klass == NULL) {
    php_error_docref(NULL, E_USER_ERROR,
                     "Message/enum type must have concrete class.");
    return;
  }
}

/**
 * Append element to the end of the repeated field.
 * @param object The element to be added.
 */
PHP_METHOD(RepeatedField, append) {
  RepeatedField *intern = (RepeatedField*)Z_OBJ_P(getThis());
  upb_arena *arena = arena_get(&intern->arena);
  zval *php_val;
  upb_msgval msgval;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &php_val) != SUCCESS ||
      !pbphp_tomsgval(php_val, &msgval, intern->type, intern->klass, arena)) {
    return;
  }

  upb_array_append(intern->array, msgval, arena_get(&intern->arena));
}

/**
 * Check whether the element at given index exists.
 * @param long The index to be checked.
 * @return bool True if the element at the given index exists.
 */
PHP_METHOD(RepeatedField, offsetExists) {
  RepeatedField *intern = (RepeatedField*)Z_OBJ_P(getThis());
  zend_long index;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
    return;
  }

  RETURN_BOOL(index >= 0 && index < upb_array_size(intern->array));
}

/**
 * Return the element at the given index.
 * This will also be called for: $ele = $arr[0]
 * @param long The index of the element to be fetched.
 * @return object The stored element at given index.
 * @exception Invalid type for index.
 * @exception Non-existing index.
 */
PHP_METHOD(RepeatedField, offsetGet) {
  RepeatedField *intern = (RepeatedField*)Z_OBJ_P(getThis());
  zend_long index;
  upb_msgval msgval;
  zval ret;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
    return;
  }

  if (index < 0 || index >= upb_array_size(intern->array)) {
    zend_error(E_USER_ERROR, "Element at %ld doesn't exist.\n", index);
    return;
  }

  msgval = upb_array_get(intern->array, index);
  pbphp_tozval(msgval, &ret, intern->type, intern->klass);
  RETURN_ZVAL(&ret, 0, 1);
}

/**
 * Assign the element at the given index.
 * This will also be called for: $arr []= $ele and $arr[0] = ele
 * @param long The index of the element to be assigned.
 * @param object The element to be assigned.
 * @exception Invalid type for index.
 * @exception Non-existing index.
 * @exception Incorrect type of the element.
 */
PHP_METHOD(RepeatedField, offsetSet) {
  RepeatedField *intern = (RepeatedField*)Z_OBJ_P(getThis());
  upb_arena *arena = arena_get(&intern->arena);
  size_t size = upb_array_size(intern->array);
  zval *offset, *val;
  int64_t index;
  upb_msgval msgval;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "zz", &offset, &val) != SUCCESS) {
    return;
  }

  if (Z_TYPE_P(offset) == IS_NULL) {
    index = size;
  } else if (!pbphp_toi64(offset, &index)) {
    return;
  }

  if (!pbphp_tomsgval(val, &msgval, intern->type, intern->klass, arena)) {
    return;
  }

  if (index == size) {
    upb_array_append(intern->array, msgval, arena_get(&intern->arena));
  } else {
    upb_array_set(intern->array, index, msgval);
  }
}

/**
 * Remove the element at the given index.
 * This will also be called for: unset($arr)
 * @param long The index of the element to be removed.
 * @exception Invalid type for index.
 * @exception The element to be removed is not at the end of the RepeatedField.
 */
PHP_METHOD(RepeatedField, offsetUnset) {
  RepeatedField *intern = (RepeatedField*)Z_OBJ_P(getThis());
  zend_long index;
  zend_long size = upb_array_size(intern->array);

  // Only the element at the end of the array can be removed.
  if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) != SUCCESS) {
    return;
  }

  if (size == 0 || index != size - 1) {
    php_error_docref(NULL, E_USER_ERROR, "Cannot remove element at %ld.\n",
                     index);
    return;
  }

  upb_array_resize(intern->array, size - 1, arena_get(&intern->arena));
}

/**
 * Return the number of stored elements.
 * This will also be called for: count($arr)
 * @return long The number of stored elements.
 */
PHP_METHOD(RepeatedField, count) {
  RepeatedField *intern = (RepeatedField*)Z_OBJ_P(getThis());

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  RETURN_LONG(upb_array_size(intern->array));
}

/**
 * Return the beginning iterator.
 * This will also be called for: foreach($arr)
 * @return object Beginning iterator.
 */
PHP_METHOD(RepeatedField, getIterator) {
  RepeatedField *intern = (RepeatedField*)Z_OBJ_P(getThis());
  zval ret;

  RepeatedFieldIter_make(&ret, getThis());
  RETURN_ZVAL(&ret, 0, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_offsetGet, 0, 0, 1)
  ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_offsetSet, 0, 0, 2)
  ZEND_ARG_INFO(0, index)
  ZEND_ARG_INFO(0, newval)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_void, 0)
ZEND_END_ARG_INFO()

static zend_function_entry repeated_field_methods[] = {
  PHP_ME(RepeatedField, __construct,  NULL,              ZEND_ACC_PUBLIC)
  PHP_ME(RepeatedField, append,       NULL,              ZEND_ACC_PUBLIC)
  PHP_ME(RepeatedField, offsetExists, arginfo_offsetGet, ZEND_ACC_PUBLIC)
  PHP_ME(RepeatedField, offsetGet,    arginfo_offsetGet, ZEND_ACC_PUBLIC)
  PHP_ME(RepeatedField, offsetSet,    arginfo_offsetSet, ZEND_ACC_PUBLIC)
  PHP_ME(RepeatedField, offsetUnset,  arginfo_offsetGet, ZEND_ACC_PUBLIC)
  PHP_ME(RepeatedField, count,        arginfo_void,      ZEND_ACC_PUBLIC)
  PHP_ME(RepeatedField, getIterator,  arginfo_void,      ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

static void repeated_field_init() {
  zend_class_entry tmp_ce;
  zend_object_handlers *h = &repeated_field_object_handlers;

  // TODO(haberman): see if we can mark this final, for robustness.
  INIT_CLASS_ENTRY(tmp_ce, "Google\\Protobuf\\Internal\\RepeatedField",
                   repeated_field_methods);

  repeated_field_ce = zend_register_internal_class(&tmp_ce);
  zend_class_implements(repeated_field_ce, 3, spl_ce_ArrayAccess,
                        zend_ce_aggregate, spl_ce_Countable);
  repeated_field_ce->create_object = repeated_field_create;

  memcpy(h, &std_object_handlers, sizeof(zend_object_handlers));
  h->dtor_obj = repeated_field_dtor;
  h->get_gc = repeated_field_get_gc;
}

// -----------------------------------------------------------------------------
// PHP RepeatedFieldIter
// -----------------------------------------------------------------------------

typedef struct {
  zend_object std;
  // TODO(haberman): does this need to be a zval so it's GC-rooted?
  RepeatedField* repeated_field;
  zend_long position;
} RepeatedFieldIter;

zend_class_entry *repeated_field_iter_ce;
static zend_object_handlers repeated_field_iter_object_handlers;

/**
 * Object handler to create a RepeatedFieldIter.
 */
zend_object* repeated_field_iter_create(zend_class_entry *class_type) {
  RepeatedFieldIter *intern = emalloc(sizeof(RepeatedFieldIter));
  zend_object_std_init(&intern->std, class_type);
  intern->std.handlers = &repeated_field_iter_object_handlers;
  intern->repeated_field = NULL;
  intern->position = 0;
  // Skip object_properties_init(), we don't allow derived classes.
  return &intern->std;
}

static void RepeatedFieldIter_make(zval *val, zval *repeated_field) {
  RepeatedFieldIter *iter;
  ZVAL_OBJ(val, repeated_field_iter_ce->create_object(repeated_field_iter_ce));
  iter = (RepeatedFieldIter*)Z_OBJ_P(val);
  iter->repeated_field = (RepeatedField*)Z_OBJ_P(repeated_field);
}

PHP_METHOD(RepeatedFieldIter, rewind) {
  RepeatedFieldIter *intern = (RepeatedFieldIter*)Z_OBJ_P(getThis());
  intern->position = 0;
}

PHP_METHOD(RepeatedFieldIter, current) {
  RepeatedFieldIter *intern = (RepeatedFieldIter*)Z_OBJ_P(getThis());
  RepeatedField* field = intern->repeated_field;
  upb_array *array = field->array;
  zend_long index = intern->position;
  upb_msgval msgval;
  zval ret;

  if (index < 0 || index >= upb_array_size(array)) {
    zend_error(E_USER_ERROR, "Element at %ld doesn't exist.\n", index);
  }

  msgval = upb_array_get(array, index);
  pbphp_tozval(msgval, &ret, field->type, field->klass);
  RETURN_ZVAL(&ret, 0, 1);
}

PHP_METHOD(RepeatedFieldIter, key) {
  RepeatedFieldIter *intern = (RepeatedFieldIter*)Z_OBJ_P(getThis());
  RETURN_LONG(intern->position);
}

PHP_METHOD(RepeatedFieldIter, next) {
  RepeatedFieldIter *intern = (RepeatedFieldIter*)Z_OBJ_P(getThis());
  ++intern->position;
}

PHP_METHOD(RepeatedFieldIter, valid) {
  RepeatedFieldIter *intern = (RepeatedFieldIter*)Z_OBJ_P(getThis());
  RETURN_BOOL(intern->position < upb_array_size(intern->repeated_field->array));
}

static zend_function_entry repeated_field_iter_methods[] = {
  PHP_ME(RepeatedFieldIter, rewind,      arginfo_void, ZEND_ACC_PUBLIC)
  PHP_ME(RepeatedFieldIter, current,     arginfo_void, ZEND_ACC_PUBLIC)
  PHP_ME(RepeatedFieldIter, key,         arginfo_void, ZEND_ACC_PUBLIC)
  PHP_ME(RepeatedFieldIter, next,        arginfo_void, ZEND_ACC_PUBLIC)
  PHP_ME(RepeatedFieldIter, valid,       arginfo_void, ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

static void repeated_field_iter_init() {
  zend_class_entry tmp_ce;
  zend_object_handlers *h = &repeated_field_iter_object_handlers;

  // TODO(haberman): see if we can mark this final, for robustness.
  INIT_CLASS_ENTRY(tmp_ce, "Google\\Protobuf\\Internal\\RepeatedFieldIter",
                   repeated_field_iter_methods);

  repeated_field_iter_ce = zend_register_internal_class(&tmp_ce);
  zend_class_implements(repeated_field_iter_ce, 1, zend_ce_iterator);
  repeated_field_iter_ce->create_object = repeated_field_iter_create;

  memcpy(h, &std_object_handlers, sizeof(zend_object_handlers));
}

// Module init /////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// GPBType
// -----------------------------------------------------------------------------

zend_class_entry* gpb_type_type;

static zend_function_entry gpb_type_methods[] = {
  ZEND_FE_END
};

void gpb_type_init() {
#define STR(str) (str), strlen(str)
  zend_class_entry class_type;
  INIT_CLASS_ENTRY(class_type, "Google\\Protobuf\\Internal\\GPBType",
                   gpb_type_methods);
  gpb_type_type = zend_register_internal_class(&class_type);
  zend_declare_class_constant_long(gpb_type_type, STR("DOUBLE"), 1);
  zend_declare_class_constant_long(gpb_type_type, STR("FLOAT"), 2);
  zend_declare_class_constant_long(gpb_type_type, STR("INT64"), 3);
  zend_declare_class_constant_long(gpb_type_type, STR("UINT64"), 4);
  zend_declare_class_constant_long(gpb_type_type, STR("INT32"), 5);
  zend_declare_class_constant_long(gpb_type_type, STR("FIXED64"), 6);
  zend_declare_class_constant_long(gpb_type_type, STR("FIXED32"), 7);
  zend_declare_class_constant_long(gpb_type_type, STR("BOOL"), 8);
  zend_declare_class_constant_long(gpb_type_type, STR("STRING"), 9);
  zend_declare_class_constant_long(gpb_type_type, STR("GROUP"), 10);
  zend_declare_class_constant_long(gpb_type_type, STR("MESSAGE"), 11);
  zend_declare_class_constant_long(gpb_type_type, STR("BYTES"), 12);
  zend_declare_class_constant_long(gpb_type_type, STR("UINT32"), 13);
  zend_declare_class_constant_long(gpb_type_type, STR("ENUM"), 14);
  zend_declare_class_constant_long(gpb_type_type, STR("SFIXED32"), 15);
  zend_declare_class_constant_long(gpb_type_type, STR("SFIXED64"), 16);
  zend_declare_class_constant_long(gpb_type_type, STR("SINT32"), 17);
  zend_declare_class_constant_long(gpb_type_type, STR("SINT64"), 18);
#undef STR
}

void array_module_init() {
  repeated_field_init();
  repeated_field_iter_init();
  gpb_type_init();
}

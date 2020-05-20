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

#include "map.h"

#include <Zend/zend_API.h>
#include <Zend/zend_interfaces.h>

#include <ext/spl/spl_iterators.h>

#include "arena.h"
#include "convert.h"
#include "php-upb.h"
#include "protobuf.h"

//#include "utf8.h"

static void MapFieldIter_make(zval *val, zval *map_field);

// -----------------------------------------------------------------------------
// MapField
// -----------------------------------------------------------------------------

typedef struct {
  zend_object std;
  zval arena;
  upb_map *map;
  upb_fieldtype_t key_type;
  upb_fieldtype_t val_type;
  const Descriptor* desc;  // When values are messages.
} MapField;

zend_class_entry *map_field_ce;
static zend_object_handlers map_field_object_handlers;

/**
 * Object handler to create a MapField.
 */
static zend_object* map_field_create(zend_class_entry *class_type) {
  MapField *intern = emalloc(sizeof(MapField));
  zend_object_std_init(&intern->std, class_type);
  intern->std.handlers = &map_field_object_handlers;
  arena_init(&intern->arena);
  intern->map = NULL;
  // Skip object_properties_init(), we don't allow derived classes.
  return &intern->std;
}

/**
 * Object handler to free a MapField.
 */
static void map_field_dtor(zend_object* obj) {
  MapField* intern = (MapField*)obj;
  pbphp_cachedel(intern->map);
  zval_ptr_dtor(&intern->arena);
  zend_object_std_dtor(&intern->std);
}

void pbphp_getmapfield(zval *val, upb_map *map, const upb_fielddef *f,
                       zval *arena) {
  if (!map) {
    ZVAL_NULL(val);
    return;
  }

  if (!pbphp_cacheget(map, val)) {
    const upb_msgdef *ent = upb_fielddef_msgsubdef(f);
    const upb_fielddef *key_f = upb_msgdef_itof(ent, 1);
    const upb_fielddef *val_f = upb_msgdef_itof(ent, 2);
    MapField *intern = emalloc(sizeof(MapField));
    zend_object_std_init(&intern->std, map_field_ce);
    intern->std.handlers = &map_field_object_handlers;
    ZVAL_COPY(&intern->arena, arena);
    intern->map = map;
    intern->key_type = upb_fielddef_type(key_f);
    intern->val_type = upb_fielddef_type(val_f);
    intern->desc = pupb_getdesc_from_msgdef(upb_fielddef_msgsubdef(val_f));
    // Skip object_properties_init(), we don't allow derived classes.
    pbphp_cacheadd(intern->map, &intern->std);
    ZVAL_OBJ(val, &intern->std);
  }
}

upb_map *pbphp_getmap(zval *val, const upb_fielddef *f, upb_arena *arena) {
  upb_fieldtype_t type = upb_fielddef_type(f);
  const upb_msgdef *ent = upb_fielddef_msgsubdef(f);
  const upb_fielddef *key_f = upb_msgdef_itof(ent, 1);
  const upb_fielddef *val_f = upb_msgdef_itof(ent, 2);
  upb_fieldtype_t key_type = upb_fielddef_type(key_f);
  upb_fieldtype_t val_type = upb_fielddef_type(val_f);
  const Descriptor *desc =
      pupb_getdesc_from_msgdef(upb_fielddef_msgsubdef(val_f));

  if (Z_ISREF_P(val)) {
    ZVAL_DEREF(val);
  }

  if (Z_TYPE_P(val) == IS_ARRAY) {
    upb_map *map = upb_map_new(arena, key_type, val_type);
    HashTable *table = HASH_OF(val);
    HashPosition pos;

    zend_hash_internal_pointer_reset_ex(table, &pos);

    while (true) {
      zval php_key;
      zval *php_val;
      upb_msgval upb_key;
      upb_msgval upb_val;

      zend_hash_get_current_key_zval_ex(table, &php_key, &pos);
      php_val = zend_hash_get_current_data_ex(table, &pos);

      if (!php_val) return map;

      if (!pbphp_tomsgval(&php_key, &upb_key, key_type, NULL, arena) ||
          !pbphp_inittomsgval(php_val, &upb_val, val_type, desc, arena)) {
        return NULL;
      }

      upb_map_set(map, upb_key, upb_val, arena);
      zend_hash_move_forward_ex(table, &pos);
      zval_dtor(&php_key);
    }
  } else if (Z_TYPE_P(val) == IS_OBJECT && Z_OBJCE_P(val) == map_field_ce) {
    MapField *intern = (MapField*)Z_OBJ_P(val);

    if (intern->key_type != key_type || intern->val_type != val_type ||
        intern->desc != desc) {
      php_error_docref(NULL, E_USER_ERROR, "Wrong type for this map field.");
      return NULL;
    }

    upb_arena_fuse(arena, arena_get(&intern->arena));
    return intern->map;
  } else {
    php_error_docref(NULL, E_USER_ERROR, "Must be a map");
    return NULL;
  }
}

/**
 * Constructs an instance of RepeatedField.
 * @param long Key type.
 * @param long Value type.
 * @param string Message/Enum class (message/enum value types only).
 */
PHP_METHOD(MapField, __construct) {
  MapField *intern = (MapField*)Z_OBJ_P(getThis());
  upb_arena *arena = arena_get(&intern->arena);
  zend_long key_type, val_type;
  zend_class_entry* klass = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll|C", &key_type, &val_type,
                            &klass) != SUCCESS) {
    return;
  }

  intern->key_type = pbphp_dtype_to_type(key_type);
  intern->val_type = pbphp_dtype_to_type(val_type);
  intern->desc = pupb_getdesc(klass);

  // Check that the key type is an allowed type.
  switch (intern->key_type) {
    case UPB_TYPE_INT32:
    case UPB_TYPE_INT64:
    case UPB_TYPE_UINT32:
    case UPB_TYPE_UINT64:
    case UPB_TYPE_BOOL:
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES:
      // These are OK.
      break;
    default:
      zend_error(E_USER_ERROR, "Invalid key type for map.");
  }

  if (intern->val_type == UPB_TYPE_MESSAGE && klass == NULL) {
    php_error_docref(NULL, E_USER_ERROR,
                     "Message/enum type must have concrete class.");
    return;
  }

  intern->map = upb_map_new(arena, intern->key_type, intern->val_type);
  pbphp_cacheadd(intern->map, &intern->std);
}

PHP_METHOD(MapField, offsetExists) {
  MapField *intern = (MapField*)Z_OBJ_P(getThis());
  zval *key;
  upb_msgval upb_key;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &key) != SUCCESS ||
      !pbphp_tomsgval(key, &upb_key, intern->key_type, intern->desc, NULL)) {
    return;
  }

  RETURN_BOOL(upb_map_get(intern->map, upb_key, NULL));
}

PHP_METHOD(MapField, offsetGet) {
  MapField *intern = (MapField*)Z_OBJ_P(getThis());
  upb_arena *arena = arena_get(&intern->arena);
  zval *key;
  zval ret;
  upb_msgval upb_key, upb_val;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &key) != SUCCESS ||
      !pbphp_tomsgval(key, &upb_key, intern->key_type, intern->desc, NULL)) {
    return;
  }

  if (!upb_map_get(intern->map, upb_key, &upb_val)) {
    zend_error(E_USER_ERROR, "Given key doesn't exist.");
    return;
  }

  pbphp_tozval(upb_val, &ret, intern->val_type, intern->desc, &intern->arena);
  RETURN_ZVAL(&ret, 0, 1);
}

PHP_METHOD(MapField, offsetSet) {
  MapField *intern = (MapField*)Z_OBJ_P(getThis());
  upb_arena *arena = arena_get(&intern->arena);
  zval *key, *val;
  upb_msgval upb_key, upb_val;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "zz", &key, &val) != SUCCESS ||
      !pbphp_tomsgval(key, &upb_key, intern->key_type, NULL, NULL) ||
      !pbphp_tomsgval(val, &upb_val, intern->val_type, intern->desc, arena)) {
    return;
  }

  upb_map_set(intern->map, upb_key, upb_val, arena);
}

PHP_METHOD(MapField, offsetUnset) {
  MapField *intern = (MapField*)Z_OBJ_P(getThis());
  zval *key;
  upb_msgval upb_key;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &key) != SUCCESS ||
      !pbphp_tomsgval(key, &upb_key, intern->key_type, NULL, NULL)) {
    return;
  }

  upb_map_delete(intern->map, upb_key);
}

PHP_METHOD(MapField, count) {
  MapField *intern = (MapField*)Z_OBJ_P(getThis());

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  RETURN_LONG(upb_map_size(intern->map));
}

PHP_METHOD(MapField, getIterator) {
  MapField *intern = (MapField*)Z_OBJ_P(getThis());
  zval ret;

  MapFieldIter_make(&ret, getThis());
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

static zend_function_entry map_field_methods[] = {
  PHP_ME(MapField, __construct,  NULL,              ZEND_ACC_PUBLIC)
  PHP_ME(MapField, offsetExists, arginfo_offsetGet, ZEND_ACC_PUBLIC)
  PHP_ME(MapField, offsetGet,    arginfo_offsetGet, ZEND_ACC_PUBLIC)
  PHP_ME(MapField, offsetSet,    arginfo_offsetSet, ZEND_ACC_PUBLIC)
  PHP_ME(MapField, offsetUnset,  arginfo_offsetGet, ZEND_ACC_PUBLIC)
  PHP_ME(MapField, count,        arginfo_void,      ZEND_ACC_PUBLIC)
  PHP_ME(MapField, getIterator,  arginfo_void,      ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

static zval *map_get_property_ptr_ptr(zval *object, zval *member, int type,
                                      void **cache_slot) {
  return NULL;
}

static HashTable *map_get_properties(zval *object TSRMLS_DC) { return NULL; }

static void map_field_init() {
  zend_class_entry tmp_ce;
  zend_object_handlers *h = &map_field_object_handlers;

  INIT_CLASS_ENTRY(tmp_ce, "Google\\Protobuf\\Internal\\MapField",
                   map_field_methods);

  map_field_ce = zend_register_internal_class(&tmp_ce);
  zend_class_implements(map_field_ce, 3, spl_ce_ArrayAccess,
                        zend_ce_aggregate, spl_ce_Countable);
  map_field_ce->ce_flags |= ZEND_ACC_FINAL;
  map_field_ce->create_object = map_field_create;

  memcpy(h, &std_object_handlers, sizeof(zend_object_handlers));
  h->dtor_obj = map_field_dtor;
  /*h->read_property = message_read_property;
  h->write_property = message_write_property;*/
  h->get_properties = map_get_properties;
  h->get_property_ptr_ptr = map_get_property_ptr_ptr;
}

// -----------------------------------------------------------------------------
// MapFieldIter
// -----------------------------------------------------------------------------

typedef struct {
  zend_object std;
  zval map_field;
  size_t position;
} MapFieldIter;

zend_class_entry *map_field_iter_ce;
static zend_object_handlers map_field_iter_object_handlers;

/**
 * Object handler to create a MapFieldIter.
 */
zend_object* map_field_iter_create(zend_class_entry *class_type) {
  MapFieldIter *intern = emalloc(sizeof(MapFieldIter));
  zend_object_std_init(&intern->std, class_type);
  intern->std.handlers = &map_field_iter_object_handlers;
  ZVAL_NULL(&intern->map_field);
  intern->position = 0;
  // Skip object_properties_init(), we don't allow derived classes.
  return &intern->std;
}

static void map_field_iter_dtor(zend_object* obj) {
  MapFieldIter* intern = (MapFieldIter*)obj;
  zval_ptr_dtor(&intern->map_field);
  zend_object_std_dtor(&intern->std);
}

static void MapFieldIter_make(zval *val, zval *map_field) {
  MapFieldIter *iter;
  ZVAL_OBJ(val, map_field_iter_ce->create_object(map_field_iter_ce));
  iter = (MapFieldIter*)Z_OBJ_P(val);
  ZVAL_COPY(&iter->map_field, map_field);
}

// -----------------------------------------------------------------------------
// PHP MapFieldIter Methods
// -----------------------------------------------------------------------------

// PHP's iterator protocol is:
//
// for ($iter->rewind(); $iter->valid(); $iter->next()) {
//   $val = $iter->key();
//   $val = $iter->current();
// }

PHP_METHOD(MapFieldIter, rewind) {
  MapFieldIter *intern = (MapFieldIter*)Z_OBJ_P(getThis());
  MapField *map_field = (MapField*)Z_OBJ_P(&intern->map_field);
  intern->position = UPB_MAP_BEGIN;
  upb_mapiter_next(map_field->map, &intern->position);
}

PHP_METHOD(MapFieldIter, current) {
  MapFieldIter *intern = (MapFieldIter*)Z_OBJ_P(getThis());
  MapField *field = (MapField*)Z_OBJ_P(&intern->map_field);
  upb_arena *arena = arena_get(&field->arena);
  upb_msgval upb_val = upb_mapiter_value(field->map, intern->position);
  zval ret;
  pbphp_tozval(upb_val, &ret, field->val_type, field->desc, &field->arena);
  RETURN_ZVAL(&ret, 0, 1);
}

PHP_METHOD(MapFieldIter, key) {
  MapFieldIter *intern = (MapFieldIter*)Z_OBJ_P(getThis());
  MapField *field = (MapField*)Z_OBJ_P(&intern->map_field);
  upb_msgval upb_key = upb_mapiter_key(field->map, intern->position);
  zval ret;
  pbphp_tozval(upb_key, &ret, field->key_type, NULL, NULL);
  RETURN_ZVAL(&ret, 0, 1);
}

PHP_METHOD(MapFieldIter, next) {
  MapFieldIter *intern = (MapFieldIter*)Z_OBJ_P(getThis());
  MapField *field = (MapField*)Z_OBJ_P(&intern->map_field);
  upb_mapiter_next(field->map, &intern->position);
}

PHP_METHOD(MapFieldIter, valid) {
  MapFieldIter *intern = (MapFieldIter*)Z_OBJ_P(getThis());
  MapField *field = (MapField*)Z_OBJ_P(&intern->map_field);
  bool done = upb_mapiter_done(field->map, intern->position);
  RETURN_BOOL(!done);
}

static zend_function_entry map_field_iter_methods[] = {
  PHP_ME(MapFieldIter, rewind,      arginfo_void, ZEND_ACC_PUBLIC)
  PHP_ME(MapFieldIter, current,     arginfo_void, ZEND_ACC_PUBLIC)
  PHP_ME(MapFieldIter, key,         arginfo_void, ZEND_ACC_PUBLIC)
  PHP_ME(MapFieldIter, next,        arginfo_void, ZEND_ACC_PUBLIC)
  PHP_ME(MapFieldIter, valid,       arginfo_void, ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

static void map_field_iter_init() {
  zend_class_entry tmp_ce;
  zend_object_handlers *h = &map_field_iter_object_handlers;

  // TODO(haberman): see if we can mark this final, for robustness.
  INIT_CLASS_ENTRY(tmp_ce, "Google\\Protobuf\\Internal\\MapFieldIter",
                   map_field_iter_methods);

  map_field_iter_ce = zend_register_internal_class(&tmp_ce);
  zend_class_implements(map_field_iter_ce, 1, zend_ce_iterator);
  map_field_iter_ce->ce_flags |= ZEND_ACC_FINAL;
  map_field_iter_ce->create_object = map_field_iter_create;

  memcpy(h, &std_object_handlers, sizeof(zend_object_handlers));
  h->dtor_obj = map_field_iter_dtor;
}

void map_module_init() {
  map_field_init();
  map_field_iter_init();
}

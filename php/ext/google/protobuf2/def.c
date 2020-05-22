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

#include "def.h"

#include <php.h>

// Must be last.
#include <Zend/zend_exceptions.h>

#include "names.h"
#include "php-upb.h"
#include "protobuf.h"

static void check_upb_status(const upb_status* status, const char* msg) {
  if (!upb_ok(status)) {
    zend_error(E_ERROR, "%s: %s\n", msg, upb_status_errmsg(status));
  }
}

static void FieldDescriptor_FromFieldDef(zval *val, const upb_fielddef *f);

// -----------------------------------------------------------------------------
// EnumValueDescriptor
// -----------------------------------------------------------------------------

typedef struct {
  zend_object std;
  const char *name;
  int32_t number;
} EnumValueDescriptor;

zend_class_entry *enum_value_descriptor_ce;
static zend_object_handlers enum_value_descriptor_object_handlers;

static zend_object *enum_value_descriptor_create(zend_class_entry *class_type) {
  // TODO(haberman): nobody should call this, how can we prohibit it?
  return NULL;
}

static void EnumValueDescriptor_Make(zval *val, const char *name,
                                     int32_t number) {
  EnumValueDescriptor *intern = emalloc(sizeof(EnumValueDescriptor));
  zend_object_std_init(&intern->std, enum_value_descriptor_ce);
  intern->std.handlers = &enum_value_descriptor_object_handlers;
  intern->name = name;
  intern->number = number;
  // Skip object_properties_init(), we don't allow derived classes.
  ZVAL_OBJ(val, &intern->std);
}

PHP_METHOD(EnumValueDescriptor, getName) {
  EnumValueDescriptor *intern = (EnumValueDescriptor*)Z_OBJ_P(getThis());
  RETURN_STRING(intern->name);
}

PHP_METHOD(EnumValueDescriptor, getNumber) {
  EnumValueDescriptor *intern = (EnumValueDescriptor*)Z_OBJ_P(getThis());
  RETURN_LONG(intern->number);
}

static zend_function_entry enum_value_descriptor_methods[] = {
  PHP_ME(EnumValueDescriptor, getName, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(EnumValueDescriptor, getNumber, NULL, ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

static void enum_value_descriptor_init() {
  zend_class_entry tmp_ce;
  zend_object_handlers *h = &enum_value_descriptor_object_handlers;

  INIT_CLASS_ENTRY(tmp_ce, "Google\\Protobuf\\EnumValueDescriptor",
                   enum_value_descriptor_methods);

  enum_value_descriptor_ce = zend_register_internal_class(&tmp_ce);
  enum_value_descriptor_ce->ce_flags |= ZEND_ACC_FINAL;
  enum_value_descriptor_ce->create_object = enum_value_descriptor_create;

  memcpy(h, &std_object_handlers, sizeof(zend_object_handlers));
}

// -----------------------------------------------------------------------------
// EnumDescriptor
// -----------------------------------------------------------------------------

typedef struct {
  zend_object std;
  const upb_enumdef *enumdef;
} EnumDescriptor;

zend_class_entry *enum_descriptor_ce;
static zend_object_handlers enum_descriptor_object_handlers;

static zend_object *enum_descriptor_create(zend_class_entry *class_type) {
  // TODO(haberman): nobody should call this, how can we prohibit it?
  return NULL;
}

void EnumDescriptor_FromClassEntry(zval *val, zend_class_entry *ce) {
  if (ce == NULL) {
    ZVAL_NULL(val);
    return;
  }

  if (!pbphp_cacheget(ce, val)) {
    EnumDescriptor* ret = emalloc(sizeof(EnumDescriptor));
    zend_object_std_init(&ret->std, enum_descriptor_ce);
    ret->std.handlers = &enum_descriptor_object_handlers;
    ret->enumdef = pbphp_namemap_get(ce);
    pbphp_cacheadd(ce, &ret->std);

    // Prevent this from ever being collected (within a request).
    GC_ADDREF(&ret->std);

    ZVAL_OBJ(val, &ret->std);
  }
}

void EnumDescriptor_FromEnumDef(zval *val, const upb_enumdef *m) {
  if (m) {
    char *classname =
        pbphp_get_classname(upb_enumdef_file(m), upb_enumdef_fullname(m));
    zend_string *str = zend_string_init(classname, strlen(classname), 0);
    zend_class_entry *ce = zend_lookup_class(str);  // May autoload the class.

    zend_string_release (str);

    if (!ce) {
      zend_error(E_ERROR, "Couldn't load generated class %s", classname);
    }

    free(classname);
    EnumDescriptor_FromClassEntry(val, ce);
  } else {
    ZVAL_NULL(val);
  }
}

PHP_METHOD(EnumDescriptor, getValue) {
  EnumDescriptor *intern = (EnumDescriptor*)Z_OBJ_P(getThis());
  zend_long index;
  zval ret;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) ==
      FAILURE) {
    zend_error(E_USER_ERROR, "Expect integer for index.\n");
    return;
  }

  int field_num = upb_enumdef_numvals(intern->enumdef);
  if (index < 0 || index >= field_num) {
    zend_error(E_USER_ERROR, "Cannot get element at %ld.\n", index);
    return;
  }

  upb_enum_iter iter;
  int i;
  for(upb_enum_begin(&iter, intern->enumdef), i = 0;
      !upb_enum_done(&iter) && i < index;
      upb_enum_next(&iter), i++);

  EnumValueDescriptor_Make(&ret, upb_enum_iter_name(&iter),
                           upb_enum_iter_number(&iter));
  RETURN_ZVAL(&ret, 0, 1);
}

PHP_METHOD(EnumDescriptor, getValueCount) {
  EnumDescriptor *intern = (EnumDescriptor*)Z_OBJ_P(getThis());
  RETURN_LONG(upb_enumdef_numvals(intern->enumdef));
}

PHP_METHOD(EnumDescriptor, getPublicDescriptor) {
  RETURN_ZVAL(getThis(), 1, 0);
}

static zend_function_entry enum_descriptor_methods[] = {
  PHP_ME(EnumDescriptor, getPublicDescriptor, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(EnumDescriptor, getValueCount, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(EnumDescriptor, getValue, NULL, ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

static void enum_descriptor_init() {
  zend_class_entry tmp_ce;
  zend_object_handlers *h = &enum_descriptor_object_handlers;

  INIT_CLASS_ENTRY(tmp_ce, "Google\\Protobuf\\EnumDescriptor",
                   enum_descriptor_methods);

  enum_descriptor_ce = zend_register_internal_class(&tmp_ce);
  enum_descriptor_ce->ce_flags |= ZEND_ACC_FINAL;
  enum_descriptor_ce->create_object = enum_descriptor_create;

  memcpy(h, &std_object_handlers, sizeof(zend_object_handlers));
}

// -----------------------------------------------------------------------------
// Oneof
// -----------------------------------------------------------------------------

typedef struct {
  zend_object std;
  const upb_oneofdef *oneofdef;
} OneofDescriptor;

zend_class_entry *oneof_descriptor_ce;
static zend_object_handlers oneof_descriptor_object_handlers;

static zend_object *oneof_descriptor_create(zend_class_entry *class_type) {
  // TODO(haberman): nobody should call this, how can we prohibit it?
  return NULL;
}

static void OneofDescriptor_FromOneofDef(zval *val, const upb_oneofdef *o) {
  if (o == NULL) {
    ZVAL_NULL(val);
    return;
  }

  if (!pbphp_cacheget(o, val)) {
    OneofDescriptor* ret = emalloc(sizeof(OneofDescriptor));
    zend_object_std_init(&ret->std, oneof_descriptor_ce);
    ret->std.handlers = &oneof_descriptor_object_handlers;
    ret->oneofdef = o;
    pbphp_cacheadd(o, &ret->std);

    // Prevent this from ever being collected (within a request).
    GC_ADDREF(&ret->std);

    ZVAL_OBJ(val, &ret->std);
  }
}

PHP_METHOD(OneofDescriptor, getName) {
  OneofDescriptor *intern = (OneofDescriptor*)Z_OBJ_P(getThis());
  RETURN_STRING(upb_oneofdef_name(intern->oneofdef));
}

PHP_METHOD(OneofDescriptor, getField) {
  OneofDescriptor *intern = (OneofDescriptor*)Z_OBJ_P(getThis());
  zend_long index;
  zval ret;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) ==
      FAILURE) {
    zend_error(E_USER_ERROR, "Expect integer for index.\n");
    return;
  }

  int field_num = upb_oneofdef_numfields(intern->oneofdef);
  if (index < 0 || index >= field_num) {
    zend_error(E_USER_ERROR, "Cannot get element at %ld.\n", index);
    return;
  }

  upb_oneof_iter iter;
  int i;
  for(upb_oneof_begin(&iter, intern->oneofdef), i = 0;
      !upb_oneof_done(&iter) && i < index;
      upb_oneof_next(&iter), i++);
  const upb_fielddef *field = upb_oneof_iter_field(&iter);

  FieldDescriptor_FromFieldDef(&ret, field);
  RETURN_ZVAL(&ret, 1, 0);
}

PHP_METHOD(OneofDescriptor, getFieldCount) {
  OneofDescriptor *intern = (OneofDescriptor*)Z_OBJ_P(getThis());
  RETURN_LONG(upb_oneofdef_numfields(intern->oneofdef));
}

static zend_function_entry oneof_descriptor_methods[] = {
  PHP_ME(OneofDescriptor, getName,  NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OneofDescriptor, getField, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OneofDescriptor, getFieldCount, NULL, ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

static void oneof_descriptor_init() {
  zend_class_entry tmp_ce;
  zend_object_handlers *h = &oneof_descriptor_object_handlers;

  INIT_CLASS_ENTRY(tmp_ce, "Google\\Protobuf\\OneofDescriptor",
                   oneof_descriptor_methods);

  oneof_descriptor_ce = zend_register_internal_class(&tmp_ce);
  oneof_descriptor_ce->ce_flags |= ZEND_ACC_FINAL;
  oneof_descriptor_ce->create_object = oneof_descriptor_create;

  memcpy(h, &std_object_handlers, sizeof(zend_object_handlers));
}

// -----------------------------------------------------------------------------
// FieldDescriptor
// -----------------------------------------------------------------------------

typedef struct {
  zend_object std;
  const upb_fielddef *fielddef;
} FieldDescriptor;

zend_class_entry *field_descriptor_ce;
static zend_object_handlers field_descriptor_object_handlers;

static zend_object *field_descriptor_create(zend_class_entry *class_type) {
  // TODO(haberman): nobody should call this, how can we prohibit it?
  return NULL;
}

static void FieldDescriptor_FromFieldDef(zval *val, const upb_fielddef *f) {
  if (f == NULL) {
    ZVAL_NULL(val);
    return;
  }

  if (!pbphp_cacheget(f, val)) {
    FieldDescriptor* ret = emalloc(sizeof(FieldDescriptor));
    zend_object_std_init(&ret->std, field_descriptor_ce);
    ret->std.handlers = &field_descriptor_object_handlers;
    ret->fielddef = f;
    pbphp_cacheadd(f, &ret->std);

    // Prevent this from ever being collected (within a request).
    GC_ADDREF(&ret->std);

    ZVAL_OBJ(val, &ret->std);
  }
}

upb_fieldtype_t to_fieldtype(upb_descriptortype_t type) {
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

PHP_METHOD(FieldDescriptor, getName) {
  FieldDescriptor *intern = (FieldDescriptor*)Z_OBJ_P(getThis());
  RETURN_STRING(upb_fielddef_name(intern->fielddef));
}

PHP_METHOD(FieldDescriptor, getNumber) {
  FieldDescriptor *intern = (FieldDescriptor*)Z_OBJ_P(getThis());
  RETURN_LONG(upb_fielddef_number(intern->fielddef));
}

PHP_METHOD(FieldDescriptor, getLabel) {
  FieldDescriptor *intern = (FieldDescriptor*)Z_OBJ_P(getThis());
  RETURN_LONG(upb_fielddef_label(intern->fielddef));
}

PHP_METHOD(FieldDescriptor, getType) {
  FieldDescriptor *intern = (FieldDescriptor*)Z_OBJ_P(getThis());
  RETURN_LONG(upb_fielddef_descriptortype(intern->fielddef));
}

PHP_METHOD(FieldDescriptor, isMap) {
  FieldDescriptor *intern = (FieldDescriptor*)Z_OBJ_P(getThis());
  RETURN_BOOL(upb_fielddef_ismap(intern->fielddef));
}

PHP_METHOD(FieldDescriptor, getEnumType) {
  FieldDescriptor *intern = (FieldDescriptor*)Z_OBJ_P(getThis());
  const upb_enumdef *e = upb_fielddef_enumsubdef(intern->fielddef);
  EnumDescriptor *desc;
  zval ret;

  if (!e) {
    zend_throw_exception_ex(NULL, 0,
                            "Cannot get enum type for non-enum field '%s'",
                            upb_fielddef_name(intern->fielddef));
    return;
  }

  EnumDescriptor_FromEnumDef(&ret, e);
  RETURN_ZVAL(&ret, 1, 0);
}

PHP_METHOD(FieldDescriptor, getMessageType) {
  FieldDescriptor *intern = (FieldDescriptor*)Z_OBJ_P(getThis());
  Descriptor* desc = Descriptor_GetFromFieldDef(intern->fielddef);
  zval ret;

  if (!desc) {
    zend_throw_exception_ex(
        NULL, 0, "Cannot get message type for non-message field '%s'",
        upb_fielddef_name(intern->fielddef));
    return;
  }

  ZVAL_OBJ(&ret, &desc->std);
  RETURN_ZVAL(&ret, 1, 0);
}

static zend_function_entry field_descriptor_methods[] = {
  PHP_ME(FieldDescriptor, getName,   NULL, ZEND_ACC_PUBLIC)
  PHP_ME(FieldDescriptor, getNumber, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(FieldDescriptor, getLabel,  NULL, ZEND_ACC_PUBLIC)
  PHP_ME(FieldDescriptor, getType,   NULL, ZEND_ACC_PUBLIC)
  PHP_ME(FieldDescriptor, isMap,     NULL, ZEND_ACC_PUBLIC)
  PHP_ME(FieldDescriptor, getEnumType, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(FieldDescriptor, getMessageType, NULL, ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

static void field_descriptor_init() {
  zend_class_entry tmp_ce;
  zend_object_handlers *h = &field_descriptor_object_handlers;

  INIT_CLASS_ENTRY(tmp_ce, "Google\\Protobuf\\FieldDescriptor",
                   field_descriptor_methods);

  field_descriptor_ce = zend_register_internal_class(&tmp_ce);
  field_descriptor_ce->ce_flags |= ZEND_ACC_FINAL;
  field_descriptor_ce->create_object = field_descriptor_create;

  memcpy(h, &std_object_handlers, sizeof(zend_object_handlers));
}

// -----------------------------------------------------------------------------
// Descriptor
// -----------------------------------------------------------------------------

zend_class_entry *descriptor_ce;
static zend_object_handlers descriptor_object_handlers;

static zend_object* descriptor_create(zend_class_entry *class_type) {
  // TODO(haberman): nobody should call this, how can we prohibit it?
  return NULL;
}

static void descriptor_dtor(zend_object* obj) {
  Descriptor* intern = (Descriptor*)obj;
  // We don't really need to do anything here, we don't allow this to be
  // collected before the end of the request.
}

void Descriptor_FromClassEntry(zval *val, zend_class_entry *ce) {
  if (ce == NULL) {
    ZVAL_NULL(val);
    return;
  }

  if (!pbphp_cacheget(ce, val)) {
    Descriptor* ret = emalloc(sizeof(Descriptor));
    zend_object_std_init(&ret->std, descriptor_ce);
    ret->std.handlers = &descriptor_object_handlers;
    ret->class_entry = ce;
    ret->msgdef = pbphp_namemap_get(ce);
    pbphp_cacheadd(ce, &ret->std);

    // Prevent this from ever being collected (within a request).
    GC_ADDREF(&ret->std);

    ZVAL_OBJ(val, &ret->std);
  }
}

Descriptor* Descriptor_GetFromClassEntry(zend_class_entry *ce) {
  zval desc;
  Descriptor_FromClassEntry(&desc, ce);
  if (Z_TYPE_P(&desc) == IS_NULL) {
    return NULL;
  } else {
    return (Descriptor*)Z_OBJ_P(&desc);
  }
}

Descriptor* Descriptor_GetFromMessageDef(const upb_msgdef *m) {
  if (m) {
    if (upb_msgdef_mapentry(m)) {
      // A bit of a hack, since map entries don't have classes.
      Descriptor* ret = emalloc(sizeof(Descriptor));
      zend_object_std_init(&ret->std, descriptor_ce);
      ret->std.handlers = &descriptor_object_handlers;
      ret->class_entry = NULL;
      ret->msgdef = m;

      // Prevent this from ever being collected (within a request).
      GC_ADDREF(&ret->std);

      return ret;
    }

    char *classname =
        pbphp_get_classname(upb_msgdef_file(m), upb_msgdef_fullname(m));
    zend_string *str = zend_string_init(classname, strlen(classname), 0);
    zend_class_entry *ce = zend_lookup_class(str);  // May autoload the class.

    zend_string_release (str);

    if (!ce) {
      zend_error(E_ERROR, "Couldn't load generated class %s", classname);
    }

    free(classname);
    return Descriptor_GetFromClassEntry(ce);
  } else {
    return NULL;
  }
}

Descriptor* Descriptor_GetFromFieldDef(const upb_fielddef *f) {
  return Descriptor_GetFromMessageDef(upb_fielddef_msgsubdef(f));
}

PHP_METHOD(Descriptor, getPublicDescriptor) {
  RETURN_ZVAL(getThis(), 1, 0);
}

PHP_METHOD(Descriptor, getFullName) {
  Descriptor *intern = (Descriptor*)Z_OBJ_P(getThis());
  RETURN_STRING(upb_msgdef_fullname(intern->msgdef));
}

PHP_METHOD(Descriptor, getField) {
  Descriptor *intern = (Descriptor*)Z_OBJ_P(getThis());
  int count = upb_msgdef_numfields(intern->msgdef);
  zval ret;
  zend_long index;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) ==
      FAILURE) {
    zend_error(E_USER_ERROR, "Expect integer for index.\n");
    return;
  }

  if (index < 0 || index >= count) {
    zend_error(E_USER_ERROR, "Cannot get element at %ld.\n", index);
    return;
  }

  upb_msg_field_iter iter;
  int i;
  for(upb_msg_field_begin(&iter, intern->msgdef), i = 0;
      !upb_msg_field_done(&iter) && i < index;
      upb_msg_field_next(&iter), i++);
  const upb_fielddef *field = upb_msg_iter_field(&iter);

  FieldDescriptor_FromFieldDef(&ret, field);
  RETURN_ZVAL(&ret, 1, 0);
}

PHP_METHOD(Descriptor, getFieldCount) {
  Descriptor *intern = (Descriptor*)Z_OBJ_P(getThis());
  RETURN_LONG(upb_msgdef_numfields(intern->msgdef));
}

PHP_METHOD(Descriptor, getOneofDecl) {
  Descriptor *intern = (Descriptor*)Z_OBJ_P(getThis());
  zend_long index;
  zval ret;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) ==
      FAILURE) {
    zend_error(E_USER_ERROR, "Expect integer for index.\n");
    return;
  }

  int field_num = upb_msgdef_numoneofs(intern->msgdef);
  if (index < 0 || index >= field_num) {
    zend_error(E_USER_ERROR, "Cannot get element at %ld.\n", index);
    return;
  }

  upb_msg_oneof_iter iter;
  int i;
  for(upb_msg_oneof_begin(&iter, intern->msgdef), i = 0;
      !upb_msg_oneof_done(&iter) && i < index;
      upb_msg_oneof_next(&iter), i++);
  const upb_oneofdef *oneof = upb_msg_iter_oneof(&iter);

  OneofDescriptor_FromOneofDef(&ret, oneof);
  RETURN_ZVAL(&ret, 1, 0);
}

PHP_METHOD(Descriptor, getOneofDeclCount) {
  Descriptor *intern = (Descriptor*)Z_OBJ_P(getThis());
  RETURN_LONG(upb_msgdef_numoneofs(intern->msgdef));
}

PHP_METHOD(Descriptor, getClass) {
  Descriptor *intern = (Descriptor*)Z_OBJ_P(getThis());
  const char* classname = ZSTR_VAL(intern->class_entry->name);
  RETURN_STRING(classname);
}


static zend_function_entry descriptor_methods[] = {
  PHP_ME(Descriptor, getClass, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Descriptor, getFullName, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Descriptor, getField, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Descriptor, getFieldCount, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Descriptor, getOneofDecl, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Descriptor, getOneofDeclCount, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Descriptor, getPublicDescriptor, NULL, ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

static void descriptor_init() {
  zend_class_entry tmp_ce;
  zend_object_handlers *h = &descriptor_object_handlers;

  INIT_CLASS_ENTRY(tmp_ce, "Google\\Protobuf\\Descriptor",
                   descriptor_methods);

  descriptor_ce = zend_register_internal_class(&tmp_ce);
  descriptor_ce->ce_flags |= ZEND_ACC_FINAL;
  descriptor_ce->create_object = descriptor_create;

  memcpy(h, &std_object_handlers, sizeof(zend_object_handlers));
  h->dtor_obj = descriptor_dtor;
}

// -----------------------------------------------------------------------------
// DescriptorPool
// -----------------------------------------------------------------------------

typedef struct DescriptorPool {
  zend_object std;
  upb_symtab *symtab;
} DescriptorPool;

zend_class_entry *descriptor_pool_ce;
static zend_object_handlers descriptor_pool_object_handlers;

static DescriptorPool *GetPool(const zval* this_ptr) {
  return (DescriptorPool*)Z_OBJ_P(this_ptr);
}

/**
 * Object handler to create an DescriptorPool.
 */
static zend_object* descriptor_pool_create(zend_class_entry *class_type) {
  DescriptorPool *intern = emalloc(sizeof(DescriptorPool));
  zend_object_std_init(&intern->std, class_type);
  intern->std.handlers = &descriptor_pool_object_handlers;
  intern->symtab = upb_symtab_new();
  // Skip object_properties_init(), we don't allow derived classes.
  return &intern->std;
}

/**
 * Object handler to free an DescriptorPool.
 */
static void descriptor_pool_dtor(zend_object* obj) {
  DescriptorPool* intern = (DescriptorPool*)obj;
  upb_symtab_free(intern->symtab);
  intern->symtab = NULL;
  zend_object_std_dtor(&intern->std);
}

void descriptor_pool_create_symtab(zval *zv, upb_symtab *symtab) {
  ZVAL_OBJ(zv, descriptor_pool_create(descriptor_pool_ce));

  if (symtab) {
    DescriptorPool *intern = GetPool(zv);
    upb_symtab_free(intern->symtab);
    intern->symtab = symtab;
  }
}

upb_symtab *descriptor_pool_steal(zval *zv) {
  DescriptorPool *intern = GetPool(zv);
  upb_symtab *ret = intern->symtab;

  // We're going to be destroyed immediately after this, but we create a new
  // empty symtab just to keep the object in a consistent state. The destructor
  // will delete this momentarily.
  intern->symtab = upb_symtab_new();

  return ret;
}

upb_symtab *descriptor_pool_getsymtab() {
  DescriptorPool *intern = GetPool(get_generated_pool());
  return intern->symtab;
}

PHP_METHOD(DescriptorPool, getGeneratedPool) {
  zval ret;
  ZVAL_COPY(&ret, get_generated_pool());
  RETURN_ZVAL(&ret, 0, 1);
}

PHP_METHOD(DescriptorPool, getDescriptorByClassName) {
  char *classname = NULL;
  zend_long classname_len;
  zend_class_entry *ce;
  zend_string *str;
  Descriptor *desc = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &classname,
                            &classname_len) == FAILURE) {
    return;
  }

  str = zend_string_init(classname, strlen(classname), 0);
  ce = zend_lookup_class(str);  // May autoload the class.
  zend_string_release (str);

  if (!ce) {
    RETURN_NULL();
  }

  desc = (Descriptor*)Descriptor_GetFromClassEntry(ce);

  if (desc) {
    zval ret;
    ZVAL_OBJ(&ret, &desc->std);
    RETURN_ZVAL(&ret, 1, 0);
  } else {
    RETURN_NULL();
  }
}

PHP_METHOD(DescriptorPool, getEnumDescriptorByClassName) {
  char *classname = NULL;
  zend_long classname_len;
  zend_class_entry *ce;
  zend_string *str;
  EnumDescriptor *desc = NULL;
  zval ret;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &classname,
                            &classname_len) == FAILURE) {
    return;
  }

  str = zend_string_init(classname, strlen(classname), 0);
  ce = zend_lookup_class(str);  // May autoload the class.
  zend_string_release (str);

  if (!ce) {
    RETURN_NULL();
  }

  EnumDescriptor_FromClassEntry(&ret, ce);
  RETURN_ZVAL(&ret, 1, 0);
}

PHP_METHOD(DescriptorPool, getDescriptorByProtoName) {
  DescriptorPool *intern = GetPool(getThis());
  char *protoname = NULL;
  zend_long protoname_len;
  zend_class_entry *ce;
  zend_string *str;
  const upb_msgdef *m;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &protoname,
                            &protoname_len) == FAILURE) {
    return;
  }

  if (*protoname == '.') protoname++;

  m = upb_symtab_lookupmsg(intern->symtab, protoname);

  if (m) {
    zval ret;
    ZVAL_OBJ(&ret, &Descriptor_GetFromMessageDef(m)->std);
    RETURN_ZVAL(&ret, 1, 0);
  } else {
    RETURN_NULL();
  }
}


bool depends_on_descriptor(const google_protobuf_FileDescriptorProto* file) {
  const upb_strview *deps;
  upb_strview name = upb_strview_makez("google/protobuf/descriptor.proto");
  size_t i, n;

  deps = google_protobuf_FileDescriptorProto_dependency(file, &n);
  for (i = 0; i < n; i++) {
    if (upb_strview_eql(deps[i], name)) {
      return true;
    }
  }

  return false;
}

static void add_name_mappings(const upb_filedef *file) {
  size_t i;
  for (i = 0; i < upb_filedef_msgcount(file); i++) {
    const upb_msgdef *m = upb_filedef_msg(file, i);
    pbphp_namemap_add(file, upb_msgdef_fullname(m), m);
  }

  for (i = 0; i < upb_filedef_enumcount(file); i++) {
    const upb_enumdef *e = upb_filedef_enum(file, i);
    pbphp_namemap_add(file, upb_enumdef_fullname(e), e);
  }
}

static void add_descriptor(DescriptorPool *pool, const char *data,
                           int data_len, upb_arena *arena) {
  size_t i, n;
  google_protobuf_FileDescriptorSet *set;
  const google_protobuf_FileDescriptorProto* const* files;

  set = google_protobuf_FileDescriptorSet_parse(data, data_len, arena);

  if (!set) {
    zend_error(E_ERROR, "Failed to parse binary descriptor\n");
    return;
  }

  files = google_protobuf_FileDescriptorSet_file(set, &n);

  for (i = 0; i < n; i++) {
    const google_protobuf_FileDescriptorProto* file = files[i];
    upb_strview name = google_protobuf_FileDescriptorProto_name(file);
    upb_status status;
    const upb_filedef *file_def;
    upb_status_clear(&status);

    if (upb_symtab_lookupfile2(pool->symtab, name.data, name.size)) {
      // Already added.
      continue;
    }

    // The PHP code generator currently special-cases descriptor.proto.  It
    // doesn't add it as a dependency even if the proto file actually does
    // depend on it.
    if (depends_on_descriptor(file)) {
      google_protobuf_FileDescriptorProto_getmsgdef(pool->symtab);
    }

    file_def = upb_symtab_addfile(pool->symtab, file, &status);
    check_upb_status(&status, "Unable to load descriptor");
    add_name_mappings(file_def);
  }
}

PHP_METHOD(DescriptorPool, internalAddGeneratedFile) {
  DescriptorPool *intern = GetPool(getThis());
  char *data = NULL;
  zend_long data_len;
  zend_bool use_nested_submsg = false;
  upb_arena *arena;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|b", &data, &data_len,
                            &use_nested_submsg) != SUCCESS) {
    return;
  }

  arena = upb_arena_new();
  add_descriptor(intern, data, data_len, arena);
  upb_arena_free(arena);
}

static zend_function_entry descriptor_pool_methods[] = {
  PHP_ME(DescriptorPool, getGeneratedPool, NULL,
         ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(DescriptorPool, getDescriptorByClassName, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(DescriptorPool, getDescriptorByProtoName, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(DescriptorPool, getEnumDescriptorByClassName, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(DescriptorPool, internalAddGeneratedFile, NULL, ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

static void descriptor_pool_init() {
  zend_class_entry tmp_ce;
  zend_object_handlers *h = &descriptor_pool_object_handlers;

  INIT_CLASS_ENTRY(tmp_ce, "Google\\Protobuf\\Internal\\DescriptorPool",
                   descriptor_pool_methods);

  descriptor_pool_ce = zend_register_internal_class(&tmp_ce);
  descriptor_pool_ce->ce_flags |= ZEND_ACC_FINAL;
  descriptor_pool_ce->create_object = descriptor_pool_create;

  memcpy(h, &std_object_handlers, sizeof(zend_object_handlers));
  h->dtor_obj = descriptor_pool_dtor;
}


// -----------------------------------------------------------------------------
// Module Init
// -----------------------------------------------------------------------------

void def_module_init() {
  oneof_descriptor_init();
  enum_value_descriptor_init();
  enum_descriptor_init();
  descriptor_pool_init();
  field_descriptor_init();
  descriptor_init();
}

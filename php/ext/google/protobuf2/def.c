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

#include "names.h"
#include "php-upb.h"
#include "protobuf.h"

static void check_upb_status(const upb_status* status, const char* msg) {
  if (!upb_ok(status)) {
    zend_error(E_ERROR, "%s: %s\n", msg, upb_status_errmsg(status));
  }
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

PHP_METHOD(DescriptorPool, getGeneratedPool) {
  zval ret;
  ZVAL_COPY(&ret, get_generated_pool());
  RETURN_ZVAL(&ret, 0, 1);
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

const Descriptor* pupb_getdesc(zend_class_entry *ce) {
  zval desc;
  Descriptor *ret;

  if (ce == NULL) return NULL;

  if (pbphp_cacheget(ce, &desc)) {
    ret = (Descriptor*)Z_OBJ_P(&desc);
    // This can never delete the descriptor, because we ensure that Desriptor and
    // DescriptorPool form a cycle that is never collected.
    zval_ptr_dtor(&desc);
  } else {
    ret = emalloc(sizeof(Descriptor));
    zend_object_std_init(&ret->std, descriptor_ce);
    ret->std.handlers = &descriptor_object_handlers;
    ret->class_entry = ce;
    ret->msgdef = pbphp_namemap_get(ce);

    // Skip object_properties_init(), we don't allow derived classes.
    pbphp_cacheadd(ce, &ret->std);

    ++GC_REFCOUNT(&ret->std);
  }


  return ret;
}

const Descriptor* pupb_getdesc_from_msgdef(const upb_msgdef *m) {
  if (m) {
    char *classname =
        pbphp_get_classname(upb_msgdef_file(m), upb_msgdef_fullname(m));
    zend_string *str = zend_string_init(classname, strlen(classname), 0);
    zend_class_entry *ce = zend_lookup_class(str);  // May autoload the class.

    zend_string_release (str);

    if (!ce) {
      zend_error(E_ERROR, "Couldn't load generated class %s", classname);
    }

    free(classname);
    return pupb_getdesc(ce);
  } else {
    return NULL;
  }
}

static zend_function_entry descriptor_methods[] = {
//  PHP_ME(Descriptor, getClass, NULL, ZEND_ACC_PUBLIC)
//  PHP_ME(Descriptor, getFullName, NULL, ZEND_ACC_PUBLIC)
//  PHP_ME(Descriptor, getField, NULL, ZEND_ACC_PUBLIC)
//  PHP_ME(Descriptor, getFieldCount, NULL, ZEND_ACC_PUBLIC)
//  PHP_ME(Descriptor, getOneofDecl, NULL, ZEND_ACC_PUBLIC)
//  PHP_ME(Descriptor, getOneofDeclCount, NULL, ZEND_ACC_PUBLIC)
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
// Module Init
// -----------------------------------------------------------------------------

void def_module_init() {
  descriptor_pool_init();
  descriptor_init();
}

#if 0

// Forward declare.
static void descriptor_init_c_instance(Descriptor* intern TSRMLS_DC);
static void descriptor_free_c(Descriptor* object TSRMLS_DC);

static void field_descriptor_init_c_instance(FieldDescriptor* intern TSRMLS_DC);
static void field_descriptor_free_c(FieldDescriptor* object TSRMLS_DC);

static void enum_descriptor_init_c_instance(EnumDescriptor* intern TSRMLS_DC);
static void enum_descriptor_free_c(EnumDescriptor* object TSRMLS_DC);

static void enum_value_descriptor_init_c_instance(
    EnumValueDescriptor *intern TSRMLS_DC);
static void enum_value_descriptor_free_c(EnumValueDescriptor *object TSRMLS_DC);

static void descriptor_pool_free_c(DescriptorPool* object TSRMLS_DC);
static void descriptor_pool_init_c_instance(DescriptorPool* pool TSRMLS_DC);

static void internal_descriptor_pool_free_c(
    InternalDescriptorPool *object TSRMLS_DC);
static void internal_descriptor_pool_init_c_instance(
    InternalDescriptorPool *pool TSRMLS_DC);

static void oneof_descriptor_free_c(Oneof* object TSRMLS_DC);
static void oneof_descriptor_init_c_instance(Oneof* pool TSRMLS_DC);

// -----------------------------------------------------------------------------
// Common Utilities
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// GPBType
// -----------------------------------------------------------------------------

zend_class_entry* gpb_type_type;

static zend_function_entry gpb_type_methods[] = {
  ZEND_FE_END
};

void gpb_type_init(TSRMLS_D) {
  zend_class_entry class_type;
  INIT_CLASS_ENTRY(class_type, "Google\\Protobuf\\Internal\\GPBType",
                   gpb_type_methods);
  gpb_type_type = zend_register_internal_class(&class_type TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("DOUBLE"),  1 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("FLOAT"),   2 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("INT64"),   3 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("UINT64"),  4 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("INT32"),   5 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("FIXED64"), 6 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("FIXED32"), 7 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("BOOL"),    8 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("STRING"),  9 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("GROUP"),   10 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("MESSAGE"), 11 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("BYTES"),   12 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("UINT32"),  13 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("ENUM"),    14 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("SFIXED32"),
                                   15 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("SFIXED64"),
                                   16 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("SINT32"), 17 TSRMLS_CC);
  zend_declare_class_constant_long(gpb_type_type, STR("SINT64"), 18 TSRMLS_CC);
}

DEFINE_CLASS(Descriptor, descriptor, "Google\\Protobuf\\Descriptor");

static void descriptor_free_c(Descriptor *self TSRMLS_DC) {
}

static void descriptor_init_c_instance(Descriptor *desc TSRMLS_DC) {
  desc->intern = NULL;
}

PHP_METHOD(Descriptor, getClass) {
  Descriptor* desc = UNBOX(Descriptor, getThis());
  DescriptorInternal* intern = desc->intern;
  register_class(intern, false TSRMLS_CC);
  const char* classname = ZSTR_VAL(intern->klass->name);
  PHP_PROTO_RETVAL_STRINGL(classname, strlen(classname), 1);
}

PHP_METHOD(Descriptor, getFullName) {
  Descriptor* desc = UNBOX(Descriptor, getThis());
  DescriptorInternal* intern = desc->intern;
  const char* fullname = upb_msgdef_fullname(intern->msgdef);
  PHP_PROTO_RETVAL_STRINGL(fullname, strlen(fullname), 1);
}

PHP_METHOD(Descriptor, getField) {
  long index;
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) ==
      FAILURE) {
    zend_error(E_USER_ERROR, "Expect integer for index.\n");
    return;
  }

  Descriptor* desc = UNBOX(Descriptor, getThis());
  DescriptorInternal* intern = desc->intern;
  int field_num = upb_msgdef_numfields(intern->msgdef);
  if (index < 0 || index >= field_num) {
    zend_error(E_USER_ERROR, "Cannot get element at %ld.\n", index);
    return;
  }

  upb_msg_field_iter iter;
  int i;
  for(upb_msg_field_begin(&iter, intern->msgdef), i = 0;
      !upb_msg_field_done(&iter) && i < index;
      upb_msg_field_next(&iter), i++);
  const upb_fielddef *field = upb_msg_iter_field(&iter);

  PHP_PROTO_HASHTABLE_VALUE field_hashtable_value = get_def_obj(field);
  if (field_hashtable_value == NULL) {
    field_hashtable_value =
        field_descriptor_type->create_object(field_descriptor_type TSRMLS_CC);
    GC_DELREF(field_hashtable_value);
    FieldDescriptor *field_php =
        UNBOX_HASHTABLE_VALUE(FieldDescriptor, field_hashtable_value);
    field_php->fielddef = field;
    add_def_obj(field, field_hashtable_value);
  }

  GC_ADDREF(field_hashtable_value);
  RETURN_OBJ(field_hashtable_value);
}

PHP_METHOD(Descriptor, getFieldCount) {
  Descriptor* desc = UNBOX(Descriptor, getThis());
  DescriptorInternal* intern = desc->intern;
  RETURN_LONG(upb_msgdef_numfields(intern->msgdef));
}

PHP_METHOD(Descriptor, getOneofDecl) {
  long index;
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) ==
      FAILURE) {
    zend_error(E_USER_ERROR, "Expect integer for index.\n");
    return;
  }

  Descriptor* desc = UNBOX(Descriptor, getThis());
  DescriptorInternal* intern = desc->intern;
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

  ZVAL_OBJ(return_value, oneof_descriptor_type->create_object(
                             oneof_descriptor_type TSRMLS_CC));
  Oneof *oneof_php = UNBOX(Oneof, return_value);
  oneof_php->oneofdef = oneof;
}

PHP_METHOD(Descriptor, getOneofDeclCount) {
  Descriptor* desc = UNBOX(Descriptor, getThis());
  DescriptorInternal* intern = desc->intern;
  RETURN_LONG(upb_msgdef_numoneofs(intern->msgdef));
}

// -----------------------------------------------------------------------------
// EnumDescriptor
// -----------------------------------------------------------------------------

static zend_function_entry enum_descriptor_methods[] = {
  PHP_ME(EnumDescriptor, getValue, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(EnumDescriptor, getValueCount, NULL, ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

DEFINE_CLASS(EnumDescriptor, enum_descriptor,
             "Google\\Protobuf\\EnumDescriptor");

static void enum_descriptor_free_c(EnumDescriptor *self TSRMLS_DC) {
}

static void enum_descriptor_init_c_instance(EnumDescriptor *self TSRMLS_DC) {
  self->intern = NULL;
}

PHP_METHOD(EnumDescriptor, getValue) {
  long index;
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) ==
      FAILURE) {
    zend_error(E_USER_ERROR, "Expect integer for index.\n");
    return;
  }

  EnumDescriptor *desc = UNBOX(EnumDescriptor, getThis());
  EnumDescriptorInternal *intern = desc->intern;
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

  ZVAL_OBJ(return_value, enum_value_descriptor_type->create_object(
                             enum_value_descriptor_type TSRMLS_CC));
  EnumValueDescriptor *enum_value_php =
      UNBOX(EnumValueDescriptor, return_value);
  enum_value_php->name = upb_enum_iter_name(&iter);
  enum_value_php->number = upb_enum_iter_number(&iter);
}

PHP_METHOD(EnumDescriptor, getValueCount) {
  EnumDescriptor *desc = UNBOX(EnumDescriptor, getThis());
  EnumDescriptorInternal *intern = desc->intern;
  RETURN_LONG(upb_enumdef_numvals(intern->enumdef));
}

// -----------------------------------------------------------------------------
// EnumValueDescriptor
// -----------------------------------------------------------------------------

static zend_function_entry enum_value_descriptor_methods[] = {
  PHP_ME(EnumValueDescriptor, getName, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(EnumValueDescriptor, getNumber, NULL, ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

DEFINE_CLASS(EnumValueDescriptor, enum_value_descriptor,
             "Google\\Protobuf\\EnumValueDescriptor");

static void enum_value_descriptor_free_c(EnumValueDescriptor *self TSRMLS_DC) {
}

static void enum_value_descriptor_init_c_instance(EnumValueDescriptor *self TSRMLS_DC) {
  self->name = NULL;
  self->number = 0;
}

PHP_METHOD(EnumValueDescriptor, getName) {
  EnumValueDescriptor *intern = UNBOX(EnumValueDescriptor, getThis());
  PHP_PROTO_RETVAL_STRINGL(intern->name, strlen(intern->name), 1);
}

PHP_METHOD(EnumValueDescriptor, getNumber) {
  EnumValueDescriptor *intern = UNBOX(EnumValueDescriptor, getThis());
  RETURN_LONG(intern->number);
}

// -----------------------------------------------------------------------------
// FieldDescriptor
// -----------------------------------------------------------------------------

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

DEFINE_CLASS(FieldDescriptor, field_descriptor,
             "Google\\Protobuf\\FieldDescriptor");

static void field_descriptor_free_c(FieldDescriptor *self TSRMLS_DC) {
}

static void field_descriptor_init_c_instance(FieldDescriptor *self TSRMLS_DC) {
  self->fielddef = NULL;
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
  FieldDescriptor *intern = UNBOX(FieldDescriptor, getThis());
  const char* name = upb_fielddef_name(intern->fielddef);
  PHP_PROTO_RETVAL_STRINGL(name, strlen(name), 1);
}

PHP_METHOD(FieldDescriptor, getNumber) {
  FieldDescriptor *intern = UNBOX(FieldDescriptor, getThis());
  RETURN_LONG(upb_fielddef_number(intern->fielddef));
}

PHP_METHOD(FieldDescriptor, getLabel) {
  FieldDescriptor *intern = UNBOX(FieldDescriptor, getThis());
  RETURN_LONG(upb_fielddef_label(intern->fielddef));
}

PHP_METHOD(FieldDescriptor, getType) {
  FieldDescriptor *intern = UNBOX(FieldDescriptor, getThis());
  RETURN_LONG(upb_fielddef_descriptortype(intern->fielddef));
}

PHP_METHOD(FieldDescriptor, isMap) {
  FieldDescriptor *intern = UNBOX(FieldDescriptor, getThis());
  RETURN_BOOL(upb_fielddef_ismap(intern->fielddef));
}

PHP_METHOD(FieldDescriptor, getEnumType) {
  FieldDescriptor *intern = UNBOX(FieldDescriptor, getThis());
  if (upb_fielddef_type(intern->fielddef) != UPB_TYPE_ENUM) {
    zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
                            "Cannot get enum type for non-enum field '%s'",
                            upb_fielddef_name(intern->fielddef));
    return;
  }
  const upb_enumdef *enumdef = upb_fielddef_enumsubdef(intern->fielddef);
  PHP_PROTO_HASHTABLE_VALUE desc_php = get_def_obj(enumdef);

  if (desc_php == NULL) {
    EnumDescriptorInternal* intern = get_enumdef_enumdesc(enumdef);

    desc_php =
        enum_descriptor_type->create_object(enum_descriptor_type TSRMLS_CC);
    GC_DELREF(desc_php);
    EnumDescriptor* desc = UNBOX_HASHTABLE_VALUE(EnumDescriptor, desc_php);
    desc->intern = intern;
    add_def_obj(enumdef, desc_php);
    add_ce_obj(intern->klass, desc_php);
  }

  GC_ADDREF(desc_php);
  RETURN_OBJ(desc_php);
}

PHP_METHOD(FieldDescriptor, getMessageType) {
  FieldDescriptor *intern = UNBOX(FieldDescriptor, getThis());
  if (upb_fielddef_type(intern->fielddef) != UPB_TYPE_MESSAGE) {
    zend_throw_exception_ex(
        NULL, 0 TSRMLS_CC, "Cannot get message type for non-message field '%s'",
        upb_fielddef_name(intern->fielddef));
    return;
  }
  const upb_msgdef *msgdef = upb_fielddef_msgsubdef(intern->fielddef);
  PHP_PROTO_HASHTABLE_VALUE desc_php = get_def_obj(msgdef);

  if (desc_php == NULL) {
    DescriptorInternal* intern = get_msgdef_desc(msgdef);

    desc_php =
        descriptor_type->create_object(descriptor_type TSRMLS_CC);
    GC_DELREF(desc_php);
    Descriptor* desc = UNBOX_HASHTABLE_VALUE(Descriptor, desc_php);
    desc->intern = intern;
    add_def_obj(msgdef, desc_php);
    add_ce_obj(intern->klass, desc_php);
  }

  GC_ADDREF(desc_php);
  RETURN_OBJ(desc_php);
}

// -----------------------------------------------------------------------------
// Oneof
// -----------------------------------------------------------------------------

static zend_function_entry oneof_descriptor_methods[] = {
  PHP_ME(Oneof, getName,  NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Oneof, getField, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Oneof, getFieldCount, NULL, ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

DEFINE_CLASS(Oneof, oneof_descriptor,
             "Google\\Protobuf\\OneofDescriptor");

static void oneof_descriptor_free_c(Oneof *self TSRMLS_DC) {
}

static void oneof_descriptor_init_c_instance(Oneof *self TSRMLS_DC) {
  self->oneofdef = NULL;
}

PHP_METHOD(Oneof, getName) {
  Oneof *intern = UNBOX(Oneof, getThis());
  const char *name = upb_oneofdef_name(intern->oneofdef);
  PHP_PROTO_RETVAL_STRINGL(name, strlen(name), 1);
}

PHP_METHOD(Oneof, getField) {
  long index;
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) ==
      FAILURE) {
    zend_error(E_USER_ERROR, "Expect integer for index.\n");
    return;
  }

  Oneof *intern = UNBOX(Oneof, getThis());
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

  PHP_PROTO_HASHTABLE_VALUE field_hashtable_value = get_def_obj(field);
  if (field_hashtable_value == NULL) {
    field_hashtable_value =
        field_descriptor_type->create_object(field_descriptor_type TSRMLS_CC);
    FieldDescriptor *field_php =
        UNBOX_HASHTABLE_VALUE(FieldDescriptor, field_hashtable_value);
    field_php->fielddef = field;
    add_def_obj(field, field_hashtable_value);
  }

  GC_ADDREF(field_hashtable_value);
  RETURN_OBJ(field_hashtable_value);
}

PHP_METHOD(Oneof, getFieldCount) {
  Oneof *intern = UNBOX(Oneof, getThis());
  RETURN_LONG(upb_oneofdef_numfields(intern->oneofdef));
}

// -----------------------------------------------------------------------------
// DescriptorPool
// -----------------------------------------------------------------------------

static zend_function_entry descriptor_pool_methods[] = {
  PHP_ME(DescriptorPool, getGeneratedPool, NULL,
         ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(DescriptorPool, getDescriptorByClassName, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(DescriptorPool, getEnumDescriptorByClassName, NULL, ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

static zend_function_entry internal_descriptor_pool_methods[] = {
  PHP_ME(InternalDescriptorPool, getGeneratedPool, NULL,
         ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(InternalDescriptorPool, internalAddGeneratedFile, NULL, ZEND_ACC_PUBLIC)
  ZEND_FE_END
};

DEFINE_CLASS(DescriptorPool, descriptor_pool,
             "Google\\Protobuf\\DescriptorPool");
DEFINE_CLASS(InternalDescriptorPool, internal_descriptor_pool,
             "Google\\Protobuf\\Internal\\DescriptorPool");

// wrapper of generated pool
zend_object *generated_pool_php;
zend_object *internal_generated_pool_php;
InternalDescriptorPoolImpl *generated_pool;
InternalDescriptorPoolImpl generated_pool_impl;  // The actual generated pool

void init_generated_pool_once(TSRMLS_D) {
  if (generated_pool == NULL) {
    internal_generated_pool_php = internal_descriptor_pool_type->create_object(
        internal_descriptor_pool_type TSRMLS_CC);
    generated_pool_php =
        descriptor_pool_type->create_object(descriptor_pool_type TSRMLS_CC);
    generated_pool = &generated_pool_impl;
  }
}

static void internal_descriptor_pool_init_c_instance(
    InternalDescriptorPool *pool TSRMLS_DC) {
  pool->intern = &generated_pool_impl;
}

static void internal_descriptor_pool_free_c(
    InternalDescriptorPool *pool TSRMLS_DC) {
}

void internal_descriptor_pool_impl_init(
    InternalDescriptorPoolImpl *pool TSRMLS_DC) {
  pool->symtab = upb_symtab_new();
  pool->fill_handler_cache =
      upb_handlercache_new(add_handlers_for_message, NULL);
  pool->pb_serialize_handler_cache = upb_pb_encoder_newcache();
  pool->json_serialize_handler_cache = upb_json_printer_newcache(false);
  pool->json_serialize_handler_preserve_cache = upb_json_printer_newcache(true);
  pool->fill_method_cache = upb_pbcodecache_new(pool->fill_handler_cache);
  pool->json_fill_method_cache = upb_json_codecache_new();
}

void internal_descriptor_pool_impl_destroy(
    InternalDescriptorPoolImpl *pool TSRMLS_DC) {
  upb_symtab_free(pool->symtab);
  upb_handlercache_free(pool->fill_handler_cache);
  upb_handlercache_free(pool->pb_serialize_handler_cache);
  upb_handlercache_free(pool->json_serialize_handler_cache);
  upb_handlercache_free(pool->json_serialize_handler_preserve_cache);
  upb_pbcodecache_free(pool->fill_method_cache);
  upb_json_codecache_free(pool->json_fill_method_cache);
}

static void descriptor_pool_init_c_instance(DescriptorPool *pool TSRMLS_DC) {
  PBPHP_ASSERT(generated_pool != NULL);
  pool->intern = generated_pool;
}

static void descriptor_pool_free_c(DescriptorPool *pool TSRMLS_DC) {
}

PHP_METHOD(DescriptorPool, getGeneratedPool) {
  init_generated_pool_once(TSRMLS_C);
  GC_ADDREF(generated_pool_php);
  RETURN_OBJ(generated_pool_php);
}

PHP_METHOD(InternalDescriptorPool, getGeneratedPool) {
  init_generated_pool_once(TSRMLS_C);
  GC_ADDREF(internal_generated_pool_php);
  RETURN_OBJ(internal_generated_pool_php);
}

PHP_METHOD(DescriptorPool, getDescriptorByClassName) {
  char *classname = NULL;
  PHP_PROTO_SIZE classname_len;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &classname,
                            &classname_len) == FAILURE) {
    return;
  }

  PHP_PROTO_CE_DECLARE pce;
  if (php_proto_zend_lookup_class(classname, classname_len, &pce) ==
      FAILURE) {
    RETURN_NULL();
  }

  PHP_PROTO_HASHTABLE_VALUE desc_php = get_ce_obj(PHP_PROTO_CE_UNREF(pce));
  if (desc_php == NULL) {
    DescriptorInternal* intern = get_ce_desc(PHP_PROTO_CE_UNREF(pce));
    if (intern == NULL) {
      RETURN_NULL();
    }

    desc_php =
        descriptor_type->create_object(descriptor_type TSRMLS_CC);
    GC_DELREF(desc_php);
    Descriptor* desc = UNBOX_HASHTABLE_VALUE(Descriptor, desc_php);
    desc->intern = intern;
    add_def_obj(intern->msgdef, desc_php);
    add_ce_obj(PHP_PROTO_CE_UNREF(pce), desc_php);
  }

  zend_class_entry* instance_ce = HASHTABLE_VALUE_CE(desc_php);

  if (!instanceof_function(instance_ce, descriptor_type TSRMLS_CC)) {
    RETURN_NULL();
  }

  GC_ADDREF(desc_php);
  RETURN_OBJ(desc_php);
}

PHP_METHOD(DescriptorPool, getEnumDescriptorByClassName) {
  char *classname = NULL;
  PHP_PROTO_SIZE classname_len;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &classname,
                            &classname_len) == FAILURE) {
    return;
  }

  PHP_PROTO_CE_DECLARE pce;
  if (php_proto_zend_lookup_class(classname, classname_len, &pce) ==
      FAILURE) {
    RETURN_NULL();
  }

  zend_class_entry* ce = PHP_PROTO_CE_UNREF(pce);

  PHP_PROTO_HASHTABLE_VALUE desc_php = get_ce_obj(ce);
  if (desc_php == NULL) {
    EnumDescriptorInternal* intern = get_class_enumdesc(ZSTR_VAL(ce->name));
    register_class(intern, true TSRMLS_CC);

    if (intern == NULL) {
      RETURN_NULL();
    }

    desc_php =
        enum_descriptor_type->create_object(enum_descriptor_type TSRMLS_CC);
    GC_DELREF(desc_php);
    EnumDescriptor* desc = UNBOX_HASHTABLE_VALUE(EnumDescriptor, desc_php);
    desc->intern = intern;
    add_def_obj(intern->enumdef, desc_php);
    add_ce_obj(ce, desc_php);
  }

  zend_class_entry* instance_ce = HASHTABLE_VALUE_CE(desc_php);

  if (!instanceof_function(instance_ce, enum_descriptor_type TSRMLS_CC)) {
    RETURN_NULL();
  }

  GC_ADDREF(desc_php);
  RETURN_OBJ(desc_php);
}
#endif

// Protocol Buffers - Google's data interchange format
// Copyright 2014 Google Inc.  All rights reserved.
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

#ifndef __GOOGLE_PROTOBUF_RUBY_PROTOBUF_H__
#define __GOOGLE_PROTOBUF_RUBY_PROTOBUF_H__

#include <ruby/ruby.h>
#include <ruby/vm.h>
#include <ruby/encoding.h>

#include "upb.h"

// Forward decls.
struct Arena;
struct DescriptorPool;
struct Descriptor;
struct FileDescriptor;
struct FieldDescriptor;
struct EnumDescriptor;
struct MessageHeader;
struct MessageBuilderContext;
struct EnumBuilderContext;
struct FileBuilderContext;
struct Builder;

typedef struct Arena Arena;
typedef struct DescriptorPool DescriptorPool;
typedef struct Descriptor Descriptor;
typedef struct FileDescriptor FileDescriptor;
typedef struct FieldDescriptor FieldDescriptor;
typedef struct OneofDescriptor OneofDescriptor;
typedef struct EnumDescriptor EnumDescriptor;
typedef struct MessageHeader MessageHeader;
typedef struct MessageBuilderContext MessageBuilderContext;
typedef struct OneofBuilderContext OneofBuilderContext;
typedef struct EnumBuilderContext EnumBuilderContext;
typedef struct FileBuilderContext FileBuilderContext;
typedef struct Builder Builder;

struct Arena {
  VALUE rb_objs;  // WeakMap of void* -> VALUE
  upb_arena *arena;
};

Arena* ruby_to_Arena(VALUE arena);

/*
 It can be a bit confusing how the C structs defined below and the Ruby
 objects interact and hold references to each other. First, a few principles:

 - Ruby's "TypedData" abstraction lets a Ruby VALUE hold a pointer to a C
   struct (or arbitrary memory chunk), own it, and free it when collected.
   Thus, each struct below will have a corresponding Ruby object
   wrapping/owning it.

 - To get back from an underlying upb {msg,enum}def to the Ruby object, we
   keep a global hashmap, accessed by get_def_obj/add_def_obj below.

 The in-memory structure is then something like:

   Ruby                        |      upb
                               |
   DescriptorPool  ------------|-----------> upb_symtab____________________
                               |                | (message types)          \
                               |                v                           \
   Descriptor   ---------------|-----------> upb_msgdef         (enum types)|
    |--> msgclass              |                |   ^                       |
    |    (dynamically built)   |                |   | (submsg fields)       |
    |--> MessageLayout         |                |   |                       /
    |--------------------------|> decoder method|   |                      /
    \--------------------------|> serialize     |   |                     /
                               |  handlers      v   |                    /
   FieldDescriptor  -----------|-----------> upb_fielddef               /
                               |                    |                  /
                               |                    v (enum fields)   /
   EnumDescriptor  ------------|-----------> upb_enumdef  <----------'
                               |
                               |
               ^               |               \___/
               `---------------|-----------------'    (get_def_obj map)
 */

// -----------------------------------------------------------------------------
// Ruby class structure definitions.
// -----------------------------------------------------------------------------

struct DescriptorPool {
  upb_symtab* symtab;
  VALUE rb_objs;  // WeakMap of void* -> VALUE
  VALUE arena;
};

struct Descriptor {
  const upb_msgdef* msgdef;
  VALUE klass;
  VALUE descriptor_pool;
};

struct FileDescriptor {
  const upb_filedef* filedef;
  VALUE descriptor_pool;  // Owns the upb_filedef.
};

struct FieldDescriptor {
  const upb_fielddef* fielddef;
  VALUE descriptor_pool;  // Owns the upb_fielddef.
};

struct OneofDescriptor {
  const upb_oneofdef* oneofdef;
  VALUE descriptor_pool;  // Owns the upb_oneofdef.
};

struct EnumDescriptor {
  const upb_enumdef* enumdef;
  VALUE module;  // begins as nil
  VALUE descriptor_pool;  // Owns the upb_enumdef.
};

struct MessageBuilderContext {
  google_protobuf_DescriptorProto* msg_proto;
  VALUE file_builder;
};

struct OneofBuilderContext {
  int oneof_index;
  VALUE message_builder;
};

struct EnumBuilderContext {
  google_protobuf_EnumDescriptorProto* enum_proto;
  VALUE file_builder;
};

struct FileBuilderContext {
  google_protobuf_FileDescriptorProto* file_proto;
  VALUE arena;
  VALUE descriptor_pool;
};

struct Builder {
  VALUE descriptor_pool;
  VALUE default_file_builder;
};

extern VALUE cArena;
extern VALUE cDescriptorPool;
extern VALUE cDescriptor;
extern VALUE cFileDescriptor;
extern VALUE cFieldDescriptor;
extern VALUE cEnumDescriptor;
extern VALUE cMessageBuilderContext;
extern VALUE cOneofBuilderContext;
extern VALUE cEnumBuilderContext;
extern VALUE cFileBuilderContext;
extern VALUE cBuilder;

extern VALUE cError;
extern VALUE cParseError;
extern VALUE cTypeError;

// We forward-declare all of the Ruby method implementations here because we
// sometimes call the methods directly across .c files, rather than going
// through Ruby's method dispatching (e.g. during message parse). It's cleaner
// to keep the list of object methods together than to split them between
// static-in-file definitions and header declarations.

void DescriptorPool_mark(void* _self);
void DescriptorPool_free(void* _self);
VALUE DescriptorPool_alloc(VALUE klass);
void DescriptorPool_register(VALUE module);
DescriptorPool* ruby_to_DescriptorPool(VALUE value);
VALUE DescriptorPool_build(int argc, VALUE* argv, VALUE _self);
VALUE DescriptorPool_lookup(VALUE _self, VALUE name);
VALUE DescriptorPool_generated_pool(VALUE _self);

extern VALUE generated_pool;

void Descriptor_mark(void* _self);
VALUE Descriptor_alloc(VALUE klass);
void Descriptor_register(VALUE module);
Descriptor* ruby_to_Descriptor(VALUE value);
VALUE Descriptor_initialize(VALUE _self, VALUE cookie, VALUE descriptor_pool,
                            VALUE ptr);
VALUE Descriptor_name(VALUE _self);
VALUE Descriptor_each(VALUE _self);
VALUE Descriptor_lookup(VALUE _self, VALUE name);
VALUE Descriptor_each_oneof(VALUE _self);
VALUE Descriptor_lookup_oneof(VALUE _self, VALUE name);
VALUE Descriptor_msgclass(VALUE _self);
VALUE Descriptor_file_descriptor(VALUE _self);
extern const rb_data_type_t _Descriptor_type;

void FileDescriptor_mark(void* _self);
VALUE FileDescriptor_alloc(VALUE klass);
void FileDescriptor_register(VALUE module);
FileDescriptor* ruby_to_FileDescriptor(VALUE value);
VALUE FileDescriptor_initialize(VALUE _self, VALUE cookie,
                                VALUE descriptor_pool, VALUE ptr);
VALUE FileDescriptor_name(VALUE _self);
VALUE FileDescriptor_syntax(VALUE _self);

void FieldDescriptor_mark(void* _self);
VALUE FieldDescriptor_alloc(VALUE klass);
void FieldDescriptor_register(VALUE module);
FieldDescriptor* ruby_to_FieldDescriptor(VALUE value);
VALUE FieldDescriptor_initialize(VALUE _self, VALUE cookie,
                                 VALUE descriptor_pool, VALUE ptr);
VALUE FieldDescriptor_name(VALUE _self);
VALUE FieldDescriptor_type(VALUE _self);
VALUE FieldDescriptor_default(VALUE _self);
VALUE FieldDescriptor_label(VALUE _self);
VALUE FieldDescriptor_number(VALUE _self);
VALUE FieldDescriptor_submsg_name(VALUE _self);
VALUE FieldDescriptor_subtype(VALUE _self);
VALUE FieldDescriptor_has(VALUE _self, VALUE msg_rb);
VALUE FieldDescriptor_clear(VALUE _self, VALUE msg_rb);
VALUE FieldDescriptor_get(VALUE _self, VALUE msg_rb);
VALUE FieldDescriptor_set(VALUE _self, VALUE msg_rb, VALUE value);
upb_fieldtype_t ruby_to_fieldtype(VALUE type);
VALUE fieldtype_to_ruby(upb_fieldtype_t type);

void OneofDescriptor_mark(void* _self);
VALUE OneofDescriptor_alloc(VALUE klass);
void OneofDescriptor_register(VALUE module);
OneofDescriptor* ruby_to_OneofDescriptor(VALUE value);
VALUE OneofDescriptor_initialize(VALUE _self, VALUE cookie,
                                 VALUE descriptor_pool, VALUE ptr);
VALUE OneofDescriptor_name(VALUE _self);
VALUE OneofDescriptor_each(VALUE _self);

void EnumDescriptor_mark(void* _self);
VALUE EnumDescriptor_alloc(VALUE klass);
VALUE EnumDescriptor_initialize(VALUE _self, VALUE cookie,
                                VALUE descriptor_pool, VALUE ptr);
void EnumDescriptor_register(VALUE module);
EnumDescriptor* ruby_to_EnumDescriptor(VALUE value);
VALUE EnumDescriptor_file_descriptor(VALUE _self);
VALUE EnumDescriptor_name(VALUE _self);
VALUE EnumDescriptor_lookup_name(VALUE _self, VALUE name);
VALUE EnumDescriptor_lookup_value(VALUE _self, VALUE number);
VALUE EnumDescriptor_each(VALUE _self);
VALUE EnumDescriptor_enummodule(VALUE _self);
extern const rb_data_type_t _EnumDescriptor_type;

void MessageBuilderContext_mark(void* _self);
VALUE MessageBuilderContext_alloc(VALUE klass);
void MessageBuilderContext_register(VALUE module);
MessageBuilderContext* ruby_to_MessageBuilderContext(VALUE value);
VALUE MessageBuilderContext_initialize(VALUE _self,
                                       VALUE _file_builder,
                                       VALUE name);
VALUE MessageBuilderContext_optional(int argc, VALUE* argv, VALUE _self);
VALUE MessageBuilderContext_required(int argc, VALUE* argv, VALUE _self);
VALUE MessageBuilderContext_repeated(int argc, VALUE* argv, VALUE _self);
VALUE MessageBuilderContext_map(int argc, VALUE* argv, VALUE _self);
VALUE MessageBuilderContext_oneof(VALUE _self, VALUE name);

void OneofBuilderContext_mark(void* _self);
VALUE OneofBuilderContext_alloc(VALUE klass);
void OneofBuilderContext_register(VALUE module);
OneofBuilderContext* ruby_to_OneofBuilderContext(VALUE value);
VALUE OneofBuilderContext_initialize(VALUE _self,
                                     VALUE descriptor,
                                     VALUE builder);
VALUE OneofBuilderContext_optional(int argc, VALUE* argv, VALUE _self);

void EnumBuilderContext_mark(void* _self);
void EnumBuilderContext_free(void* _self);
VALUE EnumBuilderContext_alloc(VALUE klass);
void EnumBuilderContext_register(VALUE module);
EnumBuilderContext* ruby_to_EnumBuilderContext(VALUE value);
VALUE EnumBuilderContext_initialize(VALUE _self, VALUE _file_builder,
                                    VALUE name);
VALUE EnumBuilderContext_value(VALUE _self, VALUE name, VALUE number);

void FileBuilderContext_mark(void* _self);
void FileBuilderContext_free(void* _self);
VALUE FileBuilderContext_alloc(VALUE klass);
void FileBuilderContext_register(VALUE module);
FileBuilderContext* ruby_to_FileBuilderContext(VALUE _self);
upb_strview FileBuilderContext_strdup(VALUE _self, VALUE rb_str);
upb_strview FileBuilderContext_strdup_name(VALUE _self, VALUE rb_str);
upb_strview FileBuilderContext_strdup_sym(VALUE _self, VALUE rb_sym);
VALUE FileBuilderContext_initialize(VALUE _self, VALUE descriptor_pool,
                                    VALUE name, VALUE options);
VALUE FileBuilderContext_add_message(VALUE _self, VALUE name);
VALUE FileBuilderContext_add_enum(VALUE _self, VALUE name);
VALUE FileBuilderContext_pending_descriptors(VALUE _self);

void Builder_mark(void* _self);
void Builder_free(void* _self);
VALUE Builder_alloc(VALUE klass);
void Builder_register(VALUE module);
Builder* ruby_to_Builder(VALUE value);
VALUE Builder_build(VALUE _self);
VALUE Builder_initialize(VALUE _self, VALUE descriptor_pool);
VALUE Builder_add_file(int argc, VALUE *argv, VALUE _self);
VALUE Builder_add_message(VALUE _self, VALUE name);
VALUE Builder_add_enum(VALUE _self, VALUE name);
VALUE Builder_finalize_to_pool(VALUE _self, VALUE pool_rb);

extern rb_encoding* kRubyStringUtf8Encoding;
extern rb_encoding* kRubyStringASCIIEncoding;
extern rb_encoding* kRubyString8bitEncoding;

// -----------------------------------------------------------------------------
// Repeated field container type.
// -----------------------------------------------------------------------------

typedef struct {
  upb_array *arr;
  upb_fieldtype_t field_type;
  VALUE field_type_class;
  VALUE arena;
} RepeatedField;

void RepeatedField_mark(void* self);
void RepeatedField_free(void* self);
VALUE RepeatedField_alloc(VALUE klass);
VALUE RepeatedField_init(int argc, VALUE* argv, VALUE self);
void RepeatedField_register(VALUE module);

extern const rb_data_type_t RepeatedField_type;
extern VALUE cRepeatedField;

RepeatedField* ruby_to_RepeatedField(VALUE value);

VALUE RepeatedField_new_this_type(VALUE _self);
VALUE RepeatedField_each(VALUE _self);
VALUE RepeatedField_index(int argc, VALUE* argv, VALUE _self);
void* RepeatedField_index_native(VALUE _self, int index);
int RepeatedField_size(VALUE _self);
VALUE RepeatedField_index_set(VALUE _self, VALUE _index, VALUE val);
void RepeatedField_reserve(RepeatedField* self, int new_size);
VALUE RepeatedField_push(VALUE _self, VALUE val);
void RepeatedField_push_native(VALUE _self, void* data);
VALUE RepeatedField_pop_one(VALUE _self);
VALUE RepeatedField_insert(int argc, VALUE* argv, VALUE _self);
VALUE RepeatedField_replace(VALUE _self, VALUE list);
VALUE RepeatedField_clear(VALUE _self);
VALUE RepeatedField_length(VALUE _self);
VALUE RepeatedField_dup(VALUE _self);
VALUE RepeatedField_deep_copy(VALUE _self);
VALUE RepeatedField_to_ary(VALUE _self);
VALUE RepeatedField_eq(VALUE _self, VALUE _other);
VALUE RepeatedField_hash(VALUE _self);
VALUE RepeatedField_inspect(VALUE _self);
VALUE RepeatedField_plus(VALUE _self, VALUE list);

// Defined in repeated_field.c; also used by Map.
void validate_type_class(upb_fieldtype_t type, VALUE klass);

// -----------------------------------------------------------------------------
// Map container type.
// -----------------------------------------------------------------------------

typedef struct {
  upb_map *map;
  upb_fieldtype_t key_type;
  upb_fieldtype_t value_type;
  VALUE value_type_class;
  VALUE arena;
} Map;

void Map_mark(void* self);
void Map_free(void* self);
VALUE Map_alloc(VALUE klass);
VALUE Map_init(int argc, VALUE* argv, VALUE self);
void Map_register(VALUE module);
VALUE Map_set_frame(VALUE self, VALUE val);

extern const rb_data_type_t Map_type;
extern VALUE cMap;

Map* ruby_to_Map(VALUE value);

VALUE Map_new_this_type(VALUE _self);
VALUE Map_each(VALUE _self);
VALUE Map_keys(VALUE _self);
VALUE Map_values(VALUE _self);
VALUE Map_index(VALUE _self, VALUE key);
VALUE Map_index_set(VALUE _self, VALUE key, VALUE value);
VALUE Map_has_key(VALUE _self, VALUE key);
VALUE Map_delete(VALUE _self, VALUE key);
VALUE Map_clear(VALUE _self);
VALUE Map_length(VALUE _self);
VALUE Map_dup(VALUE _self);
VALUE Map_deep_copy(VALUE _self);
VALUE Map_eq(VALUE _self, VALUE _other);
VALUE Map_hash(VALUE _self);
VALUE Map_to_h(VALUE _self);
VALUE Map_inspect(VALUE _self);
VALUE Map_merge(VALUE _self, VALUE hashmap);
VALUE Map_merge_into_self(VALUE _self, VALUE hashmap);

typedef struct {
  Map* self;
  upb_strtable_iter it;
} Map_iter;

void Map_begin(VALUE _self, Map_iter* iter);
void Map_next(Map_iter* iter);
bool Map_done(Map_iter* iter);
VALUE Map_iter_key(Map_iter* iter);
VALUE Map_iter_value(Map_iter* iter);

// -----------------------------------------------------------------------------
// Message type
// -----------------------------------------------------------------------------

struct MessageHeader {
  upb_msg *msg;
  Descriptor* descriptor;      // kept alive by self.class.descriptor reference.
  VALUE arena;
};

extern rb_data_type_t Message_type;

VALUE build_class_from_descriptor(VALUE descriptor);
void* Message_data(void* msg);
void Message_mark(void* self);
void Message_free(void* self);
VALUE Message_alloc(VALUE klass);
VALUE Message_method_missing(int argc, VALUE* argv, VALUE _self);
VALUE Message_initialize(int argc, VALUE* argv, VALUE _self);
VALUE Message_dup(VALUE _self);
VALUE Message_deep_copy(VALUE _self);
VALUE Message_eq(VALUE _self, VALUE _other);
VALUE Message_hash(VALUE _self);
VALUE Message_inspect(VALUE _self);
VALUE Message_to_h(VALUE _self);
VALUE Message_index(VALUE _self, VALUE field_name);
VALUE Message_index_set(VALUE _self, VALUE field_name, VALUE value);
VALUE Message_descriptor(VALUE klass);
VALUE Message_decode(VALUE klass, VALUE data);
VALUE Message_encode(VALUE klass, VALUE msg_rb);
VALUE Message_decode_json(int argc, VALUE* argv, VALUE klass);
VALUE Message_encode_json(int argc, VALUE* argv, VALUE klass);

VALUE Google_Protobuf_discard_unknown(VALUE self, VALUE msg_rb);
VALUE Google_Protobuf_deep_copy(VALUE self, VALUE obj);

VALUE build_module_from_enumdesc(VALUE _enumdesc);
VALUE enum_lookup(VALUE self, VALUE number);
VALUE enum_resolve(VALUE self, VALUE sym);
VALUE enum_descriptor(VALUE self);

// Maximum depth allowed during encoding, to avoid stack overflows due to
// cycles.
#define ENCODE_MAX_NESTING 63

// -----------------------------------------------------------------------------
// A cache of frozen string objects to use as field defaults.
// -----------------------------------------------------------------------------
VALUE get_frozen_string(const char* data, size_t size, bool binary);

// -----------------------------------------------------------------------------
// Global map from upb {msg,enum}defs to wrapper Descriptor/EnumDescriptor
// instances.
// -----------------------------------------------------------------------------
VALUE get_msgdef_obj(VALUE descriptor_pool, const upb_msgdef* def);
VALUE get_enumdef_obj(VALUE descriptor_pool, const upb_enumdef* def);
VALUE get_fielddef_obj(VALUE descriptor_pool, const upb_fielddef* def);
VALUE get_filedef_obj(VALUE descriptor_pool, const upb_filedef* def);
VALUE get_oneofdef_obj(VALUE descriptor_pool, const upb_oneofdef* def);

// -----------------------------------------------------------------------------
// Utilities.
// -----------------------------------------------------------------------------

VALUE weakmap_new();

void check_upb_status(const upb_status* status, const char* msg);

#define CHECK_UPB(code, msg) do {                                             \
    upb_status status = UPB_STATUS_INIT;                                      \
    code;                                                                     \
    check_upb_status(&status, msg);                                           \
} while (0)

extern ID descriptor_instancevar_interned;

// A distinct object that is not accessible from Ruby.  We use this as a
// constructor argument to enforce that certain objects cannot be created from
// Ruby.
extern VALUE c_only_cookie;

#ifdef NDEBUG
#define UPB_ASSERT(expr) do {} while (false && (expr))
#else
#define UPB_ASSERT(expr) assert(expr)
#endif

#define UPB_UNUSED(var) (void)var

#endif  // __GOOGLE_PROTOBUF_RUBY_PROTOBUF_H__

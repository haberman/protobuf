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

#include "protobuf.h"

/*
 * call-seq:
 *     MessageClass.decode(data) => message
 *
 * Decodes the given data (as a string containing bytes in protocol buffers wire
 * format) under the interpretration given by this message class's definition
 * and returns a message object with the corresponding field values.
 */
VALUE Message_decode(VALUE klass, VALUE data) {
  VALUE descriptor = rb_ivar_get(klass, descriptor_instancevar_interned);
  Descriptor* desc = ruby_to_Descriptor(descriptor);
  VALUE msgklass = Descriptor_msgclass(descriptor);
  VALUE msg_rb = rb_class_new_instance(0, NULL, msgklass);
  const upb_msglayout *layout = upb_msgdef_layout(desc->msgdef);
  MessageHeader* msg;
  Arena* arena;
  const char *buf;
  size_t len;

  if (TYPE(data) != T_STRING) {
    rb_raise(rb_eArgError, "Expected string for binary protobuf data.");
  }

  TypedData_Get_Struct(msg_rb, MessageHeader, &Message_type, msg);
  TypedData_Get_Struct(msg->arena, Arena, &Arena_type, arena);
  buf = RSTRING_PTR(data);
  len = RSTRING_LEN(data);

  if (!upb_decode(buf, len, msg->msg, layout, arena->arena)) {
    rb_raise(rb_eRuntimeError, "binary protobuf failed to parse");
  }

  return msg_rb;
}

/*
 * call-seq:
 *     MessageClass.decode_json(data, options = {}) => message
 *
 * Decodes the given data (as a string containing bytes in protocol buffers wire
 * format) under the interpretration given by this message class's definition
 * and returns a message object with the corresponding field values.
 *
 *  @param options [Hash] options for the decoder
 *   ignore_unknown_fields: set true to ignore unknown fields (default is to
 *   raise an error)
 */
VALUE Message_decode_json(int argc, VALUE* argv, VALUE klass) {
  VALUE descriptor = rb_ivar_get(klass, descriptor_instancevar_interned);
  Descriptor* desc = ruby_to_Descriptor(descriptor);
  DescriptorPool* pool = ruby_to_DescriptorPool(generated_pool);
  VALUE msgklass = Descriptor_msgclass(descriptor);
  VALUE msg_rb = rb_class_new_instance(0, NULL, msgklass);
  VALUE data;
  int options = 0;

  if (argc < 1 || argc > 2) {
    rb_raise(rb_eArgError, "Expected 1 or 2 arguments.");
  } else if (TYPE(argv[0]) != T_STRING) {
    rb_raise(rb_eArgError, "Expected string for JSON data.");
  } else if (argc == 2 && TYPE(argv[1]) != T_HASH) {
    rb_raise(rb_eArgError, "Expected hash arguments.");
  }

  data = argv[0];

  if (argc == 2) {
    VALUE hash_args = argv[1];
    VALUE ignore_key = ID2SYM(rb_intern("ignore_unknown_fields"));
    VALUE ignore = rb_hash_lookup2(hash_args, ignore_key, Qfalse);;

    if (RTEST(ignore)) options |= UPB_JSONDEC_IGNOREUNKNOWN;
  }

  // TODO(cfallin): Check and respect string encoding. If not UTF-8, we need to
  // convert, because string handlers pass data directly to message string
  // fields.

  {
    MessageHeader* msg;
    Arena* arena;
    upb_status status;
    const char* buf = RSTRING_PTR(data);
    size_t len = RSTRING_LEN(data);

    TypedData_Get_Struct(msg_rb, MessageHeader, &Message_type, msg);
    TypedData_Get_Struct(msg->arena, Arena, &Arena_type, arena);
    buf = RSTRING_PTR(data);

    if (!upb_json_decode(buf, len, msg->msg, desc->msgdef, pool->symtab,
                         options, arena->arena, &status)) {
      rb_raise(rb_eRuntimeError, "Error parsing JSON: %s",
               upb_status_errmsg(&status));
    }
  }

  return msg_rb;
}

// -----------------------------------------------------------------------------
// Serializing.
// -----------------------------------------------------------------------------

/*
 * call-seq:
 *     MessageClass.encode(msg) => bytes
 *
 * Encodes the given message object to its serialized form in protocol buffers
 * wire format.
 */
VALUE Message_encode(VALUE klass, VALUE msg_rb) {
  VALUE descriptor = rb_ivar_get(klass, descriptor_instancevar_interned);
  Descriptor* desc = ruby_to_Descriptor(descriptor);
  upb_arena *arena = upb_arena_new();
  upb_msg *msg;
  size_t size;
  char *data;

  TypedData_Get_Struct(msg_rb, MessageHeader, &Message_type, msg);
  data = upb_encode(msg->msg, upb_msgdef_layout(desc->msgdef), arena->arena,
                    &size);

  if (!data) {
    rb_raise(rb_eRuntimeError, "Error serializing protobuf");
  }

  return rb_str_new(data, size);
}

/*
 * call-seq:
 *     MessageClass.encode_json(msg, options = {}) => json_string
 *
 * Encodes the given message object into its serialized JSON representation.
 * @param options [Hash] options for the decoder
 *  preserve_proto_fieldnames: set true to use original fieldnames (default is to camelCase)
 *  emit_defaults: set true to emit 0/false values (default is to omit them)
 */
VALUE Message_encode_json(int argc, VALUE* argv, VALUE klass) {
  VALUE descriptor = rb_ivar_get(klass, descriptor_instancevar_interned);
  Descriptor* desc = ruby_to_Descriptor(descriptor);
  VALUE msg_rb;
  VALUE ret;
  int options;
  char *data;
  size_t size;

  if (argc < 1 || argc > 2) {
    rb_raise(rb_eArgError, "Expected 1 or 2 arguments.");
  } else if (argc == 2 && TYPE(argv[1]) != T_HASH) {
    rb_raise(rb_eArgError, "Expected hash arguments.");
  }

  msg_rb = argv[0];

  if (argc == 2) {
    VALUE hash_args = argv[1];
    VALUE preserve_key = ID2SYM(rb_intern("preserve_proto_fieldnames"));
    VALUE emit_key = ID2SYM(rb_intern("emit_defaults"));
    VALUE preserve = rb_hash_lookup2(hash_args, preserve_key, Qfalse);
    VALUE emit = rb_hash_lookup2(hash_args, emit_key, Qfalse);

    if (RTEST(preserve)) options |= UPB_JSONENC_PROTONAMES;
    if (RTEST(emit)) options |= UPB_JSONENC_EMITDEFAULTS;
  }

  size = upb_json_encode(msg->msg, desc->msgdef, pool->symtab, options, NULL, 0,
                         &status);

  if (size == -1) goto err;

  ret = rb_str_buf_new(size + 1);
  size = upb_json_encode(msg->msg, desc->msgdef, pool->symtab, options,
                         RSTRING_PTR(ret), size + 1, &status);

  if (size == -1) goto err;

  rb_str_resize(ret, size);
  return ret;

err:
  rb_raise(rb_eRuntimeError, "Error serializing JSON: %s",
           upb_status_errmsg(&status));
}

/*
 * call-seq:
 *     Google::Protobuf.discard_unknown(msg)
 *
 * Discard unknown fields in the given message object and recursively discard
 * unknown fields in submessages.
 */
VALUE Google_Protobuf_discard_unknown(VALUE self, VALUE msg_rb) {
  VALUE klass = CLASS_OF(msg_rb);
  VALUE descriptor = rb_ivar_get(klass, descriptor_instancevar_interned);
  Descriptor* desc = ruby_to_Descriptor(descriptor);
  if (klass == cRepeatedField || klass == cMap) {
    rb_raise(rb_eArgError, "Expected proto msg for discard unknown.");
  } else {
    rb_raise(rb_eArgError, "Not Yet Implemented");
  }
  return Qnil;
}

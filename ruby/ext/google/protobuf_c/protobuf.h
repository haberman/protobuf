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

#include "ruby-upb.h"
#include "defs.h"

// These operate on a map field (i.e., a repeated field of submessages whose
// submessage type is a map-entry msgdef).
const upb_fielddef* map_field_key(const upb_fielddef* field);
const upb_fielddef* map_field_value(const upb_fielddef* field);

// These operate on a map-entry msgdef.
const upb_fielddef* map_entry_key(const upb_msgdef* msgdef);
const upb_fielddef* map_entry_value(const upb_msgdef* msgdef);

// -----------------------------------------------------------------------------
// Message class creation.
// -----------------------------------------------------------------------------

VALUE Arena_new();
upb_arena *Arena_get(VALUE arena);

VALUE build_class_from_descriptor(VALUE descriptor);
VALUE build_module_from_enumdesc(VALUE _enumdesc);

// -----------------------------------------------------------------------------
// A cache of frozen string objects to use as field defaults.
// -----------------------------------------------------------------------------
VALUE get_frozen_string(const char* data, size_t size, bool binary);

// -----------------------------------------------------------------------------
// Global object cache from upb array/map/message/symtab to wrapper object.
// -----------------------------------------------------------------------------

// This is a conceptually "weak" cache, in that it does not prevent "val" from
// being collected.  We use this to cache Ruby wrapper objects around the
// underlying C objects.

// Adds an entry to the cache. The "arena" parameter must give the arena that
// "key" was allocated from.  In Ruby <2.7.0, it will be used to remove the key
// from the cache when the arena is destroyed.
void ObjectCache_Add(const void* key, VALUE val, upb_arena *arena);

// Returns the cached object for this key, if any. Otherwise returns Qnil.
VALUE ObjectCache_Get(const void* key);

// Pins the given object so it is GC-rooted. This guarantees that the "frozen"
// bit on the object will be remembered.
void ObjectCache_Pin(const void* key, VALUE val, upb_arena *arena);

// -----------------------------------------------------------------------------
// StringBuilder, for inspect
// -----------------------------------------------------------------------------

struct StringBuilder;
typedef struct StringBuilder StringBuilder;

StringBuilder* StringBuilder_New();
void StringBuilder_Free(StringBuilder* b);
void StringBuilder_Printf(StringBuilder* b, const char *fmt, ...);
VALUE StringBuilder_ToRubyString(StringBuilder* b);

void StringBuilder_PrintMsgval(StringBuilder* b, upb_msgval val, TypeInfo info);

// -----------------------------------------------------------------------------
// Utilities.
// -----------------------------------------------------------------------------

extern VALUE cTypeError;

#ifdef NDEBUG
#define PBRUBY_ASSERT(expr) do {} while (false && (expr))
#else
#define PBRUBY_ASSERT(expr) assert(expr)
#endif

#define UPB_UNUSED(var) (void)var

#endif  // __GOOGLE_PROTOBUF_RUBY_PROTOBUF_H__

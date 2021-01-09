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

#ifndef RUBY_PROTOBUF_MESSAGE_H_
#define RUBY_PROTOBUF_MESSAGE_H_

#include <ruby/ruby.h>

#include "protobuf.h"
#include "ruby-upb.h"

upb_msg* Message_GetUpbMessage(VALUE value, const upb_msgdef* m,
                               const char* name, upb_arena* arena);
const upb_msg *Message_Get(VALUE value, const upb_msgdef **m);
upb_msg *Message_GetMutable(VALUE value, const upb_msgdef **m);
VALUE Message_GetArena(VALUE value);

VALUE Message_GetRubyWrapper(upb_msg* msg, const upb_msgdef* m, VALUE arena);
void Message_PrintMessage(StringBuilder* b, const upb_msg* msg,
                          const upb_msgdef* m);
uint64_t Message_Hash(const upb_msg *msg, const upb_msgdef *m, uint64_t seed);
upb_msg* Message_deep_copy(const upb_msg* msg, const upb_msgdef* m,
                           upb_arena *arena);
bool Message_Equal(const upb_msg *m1, const upb_msg *m2, const upb_msgdef *m);
void Message_CheckClass(VALUE klass);
VALUE Scalar_CreateHash(upb_msgval val, TypeInfo type_info);

VALUE MessageOrEnum_GetDescriptor(VALUE klass);

void Message_register(VALUE protobuf);

#endif  // RUBY_PROTOBUF_MESSAGE_H_

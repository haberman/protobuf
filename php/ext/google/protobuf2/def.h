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

#ifndef PHP_PROTOBUF_DEF_H_
#define PHP_PROTOBUF_DEF_H_

#include <php.h>

#include "php-upb.h"

void def_module_init();

// Creates a new DescriptorPool to wrap the given symtab. The DescriptorPool
// takes ownership of the given symtab. If symtab is NULL, the DescriptorPool
// will create an empty symtab instead.
void DescriptorPool_CreateWithSymbolTable(zval *zv, upb_symtab *symtab);

// Given a zval representing a DescriptorPool, steals and returns its symtab,
// which is now owned by the caller.
upb_symtab *DescriptorPool_Steal(zval *zv);

upb_symtab *DescriptorPool_GetSymbolTable();

typedef struct Descriptor {
  zend_object std;
  const upb_msgdef *msgdef;
  zend_class_entry *class_entry;
} Descriptor;

void Descriptor_FromClassEntry(zval *val, zend_class_entry *ce);
Descriptor* Descriptor_GetFromClassEntry(zend_class_entry *ce);
Descriptor* Descriptor_GetFromMessageDef(const upb_msgdef *m);
Descriptor* Descriptor_GetFromFieldDef(const upb_fielddef *f);

#endif  // PHP_PROTOBUF_DEF_H_

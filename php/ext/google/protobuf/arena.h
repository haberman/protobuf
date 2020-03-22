
#ifndef PHP_PROTOBUF_ARENA_H_
#define PHP_PROTOBUF_ARENA_H_

#include <Zend/zend_API.h>

void arena_init(TSRMLS_D);

// Creates and returns a new arena object.
zend_object *arena_create(zend_class_entry *class_type TSRMLS_DC);

// Gets the underlying upb_arena from this arena object.
upb_arena *arena_get(zend_object *arena);

// Merge the lifetimes of these two arenas, such that the memory allocated on
// the arenas outlives the union of their lifetimes.
void arena_merge(zend_object *from_arena, zend_object *to_arena);

#endif  // PHP_PROTOBUF_ARENA_H_

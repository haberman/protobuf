
#include <Zend/zend_API.h>

#include "core-upb.h"

typedef struct Arena {
  zend_object std;
  upb_arena* arena;
} Arena;

zend_class_entry *arena_ce;
static zend_object_handlers arena_object_handlers;

static zend_object* arena_create(zend_class_entry *class_type) {
  Arena *intern = emalloc(sizeof(Arena));
  zend_object_std_init(&intern->std, class_type);
  intern->std.handlers = &arena_object_handlers;
  intern->arena = upb_arena_new();
  // Skip object_properties_init(), we don't allow derived classes.
  return &intern->std;
}

static void arena_free(zend_object* obj) {
  Arena* intern = (Arena*)obj;
  upb_arena_free(intern->arena);
  zend_object_std_dtor(&intern->std);
}

void arena_init(zval* val) {
  ZVAL_OBJ(val, arena_create(arena_ce));
}

void arena_fuse(zend_object *_arena_a, zend_object *_arena_b) {
  Arena *a = (Arena*)_arena_a;
  Arena *b = (Arena*)_arena_b;
  upb_arena_fuse(a->arena, b->arena);
}

upb_arena *arena_get(zval *val) {
  Arena *a = (Arena*)Z_OBJ_P(val);
  return a->arena;
}

// No public methods.
static const zend_function_entry arena_functions[] = {
  ZEND_FE_END
};

// Module init /////////////////////////////////////////////////////////////////

void arena_module_init() {
  zend_class_entry tmp_ce;

  INIT_CLASS_ENTRY(tmp_ce, "Google\\Protobuf\\Internal\\Arena",
                   arena_functions);
  arena_ce = zend_register_internal_class(&tmp_ce);
  arena_ce->create_object = arena_create;  // Omit to forbid creation from PHP?

  memcpy(&arena_object_handlers, &std_object_handlers,
         sizeof(zend_object_handlers));
  arena_object_handlers.free_obj = arena_free;
}

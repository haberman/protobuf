
#include <Zend/zend_API.h>

#include "protobuf.h"
#include "upb.h"

// ArenaGroup //////////////////////////////////////////////////////////////////

static zend_object_handlers arenagroup_object_handlers;

typedef struct {
  zend_object std;
  upb_inttable cleanup;  // Arenas to cleanup, keys are upb_arena*
  upb_arena* arena;      // Arena we actually allocate from.
} ArenaGroup;

zend_class_entry *arenagroup_ce;
static zend_object_handlers arenagroup_object_handlers;

zend_object* arenagroup_create(zend_class_entry *class_type TSRMLS_DC) {
  ArenaGroup *intern = emalloc(sizeof(ArenaGroup));
  zend_object_std_init(&intern->std, class_type TSRMLS_CC);
  intern->std.handlers = &arenagroup_object_handlers;
  // Skip object_properties_init(), we don't allow derived classes.

  // Every arena group starts out with a single arena.
  intern->arena = upb_arena_new();
  upb_inttable_init(&intern->cleanup, UPB_CTYPE_PTR);
  upb_inttable_insertptr(&intern->cleanup, intern->arena, upb_value_ptr(NULL));

  return &intern->std;
}

static void arenagroup_free(zend_object* obj) {
  ArenaGroup* intern = (ArenaGroup*)obj;
  zend_object_std_dtor(&intern->std TSRMLS_CC);

  if (intern->arena) {
    // Cleanup arenas.
    upb_inttable_iter i;
    upb_inttable_begin(&i, &intern->cleanup);
    for(; !upb_inttable_done(&i); upb_inttable_next(&i)) {
      upb_arena *arena = (upb_arena*)upb_inttable_iter_key(&i);
      upb_arena_free(arena);
    }
  }

  upb_inttable_uninit(&intern->cleanup);

  efree(intern);
}

// No public methods.
static const zend_function_entry arenagroup_functions[] = {
  ZEND_FE_END
};

// ArenaGroup //////////////////////////////////////////////////////////////////

typedef struct Arena {
  zend_object std;
  zend_object *group;
  struct Arena *next;  // Circularly linked with all arenas in the group.
} Arena;

zend_class_entry *arena_ce;
static zend_object_handlers arena_object_handlers;

zend_object* arena_create(zend_class_entry *class_type TSRMLS_DC) {
  Arena *intern = emalloc(sizeof(Arena));
  zend_object_std_init(&intern->std, class_type TSRMLS_CC);
  intern->std.handlers = &arena_object_handlers;
  intern->group = arenagroup_create(arenagroup_ce TSRMLS_CC);
  intern->next = intern;
  // Skip object_properties_init(), we don't allow derived classes.
  return &intern->std;
}

static void arena_free(zend_object* obj) {
  Arena* intern = (Arena*)obj;
  GC_DELREF(intern->group);
  zend_object_std_dtor(&intern->std TSRMLS_CC);
  efree(intern);
}

void arena_merge(zend_object *_from_arena, zend_object *_to_arena) {
  Arena *from = (Arena*)_from_arena;
  Arena *to = (Arena*)_to_arena;
  ArenaGroup *from_group = (ArenaGroup*)from->group;
  ArenaGroup *to_group = (ArenaGroup*)to->group;

  // Move cleanup arenas.
  upb_inttable_iter i;
  upb_inttable_begin(&i, &from_group->cleanup);
  for(; !upb_inttable_done(&i); upb_inttable_next(&i)) {
    upb_arena *arena = (upb_arena*)upb_inttable_iter_key(&i);
    upb_inttable_insertptr(&to_group->cleanup, arena, upb_value_ptr(NULL));
  }
  from_group->arena = NULL;  // Signal not to clean up.

  // Update all arenas in the from group to point to to_group.
  Arena *iter = from;
  do {
    GC_DELREF(iter->group);
    GC_ADDREF(to->group);
    iter->group = to->group;
    iter = iter->next;
  } while (iter != from);

  // Merge linked lists so all arenas are in a single list.
  Arena *tmp = from->next;
  from->next = to->next;
  to->next = tmp;
}

upb_arena *arena_get(zend_object *_arena) {
  Arena *arena = (Arena*)_arena;
  ArenaGroup *group = (ArenaGroup*)arena->group;
  return group->arena;
}

// No public methods.
static const zend_function_entry arena_functions[] = {
  ZEND_FE_END
};

// Module init /////////////////////////////////////////////////////////////////

void arena_init(TSRMLS_D) {
  zend_class_entry tmp_ce;

  // Arena
  // TODO(haberman): see if we can mark this final, for robustness.
  INIT_CLASS_ENTRY(tmp_ce, "Google\\Protobuf\\Internal\\Arena",
                   arena_functions);
  arena_ce = zend_register_internal_class(&tmp_ce TSRMLS_CC);
  arena_ce->create_object = arena_create;  // Omit to forbid creation from PHP?

  memcpy(&arena_object_handlers, &std_object_handlers,
         sizeof(zend_object_handlers));
  arena_object_handlers.free_obj = arena_free;

  // ArenaGroup
  // TODO(haberman): see if we can mark this final, for robustness.
  INIT_CLASS_ENTRY(tmp_ce, "Google\\Protobuf\\Internal\\ArenaGroup",
                   arenagroup_functions);
  arenagroup_ce = zend_register_internal_class(&tmp_ce TSRMLS_CC);
  // Omit to forbid creation from PHP?
  arenagroup_ce->create_object = arenagroup_create;

  memcpy(&arenagroup_object_handlers, &std_object_handlers,
         sizeof(zend_object_handlers));
  arenagroup_object_handlers.free_obj = arenagroup_free;
}

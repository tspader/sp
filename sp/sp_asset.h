#ifndef SP_ASSET_H
#define SP_ASSET_H

#include "sp.h"

typedef enum {
  SP_ASSET_STATE_QUEUED,
  SP_ASSET_STATE_IMPORTED,
  SP_ASSET_STATE_COMPLETED,
} sp_asset_state_t;

typedef enum {
  SP_ASSET_KIND_NONE,
} sp_builtin_asset_kind_t;

typedef u32 sp_asset_kind_t;

typedef struct {
  sp_asset_kind_t kind;
  sp_asset_state_t state;
  sp_str_t name;
  sp_atomic_ptr_t data;
} sp_asset_t;

#define sp_asset_data(asset, T) ((T*)sp_atomic_ptr_get(&(asset)->data))

typedef struct sp_asset_registry sp_asset_registry_t;
typedef struct sp_asset_import_context sp_asset_import_context_t;

SP_TYPEDEF_FN(void, sp_asset_import_fn_t, sp_asset_import_context_t* context);
SP_TYPEDEF_FN(void, sp_asset_completion_fn_t, sp_asset_import_context_t* context);

typedef struct {
  sp_asset_import_fn_t on_import;
  sp_asset_completion_fn_t on_completion;
  sp_asset_kind_t kind;
  void* fallback;
} sp_asset_importer_config_t;

typedef struct {
  sp_asset_import_fn_t on_import;
  sp_asset_completion_fn_t on_completion;
  sp_asset_registry_t* registry;
  sp_asset_kind_t kind;
  void* fallback;
  sp_ht_a(sp_str_t, sp_asset_t*) assets;
} sp_asset_importer_t;

struct sp_asset_import_context {
  sp_asset_registry_t* registry;
  sp_asset_importer_t* importer;
  sp_asset_t* asset;
  void* user_data;
};

#define SP_ASSET_REGISTRY_CONFIG_MAX_IMPORTERS 32
typedef struct {
  sp_asset_importer_config_t importers[SP_ASSET_REGISTRY_CONFIG_MAX_IMPORTERS];
} sp_asset_registry_config_t;

struct sp_asset_registry {
  sp_mutex_t mutex;
  sp_mutex_t import_mutex;
  sp_mutex_t completion_mutex;
  sp_semaphore_t semaphore;
  sp_thread_t thread;
  sp_mem_arena_t* arena;
  sp_da(sp_asset_importer_t) importers;
  sp_rb(sp_asset_import_context_t) import_queue;
  sp_rb(sp_asset_import_context_t) completion_queue;
  bool shutdown_requested;
};

void              sp_asset_registry_init(sp_asset_registry_t* r, sp_asset_registry_config_t config);
void              sp_asset_registry_shutdown(sp_asset_registry_t* r);
sp_asset_t*       sp_asset_registry_import(sp_asset_registry_t* r, sp_asset_kind_t k, sp_str_t name, void* user_data);
sp_asset_t*       sp_asset_registry_add(sp_asset_registry_t* r, sp_asset_kind_t k, sp_str_t name, void* data);
sp_asset_t*       sp_asset_registry_find(sp_asset_registry_t* r, sp_asset_kind_t kind, sp_str_t name);
void              sp_asset_registry_process_completions(sp_asset_registry_t* r);
sp_asset_importer_t* sp_asset_registry_find_importer(sp_asset_registry_t* r, sp_asset_kind_t kind);
s32               sp_asset_registry_thread_fn(void* user_data);
#endif




#if defined(SP_IMPLEMENTATION) && !defined(SP_ASSET_IMPLEMENTATION)
  #define SP_ASSET_IMPLEMENTATION
#endif

#if defined(SP_ASSET_IMPLEMENTATION)
void sp_asset_registry_init(sp_asset_registry_t* registry, sp_asset_registry_config_t config) {
  sp_mutex_init(&registry->mutex, SP_MUTEX_PLAIN);
  sp_mutex_init(&registry->import_mutex, SP_MUTEX_PLAIN);
  sp_mutex_init(&registry->completion_mutex, SP_MUTEX_PLAIN);
  sp_semaphore_init(&registry->semaphore);
  registry->shutdown_requested = false;
  registry->import_queue = SP_NULLPTR;
  registry->completion_queue = SP_NULLPTR;
  registry->arena = sp_mem_arena_new();
  sp_da_init(sp_mem_arena_as_allocator(registry->arena), registry->importers);

  sp_context_push_allocator(sp_mem_arena_as_allocator(registry->arena));

  sp_for(index, SP_ASSET_REGISTRY_CONFIG_MAX_IMPORTERS) {
    sp_asset_importer_config_t* cfg = &config.importers[index];
    if (cfg->kind == SP_ASSET_KIND_NONE) break;

    sp_asset_importer_t importer = (sp_asset_importer_t){
      .on_import = cfg->on_import,
      .on_completion = cfg->on_completion,
      .registry = registry,
      .kind = cfg->kind,
      .fallback = cfg->fallback,
      .assets = SP_NULLPTR,
    };
    sp_str_ht_init_a(sp_mem_arena_as_allocator(registry->arena), importer.assets);
    sp_da_push(registry->importers, importer);
  }

  sp_context_pop();

  sp_thread_init(&registry->thread, sp_asset_registry_thread_fn, registry);
}

void sp_asset_registry_shutdown(sp_asset_registry_t* registry) {
  sp_mutex_lock(&registry->mutex);
  registry->shutdown_requested = true;
  sp_mutex_unlock(&registry->mutex);

  sp_semaphore_signal(&registry->semaphore);
  sp_thread_join(&registry->thread);

  sp_mutex_destroy(&registry->mutex);
  sp_mutex_destroy(&registry->import_mutex);
  sp_mutex_destroy(&registry->completion_mutex);
  sp_semaphore_destroy(&registry->semaphore);

  sp_mem_arena_destroy(registry->arena);
}

sp_asset_importer_t* sp_asset_registry_find_importer(sp_asset_registry_t* r, sp_asset_kind_t kind) {
  sp_da_for(r->importers, index) {
    sp_asset_importer_t* importer = &r->importers[index];
    if (importer->kind == kind) {
      return importer;
    }
  }
  return SP_NULLPTR;
}

static sp_asset_t* sp_asset_registry_alloc(sp_asset_registry_t* r, sp_asset_importer_t* importer, sp_str_t name, void* data) {
  sp_context_push_allocator(sp_mem_arena_as_allocator(r->arena));

  sp_asset_t* asset = sp_alloc_type(sp_asset_t);
  asset->kind = importer->kind;
  asset->state = SP_ASSET_STATE_QUEUED;
  asset->name = sp_str_copy(name);
  sp_atomic_ptr_set(&asset->data, data);

  sp_str_ht_insert(importer->assets, sp_str_copy(name), asset);

  sp_context_pop();
  return asset;
}

sp_asset_t* sp_asset_registry_add(sp_asset_registry_t* r, sp_asset_kind_t k, sp_str_t name, void* data) {
  sp_asset_importer_t* importer = sp_asset_registry_find_importer(r, k);
  SP_ASSERT(importer);

  sp_mutex_lock(&r->mutex);
  sp_asset_t* asset = sp_asset_registry_alloc(r, importer, name, data);
  asset->state = SP_ASSET_STATE_COMPLETED;
  sp_mutex_unlock(&r->mutex);
  return asset;
}

sp_asset_t* sp_asset_registry_find(sp_asset_registry_t* r, sp_asset_kind_t kind, sp_str_t name) {
  sp_mutex_lock(&r->mutex);
  sp_asset_importer_t* importer = sp_asset_registry_find_importer(r, kind);
  if (!importer || sp_str_ht_empty(importer->assets) || !sp_str_ht_get(importer->assets, name)) {
    sp_mutex_unlock(&r->mutex);
    return SP_NULLPTR;
  }
  sp_asset_t* found = *sp_str_ht_get(importer->assets, name);
  sp_mutex_unlock(&r->mutex);
  return found;
}

sp_asset_t* sp_asset_registry_import(sp_asset_registry_t* r, sp_asset_kind_t k, sp_str_t name, void* user_data) {
  sp_asset_importer_t* importer = sp_asset_registry_find_importer(r, k);
  SP_ASSERT(importer);

  sp_mutex_lock(&r->mutex);
  sp_asset_t* asset = sp_asset_registry_alloc(r, importer, name, importer->fallback);
  sp_mutex_unlock(&r->mutex);

  sp_asset_import_context_t context = {
    .registry = r,
    .importer = importer,
    .asset = asset,
    .user_data = user_data,
  };

  sp_mutex_lock(&r->import_mutex);
  sp_rb_push(r->import_queue, context);
  sp_mutex_unlock(&r->import_mutex);

  sp_semaphore_signal(&r->semaphore);

  return asset;
}

void sp_asset_registry_process_completions(sp_asset_registry_t* r) {
  sp_mutex_lock(&r->completion_mutex);
  while (!sp_rb_empty(r->completion_queue)) {
    sp_asset_import_context_t context = *sp_rb_peek(r->completion_queue);
    sp_rb_pop(r->completion_queue);
    sp_mutex_unlock(&r->completion_mutex);

    context.importer->on_completion(&context);

    sp_mutex_lock(&r->mutex);
    context.asset->state = SP_ASSET_STATE_COMPLETED;
    sp_mutex_unlock(&r->mutex);

    sp_mutex_lock(&r->completion_mutex);
  }
  sp_mutex_unlock(&r->completion_mutex);
}

s32 sp_asset_registry_thread_fn(void* user_data) {
  sp_asset_registry_t* registry = (sp_asset_registry_t*)user_data;
  while (true) {
    sp_semaphore_wait(&registry->semaphore);

    sp_mutex_lock(&registry->mutex);
    bool shutdown = registry->shutdown_requested;
    sp_mutex_unlock(&registry->mutex);
    if (shutdown) break;

    sp_mutex_lock(&registry->import_mutex);

    while (!sp_rb_empty(registry->import_queue)) {
      sp_asset_import_context_t context = *sp_rb_peek(registry->import_queue);
      sp_rb_pop(registry->import_queue);
      sp_mutex_unlock(&registry->import_mutex);

      context.importer->on_import(&context);

      sp_mutex_lock(&registry->mutex);
      context.asset->state = SP_ASSET_STATE_IMPORTED;
      sp_mutex_unlock(&registry->mutex);

      sp_mutex_lock(&registry->completion_mutex);
      sp_rb_push(registry->completion_queue, context);
      sp_mutex_unlock(&registry->completion_mutex);

      sp_mutex_lock(&registry->import_mutex);
    }

    sp_mutex_unlock(&registry->import_mutex);
  }

  return 0;
}

#endif // SP_IMPLEMENTATION

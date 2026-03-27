#ifndef SP_ASSET_H
#define SP_ASSET_H

#include "sp.h"

typedef struct {
  sp_allocator_t allocator;
  void* value;
  sp_atomic_s32_t ready;
  u32 size;
} sp_future_t;

sp_future_t* sp_future_create(u32 size);
void sp_future_set_value(sp_future_t* future, void* data);
void sp_future_destroy(sp_future_t* future);

typedef enum {
  SP_ASSET_STATE_QUEUED,
  SP_ASSET_STATE_IMPORTED,
  SP_ASSET_STATE_COMPLETED,
} sp_asset_state_t;

typedef enum {
  SP_ASSET_KIND_NONE,
} sp_builtin_asset_kind_t;

typedef u32 sp_asset_kind_t;

typedef struct sp_asset_registry sp_asset_registry_t;
typedef struct sp_asset_import_context sp_asset_import_context_t;

SP_TYPEDEF_FN(void, sp_asset_import_fn_t, sp_asset_import_context_t* context);
SP_TYPEDEF_FN(void, sp_asset_completion_fn_t, sp_asset_import_context_t* context);

typedef struct {
  sp_asset_kind_t kind;
  sp_asset_state_t state;
  sp_str_t name;
  void* data;
} sp_asset_t;

typedef struct {
  sp_asset_import_fn_t on_import;
  sp_asset_completion_fn_t on_completion;
  sp_asset_kind_t kind;
} sp_asset_importer_config_t;

typedef struct {
  sp_asset_import_fn_t on_import;
  sp_asset_completion_fn_t on_completion;
  sp_asset_registry_t* registry;
  sp_asset_kind_t kind;
} sp_asset_importer_t;

struct sp_asset_import_context {
  sp_asset_registry_t* registry;
  sp_asset_importer_t* importer;
  sp_future_t* future;
  void* user_data;
  u32 asset_index;
};

#define sp_asset_import_context_get_asset(ctx) (&(ctx)->registry->assets[(ctx)->asset_index])

#define SP_ASSET_REGISTRY_CONFIG_MAX_IMPORTERS 32
typedef struct {
  sp_asset_importer_config_t importers [SP_ASSET_REGISTRY_CONFIG_MAX_IMPORTERS];
} sp_asset_registry_config_t;

struct sp_asset_registry {
  sp_mutex_t mutex;
  sp_mutex_t import_mutex;
  sp_mutex_t completion_mutex;
  sp_semaphore_t semaphore;
  sp_thread_t thread;
  sp_da(sp_asset_t) assets;
  sp_da(sp_asset_importer_t) importers;
  sp_rb(sp_asset_import_context_t) import_queue;
  sp_rb(sp_asset_import_context_t) completion_queue;
  bool shutdown_requested;
};

void sp_asset_registry_init(sp_asset_registry_t* r, sp_asset_registry_config_t config);
void sp_asset_registry_shutdown(sp_asset_registry_t* r);
sp_future_t* sp_asset_registry_import(sp_asset_registry_t* r, sp_asset_kind_t k, sp_str_t name, void* user_data);
sp_asset_t* sp_asset_registry_add(sp_asset_registry_t* r, sp_asset_kind_t k, sp_str_t name, void* user_data);
sp_asset_t* sp_asset_registry_find(sp_asset_registry_t* r, sp_asset_kind_t kind, sp_str_t name);
void sp_asset_registry_process_completions(sp_asset_registry_t* r);
sp_asset_t* sp_asset_registry_reserve(sp_asset_registry_t* r);
sp_asset_importer_t* sp_asset_registry_find_importer(sp_asset_registry_t* r, sp_asset_kind_t kind);
s32 sp_asset_registry_thread_fn(void* user_data);

#if defined(SP_IMPLEMENTATION)

sp_future_t* sp_future_create(u32 size) {
  sp_future_t* future = (sp_future_t*)sp_alloc(sizeof(sp_future_t));
  future->allocator = sp_context_get()->allocator;
  sp_atomic_s32_set(&future->ready, 0);
  future->value = sp_alloc(size);
  future->size = size;
  return future;
}

void sp_future_destroy(sp_future_t* future) {
  sp_context_push_allocator(future->allocator);
  sp_free(future);
  sp_context_pop();
}

void sp_future_set_value(sp_future_t* future, void* value) {
  sp_mem_copy(value, future->value, future->size);
  sp_atomic_s32_set(&future->ready, 1);
}

void sp_asset_registry_init(sp_asset_registry_t* registry, sp_asset_registry_config_t config) {
  sp_mutex_init(&registry->mutex, SP_MUTEX_PLAIN);
  sp_mutex_init(&registry->import_mutex, SP_MUTEX_PLAIN);
  sp_mutex_init(&registry->completion_mutex, SP_MUTEX_PLAIN);

  sp_semaphore_init(&registry->semaphore);
  registry->shutdown_requested = false;

  registry->import_queue = SP_NULLPTR;
  registry->completion_queue = SP_NULLPTR;

  sp_for(index, SP_ASSET_REGISTRY_CONFIG_MAX_IMPORTERS) {
    sp_asset_importer_config_t* cfg = &config.importers[index];
    if (cfg->kind == SP_ASSET_KIND_NONE) break;

    sp_asset_importer_t importer = (sp_asset_importer_t) {
      .on_import = cfg->on_import,
      .on_completion = cfg->on_completion,
      .registry = registry,
      .kind = cfg->kind,
    };
    sp_dyn_array_push(registry->importers, importer);
  }

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
}

void sp_asset_registry_process_completions(sp_asset_registry_t* registry) {
  sp_mutex_lock(&registry->completion_mutex);
  while (!sp_rb_empty(registry->completion_queue)) {
    sp_asset_import_context_t context = *sp_rb_peek(registry->completion_queue);
    sp_rb_pop(registry->completion_queue);
    sp_mutex_unlock(&registry->completion_mutex);

    context.importer->on_completion(&context);

    sp_mutex_lock(&registry->mutex);
    sp_asset_t* asset = &registry->assets[context.asset_index];
    asset->state = SP_ASSET_STATE_COMPLETED;
    sp_future_set_value(context.future, &asset);
    sp_mutex_unlock(&registry->mutex);

    sp_mutex_lock(&registry->completion_mutex);
  }
  sp_mutex_unlock(&registry->completion_mutex);
}

sp_asset_t* sp_asset_registry_reserve(sp_asset_registry_t* registry) {
  sp_asset_t memory = SP_ZERO_INITIALIZE();
  sp_dyn_array_push(registry->assets, memory);
  return sp_dyn_array_back(registry->assets);
}

sp_asset_t* sp_asset_registry_add(sp_asset_registry_t* registry, sp_asset_kind_t kind, sp_str_t name, void* user_data) {
  sp_mutex_lock(&registry->mutex);
  sp_asset_t* asset = sp_asset_registry_reserve(registry);
  asset->kind = kind;
  asset->name = sp_str_copy(name);
  asset->state = SP_ASSET_STATE_COMPLETED;
  asset->data = user_data;
  sp_mutex_unlock(&registry->mutex);
  return asset;
}

sp_future_t* sp_asset_registry_import(sp_asset_registry_t* registry, sp_asset_kind_t kind, sp_str_t name, void* user_data) {
  sp_asset_importer_t* importer = sp_asset_registry_find_importer(registry, kind);
  SP_ASSERT(importer);

  sp_mutex_lock(&registry->mutex);
  sp_asset_t* asset = sp_asset_registry_reserve(registry);
  asset->kind = kind;
  asset->name = sp_str_copy(name);
  asset->state = SP_ASSET_STATE_QUEUED;
  u32 asset_index = sp_dyn_array_size(registry->assets) - 1;
  sp_mutex_unlock(&registry->mutex);

  sp_asset_import_context_t context = {
    .registry = registry,
    .importer = importer,
    .future = sp_future_create(sizeof(sp_asset_t*)),
    .user_data = user_data,
    .asset_index = asset_index,
  };

  sp_mutex_lock(&registry->import_mutex);
  sp_rb_push(registry->import_queue, context);
  sp_mutex_unlock(&registry->import_mutex);

  sp_semaphore_signal(&registry->semaphore);

  return context.future;
}

sp_asset_t* sp_asset_registry_find(sp_asset_registry_t* registry, sp_asset_kind_t kind, sp_str_t name) {
  sp_mutex_lock(&registry->mutex);
  sp_asset_t* found = SP_NULLPTR;
  sp_dyn_array_for(registry->assets, index) {
    sp_asset_t* asset = registry->assets + index;
    if (asset->kind == kind && sp_str_equal(asset->name, name)) {
      found = asset;
      break;
    }
  }
  sp_mutex_unlock(&registry->mutex);
  return found;
}

sp_asset_importer_t* sp_asset_registry_find_importer(sp_asset_registry_t* registry, sp_asset_kind_t kind) {
  sp_mutex_lock(&registry->mutex);
  sp_dyn_array_for(registry->importers, index) {
    sp_asset_importer_t* importer = &registry->importers[index];
    if (importer->kind == kind) {
      sp_mutex_unlock(&registry->mutex);
      return importer;
    }
  }

  sp_mutex_unlock(&registry->mutex);
  return SP_NULLPTR;
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
      sp_asset_t* asset = &registry->assets[context.asset_index];
      asset->state = SP_ASSET_STATE_IMPORTED;
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

#endif // SP_ASSET_IMPLEMENTATION
#endif // SP_ASSET_H

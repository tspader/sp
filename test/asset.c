#define SP_APP
#include "sp/sp_asset.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

// Test asset type (user-defined, not in sp.h)
typedef enum {
  SP_ASSET_KIND_TEST = 1000,
} sp_test_asset_kind_t;

typedef struct {
  sp_str_t content;
  s32 value;
} sp_test_asset_data_t;

static sp_test_asset_data_t s_fallback = {
  .content = { .data = "default", .len = 7 },
  .value = -1,
};

void sp_test_asset_noop_import(sp_asset_import_context_t* context) { (void)context; }
void sp_test_asset_noop_complete(sp_asset_import_context_t* context) { (void)context; }

// Simple importer that just copies the data
void sp_test_asset_import(sp_asset_import_context_t* context) {
  sp_test_asset_data_t* input = (sp_test_asset_data_t*)context->user_data;
  sp_test_asset_data_t* data = (sp_test_asset_data_t*)sp_alloc(sizeof(sp_test_asset_data_t));
  data->content = sp_str_copy(input->content);
  data->value = input->value;

  sp_atomic_ptr_set(&context->asset->data, data);
}

void sp_test_asset_complete(sp_asset_import_context_t* context) {
  // Nothing special to do on completion for test assets
  (void)context;
}

// Test: Basic synchronous add and find
UTEST(asset_registry, basic_add_and_find) {
  sp_context_push_allocator(sp_mem_os_new());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = {
    .importers = {
      { .kind = SP_ASSET_KIND_TEST, .on_import = sp_test_asset_noop_import, .on_completion = sp_test_asset_noop_complete },
    }
  };
  sp_asset_registry_init(&registry, config);

  // Add an asset
  sp_test_asset_data_t* data1 = (sp_test_asset_data_t*)sp_alloc(sizeof(sp_test_asset_data_t));
  data1->content = SP_LIT("test content");
  data1->value = 42;

  sp_asset_t* added = sp_asset_registry_add(&registry, SP_ASSET_KIND_TEST, SP_LIT("test_asset"), data1);
  ASSERT_NE(added, SP_NULLPTR);
  ASSERT_EQ(added->kind, SP_ASSET_KIND_TEST);
  ASSERT_TRUE(sp_str_equal(added->name, SP_LIT("test_asset")));
  ASSERT_EQ(added->state, SP_ASSET_STATE_COMPLETED);
  ASSERT_EQ(sp_asset_data(added, sp_test_asset_data_t), data1);

  // Find the asset
  sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT("test_asset"));
  ASSERT_NE(found, SP_NULLPTR);
  ASSERT_EQ(found, added);
  ASSERT_EQ(sp_asset_data(found, sp_test_asset_data_t), data1);

  // Find non-existent asset
  sp_asset_t* not_found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT("nonexistent"));
  ASSERT_EQ(not_found, SP_NULLPTR);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

// Test: Multiple assets with same name but different types
UTEST(asset_registry, same_name_different_types) {
  sp_context_push_allocator(sp_mem_os_new());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = {
    .importers = {
      { .kind = 1001, .on_import = sp_test_asset_noop_import, .on_completion = sp_test_asset_noop_complete },
      { .kind = 1002, .on_import = sp_test_asset_noop_import, .on_completion = sp_test_asset_noop_complete },
      { .kind = 1003, .on_import = sp_test_asset_noop_import, .on_completion = sp_test_asset_noop_complete },
    }
  };
  sp_asset_registry_init(&registry, config);

  // Add assets with same name but different types
  sp_asset_registry_add(&registry, 1001, SP_LIT("shared_name"), (void*)0x1);
  sp_asset_registry_add(&registry, 1002, SP_LIT("shared_name"), (void*)0x2);
  sp_asset_registry_add(&registry, 1003, SP_LIT("shared_name"), (void*)0x3);

  // Find each one
  sp_asset_t* asset1 = sp_asset_registry_find(&registry, 1001, SP_LIT("shared_name"));
  sp_asset_t* asset2 = sp_asset_registry_find(&registry, 1002, SP_LIT("shared_name"));
  sp_asset_t* asset3 = sp_asset_registry_find(&registry, 1003, SP_LIT("shared_name"));

  ASSERT_NE(asset1, SP_NULLPTR);
  ASSERT_NE(asset2, SP_NULLPTR);
  ASSERT_NE(asset3, SP_NULLPTR);

  ASSERT_EQ(sp_atomic_ptr_get(&asset1->data), (void*)0x1);
  ASSERT_EQ(sp_atomic_ptr_get(&asset2->data), (void*)0x2);
  ASSERT_EQ(sp_atomic_ptr_get(&asset3->data), (void*)0x3);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

// Test: String copying (verify names are copied, not referenced)
UTEST(asset_registry, string_copying) {
  sp_context_push_allocator(sp_mem_os_new());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = {
    .importers = {
      { .kind = SP_ASSET_KIND_TEST, .on_import = sp_test_asset_noop_import, .on_completion = sp_test_asset_noop_complete },
    }
  };
  sp_asset_registry_init(&registry, config);

  // Create a temporary string
  c8 temp_buffer[32];
  snprintf(temp_buffer, sizeof(temp_buffer), "temp_asset");
  sp_str_t temp_name = sp_str_from_cstr(temp_buffer);

  // Add asset with temporary name
  sp_asset_t* asset = sp_asset_registry_add(&registry, SP_ASSET_KIND_TEST, temp_name, SP_NULLPTR);

  // Modify the original buffer
  snprintf(temp_buffer, sizeof(temp_buffer), "modified!");

  // The asset's name should still be intact
  ASSERT_TRUE(sp_str_equal(asset->name, SP_LIT("temp_asset")));
  ASSERT_FALSE(sp_str_equal(asset->name, sp_str_from_cstr(temp_buffer)));

  // Should still be findable with original name
  sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT("temp_asset"));
  ASSERT_EQ(found, asset);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

UTEST(asset_registry, null_user_data) {
  sp_context_push_allocator(sp_mem_os_new());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = {
    .importers = {
      { .kind = SP_ASSET_KIND_TEST, .on_import = sp_test_asset_noop_import, .on_completion = sp_test_asset_noop_complete },
    }
  };
  sp_asset_registry_init(&registry, config);

  // Add asset with NULL data
  sp_asset_t* asset = sp_asset_registry_add(&registry, SP_ASSET_KIND_TEST, SP_LIT("null_asset"), SP_NULLPTR);
  ASSERT_NE(asset, SP_NULLPTR);
  ASSERT_EQ(sp_atomic_ptr_get(&asset->data), SP_NULLPTR);

  // Should be findable
  sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT("null_asset"));
  ASSERT_EQ(found, asset);
  ASSERT_EQ(sp_atomic_ptr_get(&found->data), SP_NULLPTR);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

UTEST(asset_registry, empty_names) {
  sp_context_push_allocator(sp_mem_os_new());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = {
    .importers = {
      { .kind = SP_ASSET_KIND_TEST, .on_import = sp_test_asset_noop_import, .on_completion = sp_test_asset_noop_complete },
    }
  };
  sp_asset_registry_init(&registry, config);

  // Add asset with empty name
  sp_asset_t* asset = sp_asset_registry_add(&registry, SP_ASSET_KIND_TEST, SP_LIT(""), (void*)0xDEAD);
  ASSERT_NE(asset, SP_NULLPTR);
  ASSERT_EQ(asset->name.len, 0);

  // Should be findable with empty name
  sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT(""));
  ASSERT_EQ(found, asset);
  ASSERT_EQ(sp_atomic_ptr_get(&found->data), (void*)0xDEAD);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

UTEST(asset_registry, import_completion_pipeline) {
  sp_context_push_allocator(sp_mem_os_new());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = {
    .importers = {
      {
        .kind = SP_ASSET_KIND_TEST,
        .on_import = sp_test_asset_import,
        .on_completion = sp_test_asset_complete,
        .fallback = &s_fallback,
      }
    }
  };
  sp_asset_registry_init(&registry, config);

  // Create test data
  sp_test_asset_data_t input_data = {
    .content = SP_LIT("async content"),
    .value = 999
  };

  // Import an asset (goes through the async pipeline)
  sp_asset_t* asset = sp_asset_registry_import(&registry, SP_ASSET_KIND_TEST, SP_LIT("async_asset"), &input_data);
  ASSERT_NE(asset, SP_NULLPTR);

  // Should be immediately findable with default data
  sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT("async_asset"));
  ASSERT_EQ(found, asset);
  ASSERT_EQ(sp_asset_data(found, sp_test_asset_data_t), &s_fallback);

  // Wait for import to complete
  sp_os_sleep_ms(50);

  // Process completions on main thread
  sp_asset_registry_process_completions(&registry);

  // Should now be completed with real data
  ASSERT_EQ(asset->state, SP_ASSET_STATE_COMPLETED);
  sp_test_asset_data_t* result_data = sp_asset_data(asset, sp_test_asset_data_t);
  ASSERT_NE(result_data, &s_fallback);
  ASSERT_NE(result_data, SP_NULLPTR);
  ASSERT_TRUE(sp_str_equal(result_data->content, SP_LIT("async content")));
  ASSERT_EQ(result_data->value, 999);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

UTEST(asset_registry, state_transitions) {
  sp_context_push_allocator(sp_mem_os_new());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = {
    .importers = {
      {
        .kind = SP_ASSET_KIND_TEST,
        .on_import = sp_test_asset_import,
        .on_completion = sp_test_asset_complete,
        .fallback = &s_fallback,
      }
    }
  };
  sp_asset_registry_init(&registry, config);

  sp_test_asset_data_t input = {
    .content = SP_LIT("state test"),
    .value = 777
  };

  // Start import - should be QUEUED initially
  sp_asset_t* asset = sp_asset_registry_import(&registry, SP_ASSET_KIND_TEST, SP_LIT("state_asset"), &input);
  ASSERT_NE(asset, SP_NULLPTR);
  ASSERT_EQ(asset->state, SP_ASSET_STATE_QUEUED);

  // Wait for import
  sp_os_sleep_ms(50);

  // Process completions
  sp_asset_registry_process_completions(&registry);

  // Should now be COMPLETED
  ASSERT_EQ(asset->state, SP_ASSET_STATE_COMPLETED);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

// Test: Concurrent find operations while importing
UTEST(asset_registry, concurrent_find_during_import) {
  sp_context_push_allocator(sp_mem_os_new());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = {
    .importers = {
      {
        .kind = SP_ASSET_KIND_TEST,
        .on_import = sp_test_asset_import,
        .on_completion = sp_test_asset_complete,
        .fallback = &s_fallback,
      }
    }
  };
  sp_asset_registry_init(&registry, config);

  // Add some assets first
  for (s32 i = 0; i < 10; i++) {
    c8 name[32];
    snprintf(name, sizeof(name), "asset_%d", i);
    sp_asset_registry_add(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name), (void*)(uintptr_t)i);
  }

  // Start importing more assets
  for (s32 i = 10; i < 20; i++) {
    c8 name[32];
    snprintf(name, sizeof(name), "asset_%d", i);
    sp_test_asset_data_t* data = (sp_test_asset_data_t*)sp_alloc(sizeof(sp_test_asset_data_t));
    data->content = sp_str_from_cstr(name);
    data->value = i;
    sp_asset_registry_import(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name), data);
  }

  // All assets should be immediately findable (sync ones with real data, async with default)
  for (s32 i = 0; i < 20; i++) {
    c8 name[32];
    snprintf(name, sizeof(name), "asset_%d", i);
    sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name));
    ASSERT_NE(found, SP_NULLPTR);
    if (i < 10) {
      ASSERT_EQ(sp_atomic_ptr_get(&found->data), (void*)(uintptr_t)i);
    }
  }

  // Let imports finish
  sp_os_sleep_ms(50);
  sp_asset_registry_process_completions(&registry);

  // Now all should be completed
  for (s32 i = 0; i < 20; i++) {
    c8 name[32];
    snprintf(name, sizeof(name), "asset_%d", i);
    sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name));
    ASSERT_NE(found, SP_NULLPTR);
    ASSERT_EQ(found->state, SP_ASSET_STATE_COMPLETED);
  }

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

// Test: Many assets stress test
UTEST(asset_registry, stress_many_assets) {
  sp_context_push_allocator(sp_mem_os_new());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = {
    .importers = {
      { .kind = SP_ASSET_KIND_TEST, .on_import = sp_test_asset_noop_import, .on_completion = sp_test_asset_noop_complete },
    }
  };
  sp_asset_registry_init(&registry, config);

  const s32 ASSET_COUNT = 1000;

  // Add many assets
  for (s32 i = 0; i < ASSET_COUNT; i++) {
    c8 name[32];
    snprintf(name, sizeof(name), "stress_%d", i);
    sp_asset_registry_add(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name), (void*)(uintptr_t)i);
  }

  // Verify all can be found
  for (s32 i = 0; i < ASSET_COUNT; i++) {
    c8 name[32];
    snprintf(name, sizeof(name), "stress_%d", i);
    sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name));
    ASSERT_NE(found, SP_NULLPTR);
    ASSERT_EQ(sp_atomic_ptr_get(&found->data), (void*)(uintptr_t)i);
  }

  // Random access pattern
  for (s32 iter = 0; iter < ASSET_COUNT * 2; iter++) {
    s32 id = (iter * 7919) % ASSET_COUNT;
    c8 name[32];
    snprintf(name, sizeof(name), "stress_%d", id);
    sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name));
    ASSERT_NE(found, SP_NULLPTR);
    ASSERT_EQ(sp_atomic_ptr_get(&found->data), (void*)(uintptr_t)id);
  }

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

// Test: Stable pointers across many inserts
UTEST(asset_registry, stable_pointers) {
  sp_context_push_allocator(sp_mem_os_new());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = {
    .importers = {
      { .kind = SP_ASSET_KIND_TEST, .on_import = sp_test_asset_noop_import, .on_completion = sp_test_asset_noop_complete },
    }
  };
  sp_asset_registry_init(&registry, config);

  // Get a pointer to the first asset
  sp_asset_t* first = sp_asset_registry_add(&registry, SP_ASSET_KIND_TEST, SP_LIT("first"), (void*)0xAAAA);
  ASSERT_NE(first, SP_NULLPTR);

  // Insert many more assets
  for (s32 i = 0; i < 500; i++) {
    c8 name[32];
    snprintf(name, sizeof(name), "filler_%d", i);
    sp_asset_registry_add(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name), (void*)(uintptr_t)i);
  }

  // The original pointer should still be valid
  ASSERT_EQ(sp_atomic_ptr_get(&first->data), (void*)0xAAAA);
  ASSERT_TRUE(sp_str_equal(first->name, SP_LIT("first")));

  // And findable
  sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT("first"));
  ASSERT_EQ(found, first);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

// Test: Default data before completion
UTEST(asset_registry, default_data_before_completion) {
  sp_context_push_allocator(sp_mem_os_new());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = {
    .importers = {
      {
        .kind = SP_ASSET_KIND_TEST,
        .on_import = sp_test_asset_import,
        .on_completion = sp_test_asset_complete,
        .fallback = &s_fallback,
      }
    }
  };
  sp_asset_registry_init(&registry, config);

  sp_test_asset_data_t input = {
    .content = SP_LIT("real content"),
    .value = 42
  };

  // Import returns immediately with stable pointer
  sp_asset_t* asset = sp_asset_registry_import(&registry, SP_ASSET_KIND_TEST, SP_LIT("my_asset"), &input);

  // Data is default before completion
  sp_test_asset_data_t* data_before = sp_asset_data(asset, sp_test_asset_data_t);
  ASSERT_EQ(data_before, &s_fallback);
  ASSERT_EQ(data_before->value, -1);

  // Wait and complete
  sp_os_sleep_ms(50);
  sp_asset_registry_process_completions(&registry);

  // Same pointer, now has real data
  sp_test_asset_data_t* data_after = sp_asset_data(asset, sp_test_asset_data_t);
  ASSERT_NE(data_after, &s_fallback);
  ASSERT_EQ(data_after->value, 42);
  ASSERT_TRUE(sp_str_equal(data_after->content, SP_LIT("real content")));

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

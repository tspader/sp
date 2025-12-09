#include "glob.h"
#include "test.h"

#include "utest.h"

typedef struct {
  sp_glob_token_type_t type;
  c8 literal;
  bool negated;
  sp_glob_char_range_t ranges[8];
  sp_glob_token_type_t alts[8][8];
} test_token_t;

typedef struct {
  sp_str_t pattern;
  test_token_t expected[16];
} parse_test_t;


void run_parse_test(int* utest_result, parse_test_t* t) {
  sp_da(sp_glob_token_t) tokens = SP_NULLPTR;
  sp_glob_err_t err = sp_glob_parse(t->pattern, &tokens);

  EXPECT_EQ(err, SP_GLOB_ERR_OK);

  u32 expected_count = 0;
  while (expected_count < 16 && t->expected[expected_count].type != SP_GLOB_TOK_NONE) {
    expected_count++;
  }

  EXPECT_EQ(sp_da_size(tokens), expected_count);

  for (u32 i = 0; i < expected_count && i < sp_da_size(tokens); i++) {
    EXPECT_EQ(tokens[i].type, t->expected[i].type);
    switch (tokens[i].type) {
      case SP_GLOB_TOK_LITERAL: {
        EXPECT_EQ(tokens[i].literal, t->expected[i].literal);
        break;
      }
      case SP_GLOB_TOK_RANGES: {
        EXPECT_EQ(tokens[i].ranges.negated, t->expected[i].negated);
        u32 range_count = 0;
        while (range_count < 8 && (t->expected[i].ranges[range_count].start || t->expected[i].ranges[range_count].end)) {
          range_count++;
        }
        EXPECT_EQ(sp_da_size(tokens[i].ranges.ranges), range_count);
        for (u32 j = 0; j < range_count; j++) {
          EXPECT_EQ(tokens[i].ranges.ranges[j].start, t->expected[i].ranges[j].start);
          EXPECT_EQ(tokens[i].ranges.ranges[j].end, t->expected[i].ranges[j].end);
        }
        break;
      }
      case SP_GLOB_TOK_ALTERNATES: {
        u32 alt_count = 0;
        while (alt_count < 8 && t->expected[i].alts[alt_count][0] != SP_GLOB_TOK_NONE) {
          alt_count++;
        }
        EXPECT_EQ(sp_da_size(tokens[i].alternates.alts), alt_count);
        for (u32 j = 0; j < alt_count; j++) {
          sp_da(sp_glob_token_t) alt = tokens[i].alternates.alts[j];
          u32 alt_len = 0;
          while (alt_len < 8 && t->expected[i].alts[j][alt_len] != SP_GLOB_TOK_NONE) {
            alt_len++;
          }
          EXPECT_EQ(sp_da_size(alt), alt_len);
          for (u32 k = 0; k < alt_len; k++) {
            EXPECT_EQ(alt[k].type, t->expected[i].alts[j][k]);
          }
        }
        break;
      }
      case SP_GLOB_TOK_NONE:
      case SP_GLOB_TOK_ANY:
      case SP_GLOB_TOK_ZERO_OR_MORE:
      case SP_GLOB_TOK_RECURSIVE_PREFIX:
      case SP_GLOB_TOK_RECURSIVE_SUFFIX:
      case SP_GLOB_TOK_RECURSIVE_ZERO_OR_MORE: {
        break;
      }
    }
  }
}

typedef struct {
  sp_str_t pattern;
  sp_glob_err_t expected_err;
} parse_error_test_t;

void run_parse_error_test(int* utest_result, parse_error_test_t* t) {
  sp_da(sp_glob_token_t) tokens = SP_NULLPTR;
  sp_glob_err_t err = sp_glob_parse(t->pattern, &tokens);
  EXPECT_EQ(err, t->expected_err);
}

SP_TEST_MAIN()

struct glob {};

UTEST_F_SETUP(glob) {
  (void)utest_fixture;
}

UTEST_F_TEARDOWN(glob) {
  (void)utest_fixture;
}

UTEST_F(glob, parse_literal_single) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("a"),
    .expected = {
      { .type = SP_GLOB_TOK_LITERAL, .literal = 'a' }
    }
  });
}

UTEST_F(glob, parse_literal_multi) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("foo"),
    .expected = {
      { .type = SP_GLOB_TOK_LITERAL, .literal = 'f' },
      { .type = SP_GLOB_TOK_LITERAL, .literal = 'o' },
      { .type = SP_GLOB_TOK_LITERAL, .literal = 'o' }
    }
  });
}

UTEST_F(glob, parse_any) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("?"),
    .expected = {
      { .type = SP_GLOB_TOK_ANY }
    }
  });
}

UTEST_F(glob, parse_star) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("*"),
    .expected = {
      { .type = SP_GLOB_TOK_ZERO_OR_MORE }
    }
  });
}

UTEST_F(glob, parse_recursive_prefix) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("**"),
    .expected = {
      { .type = SP_GLOB_TOK_RECURSIVE_PREFIX }
    }
  });
}

UTEST_F(glob, parse_recursive_prefix_with_slash) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("**/foo"),
    .expected = {
      { .type = SP_GLOB_TOK_RECURSIVE_PREFIX },
      { .type = SP_GLOB_TOK_LITERAL, .literal = 'f' },
      { .type = SP_GLOB_TOK_LITERAL, .literal = 'o' },
      { .type = SP_GLOB_TOK_LITERAL, .literal = 'o' }
    }
  });
}

UTEST_F(glob, parse_recursive_suffix) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("foo/**"),
    .expected = {
      { .type = SP_GLOB_TOK_LITERAL, .literal = 'f' },
      { .type = SP_GLOB_TOK_LITERAL, .literal = 'o' },
      { .type = SP_GLOB_TOK_LITERAL, .literal = 'o' },
      { .type = SP_GLOB_TOK_LITERAL, .literal = '/' },
      { .type = SP_GLOB_TOK_RECURSIVE_SUFFIX }
    }
  });
}

UTEST_F(glob, parse_recursive_zero_or_more) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("a/**/b"),
    .expected = {
      { .type = SP_GLOB_TOK_LITERAL, .literal = 'a' },
      { .type = SP_GLOB_TOK_LITERAL, .literal = '/' },
      { .type = SP_GLOB_TOK_RECURSIVE_ZERO_OR_MORE },
      { .type = SP_GLOB_TOK_LITERAL, .literal = 'b' }
    }
  });
}

UTEST_F(glob, parse_class_range) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("[a-z]"),
    .expected = {
      {
        .type = SP_GLOB_TOK_RANGES,
        .negated = false,
        .ranges = { { 'a', 'z' } }
      }
    }
  });
}

UTEST_F(glob, parse_class_multi) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("[abc]"),
    .expected = {
      {
        .type = SP_GLOB_TOK_RANGES,
        .negated = false,
        .ranges = { { 'a', 'a' }, { 'b', 'b' }, { 'c', 'c' } }
      }
    }
  });
}

UTEST_F(glob, parse_class_negated) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("[!a-z]"),
    .expected = {
      {
        .type = SP_GLOB_TOK_RANGES,
        .negated = true,
        .ranges = { { 'a', 'z' } }
      }
    }
  });
}

UTEST_F(glob, parse_class_negated_caret) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("[^0-9]"),
    .expected = {
      {
        .type = SP_GLOB_TOK_RANGES,
        .negated = true,
        .ranges = { { '0', '9' } }
      }
    }
  });
}

UTEST_F(glob, parse_class_unclosed) {
  run_parse_error_test(utest_result, &(parse_error_test_t) {
    .pattern = sp_str_lit("["),
    .expected_err = SP_GLOB_ERR_UNCLOSED_CLASS
  });
}

UTEST_F(glob, parse_class_invalid_range) {
  run_parse_error_test(utest_result, &(parse_error_test_t) {
    .pattern = sp_str_lit("[z-a]"),
    .expected_err = SP_GLOB_ERR_INVALID_RANGE
  });
}

UTEST_F(glob, parse_alternates_simple) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("{a,b}"),
    .expected = {
      {
        .type = SP_GLOB_TOK_ALTERNATES,
        .alts = {
          { SP_GLOB_TOK_LITERAL },
          { SP_GLOB_TOK_LITERAL }
        }
      }
    }
  });
}

UTEST_F(glob, parse_alternates_wildcards) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("{*.c,*.h}"),
    .expected = {
      {
        .type = SP_GLOB_TOK_ALTERNATES,
        .alts = {
          { SP_GLOB_TOK_ZERO_OR_MORE, SP_GLOB_TOK_LITERAL, SP_GLOB_TOK_LITERAL },
          { SP_GLOB_TOK_ZERO_OR_MORE, SP_GLOB_TOK_LITERAL, SP_GLOB_TOK_LITERAL }
        }
      }
    }
  });
}

UTEST_F(glob, parse_alternates_unclosed) {
  run_parse_error_test(utest_result, &(parse_error_test_t) {
    .pattern = sp_str_lit("{a,b"),
    .expected_err = SP_GLOB_ERR_UNCLOSED_ALTERNATES
  });
}

UTEST_F(glob, parse_star_txt) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("*.txt"),
    .expected = {
      { .type = SP_GLOB_TOK_ZERO_OR_MORE },
      { .type = SP_GLOB_TOK_LITERAL, .literal = '.' },
      { .type = SP_GLOB_TOK_LITERAL, .literal = 't' },
      { .type = SP_GLOB_TOK_LITERAL, .literal = 'x' },
      { .type = SP_GLOB_TOK_LITERAL, .literal = 't' }
    }
  });
}

///////////
// MATCH //
///////////
typedef struct {
  sp_str_t pattern;
  sp_str_t path;
  bool expected;
} match_test_t;

void run_match_test(int* utest_result, match_test_t* t) {
  sp_glob_t* g = sp_glob_new(t->pattern);
  EXPECT_TRUE(g != SP_NULLPTR);
  if (g) {
    bool result = sp_glob_match(g, t->path);
    EXPECT_EQ(result, t->expected);
  }
}

// Literal matching
UTEST_F(glob, match_literal_exact) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a"),
    .path = sp_str_lit("a"),
    .expected = true
  });
}

UTEST_F(glob, match_literal_multi) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("foo.txt"),
    .path = sp_str_lit("foo.txt"),
    .expected = true
  });
}

UTEST_F(glob, match_literal_mismatch) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a"),
    .path = sp_str_lit("b"),
    .expected = false
  });
}

UTEST_F(glob, match_literal_path_mismatch) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a"),
    .path = sp_str_lit("foo/a"),
    .expected = false
  });
}

// Single char wildcard (?)
UTEST_F(glob, match_any_single) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a?b"),
    .path = sp_str_lit("aXb"),
    .expected = true
  });
}

UTEST_F(glob, match_any_missing) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a?b"),
    .path = sp_str_lit("ab"),
    .expected = false
  });
}

UTEST_F(glob, match_any_triple) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("???"),
    .path = sp_str_lit("abc"),
    .expected = true
  });
}

// Zero-or-more wildcard (*)
UTEST_F(glob, match_star_anything) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("*"),
    .path = sp_str_lit("anything"),
    .expected = true
  });
}

UTEST_F(glob, match_star_extension) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("*.txt"),
    .path = sp_str_lit("foo.txt"),
    .expected = true
  });
}

UTEST_F(glob, match_star_dot_only) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("*.txt"),
    .path = sp_str_lit(".txt"),
    .expected = true
  });
}

UTEST_F(glob, match_star_empty) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a*b"),
    .path = sp_str_lit("ab"),
    .expected = true
  });
}

UTEST_F(glob, match_star_multi) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a*b"),
    .path = sp_str_lit("aXXXb"),
    .expected = true
  });
}

UTEST_F(glob, match_star_double) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a*b*c"),
    .path = sp_str_lit("abc"),
    .expected = true
  });
}

UTEST_F(glob, match_star_double_content) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a*b*c"),
    .path = sp_str_lit("aXbYc"),
    .expected = true
  });
}

UTEST_F(glob, match_star_double_fail) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a*b*c"),
    .path = sp_str_lit("abcd"),
    .expected = false
  });
}

UTEST_F(glob, match_star_empty_string) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("*"),
    .path = sp_str_lit(""),
    .expected = true
  });
}

// Recursive wildcard (**)
UTEST_F(glob, match_recursive_prefix_direct) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("**/foo"),
    .path = sp_str_lit("foo"),
    .expected = true
  });
}

UTEST_F(glob, match_recursive_prefix_one_dir) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("**/foo"),
    .path = sp_str_lit("a/foo"),
    .expected = true
  });
}

UTEST_F(glob, match_recursive_prefix_deep) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("**/foo"),
    .path = sp_str_lit("a/b/c/foo"),
    .expected = true
  });
}

UTEST_F(glob, match_recursive_prefix_no_boundary) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("**/foo"),
    .path = sp_str_lit("foofoo"),
    .expected = false
  });
}

UTEST_F(glob, match_recursive_suffix_direct) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("foo/**"),
    .path = sp_str_lit("foo/"),
    .expected = true
  });
}

UTEST_F(glob, match_recursive_suffix_one) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("foo/**"),
    .path = sp_str_lit("foo/bar"),
    .expected = true
  });
}

UTEST_F(glob, match_recursive_suffix_deep) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("foo/**"),
    .path = sp_str_lit("foo/bar/baz"),
    .expected = true
  });
}

UTEST_F(glob, match_recursive_middle_direct) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a/**/b"),
    .path = sp_str_lit("a/b"),
    .expected = true
  });
}

UTEST_F(glob, match_recursive_middle_one) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a/**/b"),
    .path = sp_str_lit("a/x/b"),
    .expected = true
  });
}

UTEST_F(glob, match_recursive_middle_deep) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a/**/b"),
    .path = sp_str_lit("a/x/y/z/b"),
    .expected = true
  });
}

UTEST_F(glob, match_recursive_all_empty) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("**"),
    .path = sp_str_lit(""),
    .expected = true
  });
}

UTEST_F(glob, match_recursive_all_anything) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("**"),
    .path = sp_str_lit("anything"),
    .expected = true
  });
}

// Character classes
UTEST_F(glob, match_class_single) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("[abc]"),
    .path = sp_str_lit("a"),
    .expected = true
  });
}

UTEST_F(glob, match_class_single_fail) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("[abc]"),
    .path = sp_str_lit("d"),
    .expected = false
  });
}

UTEST_F(glob, match_class_range) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("[a-z]"),
    .path = sp_str_lit("m"),
    .expected = true
  });
}

UTEST_F(glob, match_class_range_fail) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("[a-z]"),
    .path = sp_str_lit("5"),
    .expected = false
  });
}

UTEST_F(glob, match_class_multi_range) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("[0-9a-z]"),
    .path = sp_str_lit("5"),
    .expected = true
  });
}

UTEST_F(glob, match_class_negated) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("[!a-z]"),
    .path = sp_str_lit("5"),
    .expected = true
  });
}

UTEST_F(glob, match_class_negated_fail) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("[!a-z]"),
    .path = sp_str_lit("m"),
    .expected = false
  });
}

// Alternates
UTEST_F(glob, match_alt_first) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("{a,b}"),
    .path = sp_str_lit("a"),
    .expected = true
  });
}

UTEST_F(glob, match_alt_second) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("{a,b}"),
    .path = sp_str_lit("b"),
    .expected = true
  });
}

UTEST_F(glob, match_alt_fail) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("{a,b}"),
    .path = sp_str_lit("c"),
    .expected = false
  });
}

UTEST_F(glob, match_alt_extension_c) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("{*.c,*.h}"),
    .path = sp_str_lit("foo.c"),
    .expected = true
  });
}

UTEST_F(glob, match_alt_extension_h) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("{*.c,*.h}"),
    .path = sp_str_lit("foo.h"),
    .expected = true
  });
}

UTEST_F(glob, match_alt_empty) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("{}"),
    .path = sp_str_lit(""),
    .expected = true
  });
}

// Edge cases
UTEST_F(glob, match_hidden_file) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("**/.*"),
    .path = sp_str_lit(".hidden"),
    .expected = true
  });
}

UTEST_F(glob, match_hidden_file_dir) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("**/.*"),
    .path = sp_str_lit("dir/.hidden"),
    .expected = true
  });
}

UTEST_F(glob, match_path_exact) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a/b"),
    .path = sp_str_lit("a/b"),
    .expected = true
  });
}

//////////////
// STRATEGY //
//////////////
typedef struct {
  sp_str_t pattern;
  sp_glob_strategy_t expected;
} strategy_test_t;

void run_strategy_test(int* utest_result, strategy_test_t* t) {
  sp_glob_t glob = SP_ZERO_INITIALIZE();
  sp_glob_err_t err = sp_glob_parse(t->pattern, &glob.tokens);
  EXPECT_EQ(err, SP_GLOB_ERR_OK);
  if (err == SP_GLOB_ERR_OK) {
    sp_glob_strategy_t strategy = sp_glob_detect_strategy(&glob);
    EXPECT_EQ(strategy, t->expected);
  }
}

UTEST_F(glob, strategy_literal) {
  run_strategy_test(utest_result, &(strategy_test_t) {
    .pattern = sp_str_lit("foo.txt"),
    .expected = SP_GLOB_STRATEGY_LITERAL
  });
}

UTEST_F(glob, strategy_basename_literal) {
  run_strategy_test(utest_result, &(strategy_test_t) {
    .pattern = sp_str_lit("**/foo"),
    .expected = SP_GLOB_STRATEGY_BASENAME_LITERAL
  });
}

UTEST_F(glob, strategy_extension_simple) {
  run_strategy_test(utest_result, &(strategy_test_t) {
    .pattern = sp_str_lit("*.txt"),
    .expected = SP_GLOB_STRATEGY_EXTENSION
  });
}

UTEST_F(glob, strategy_extension_recursive) {
  run_strategy_test(utest_result, &(strategy_test_t) {
    .pattern = sp_str_lit("**/*.rs"),
    .expected = SP_GLOB_STRATEGY_EXTENSION
  });
}

UTEST_F(glob, strategy_prefix) {
  run_strategy_test(utest_result, &(strategy_test_t) {
    .pattern = sp_str_lit("src/*"),
    .expected = SP_GLOB_STRATEGY_PREFIX
  });
}

UTEST_F(glob, strategy_suffix) {
  run_strategy_test(utest_result, &(strategy_test_t) {
    .pattern = sp_str_lit("**/foo/bar"),
    .expected = SP_GLOB_STRATEGY_SUFFIX
  });
}

UTEST_F(glob, strategy_suffix_single_star) {
  run_strategy_test(utest_result, &(strategy_test_t) {
    .pattern = sp_str_lit("*/foo"),
    .expected = SP_GLOB_STRATEGY_SUFFIX
  });
}

UTEST_F(glob, strategy_recursive_only) {
  run_strategy_test(utest_result, &(strategy_test_t) {
    .pattern = sp_str_lit("**"),
    .expected = SP_GLOB_STRATEGY_FALLBACK
  });
}

//////////////
// GLOB SET //
//////////////
UTEST_F(glob, set_basic) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("*.c")));
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("*.h")));
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("Makefile")));
  sp_glob_set_build(set);

  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("foo.c")));
  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("foo.h")));
  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("Makefile")));
  EXPECT_FALSE(sp_glob_set_match(set, sp_str_lit("foo.rs")));
}

UTEST_F(glob, set_matches_indices) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("*.c")));   // idx 0
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("*.h")));   // idx 1
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("Makefile"))); // idx 2
  sp_glob_set_build(set);

  sp_da(u32) indices = SP_NULLPTR;

  sp_glob_set_matches(set, sp_str_lit("foo.c"), &indices);
  EXPECT_EQ(sp_da_size(indices), 1u);
  if (sp_da_size(indices) > 0) {
    EXPECT_EQ(indices[0], 0u);
  }

  indices = SP_NULLPTR;
  sp_glob_set_matches(set, sp_str_lit("foo.h"), &indices);
  EXPECT_EQ(sp_da_size(indices), 1u);
  if (sp_da_size(indices) > 0) {
    EXPECT_EQ(indices[0], 1u);
  }

  indices = SP_NULLPTR;
  sp_glob_set_matches(set, sp_str_lit("Makefile"), &indices);
  EXPECT_EQ(sp_da_size(indices), 1u);
  if (sp_da_size(indices) > 0) {
    EXPECT_EQ(indices[0], 2u);
  }

  indices = SP_NULLPTR;
  sp_glob_set_matches(set, sp_str_lit("foo.rs"), &indices);
  EXPECT_EQ(sp_da_size(indices), 0u);
}

UTEST_F(glob, set_nested_path) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("*.c")));
  sp_glob_set_build(set);

  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("src/foo.c")));
}

UTEST_F(glob, set_basename_literal) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("**/foo")));  // idx 0
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("*.foo")));   // idx 1
  sp_glob_set_build(set);

  sp_da(u32) indices = SP_NULLPTR;

  sp_glob_set_matches(set, sp_str_lit("foo"), &indices);
  EXPECT_EQ(sp_da_size(indices), 1u);
  if (sp_da_size(indices) > 0) {
    EXPECT_EQ(indices[0], 0u);
  }

  indices = SP_NULLPTR;
  sp_glob_set_matches(set, sp_str_lit("a/foo"), &indices);
  EXPECT_EQ(sp_da_size(indices), 1u);
  if (sp_da_size(indices) > 0) {
    EXPECT_EQ(indices[0], 0u);
  }

  indices = SP_NULLPTR;
  sp_glob_set_matches(set, sp_str_lit("bar.foo"), &indices);
  EXPECT_EQ(sp_da_size(indices), 1u);
  if (sp_da_size(indices) > 0) {
    EXPECT_EQ(indices[0], 1u);
  }
}

UTEST_F(glob, set_prefix) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("src/*")));
  sp_glob_set_build(set);

  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("src/main.c")));
  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("src/foo/bar.c")));
  EXPECT_FALSE(sp_glob_set_match(set, sp_str_lit("lib/main.c")));
}

UTEST_F(glob, set_suffix) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("**/foo/bar")));
  sp_glob_set_build(set);

  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("foo/bar")));
  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("a/foo/bar")));
  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("a/b/foo/bar")));
  EXPECT_FALSE(sp_glob_set_match(set, sp_str_lit("foo/baz")));
}

UTEST_F(glob, set_fallback) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("a?b")));
  sp_glob_set_build(set);

  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("aXb")));
  EXPECT_FALSE(sp_glob_set_match(set, sp_str_lit("ab")));
}

UTEST_F(glob, set_empty) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_build(set);

  EXPECT_FALSE(sp_glob_set_match(set, sp_str_lit("foo.c")));
  EXPECT_FALSE(sp_glob_set_match(set, sp_str_lit("")));

  sp_da(u32) indices = SP_NULLPTR;
  sp_glob_set_matches(set, sp_str_lit("foo.c"), &indices);
  EXPECT_EQ(sp_da_size(indices), 0u);
}

UTEST_F(glob, set_multiple_matches) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("*.c")));    // idx 0 - EXTENSION
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("**")));     // idx 1 - FALLBACK (matches all)
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("**/foo.c"))); // idx 2 - BASENAME_LITERAL
  sp_glob_set_build(set);

  sp_da(u32) indices = SP_NULLPTR;
  sp_glob_set_matches(set, sp_str_lit("foo.c"), &indices);

  // Should match all three
  EXPECT_EQ(sp_da_size(indices), 3u);
}

UTEST_F(glob, set_recursive_extension) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("**/*.rs")));
  sp_glob_set_build(set);

  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("foo.rs")));
  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("src/main.rs")));
  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("a/b/c/lib.rs")));
  EXPECT_FALSE(sp_glob_set_match(set, sp_str_lit("foo.c")));
}

UTEST_F(glob, set_duplicate_extension) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("*.c")));  // idx 0
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("*.c")));  // idx 1
  sp_glob_set_build(set);

  sp_da(u32) indices = SP_NULLPTR;
  sp_glob_set_matches(set, sp_str_lit("foo.c"), &indices);

  // Both should match
  EXPECT_EQ(sp_da_size(indices), 2u);
}

UTEST_F(glob, parse_empty) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit(""),
    .expected = { { .type = SP_GLOB_TOK_NONE } }
  });
}

UTEST_F(glob, parse_class_literal_bracket) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("[]]"),
    .expected = {
      {
        .type = SP_GLOB_TOK_RANGES,
        .negated = false,
        .ranges = { { ']', ']' } }
      }
    }
  });
}

UTEST_F(glob, parse_class_literal_dash_end) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("[a-]"),
    .expected = {
      {
        .type = SP_GLOB_TOK_RANGES,
        .negated = false,
        .ranges = { { 'a', 'a' }, { '-', '-' } }
      }
    }
  });
}

UTEST_F(glob, parse_alternates_empty_branch) {
  sp_da(sp_glob_token_t) tokens = SP_NULLPTR;
  sp_glob_err_t err = sp_glob_parse(sp_str_lit("{,a}"), &tokens);
  EXPECT_EQ(err, SP_GLOB_ERR_OK);
  EXPECT_EQ(sp_da_size(tokens), 1u);
  if (sp_da_size(tokens) > 0) {
    EXPECT_EQ(tokens[0].type, SP_GLOB_TOK_ALTERNATES);
    EXPECT_EQ(sp_da_size(tokens[0].alternates.alts), 2u);
    if (sp_da_size(tokens[0].alternates.alts) >= 2) {
      EXPECT_EQ(sp_da_size(tokens[0].alternates.alts[0]), 0u);
      EXPECT_EQ(sp_da_size(tokens[0].alternates.alts[1]), 1u);
    }
  }
}

UTEST_F(glob, parse_alternates_class_inside) {
  run_parse_test(utest_result, &(parse_test_t) {
    .pattern = sp_str_lit("{[a-z],b}"),
    .expected = {
      {
        .type = SP_GLOB_TOK_ALTERNATES,
        .alts = {
          { SP_GLOB_TOK_RANGES },
          { SP_GLOB_TOK_LITERAL }
        }
      }
    }
  });
}

UTEST_F(glob, match_any_no_slash) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a?b"),
    .path = sp_str_lit("a/b"),
    .expected = false
  });
}

// UTEST_F(glob, match_star_no_cross_slash) {
//   run_match_test(utest_result, &(match_test_t) {
//     .pattern = sp_str_lit("foo*"),
//     .path = sp_str_lit("foo/bar"),
//     .expected = false
//   });
// }

UTEST_F(glob, match_class_exhausted_path) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("foo[a-z]"),
    .path = sp_str_lit("foo"),
    .expected = false
  });
}

UTEST_F(glob, match_alternate_with_remaining) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("{a,b}c"),
    .path = sp_str_lit("ac"),
    .expected = true
  });
}

UTEST_F(glob, match_extension_multiple_dots) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("*.txt"),
    .path = sp_str_lit("foo.bar.txt"),
    .expected = true
  });
}

UTEST_F(glob, match_recursive_suffix_no_trailing) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a/**"),
    .path = sp_str_lit("a"),
    .expected = false
  });
}

UTEST_F(glob, strategy_empty) {
  run_strategy_test(utest_result, &(strategy_test_t) {
    .pattern = sp_str_lit(""),
    .expected = SP_GLOB_STRATEGY_LITERAL
  });
}

UTEST_F(glob, strategy_single_star) {
  run_strategy_test(utest_result, &(strategy_test_t) {
    .pattern = sp_str_lit("*"),
    .expected = SP_GLOB_STRATEGY_SUFFIX
  });
}

UTEST_F(glob, strategy_extension_with_wildcard) {
  run_strategy_test(utest_result, &(strategy_test_t) {
    .pattern = sp_str_lit("*.t?t"),
    .expected = SP_GLOB_STRATEGY_FALLBACK
  });
}

UTEST_F(glob, strategy_star_in_middle) {
  run_strategy_test(utest_result, &(strategy_test_t) {
    .pattern = sp_str_lit("foo*bar"),
    .expected = SP_GLOB_STRATEGY_FALLBACK
  });
}

UTEST_F(glob, set_no_extension) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("*.txt")));
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("*.c")));
  sp_glob_set_build(set);

  EXPECT_FALSE(sp_glob_set_match(set, sp_str_lit("Makefile")));
  EXPECT_FALSE(sp_glob_set_match(set, sp_str_lit("README")));
}

UTEST_F(glob, set_multiple_prefixes) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("src/*")));
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("src/foo/*")));
  sp_glob_set_build(set);

  sp_da(u32) indices = SP_NULLPTR;
  sp_glob_set_matches(set, sp_str_lit("src/foo/bar.c"), &indices);
  EXPECT_EQ(sp_da_size(indices), 2u);
}

UTEST_F(glob, set_multiple_dots) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("*.baz")));
  sp_glob_set_build(set);

  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("foo.bar.baz")));
}

UTEST_F(glob, set_single_component_suffix) {
  sp_glob_set_t* set = sp_glob_set_new();
  sp_glob_set_add(set, sp_glob_new(sp_str_lit("**/bar")));
  sp_glob_set_build(set);

  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("bar")));
  EXPECT_TRUE(sp_glob_set_match(set, sp_str_lit("foo/bar")));
}

// Test ** not at component boundary (should be treated as two *)
UTEST_F(glob, match_double_star_not_boundary) {
  // a**b should match like a*b (two * in sequence)
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a**b"),
    .path = sp_str_lit("ab"),
    .expected = true
  });
}

UTEST_F(glob, match_double_star_not_boundary_content) {
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a**b"),
    .path = sp_str_lit("aXXXb"),
    .expected = true
  });
}

UTEST_F(glob, match_double_star_not_boundary_no_slash) {
  // * doesn't cross /, so a**b should not match a/b
  run_match_test(utest_result, &(match_test_t) {
    .pattern = sp_str_lit("a**b"),
    .path = sp_str_lit("a/b"),
    .expected = false
  });
}

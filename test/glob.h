#ifndef SPN_TEST_GLOB_H
#define SPN_TEST_GLOB_H

#include "sp.h"

typedef enum {
  SP_GLOB_TOK_NONE,
  SP_GLOB_TOK_LITERAL,                // a
  SP_GLOB_TOK_ANY,                    // ?
  SP_GLOB_TOK_ZERO_OR_MORE,           // *
  SP_GLOB_TOK_RECURSIVE_PREFIX,       // **/foo
  SP_GLOB_TOK_RECURSIVE_SUFFIX,       // foo/**
  SP_GLOB_TOK_RECURSIVE_ZERO_OR_MORE, // foo/**/bar
  SP_GLOB_TOK_RANGES,                 // [a-z]
  SP_GLOB_TOK_ALTERNATES,             // {a,b}
} sp_glob_token_type_t;

typedef struct {
  c8 start;
  c8 end;
} sp_glob_char_range_t;

typedef struct sp_glob_token_t sp_glob_token_t;
struct sp_glob_token_t {
  sp_glob_token_type_t type;
  union {
    c8 literal;
    struct {
      bool negated;
      sp_da(sp_glob_char_range_t) ranges;
    } ranges;
    struct {
      sp_da(sp_da(sp_glob_token_t)) alts;
    } alternates;
  };
};

typedef enum {
  SP_GLOB_ERR_OK,
  SP_GLOB_ERR_UNCLOSED_CLASS,
  SP_GLOB_ERR_INVALID_RANGE,
  SP_GLOB_ERR_UNCLOSED_ALTERNATES,
} sp_glob_err_t;

typedef enum {
  SP_GLOB_STRATEGY_LITERAL,          // foo.txt
  SP_GLOB_STRATEGY_BASENAME_LITERAL, // **/foo
  SP_GLOB_STRATEGY_EXTENSION,        // **/*.txt
  SP_GLOB_STRATEGY_PREFIX,           // src/*
  SP_GLOB_STRATEGY_SUFFIX,           // **/foo/bar
  SP_GLOB_STRATEGY_FALLBACK,         // anything else; can't use a heuristic, have to do a full match
} sp_glob_strategy_t;

typedef struct sp_glob_t {
  sp_da(sp_glob_token_t) tokens;
  sp_glob_strategy_t strategy;
  sp_str_t pattern;
  sp_str_t literal; // view into pattern for fast-path matching
} sp_glob_t;

typedef struct {
  sp_str_t path;
  sp_str_t basename;
  sp_str_t ext;
} sp_glob_candidate_t;

typedef struct {
  sp_str_t literal;
  u32 idx;
} sp_glob_set_prefix_entry_t;

typedef struct {
  sp_str_t suffix;
  u32 idx;
  bool component; // must match at path component boundary
} sp_glob_set_suffix_entry_t;

typedef struct {
  sp_glob_t* glob;
  u32 idx;
} sp_glob_set_fallback_entry_t;

typedef sp_ht(sp_str_t, sp_da(u32)) sp_glob_set_index_table_t;

typedef struct sp_glob_set_t {
  sp_da(sp_glob_t*) globs;

  sp_glob_set_index_table_t literal;
  sp_glob_set_index_table_t base_name;
  sp_glob_set_index_table_t extension;
  sp_da(sp_glob_set_prefix_entry_t) prefixes;
  sp_da(sp_glob_set_suffix_entry_t) suffixes;
  sp_da(sp_glob_set_fallback_entry_t) fallbacks;
} sp_glob_set_t;

sp_glob_t*          sp_glob_new(sp_str_t pattern);
bool                sp_glob_match(sp_glob_t* g, sp_str_t path);
sp_glob_set_t*      sp_glob_set_new();
void                sp_glob_set_add(sp_glob_set_t* set, sp_glob_t* g);
void                sp_glob_set_build(sp_glob_set_t* set);
bool                sp_glob_set_match(sp_glob_set_t* set, sp_str_t path);
void                sp_glob_set_matches(sp_glob_set_t* set, sp_str_t path, sp_da(u32)* out);

void                sp_glob_push_literal(sp_da(sp_glob_token_t)* tokens, c8 c);
void                sp_glob_push_token(sp_da(sp_glob_token_t)* tokens, sp_glob_token_type_t type);
sp_glob_err_t       sp_glob_parse_star(sp_str_t pattern, u32* it, sp_da(sp_glob_token_t)* tokens);
sp_glob_err_t       sp_glob_parse_class(sp_str_t pattern, u32* i, sp_da(sp_glob_token_t)* out);
sp_glob_err_t       sp_glob_parse_alternates(sp_str_t pattern, u32* it, sp_da(sp_glob_token_t)* tokens);
sp_glob_err_t       sp_glob_parse(sp_str_t pattern, sp_da(sp_glob_token_t)* tokens);
bool                sp_glob_match_impl(sp_da(sp_glob_token_t) tokens, u32 ti, sp_str_t path, u32 pi);
bool                sp_glob_match_class(sp_glob_token_t* tok, c8 c);
bool                sp_glob_match_alternates(sp_glob_token_t* tok, sp_da(sp_glob_token_t) tokens, u32 ti, sp_str_t path, u32 pi);
bool                sp_glob_match_tokens(sp_da(sp_glob_token_t) tokens, sp_str_t path);
sp_glob_strategy_t  sp_glob_detect_strategy(sp_glob_t* glob);
sp_glob_candidate_t sp_glob_candidate_new(sp_str_t path);
void                sp_glob_set_ht_add(sp_glob_set_index_table_t* ht, sp_str_t key, u32 idx);
void                sp_glob_set_ht_collect(sp_glob_set_index_table_t ht, sp_str_t key, sp_da(u32)* out);
bool                sp_glob_set_suffix_match(sp_glob_set_suffix_entry_t* e, sp_str_t path);



void sp_glob_push_literal(sp_da(sp_glob_token_t)* tokens, c8 c) {
  sp_da_push(*tokens, ((sp_glob_token_t) { .type = SP_GLOB_TOK_LITERAL, .literal = c }));
}

void sp_glob_push_token(sp_da(sp_glob_token_t)* tokens, sp_glob_token_type_t type) {
  sp_da_push(*tokens, ((sp_glob_token_t) { .type = type }));
}

sp_glob_err_t sp_glob_parse_star(sp_str_t pattern, u32* it, sp_da(sp_glob_token_t)* tokens) {
  u32 pos = *it;
  u32 len = pattern.len;

  if (pos + 1 < len && pattern.data[pos + 1] == '*') {
    (*it)++;
    pos = *it;

    bool at_start = (pos == 1);
    bool at_end = (pos + 1 >= len);
    bool next_is_sep = (!at_end && pattern.data[pos + 1] == '/');
    bool prev_is_sep = (pos >= 2 && pattern.data[pos - 2] == '/');

    if (at_start && (at_end || next_is_sep)) {
      sp_glob_push_token(tokens, SP_GLOB_TOK_RECURSIVE_PREFIX);
      if (next_is_sep) {
        (*it)++;
      }
    } else if (prev_is_sep && at_end) {
      sp_glob_push_token(tokens, SP_GLOB_TOK_RECURSIVE_SUFFIX);
    } else if (prev_is_sep && next_is_sep) {
      sp_glob_push_token(tokens, SP_GLOB_TOK_RECURSIVE_ZERO_OR_MORE);
      (*it)++;
    } else {
      sp_glob_push_token(tokens, SP_GLOB_TOK_ZERO_OR_MORE);
      sp_glob_push_token(tokens, SP_GLOB_TOK_ZERO_OR_MORE);
    }
  } else {
    sp_glob_push_token(tokens, SP_GLOB_TOK_ZERO_OR_MORE);
  }

  return SP_GLOB_ERR_OK;
}

sp_glob_err_t sp_glob_parse_class(sp_str_t pattern, u32* i, sp_da(sp_glob_token_t)* out) {
  u32 pos = *i + 1;
  u32 len = pattern.len;

  if (pos >= len) {
    return SP_GLOB_ERR_UNCLOSED_CLASS;
  }

  bool negated = false;
  c8 c = sp_str_at(pattern, pos);
  if (c == '!' || c == '^') {
    negated = true;
    pos++;
  }

  sp_da(sp_glob_char_range_t) ranges = SP_NULLPTR;
  c8 last_char = 0;
  bool has_content = false;

  while (pos < len) {
    c = sp_str_at(pattern, pos);

    if (c == ']' && has_content) {
      sp_da_push(*out, ((sp_glob_token_t) {
        .type = SP_GLOB_TOK_RANGES,
        .ranges = { .negated = negated, .ranges = ranges }
      }));
      *i = pos;
      return SP_GLOB_ERR_OK;
    }

    if (has_content && pos + 1 < len && c == '-') {
      c8 peek = pattern.data[pos + 1];
      if (peek != ']') {
        c8 range_start = last_char;
        c8 range_end = peek;
        if (range_start > range_end) {
          return SP_GLOB_ERR_INVALID_RANGE;
        }
        if (!sp_da_empty(ranges)) {
          ranges[sp_da_size(ranges) - 1].end = range_end;
        }
        pos += 2;
        last_char = range_end;
        continue;
      }
    }


    sp_da_push(ranges, ((sp_glob_char_range_t) { .start = c, .end = c }));
    last_char = c;
    has_content = true;
    pos++;
  }

  return SP_GLOB_ERR_UNCLOSED_CLASS;
}

sp_glob_err_t sp_glob_parse_alternates(sp_str_t pattern, u32* it, sp_da(sp_glob_token_t)* tokens) {
  u32 pos = *it + 1;
  u32 len = pattern.len;

  sp_da(sp_da(sp_glob_token_t)) alts = SP_NULLPTR;
  sp_da(sp_glob_token_t) alt = SP_NULLPTR;

  while (pos < len) {
    c8 c = sp_str_at(pattern, (s32)pos);

    if (c == '}') {
      sp_da_push(alts, alt);
      sp_da_push(*tokens, ((sp_glob_token_t) {
        .type = SP_GLOB_TOK_ALTERNATES,
        .alternates = {.alts = alts}
      }));
      *it = pos;
      return SP_GLOB_ERR_OK;
    }

    if (c == ',') {
      sp_da_push(alts, alt);
      alt = SP_NULLPTR;
      pos++;
      continue;
    }

    switch (c) {
      case '?': {
        sp_glob_push_token(&alt, SP_GLOB_TOK_ANY);
        break;
      }
      case '*': {
        sp_try(sp_glob_parse_star(pattern, &pos, &alt));
        break;
      }
      case '[': {
        sp_try(sp_glob_parse_class(pattern, &pos, &alt));
        break;
      }
      default: {
        sp_glob_push_literal(&alt, c);
        break;
      }
    }
    pos++;
  }

  return SP_GLOB_ERR_UNCLOSED_ALTERNATES;
}

sp_glob_err_t sp_glob_parse(sp_str_t pattern, sp_da(sp_glob_token_t)* tokens) {
  for (u32 it = 0; it < pattern.len;) {
    c8 c = pattern.data[it];
    switch (c) {
      case '?': { sp_glob_push_token(tokens, SP_GLOB_TOK_ANY); break; }
      case '*': { sp_try(sp_glob_parse_star(pattern, &it, tokens)); break; }
      case '[': { sp_try(sp_glob_parse_class(pattern, &it, tokens)); break; }
      case '{': { sp_try(sp_glob_parse_alternates(pattern, &it, tokens)); break; }
      default:  { sp_glob_push_literal(tokens, c); break; }
    }
    it++;
  }
  return SP_GLOB_ERR_OK;
}

bool sp_glob_match_class(sp_glob_token_t* tok, c8 c) {
  sp_da_for(tok->ranges.ranges, i) {
    sp_glob_char_range_t* r = &tok->ranges.ranges[i];
    if (c >= r->start && c <= r->end) {
      return !tok->ranges.negated;
    }
  }
  return tok->ranges.negated;
}

bool sp_glob_match_alternates(sp_glob_token_t* tok, sp_da(sp_glob_token_t) tokens, u32 ti, sp_str_t path, u32 pi) {
  sp_da_for(tok->alternates.alts, i) {
    sp_da(sp_glob_token_t) alt = tok->alternates.alts[i];

    sp_da(sp_glob_token_t) combined = SP_NULLPTR;
    sp_da_for(alt, j) {
      sp_da_push(combined, alt[j]);
    }
    u32 remaining = sp_da_size(tokens);
    for (u32 j = ti; j < remaining; j++) {
      sp_da_push(combined, tokens[j]);
    }

    if (sp_glob_match_impl(combined, 0, path, pi)) {
      return true;
    }
  }
  return false;
}

bool sp_glob_match_impl(sp_da(sp_glob_token_t) tokens, u32 ti, sp_str_t path, u32 pi) {
  u32 tok_len = sp_da_size(tokens);
  u32 path_len = path.len;

  while (ti < tok_len) {
    sp_glob_token_t* tok = &tokens[ti];

    switch (tok->type) {
      case SP_GLOB_TOK_NONE: {
        return false;
      }
      case SP_GLOB_TOK_LITERAL: {
        if (pi >= path_len) return false;
        if (path.data[pi] != tok->literal) return false;

        pi++;
        ti++;
        break;
      }
      case SP_GLOB_TOK_ANY: {
        if (pi >= path_len) return false;
        if (path.data[pi] == '/') return false;

        pi++;
        ti++;
        break;
      }
      case SP_GLOB_TOK_ZERO_OR_MORE: {
        ti++;
        for (u32 k = pi; k <= path_len; k++) {
          if (k > pi && path.data[k - 1] == '/') {
            break;
          }
          if (sp_glob_match_impl(tokens, ti, path, k)) {
            return true;
          }
        }
        return false;
      }
      case SP_GLOB_TOK_RECURSIVE_PREFIX: {
        ti++;
        if (ti >= tok_len) return true;

        if (sp_glob_match_impl(tokens, ti, path, pi)) {
          return true;
        }

        for (u32 k = pi; k < path_len; k++) {
          if (sp_str_at(path, (s32)k) == '/') {
            if (sp_glob_match_impl(tokens, ti, path, k + 1)) {
              return true;
            }
          }
        }
        return false;
      }
      case SP_GLOB_TOK_RECURSIVE_SUFFIX: {
        return true;
      }
      case SP_GLOB_TOK_RECURSIVE_ZERO_OR_MORE: {
        ti++;
        if (sp_glob_match_impl(tokens, ti, path, pi)) {
          return true;
        }
        for (u32 k = pi; k < path_len; k++) {
          if (sp_str_at(path, k) == '/') {
            if (sp_glob_match_impl(tokens, ti, path, k + 1)) {
              return true;
            }
          }
        }
        return false;
      }
      case SP_GLOB_TOK_RANGES: {
        if (pi >= path_len) return false;

        c8 c = sp_str_at(path, pi);
        if (!sp_glob_match_class(tok, c)) {
          return false;
        }
        pi++;
        ti++;
        break;
      }
      case SP_GLOB_TOK_ALTERNATES: {
        return sp_glob_match_alternates(tok, tokens, ti + 1, path, pi);
      }
    }
  }

  return pi >= path_len;
}

bool sp_glob_match_tokens(sp_da(sp_glob_token_t) tokens, sp_str_t path) {
  return sp_glob_match_impl(tokens, 0, path, 0);
}

sp_glob_strategy_t sp_glob_detect_strategy(sp_glob_t* glob) {
  sp_da(sp_glob_token_t) tokens = glob->tokens;
  u32 len = sp_da_size(tokens);
  if (!len) return SP_GLOB_STRATEGY_LITERAL;

  sp_glob_token_type_t first_type = tokens[0].type;
  sp_glob_token_type_t last_type = tokens[len - 1].type;

  bool is_first_recursive_prefix = (first_type == SP_GLOB_TOK_RECURSIVE_PREFIX) && (len > 1);
  bool is_first_zero_or_more = (first_type == SP_GLOB_TOK_ZERO_OR_MORE);
  bool is_last_zero_or_more = (last_type == SP_GLOB_TOK_ZERO_OR_MORE);

  bool is_all_literals = (first_type == SP_GLOB_TOK_LITERAL);
  bool is_all_literals_after_first = true;
  bool is_all_literals_except_last = (len < 2) || (first_type == SP_GLOB_TOK_LITERAL);
  bool is_slash_after_first = false;

  u32 ext_start = is_first_recursive_prefix ? 1 : 0;
  bool is_ext_prefix_ok = (ext_start + 2 < len) &&
                          (tokens[ext_start].type == SP_GLOB_TOK_ZERO_OR_MORE) &&
                          (tokens[ext_start + 1].type == SP_GLOB_TOK_LITERAL) &&
                          (tokens[ext_start + 1].literal == '.');
  bool is_extension = is_ext_prefix_ok;

  for (u32 i = 1; i < len; i++) {
    bool is_literal = (tokens[i].type == SP_GLOB_TOK_LITERAL);

    is_all_literals_after_first = is_all_literals_after_first && is_literal;
    if (i < len - 1) {
      is_all_literals_except_last = is_all_literals_except_last && is_literal;
    }
    if (is_literal && tokens[i].literal == '/') {
      is_slash_after_first = true;
    }
    if (i > ext_start + 1 && !is_literal) {
      is_extension = false;
    }
  }

  is_all_literals = is_all_literals && is_all_literals_after_first;

  if (is_all_literals) {
    return SP_GLOB_STRATEGY_LITERAL;
  }

  if (is_first_recursive_prefix && is_all_literals_after_first && !is_slash_after_first) {
    return SP_GLOB_STRATEGY_BASENAME_LITERAL;
  }

  if (is_extension) {
    return SP_GLOB_STRATEGY_EXTENSION;
  }

  if (len >= 2 && is_last_zero_or_more && is_all_literals_except_last) {
    return SP_GLOB_STRATEGY_PREFIX;
  }

  if ((is_first_zero_or_more || is_first_recursive_prefix) && is_all_literals_after_first) {
    if (is_first_zero_or_more || is_slash_after_first) {
      return SP_GLOB_STRATEGY_SUFFIX;
    }
  }

  return SP_GLOB_STRATEGY_FALLBACK;
}

sp_glob_t* sp_glob_new(sp_str_t pattern) {
  sp_glob_t* g = SP_ALLOC(sp_glob_t);
  g->pattern = pattern;

  if (sp_glob_parse(pattern, &g->tokens)) {
    return SP_NULLPTR;
  }

  g->strategy = sp_glob_detect_strategy(g);

  u32 len = pattern.len;

  switch (g->strategy) {
    case SP_GLOB_STRATEGY_LITERAL: {
      g->literal = pattern;
      break;
    }
    case SP_GLOB_STRATEGY_BASENAME_LITERAL: {
      SP_ASSERT(len >= 3);
      g->literal = sp_str_suffix(pattern, len - 3);
      break;
    }
    case SP_GLOB_STRATEGY_EXTENSION: {
      g->literal = sp_str_cleave_c8(pattern, '.').second;
      break;
    }
    case SP_GLOB_STRATEGY_PREFIX: {
      g->literal = sp_str_prefix(pattern, pattern.len - 1);
      break;
    }
    case SP_GLOB_STRATEGY_SUFFIX: {
      if (sp_str_starts_with(pattern, sp_str_lit("**"))) {
        g->literal = sp_str_suffix(pattern, pattern.len - 2);
      }
      else {
        g->literal = sp_str_suffix(pattern, pattern.len - 1);
      }
      break;
    }
    case SP_GLOB_STRATEGY_FALLBACK: {
      break;
    }
  }

  return g;
}

bool sp_glob_match(sp_glob_t* g, sp_str_t path) {
  SP_ASSERT(g);

  switch (g->strategy) {
    case SP_GLOB_STRATEGY_LITERAL: {
      return sp_str_equal(g->literal, path);
    }
    case SP_GLOB_STRATEGY_BASENAME_LITERAL: {
      sp_str_t basename = sp_fs_get_name(path);
      return sp_str_equal(g->literal, basename);
    }
    case SP_GLOB_STRATEGY_EXTENSION: {
      return sp_str_ends_with(path, g->literal);
    }
    case SP_GLOB_STRATEGY_PREFIX: {
      return sp_str_starts_with(path, g->literal);
    }
    case SP_GLOB_STRATEGY_SUFFIX: {
      if (sp_str_ends_with(path, g->literal)) {
        return true;
      }

      if (sp_str_starts_with(g->literal, sp_str_lit("/"))) {
        return sp_str_equal(path, sp_str_suffix(g->literal, g->literal.len - 1));
      }

      return false;
    }
    case SP_GLOB_STRATEGY_FALLBACK: {
      return sp_glob_match_tokens(g->tokens, path);
    }
  }

  return false;
}

sp_glob_candidate_t sp_glob_candidate_new(sp_str_t path) {
  return (sp_glob_candidate_t){
    .path = path,
    .basename = sp_fs_get_name(path),
    .ext = sp_fs_get_ext(path),
  };
}

sp_glob_set_t* sp_glob_set_new() {
  sp_glob_set_t* set = SP_ALLOC(sp_glob_set_t);
  sp_ht_set_fns(set->literal, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);
  sp_ht_set_fns(set->base_name, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);
  sp_ht_set_fns(set->extension, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);
  return set;
}

void sp_glob_set_add(sp_glob_set_t* set, sp_glob_t* g) {
  SP_ASSERT(set);
  SP_ASSERT(g);
  sp_da_push(set->globs, g);
}

void sp_glob_set_ht_add(sp_glob_set_index_table_t* ht, sp_str_t key, u32 idx) {
  sp_da(u32)* indices = sp_ht_getp(*ht, key);
  if (!indices) {
    sp_ht_insert(*ht, key, SP_NULLPTR);
    indices = sp_ht_getp(*ht, key);
  }
  sp_da_push(*indices, idx);
}

void sp_glob_set_build(sp_glob_set_t* set) {
  SP_ASSERT(set);

  sp_da_for(set->globs, i) {
    sp_glob_t* g = set->globs[i];
    u32 idx = (u32)i;

    switch (g->strategy) {
      case SP_GLOB_STRATEGY_LITERAL: {
        sp_glob_set_ht_add(&set->literal, g->literal, idx);
        break;
      }
      case SP_GLOB_STRATEGY_BASENAME_LITERAL: {
        sp_glob_set_ht_add(&set->base_name, g->literal, idx);
        break;
      }
      case SP_GLOB_STRATEGY_EXTENSION: {
        sp_glob_set_ht_add(&set->extension, g->literal, idx);
        break;
      }
      case SP_GLOB_STRATEGY_PREFIX: {
        sp_glob_set_prefix_entry_t entry = {.literal = g->literal, .idx = idx};
        sp_da_push(set->prefixes, entry);
        break;
      }
      case SP_GLOB_STRATEGY_SUFFIX: {
        sp_da_push(set->suffixes, ((sp_glob_set_suffix_entry_t) {
          .suffix = g->literal,
          .idx = idx,
          .component = sp_str_starts_with(g->literal, sp_str_lit("/"))
        }));
        break;
      }
      case SP_GLOB_STRATEGY_FALLBACK: {
        sp_glob_set_fallback_entry_t entry = {.glob = g, .idx = idx};
        sp_da_push(set->fallbacks, entry);
        break;
      }
    }
  }
}

void sp_glob_set_ht_collect(sp_glob_set_index_table_t ht, sp_str_t key, sp_da(u32)* out) {
  sp_da(u32)* indices = sp_ht_getp(ht, key);
  if (indices) {
    sp_da_for(*indices, i) {
      sp_da_push(*out, (*indices)[i]);
    }
  }
}

bool sp_glob_set_suffix_match(sp_glob_set_suffix_entry_t* e, sp_str_t path) {
  if (sp_str_ends_with(path, e->suffix)) {
    return true;
  }
  if (e->component && !sp_str_empty(e->suffix) && e->suffix.data[0] == '/') {
    sp_str_t suffix_no_slash = sp_str_sub(e->suffix, 1, (s32)(e->suffix.len - 1));
    if (sp_str_equal(path, suffix_no_slash)) {
      return true;
    }
  }
  return false;
}

void sp_glob_set_matches(sp_glob_set_t* set, sp_str_t path, sp_da(u32)* out) {
  SP_ASSERT(set);
  SP_ASSERT(out);

  sp_glob_candidate_t c = sp_glob_candidate_new(path);

  sp_glob_set_ht_collect(set->literal, c.path, out);
  sp_glob_set_ht_collect(set->base_name, c.basename, out);
  if (!sp_str_empty(c.ext)) {
    sp_glob_set_ht_collect(set->extension, c.ext, out);
  }

  sp_da_for(set->prefixes, it) {
    sp_glob_set_prefix_entry_t* prefix = &set->prefixes[it];
    if (sp_str_starts_with(c.path, prefix->literal)) {
      sp_da_push(*out, prefix->idx);
    }
  }

  sp_da_for(set->suffixes, it) {
    if (sp_glob_set_suffix_match(&set->suffixes[it], c.path)) {
      sp_da_push(*out, set->suffixes[it].idx);
    }
  }

  sp_da_for(set->fallbacks, it) {
    sp_glob_set_fallback_entry_t* e = &set->fallbacks[it];
    if (sp_glob_match_tokens(e->glob->tokens, c.path)) {
      sp_da_push(*out, e->idx);
    }
  }
}

bool sp_glob_set_match(sp_glob_set_t* set, sp_str_t path) {
  SP_ASSERT(set);

  sp_glob_candidate_t c = sp_glob_candidate_new(path);

  if (sp_ht_key_exists(set->literal, c.path)) {
    return true;
  }
  if (sp_ht_key_exists(set->base_name, c.basename)) {
    return true;
  }
  if (!sp_str_empty(c.ext) && sp_ht_key_exists(set->extension, c.ext)) {
    return true;
  }

  sp_da_for(set->prefixes, i) {
    if (sp_str_starts_with(c.path, set->prefixes[i].literal)) {
      return true;
    }
  }

  sp_da_for(set->suffixes, i) {
    if (sp_glob_set_suffix_match(&set->suffixes[i], c.path)) {
      return true;
    }
  }

  sp_da_for(set->fallbacks, i) {
    if (sp_glob_match_tokens(set->fallbacks[i].glob->tokens, c.path)) {
      return true;
    }
  }

  return false;
}

#endif // SPN_TEST_GLOB_H

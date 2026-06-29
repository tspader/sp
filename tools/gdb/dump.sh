#!/usr/bin/env bash
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd "$here/../.." && pwd)"

cc="${CC:-cc}"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

fixture="$tmp/gdb_fixture"
"$cc" -std=c99 -g -DSP_IMPLEMENTATION -I"$root" -I"$root/test/tools" \
  -o "$fixture" "$root/test/gdb_fixture.c"

scripts=(
  sp_str_t.py sp_tm_epoch_t.py sp_da.py sp_ht.py
  sp_om.py sp_mem_arena.py sp_ps.py
)
source_args=()
for s in "${scripts[@]}"; do
  source_args+=(-ex "source $here/$s")
done

bold=$'\033[1m'; cyan=$'\033[36m'; reset=$'\033[0m'

dump() {
  local label="$1" mark="$2" command="$3"
  printf '%s%s── %s %s(%s)%s\n' "$bold" "$cyan" "$label" "$reset$cyan" "$command" "$reset"
  gdb -q -nx -batch \
    -ex 'set debuginfod enabled off' -ex 'set pagination off' \
    "${source_args[@]}" \
    -ex "break $mark" -ex run \
    -ex 'echo __SP_BEGIN__\n' -ex "$command" -ex 'echo __SP_END__\n' \
    -ex kill "$fixture" 2>/dev/null \
    | awk '/__SP_BEGIN__/{f=1;next} /__SP_END__/{f=0} f'
  echo
}

dump "sp_str_t"               brk_str          "print subject"
dump "sp_str_t (empty)"       brk_str_empty    "print subject"
dump "sp_tm_epoch_t"          brk_tm           "print subject"

dump "sp_da (null)"           brk_da_null      "da subject"
dump "sp_da (empty)"          brk_da_empty     "da subject"
dump "sp_da (ints)"           brk_da_ints      "da subject"
dump "sp_da (cstrs)"          brk_da_cstrs     "da subject"

dump "sp_ht (null)"           brk_ht_null      "print subject"
dump "sp_ht (empty)"          brk_ht_empty     "print subject"
dump "sp_ht (ints)"           brk_ht_ints      "print subject"
dump "sp_ht (cstr vals)"      brk_ht_cstrval   "print subject"
dump "sp_ht (str keys)"       brk_ht_strkey    "print subject"
dump "sp_ht (tombstone)"      brk_ht_tombstone "print subject"
dump "sp_ht (struct kv)"      brk_ht_struct    "print subject"

dump "sp_om (null)"           brk_om_null      "print subject"
dump "sp_om (str keys)"       brk_om_strkey    "print subject"
dump "sp_om (struct vals)"    brk_om_structval "print subject"

dump "sp_mem_arena (summary)" brk_arena        "print subject"
dump "sp_mem_arena (chart)"   brk_arena        "arena subject"

dump "sp_ps_config_t"         brk_ps_config    "print subject"
dump "sp_env_var_t"           brk_env_var      "print subject"

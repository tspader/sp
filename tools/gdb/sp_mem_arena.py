import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sp_gdb


def walk_blocks(arena):
    current = int(arena["current"])
    block = arena["head"]
    index = 0
    passed_current = False
    while int(block):
        b = block.dereference()
        is_current = int(block) == current
        status = "current" if is_current else ("free" if passed_current else "live")
        used = 0 if passed_current else int(b["bytes_used"])
        yield index, int(b["capacity"]), used, status
        if is_current:
            passed_current = True
        block = b["next"]
        index += 1


def _totals(arena):
    used = cap = blocks = 0
    for _index, block_cap, block_used, _status in walk_blocks(arena):
        cap += block_cap
        used += block_used
        blocks += 1
    return used, cap, blocks


def _bar(used, capacity, width=32):
    used, capacity = int(used), int(capacity)
    filled = 0 if capacity <= 0 else min(width, int(round(width * used / float(capacity))))
    if used > 0 and filled == 0:
        filled = 1
    return sp_gdb.dim("[") + "█" * filled + sp_gdb.dim("·" * (width - filled) + "]")


class SpMemArenaPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        arena = sp_gdb.deref(self.val)
        if arena is None:
            return "(null) sp_mem_arena_t"
        used, cap, blocks = _totals(arena)
        return "sp_mem_arena_t {%s / %s, %d block%s}" % (
            sp_gdb.human_bytes(used),
            sp_gdb.human_bytes(cap),
            blocks,
            "" if blocks == 1 else "s",
        )


def _match(val):
    if sp_gdb.named_type(val) == "sp_mem_arena_t":
        return SpMemArenaPrinter(val)
    return None


class ArenaCommand(sp_gdb.Command):
    """Draw an sp_mem_arena block usage chart. Usage: arena <variable>"""

    USAGE = "Usage: arena <variable>"

    def __init__(self):
        super(ArenaCommand, self).__init__("arena")

    def run(self, argument):
        val = sp_gdb.eval_one(argument, self.USAGE)
        if val is None:
            return
        arena = sp_gdb.deref(val)
        if arena is None:
            print("(null)")
            return

        used, cap, blocks = _totals(arena)
        print("%s / %s across %d block%s" % (
            sp_gdb.human_bytes(used), sp_gdb.human_bytes(cap),
            blocks, "" if blocks == 1 else "s"))
        for index, block_cap, block_used, status in walk_blocks(arena):
            tag = "[%d]" % index
            if status == "current":
                tag = sp_gdb.green(tag)
            elif status == "free":
                tag = sp_gdb.dim(tag)
            print("%s %s %s / %s" % (
                tag, _bar(block_used, block_cap),
                sp_gdb.human_bytes(block_used), sp_gdb.human_bytes(block_cap)))


sp_gdb.register_lookup("sp_mem_arena", _match)
ArenaCommand()

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sp_gdb

_HT_FIELDS = {"data", "size", "capacity", "info", "tmp_key", "tmp_val"}


class SpHtPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        if int(self.val) == 0:
            return "(null) sp_ht"
        ht = self.val.dereference()
        return "sp_ht of length %d" % int(ht["size"])

    def children(self):
        for i, (key, val) in enumerate(sp_gdb.ht_entries(self.val)):
            yield ("key%d" % i, key)
            yield ("val%d" % i, val)

    def display_hint(self):
        return "map"


def _match(val):
    fields = sp_gdb.struct_pointer_fields(val)
    if fields is not None and _HT_FIELDS <= fields:
        return SpHtPrinter(val)
    return None


sp_gdb.register_lookup("sp_ht", _match)

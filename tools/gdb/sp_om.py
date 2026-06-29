import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sp_gdb

_OM_FIELDS = {"arenas", "order", "index", "temp"}


class SpOmPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        if int(self.val) == 0:
            return "(null) sp_om"
        om = self.val.dereference()
        return "sp_om of length %d" % sp_gdb.da_size(om["order"])

    def children(self):
        if int(self.val) == 0:
            return
        om = self.val.dereference()
        order = om["order"]
        size = sp_gdb.da_size(order)

        keys = {}
        for key, val_ptr in sp_gdb.ht_entries(om["index"]):
            keys[int(val_ptr)] = key

        shown = min(size, sp_gdb.MAX_ELEMENTS)
        for i in range(shown):
            val_ptr = order[i]
            key = keys.get(int(val_ptr))
            yield ("key%d" % i, key if key is not None else val_ptr)
            yield ("val%d" % i, val_ptr.dereference() if int(val_ptr) else val_ptr)

    def display_hint(self):
        return "map"


def _match(val):
    fields = sp_gdb.struct_pointer_fields(val)
    if fields is not None and _OM_FIELDS <= fields:
        return SpOmPrinter(val)
    return None


sp_gdb.register_lookup("sp_om", _match)

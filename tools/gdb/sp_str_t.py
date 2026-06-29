import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gdb
import sp_gdb


class SpStrPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        try:
            length = int(self.val["len"])
            data = self.val["data"]
            if int(data) == 0:
                return "<null>" if length else ""
            if length == 0:
                return ""
            return data.string(length=length, encoding="utf-8", errors="replace")
        except gdb.error as e:
            return "<sp_str_t: %s>" % e

    def display_hint(self):
        return "string"


sp_gdb.register_named("sp_str", {"sp_str_t": SpStrPrinter})

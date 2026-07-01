import os
import sys
from datetime import datetime, timezone

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gdb
import sp_gdb


class SpTmEpochPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        try:
            s = int(self.val["s"])
            ns = int(self.val["ns"])
        except gdb.error as e:
            return "<sp_tm_epoch_t: %s>" % e
        try:
            iso = datetime.fromtimestamp(s, tz=timezone.utc).strftime("%Y-%m-%dT%H:%M:%S")
        except (ValueError, OverflowError, OSError):
            return "<sp_tm_epoch_t: s=%d ns=%d>" % (s, ns)
        return "%s.%09dZ" % (iso, ns)


sp_gdb.register_named("sp_tm_epoch", {"sp_tm_epoch_t": SpTmEpochPrinter})

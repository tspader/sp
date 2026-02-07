import gdb
import gdb.printing
from datetime import datetime, timezone

class SpTmEpochPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        try:
            s = int(self.val['s'])
            ns = int(self.val['ns'])

            dt = datetime.fromtimestamp(s, tz=timezone.utc)
            iso = dt.strftime('%Y-%m-%dT%H:%M:%S')
            return f'{iso}.{ns:09d}Z'

        except Exception as e:
            return f'<sp_tm_epoch_t: error: {e}>'

def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("sp_tm_epoch")
    pp.add_printer('sp_tm_epoch_t', '^sp_tm_epoch_t$', SpTmEpochPrinter)
    return pp

gdb.printing.register_pretty_printer(
    gdb.current_objfile(),
    build_pretty_printer(),
    replace=True
)

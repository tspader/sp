import gdb
import gdb.printing

class SpStrPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        try:
            length = int(self.val['len'])
            data_ptr = self.val['data']

            if length == 0:
                return ''

            if data_ptr == 0:
                return '<null>'

            string_data = data_ptr.string(length=length, encoding='utf-8', errors='replace')
            return string_data

        except Exception as e:
            return f'<sp_str_t: error reading string: {e}>'

    def display_hint(self):
        return 'string'

def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("sp_str")
    pp.add_printer('sp_str_t', '^sp_str_t$', SpStrPrinter)
    return pp

gdb.printing.register_pretty_printer(
    gdb.current_objfile(),
    build_pretty_printer(),
    replace=True
)
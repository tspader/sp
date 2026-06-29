import sys
import gdb
import gdb.printing

MAX_ELEMENTS = 4096


def enum_value(enum_name, member, default=None):
    try:
        return int(gdb.lookup_type(enum_name)[member].enumval)
    except (gdb.error, KeyError):
        return default


def deref(val):
    t = val.type.strip_typedefs()
    if t.code == gdb.TYPE_CODE_PTR:
        return None if int(val) == 0 else val.dereference()
    return val


def is_char_ptr(val):
    t = val.type.strip_typedefs()
    if t.code != gdb.TYPE_CODE_PTR:
        return False
    target = t.target().strip_typedefs()
    return target.code in (gdb.TYPE_CODE_INT, gdb.TYPE_CODE_CHAR) and target.sizeof == 1


def format_value(val):
    if is_char_ptr(val):
        if int(val) == 0:
            return "(null)"
        try:
            return '"%s"' % val.string(encoding="utf-8", errors="replace")
        except gdb.error:
            return str(val)
    return str(val).strip()


def named_type(val):
    t = val.type.strip_typedefs()
    if t.code == gdb.TYPE_CODE_PTR:
        return t.target().name
    return val.type.name


def struct_pointer_fields(val):
    try:
        t = val.type.strip_typedefs()
        if t.code != gdb.TYPE_CODE_PTR:
            return None
        target = t.target().strip_typedefs()
        if target.code != gdb.TYPE_CODE_STRUCT:
            return None
        return set(f.name for f in target.fields() if f.name)
    except gdb.error:
        return None


def da_header(arr):
    if int(arr) == 0:
        return None
    header_type = gdb.lookup_type("sp_da_header_t")
    base = arr.cast(gdb.lookup_type("char").pointer()) - header_type.sizeof
    return base.cast(header_type.pointer()).dereference()


def da_size(arr):
    header = da_header(arr)
    return int(header["size"]) if header is not None else 0


def ht_entries(ht_ptr):
    if int(ht_ptr) == 0:
        return
    ht = ht_ptr.dereference()
    active = enum_value("sp_ht_entry_state", "SP_HT_ENTRY_ACTIVE", 1)
    data = ht["data"]
    capacity = int(ht["capacity"])
    for i in range(min(capacity, MAX_ELEMENTS)):
        entry = data[i]
        if int(entry["state"]) == active:
            yield entry["key"], entry["val"]


def color_enabled():
    try:
        if not sys.stdout.isatty():
            return False
    except Exception:
        return False
    try:
        return bool(gdb.parameter("style enabled"))
    except (gdb.error, RuntimeError):
        return True


def _wrap(text, code):
    return "\x1b[%sm%s\x1b[0m" % (code, text) if color_enabled() else text


def dim(text):
    return _wrap(text, "90")


def green(text):
    return _wrap(text, "32")


def human_bytes(n):
    n = int(n)
    if n < 1024:
        return "%dB" % n
    if n < 1024 * 1024:
        return "%.1fKB" % (n / 1024.0)
    return "%.1fMB" % (n / (1024.0 * 1024.0))


class Command(gdb.Command):
    def __init__(self, name):
        super(Command, self).__init__(name, gdb.COMMAND_DATA)

    def invoke(self, argument, from_tty):
        try:
            self.run(argument)
        except gdb.error as e:
            print("Error: %s" % e)

    def run(self, argument):
        raise NotImplementedError


def eval_one(argument, usage):
    args = gdb.string_to_argv(argument)
    if len(args) != 1:
        print(usage)
        return None
    return gdb.parse_and_eval(args[0])


def register_named(collection, printers):
    pp = gdb.printing.RegexpCollectionPrettyPrinter(collection)
    for type_name, cls in printers.items():
        pp.add_printer(type_name, "^" + type_name + "$", cls)
    gdb.printing.register_pretty_printer(gdb.current_objfile(), pp, replace=True)


def register_lookup(tag, matcher):
    matcher.sp_tag = tag
    gdb.pretty_printers[:] = [
        p for p in gdb.pretty_printers if getattr(p, "sp_tag", None) != tag
    ]
    gdb.pretty_printers.append(matcher)

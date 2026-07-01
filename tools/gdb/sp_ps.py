import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gdb
import sp_gdb


def _io_modes(io):
    yield ("stdin", io["in"]["mode"])
    yield ("stdout", io["out"]["mode"])
    yield ("stderr", io["err"]["mode"])


class SpPsConfigPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "sp_ps_config_t"

    def children(self):
        yield ("command", self.val["command"])
        args = self.val["args"]
        count = args.type.range()[1] + 1
        shown = 0
        for i in range(count):
            arg = args[i]
            try:
                if int(arg["len"]) > 0:
                    yield ("arg%d" % shown, arg)
                    shown += 1
            except gdb.error:
                break
        if int(self.val["cwd"]["len"]) > 0:
            yield ("cwd", self.val["cwd"])
        for entry in _io_modes(self.val["io"]):
            yield entry


class SpPsPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "sp_ps_t"

    def children(self):
        for entry in _io_modes(self.val["io"]):
            yield entry


class SpEnvVarPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "sp_env_var_t"

    def children(self):
        yield ("key", self.val["key"])
        yield ("value", self.val["value"])


sp_gdb.register_named("sp_ps", {
    "sp_ps_config_t": SpPsConfigPrinter,
    "sp_ps_t": SpPsPrinter,
    "sp_env_var_t": SpEnvVarPrinter,
})

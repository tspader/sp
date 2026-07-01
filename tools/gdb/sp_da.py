import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sp_gdb


class DaCommand(sp_gdb.Command):
    """Print an sp_da dynamic array. Usage: da <variable>"""

    USAGE = "Usage: da <variable>"

    def __init__(self):
        super(DaCommand, self).__init__("da")

    def run(self, argument):
        arr = sp_gdb.eval_one(argument, self.USAGE)
        if arr is None:
            return
        if int(arr) == 0:
            print("(null)")
            return

        header = sp_gdb.da_header(arr)
        size = int(header["size"])
        capacity = int(header["capacity"])
        print("size = %d, capacity = %d" % (size, capacity))
        if size == 0:
            print("(empty)")
            return

        shown = min(size, sp_gdb.MAX_ELEMENTS)
        for i in range(shown):
            print("[%d] = %s" % (i, sp_gdb.format_value(arr[i])))
        if size > shown:
            print("... (%d more)" % (size - shown))


DaCommand()

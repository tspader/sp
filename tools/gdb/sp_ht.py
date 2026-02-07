import gdb

class SpHtCommand(gdb.Command):
    """Display sp_ht hash table contents.
    Usage: sp_ht <variable>
    """

    def __init__(self):
        super(SpHtCommand, self).__init__("ht", gdb.COMMAND_DATA)

    def invoke(self, argument, from_tty):
        args = gdb.string_to_argv(argument)
        if len(args) != 1:
            print("Usage: sp_ht <variable>")
            return

        try:
            ht_ptr = gdb.parse_and_eval(args[0])
        except gdb.error as e:
            print(f"Error: {e}")
            return

        if int(ht_ptr) == 0:
            print("(null)")
            return

        try:
            ht = ht_ptr.dereference()
            size = int(ht['size'])
            capacity = int(ht['capacity'])
            data = ht['data']

            print(f"size = {size}, capacity = {capacity}")

            if size == 0:
                print("(empty)")
                return

            count = 0
            for i in range(capacity):
                entry = data[i]
                state = int(entry['state'])
                if state == 1:
                    key = entry['key']
                    val = entry['val']
                    print(f"[{count}] {key} => {val}")
                    count += 1

        except gdb.error as e:
            print(f"Error reading hash table: {e}")

SpHtCommand()

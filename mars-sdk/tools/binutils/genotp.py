import struct
import os
import sys

class user_cmd_item_t(struct.Struct):
    """Structure representing a user command item."""

    format = "<IBBBB"  # Little-endian format string

    def __init__(self, reg_addr, reg_start_bit, reg_end_bit, tbl_row, tbl_col):
        self.reg_addr = reg_addr
        self.reg_start_bit = reg_start_bit
        self.reg_end_bit = reg_end_bit
        self.tbl_row = tbl_row
        self.tbl_col = tbl_col


def generate_opt_table(output, kvlist):
    """Generates a binary file called opt_table.bin with the specified structure."""

    length = len(kvlist)
    if length > 16:
        print("Error: The maximum number of items is 16.")
        return
    
    item_num = length * 2  # Each item is split into two 16-bit writes

    while length < 16:
        kvlist.append((0, 0))
        length += 1

    with open(output, "wb") as f:
        data_table = []
        # write the 16 * 32-bit words (4 bytes each)
        for addr, value in kvlist:
            data_table.append(value.to_bytes(4, "little"))

        # Write the data table
        f.write(b"".join(data_table))

        # Write the fixed words
        f.write(b"LSAI")  # Word-"LSAI"

        # Calculate and write the table info checksum
        checksum = sum(int.from_bytes(word, "little") for word in data_table)
        checksum = checksum & 0xFFFFFFFF  # Truncate to 32 bits
        f.write(checksum.to_bytes(4, "little"))

        # Write the item number
        f.write(item_num.to_bytes(1, "little"))
        f.write(b"\x00\x00\x00")
    
        items = []
        index = 0
        for addr, value in kvlist:
            item_gen_16lsb = user_cmd_item_t(
                reg_addr=addr,
                reg_start_bit=0,
                reg_end_bit=15,
                tbl_row=index,
                tbl_col=0
            )
            item_gen_16msb = user_cmd_item_t(
                reg_addr=addr,
                reg_start_bit=16,
                reg_end_bit=31,
                tbl_row=index,
                tbl_col=16
            )
            items.append(item_gen_16lsb)
            items.append(item_gen_16msb)
            index += 1
            if (2 * index) >= item_num:
                break
        
        checksum = 0
        for item in items:
            packed_data = struct.pack("<IBBBB", item.reg_addr, item.reg_start_bit, item.reg_end_bit, item.tbl_row, item.tbl_col)  # Pack the item into bytes
            words = struct.iter_unpack("<I", packed_data)  # Unpack words (4 bytes each)
            for word in words:
                checksum += word[0]  # Add each word to the checksum
        
        checksum = checksum & 0xFFFFFFFF  # Truncate to 32 bits
        f.write(checksum.to_bytes(4, "little"))

        for item in items:
            # Write the item's data to the file
            f.write(item.reg_addr.to_bytes(4, "little"))
            f.write(item.reg_start_bit.to_bytes(1, "little"))
            f.write(item.reg_end_bit.to_bytes(1, "little"))
            f.write(item.tbl_row.to_bytes(1, "little"))
            f.write(item.tbl_col.to_bytes(1, "little"))


if __name__ == "__main__":
    kvlist = [
        # ( {destination_adress}, {written_value})
        (0x4F000064, 0x1080),
        # Add more tuples as needed, up to 16 items
    ]
    if len(sys.argv) > 1:
        output = os.path.join(os.path.dirname(sys.argv[0]), sys.argv[1])
        print("OTPTAB:", sys.argv[1])
        generate_opt_table(output, kvlist)


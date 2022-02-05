from sys import argv

if len(argv) != 3:
    print(f"syntax: {argv[0]} [c64++ trace] [vice trace]")
    quit()

print(f"load {argv[1]}")
with open(argv[1]) as f:
    emu_trace = f.read().splitlines()
print(f"load {argv[2]}")
with open(argv[2]) as f:
    vice_trace = f.read().splitlines()
print(f"compare..")

emu_pc = list(map(lambda s: s[3:7], emu_trace))
vice_pc = list(map(lambda s: s[3:7], vice_trace))

idx_emu = 0
idx_vice = 0

emu_buffer = []
vice_buffer = []

def print_buffer(start_emu, start_vice):
    global emu_buffer, vice_buffer
    num_both = min(len(emu_buffer), len(vice_buffer))
    for i in range(num_both):
        print(f"! c64++ {emu_trace[start_emu + i]} | vice {vice_trace[start_vice + i]}")
    if len(emu_buffer) > len(vice_buffer):
        for i in range(len(emu_buffer) - num_both):
            print(f"! c64++ {emu_trace[start_emu + num_both + i]} |")
    else:
        for i in range(len(vice_buffer) - num_both):
            print(f"!                                                                           | vice {vice_trace[start_vice + num_both + i]}")
    emu_buffer = []
    vice_buffer = []  

while(True):
    if idx_emu >= len(emu_trace) or idx_vice >= len(vice_trace):
        for i in range(idx_emu, len(emu_trace)):
            emu_buffer.append(emu_trace[i])
        for i in range(idx_vice, len(vice_trace)):
            vice_buffer.append(vice_trace[i])
        print_buffer(len(emu_trace) - len(emu_buffer), idx_vice - len(vice_buffer))
        break

    if vice_pc[idx_vice] in emu_buffer:
        start_emu = idx_emu - len(emu_buffer)
        start_vice = idx_vice - len(vice_buffer)
        num_both = emu_buffer.index(vice_pc[idx_vice])
        offset = len(emu_buffer) - num_both
        emu_buffer = emu_buffer[:num_both]
        print_buffer(start_emu, start_vice)
        idx_emu -= offset
    if emu_pc[idx_emu] in vice_buffer:
        start_emu = idx_emu - len(emu_buffer)
        start_vice = idx_vice - len(vice_buffer)
        num_both = vice_buffer.index(emu_pc[idx_emu])
        offset = len(vice_buffer) - num_both
        vice_buffer = vice_buffer[:num_both]
        print_buffer(start_emu, start_vice)
        idx_vice -= offset

    if emu_pc[idx_emu] == vice_pc[idx_vice]:
        emu_reg = emu_trace[idx_emu][38:67]
        vice_reg = vice_trace[idx_vice][38:67]
        emu_reg = emu_reg[:18] + "FF" + emu_reg[20:]
        vice_reg = vice_reg[:18] + "FF" + vice_reg[20:]
        if emu_reg != vice_reg:
            print(f"R c64++ {emu_trace[idx_emu]} | vice {vice_trace[idx_vice]}")
        else:
            print(f"  c64++ {emu_trace[idx_emu]} | vice {vice_trace[idx_vice]}")
    else:
        emu_buffer.append(emu_pc[idx_emu])
        vice_buffer.append(vice_pc[idx_vice])
    idx_emu += 1
    idx_vice += 1
#!/usr/bin/python

import termios, sys
port = "/dev/ttyS0"
#port = "/dev/stdout"

def read_file(fname):
    f = open(fname, "r")
    mem = ""
    id = ""
    for L in f:
        L = L.strip()
        if id == "":
            id = L
        else:
            mem += L
    f.close()
    return [mem, id]

def write_file(fname, mem, id):
    f = open(fname, "w")
    f.write(id)
    f.write("\n")
    while len(mem) > 0:
        if len(mem) > 32:
            f.write(mem[:32])
            mem = mem[32:]
            f.write("\n")
        else:
            f.write(mem)
            mem = ""
            f.write("\n")
    f.close()


def push(f, mem):
    addr = int(f.read(4), 16)
    ln = int(f.read(2), 16)
    print "<<", addr, ln
    real_len = max([min([ln, len(mem)/2 - addr]), 0])
    f.write(mem[addr*2:(addr+real_len)*2])
    f.write("ff" * (ln - real_len))
    print ">> " + (mem[addr*2:(addr+real_len)*2]) + ("ff" * (ln - real_len))

def pull(f, mem):
    addr = int(f.read(4), 16)
    ln = int(f.read(2), 16)
    print "<<", addr, ln
    if len(mem) < (addr + ln)*2:
        mem += "f" * ((addr + ln)*2 - len(mem))
    r = f.read(ln*2)
    mem = mem[:addr*2] + r + mem[(addr+ln)*2:]
    print "<<", r
    return mem

def dump_log(f):
    s = ""
    while True:
        r = f.read(1)
        if r == "\n":
            print s
            return
        s += r

def run_command(f, mem, id):
    r = f.read(1)
    print "<<", r
    if r == "I":
        f.write(id)
        print ">>", id
    elif r == "R":
        push(f, mem)
    elif r == "W":
        mem = pull(f, mem)
    elif r == "X":
        dump_log(f)
    return mem
            
fname = sys.argv[1]
print "Touching", fname
if fname != "-i":
    mem, id = read_file(fname)

f = open(port, "r+", 0)
old_fl = termios.tcgetattr(f)
new_fl = old_fl
new_fl[3] = new_fl[3] & ~termios.ICANON
#new_fl[4] = termios.B115200
#new_fl[5] = termios.B115200
new_fl[4] = termios.B19200
new_fl[5] = termios.B19200

termios.tcsetattr(f, termios.TCSANOW, new_fl)
f.write('X')
if fname == "-i":
    r = f.read(1)
    if r == ">":
        f.write("c")
        f.write("2")
        f.write("\r")
        f.write("\n")
    else:
        print "bad luck"
else:
    try:
        while True:
            mem = run_command(f, mem, id)
    except KeyboardInterrupt:
        pass
    write_file(fname, mem, id)
termios.tcsetattr(f, termios.TCSANOW, old_fl)
f.close()


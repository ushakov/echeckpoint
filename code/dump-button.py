#!/usr/bin/python

import sys

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

def hexdigit(s):
    if s >= '0' and s <= '9':
        return ord(s) - ord('0')
    return ord(s) - ord('a') + 10

def hexbyte(s):
    return hexdigit(s[0])*16 + hexdigit(s[1])

def hexnum(s):
    n = 0
    for i in range(len(s)-2, -1, -2):
        n *= 256
        n += hexbyte(s[i:i+2])
    return n

def revhexnum(s):
    n = 0
    for i in range(0, len(s), 2):
        n *= 256
        n += hexbyte(s[i:i+2])
    return n

def peek(mem, addr, n):
    return hexnum(mem[addr*2:addr*2 + n*2])

def revpeek(mem, addr, n):
    return revhexnum(mem[addr*2:addr*2 + n*2])

def parse_button(mem):
    butt = {}
    mmbid = peek(mem, 0, 2)
    butt['mmbid'] = mmbid
    butt['blocks'] = []
    # then, dump blocks
    addr = 2
    while addr*2 < len(mem):
        L = peek(mem, addr, 2)
        if L == 0:
            break
        cp = peek(mem, addr + 2, 1)
        block = peek(mem, addr + 3, 2)
        cs = peek(mem, addr + 5, 1)
        buttblock = {}
        buttblock['len'] = L
        buttblock['cp'] = cp
        buttblock['block'] = block
        buttblock['cs'] = cs
        buttblock['events'] = []
        t = 6
        i = 1
        while t < L:
            b = revpeek(mem, addr + t, 6)
            time = peek(mem, addr + t + 6, 3)
            seconds = time % 256
            time /= 256
            minutes = time % 256
            time /= 256
            hours = time % 32
            time /= 32
            days = time
            ev = {}
            ev['button'] = b
            ev['time'] = "%dd %02d:%02d:%02d" % (days, hours, minutes, seconds)
            buttblock['events'] += [ev]
            t += 9
            i += 1
        addr += L
        butt['blocks'] += [buttblock]
    return butt

def dump_all(butt):
    print "mmbid:", butt['mmbid']
    n = 0
    for block in butt['blocks']:
        print "Block", n
        n += 1
        print "  len:", block['len']
        print "  CP:", block['cp']
        print "  BL:", block['block']
        print "  checksum:", block['cs']
        e = 0
        for ev in block['events']:
            print "  Event", e
            e += 1
            print "    button:", hex(ev['button'])
            print "    time:", ev['time']
        print

def dump_blocks(butt):
    print "mmbid:", butt['mmbid']
    for block in butt['blocks']:
        print "CP:%02.2d BL:%04.4d ev:%d" % (block['cp'], block['block'], len(block['events']))

def add_to_roster(butt, roster):
    for block in butt['blocks']:
        pair = (block['cp'], block['block'])
        if pair not in roster:
            roster[pair] = 1
        else:
            roster[pair] += 1
    return roster

def dump_roster(roster):
    bln = {}
    for pair in roster:
        bln[pair[1]] = 1
    blocks = sorted(bln.keys())
 
    cpn = {}
    for pair in roster:
        cpn[pair[0]] = 1
    cps = sorted(cpn.keys())

    for cp in cps:
        print "%02.2d: " % cp,
        for bl in blocks:
            pair = (cp, bl)
            if pair in roster:
                print " %02.2d" % roster[pair],
            else:
                print " 00",
        print

def main():
    verbose = True
    roster = False
    t = 1
    if sys.argv[t] == "-q":
        verbose = False
        t += 1
    if sys.argv[t] == "-r":
        roster = True
        t += 1
    if roster:
        r = {}
        for fname in sys.argv[t:]:
            [mem, id] = read_file(fname)
            b = parse_button(mem)
            r = add_to_roster(b, r)
            print "roster:", len(r)
        dump_roster(r)
    else:
        fname = sys.argv[t]
        [mem, id] = read_file(fname)
        b = parse_button(mem)
        if verbose:
            dump_all(b)
        else:
            dump_blocks(b)

if __name__ == "__main__":
    main()
        

#!/usr/bin/python

import sys

def hexdigit(d):
    if d >= 10:
        return chr(d - 10 + ord('a'))
    else:
        return chr(d + ord('0'))

def hexbyte(b):
    return hexdigit(b / 16) + hexdigit(b % 16)
    
def hexnum(n, width):
    s = ""
    for i in range(width):
        byte = n % 256
        s += hexbyte(byte)
        n /= 256
    return s

def hexid(n):
    s = ""
    for i in range(6):
        byte = n % 256
        s = hexbyte(byte) + s
        n /= 256
    return s

# t in minutes
def hextime(t):
    m = t % 60
    t /= 60
    h = t % 24
    t /= 24
    d = t
    dh = d * 32 + h
    return hexbyte(57) + hexbyte(m) + hexbyte(dh)

def getdigit(s):
    if s >= '0' and s <= '9':
        return ord(s) - ord('0')
    return ord(s) - ord('a') + 10

def getbyte(s):
    return getdigit(s[0])*16 + getdigit(s[1])

def checksum(mem):
    c = 0
    for i in range(0, len(mem), 2):
        if i != 10:
            b = getbyte(mem[i:i+2])
            c <<= 1
            c ^= b
            c &= 0xff
    return c
            
def event(n, wm):
    d = hexid(wm * 65536 + n)
    t = hextime(60*wm + n)
    return d + t

def block(cp, bn, n, wm):
    head = ""
    head += hexbyte(cp)
    head += hexnum(bn, 2)
    body = ""
    for i in range(n):
        body += event(i, wm)
    blen = len(body) / 2 + 6
    head = hexnum(blen, 2) + head
    c = checksum(head + "00" + body)
    return head + hexbyte(c) + body

def buttonfile(id, mmbid, blocks):
    f = "0c" + hexid(id) + "ff\n"
    f += hexnum(mmbid, 2) + "\n"
    f += "".join(blocks)
    f += "0000"
    fl = 4 + sum(map(lambda(x):len(x)/2, blocks))
    if fl > 8192:
        print "Bad things! len=%d" % fl
        exit
    print "Mem used =", fl
    return f

# button id (=watermark), start cp, end cp, n-blocks

batch = [
    [50, 10, 19, 5], 
    [51, 20, 29, 5], 
    [52, 30, 39, 5],
    [53, 40, 49, 5],
    ]

def batchtask(id, start, end, nbl):
    blocks = []
    for cp in range(start, end+1):
        for bl in range(nbl):
            b = block(cp, bl, 17, id)
            blocks += [b]
    return buttonfile(id, 1, blocks)

def main():
    for task in batch:
        fname = "pb%02.2d.b" % task[0]
        print "Writing", fname
        f = open(fname, "w")
        f.write(batchtask(*task))
        f.close()

if __name__ == "__main__":
    main()


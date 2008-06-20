import pylab
import csv

cp = None
pl = None

def load(filename):
    r = csv.reader(open(filename), dialect='excel-tab')
    q = []
    for t in r:
        q += [t]
    q = q[1:]
    return q

def remap(arr, column, retain):
    r = {}
    for t in arr:
        if not t[column] in r:
            r[t[column]] = []
        rt = []
        for c in retain:
            rt += [t[c]]
        r[t[column]] += [rt]
    return r

def graph(arr, x, ys):
    xx = map(lambda(t):t[x], arr)
    for c in ys:
        yy = map(lambda(t):t[c], arr)
        pylab.plot(xx, yy)

def cp_load(name):
    global cp
    r = load(name)
    cp = remap(r, 2, [1,3,4])
    for t in cp:
        for tt in cp[t]:
            tt += [int(tt[1]) - int(tt[2])]
            for y in tt:
                y = str(y)
    # fields:
    #  0 -- tick
    #  1 -- total storage
    #  2 -- other storage
    #  3 -- self storage
    return cp

def cp_graph(cp):
    for t in cp:
        graph(cp[t], 0, [2,3])

def all_cp(name):
    t = cp_load(name)
    cp_graph(t)


def pl_load(name):
    global pl
    r = load(name)
    pl = remap(r, 2, [1,3,4,5])
    for p in pl.keys():
        for t in pl[p]:
            if t[1] == "-":
                t[1] = 0
            t[2] = int(t[2][3:])
    # fields:
    #  0 -- tick
    #  1 -- storage
    #  2 -- cp taken now
    #  3 -- total cps taken
    return pl
    
def pl_graph(pl):
    for t in pl:
        graph(pl[t], 0, [1,3])

def all_pl(name):
    t = pl_load(name)
    pl_graph(t)

def test():
    global cpa
    cpa = 1


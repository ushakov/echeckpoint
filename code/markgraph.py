import gv, sys

def mark():
    ok = False
    n = gv.firstnode(gr)
    while gv.ok(n):
        if gv.getv(n, 'color') == 'green':
            nh = gv.firsthead(n)
            while gv.ok(nh):
                if gv.getv(nh, 'color') != 'green':
                    gv.setv(nh, 'color', 'green')
                    ok = True
                nh = gv.nexthead(n, nh)
        n = gv.nextnode(gr, n)
    return ok

if __name__ == "__main__":
    name = sys.argv[1]
    if name[-4:] != '.dot':
        print "wrong name", name
        exit
    name = name[:-4]
    gr = gv.read(name + '.dot')
    m = gv.findnode(gr, 'main')
    gv.setv(m, 'color', 'green')
    while mark():
        pass
    n = gv.firstnode(gr)
    while gv.ok(n):
        if gv.getv(n, 'color') != 'green':
            gv.setv(n, 'fillcolor', 'red')
            gv.setv(n, 'style', 'filled')
        in_degree = 0
        e = gv.firstin(n)
        while gv.ok(e):
            in_degree += 1
            e = gv.nextin(n, e)
        if in_degree == 1:
            gv.setv(n, 'shape', 'diamond')
        n = gv.nextnode(gr, n)
    gv.write(gr, name + '-new.dot')
    gv.layout(gr, 'dot')
    gv.render(gr)
    gv.render(gr, 'fig', name + '.fig')

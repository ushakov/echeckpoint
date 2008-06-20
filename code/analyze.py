import sys
import re

sections = ['text','data', 'bss']

sizes = {}
sec_sizes = {}

name = sys.argv[1]
f = open(name, "r")
cur_sec = ''
for line in f:
    line = line.rstrip()
    num = line[16:34]
    try:
        size = int(line[35:45], 0)
    except ValueError:
        size = -1
    if not re.match("0x[0-9a-f]*", num):
        continue
    sec_boundary = False
    if line.startswith('.'):
        for s in sections:
            if line[1:].startswith(s):
                cur_sec = s
                if not cur_sec in sizes:
                    sizes[cur_sec] = {}
                    sec_sizes[cur_sec] = size
                    sec_boundary = True
    if re.search("\.e%s" % (cur_sec), line):
        cur_sec = ""
        sec_boundary = True
    if re.search("PROVIDE.*__%s_end" % (cur_sec), line):
        cur_sec = ""
        sec_boundary = True
    if sec_boundary or cur_sec == "":
        continue
    # print "[%s] --- %s" % (cur_sec, line)
    if size != -1:
        fname = line[46:]
        if not fname in sizes[cur_sec]:
            sizes[cur_sec][fname] = size
        else:
            sizes[cur_sec][fname] += size

print "files in sections:"
print "=================="
ff = {}

for s in sizes:
    print
    print s
    print "-----"
    for f in sizes[s]:
        print f, sizes[s][f]
        if not f in ff:
            ff[f] = sizes[s][f]
        else:
            ff[f] += sizes[s][f]
print

# print "total in files:"
# print "========="

# sum = 0
# for f in ff:
#     print f, ff[f]
#     sum += ff[f]
# print "---total", sum
# print

print "sections:"
print "========="

for s in sec_sizes:
    print s, sec_sizes[s]

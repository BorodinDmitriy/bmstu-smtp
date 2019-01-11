#!/usr/bin/python

import re, sys

re1 = re.compile('^\s*\{\s*tst\s*=(.+?);\s*tev\s*=(.+?);\s*next\s*=(.+?);\s*\}.*$')

def fsm2dot(lines):
    vs = dict()
    es = dict()
    al = dict()
    for l in lines:
        m = re1.match(l)
        if m:
            src = m.group(1).strip()
            evn = m.group(2).strip()
            dst = m.group(3).strip()
            if src == '"*"':
                e = (evn, dst)
                if not al.has_key(e):
                   al[e] = True
            else:
                e = (src, evn, dst)
                if not es.has_key(e):
                    es[e] = True
                if not vs.has_key(src):
                    vs[src] = True
            if not vs.has_key(dst):
                vs[dst] = True
    print_header()
    for e in al.keys():
        for v in vs.keys():
            print_edge(v, e[0], e[1])
    for e in es.keys():
         print_edge(e[0], e[1], e[2])
    print_footer()


def norm(s):
    return '"' + s.replace('_', ' ') + '"'


def print_header():
    print 'digraph test {'
    print 'rankdir=LR;'
    # Let's make graph more compact
    print 'ranksep=0.1; nodesep=0.1; defaultdist = 0.1; len = 0.1;'


def print_edge(src, edge, dst):
    print '%s -> %s [label=%s];' % (norm(src), norm(dst), norm(edge))


def print_footer():
    print '}'


if len(sys.argv) != 2:
    sys.stderr.write('usage: ' + sys.argv[0] + '<fsm.def>\n')
    exit(-1)
else:
    fsm2dot(open(sys.argv[1], 'r').readlines())

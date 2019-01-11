#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import string


class TooEarlyEx(Exception):
    def __init__(self, variable):
        self.variable = variable


re_broke = re.compile('^(.+?)(\s*)\\\s*$')
re_vardef = re.compile('^([.\w]+)\s*=\s*(.*?)\s*$')
re_varadd = re.compile('^([.\w]+)\s*\+=\s*(.*?)\s*$')
re_func = re.compile('^(.*?)\$\((\w+) (.*)$')
re_var = re.compile('^(.*?)(?<!\$)\$\(([\w.]+)\)(.*)$')
re_comment = re.compile('^(.*?)\#.*$')

re_params2 = re.compile('^(.*?)\s*,\s*(.*)$')
re_params3 = re.compile('^(.*?)\s*,\s*(.*?)\s*,\s*(.*)$')


def f_wildcard(param, vlist):
    mask = calc_value(param, vlist)
    f = os.popen('ls ' + mask)
    return ' '.join([l.strip() for l in f.readlines() if len(l.strip()) > 0])


def f_addprefix(param, vlist):
    m = re_params2.match(param)
    if not m:
        return param
    p = m.group(1)
    p2 = calc_value(m.group(2), vlist)
    lst = [calc_value(l, vlist) for l in p2.split()]
    return ' '.join([p + l for l in lst])


def f_patsubst(param, vlist):
    m = re_params3.match(param)
    if not m:
        raise Exception('Invalid patsubst params: ', param)
    p_src = m.group(1)
    p_rep = m.group(2)
    p2 = calc_value(m.group(3), vlist)
    lst = [calc_value(l, vlist) for l in p2.split()]
    dst = []
    p_src = re.escape(p_src).replace('\%', '(.*?)')
    re1 = re.compile('^' + p_src + '$')
    for l in lst:
        m = re1.match(l)
        if m:
            new = p_rep.replace('%', m.group(1))
            dst.append(new)
        else:
            dst.append(l)
    return ' '.join(dst)


def f_shell(param, vlist):
    f = os.popen(calc_value(param, vlist))
    l = [l.strip() for l in f.readlines() if len(l.strip()) > 0]
    return ' '.join(l)


def f_foreach(param, vlist):
    m = re_params3.match(param)
    if not m:
        raise Exception('Invalid foreach params: ', param)
    v = m.group(1)
    src = calc_value(m.group(2), vlist)
    dst = m.group(3)
    lst = [dst.replace('$(' + v + ')', l).strip() for l in src.split()]
    return calc_value(' '.join(lst), vlist)


def f_findstring(param, vlist):
    m = re_params2.match(param)
    if not m:
        raise Exception('Invalid findstring params: ', param)
    sub = calc_value(m.group(1), vlist)
    s = calc_value(m.group(2), vlist)
    if string.find(s, sub) == -1:
        return ""
    else:
        return sub


funcs = {
    'wildcard': f_wildcard,
    'addprefix': f_addprefix,
    'patsubst': f_patsubst,
    'shell': f_shell,
    'foreach': f_foreach,
    'findstring': f_findstring,
    }


def read_input():
    lines = []
    add_fl = False
    try:
        while 1:
            line = raw_input()
            m = re_comment.match(line)
            if m:
                line = m.group(1)
            if len(line) > 0:
                if add_fl:
                    lines[-1] += ' ' + line
                else:
                    lines.append(line)
            add_fl = re_broke.match(line)
    except EOFError:
        return lines


def find_vars(lines):
    vlist = dict()
    for l in lines:
        m = re_vardef.match(l)
        if m:
            vlist[m.group(1)] = m.group(2)
        else:
            m = re_varadd.match(l)
            if m:
                try:
                    vlist[m.group(1)] = vlist[m.group(1)] + ' ' + m.group(2)
                except:
                    vlist[m.group(1)] = m.group(2)
        #print vlist.keys()
    return vlist


def no_vars(line, vlist):
    l = line
    while 1:
        m = re_var.match(l)
        if m:
            var = m.group(2)
            try:
                value = vlist[var]
            except:
                raise TooEarlyEx(var)
            l = m.group(1) + value + m.group(3)
        else:
            break
    return l


def split_args(remained):
    cnt = 1
    for p in xrange(len(remained)):
        c = remained[p]
        if c == '(':
            cnt += 1
        elif c == ')':
            cnt -= 1
        if cnt == 0:
            return remained[:p], remained[p + 1:]
    raise Exception('Cannot find params: [' + remained + ']')


def calc_value(value, known_vars):
    v = value
    # calc functions
    while 1:
        m = re_func.match(v)
        if m:
            prefix = m.group(1)
            fnc = m.group(2)
            args, suffix = split_args(m.group(3))
            #print 'func in:', v
            #print '2:', m.group(2)
            #print '3:', m.group(3)
            try:
                fnc = m.group(2)
                f = funcs[fnc]
            except:
                raise Exception('Unknown function: [' + fnc + ']')
            r = f(args, known_vars)
            if r == None:
                return None
            v = prefix + r + suffix
        else:
            break
    return no_vars(v, known_vars)


def calc_vars(vlist):
    changed = True
    new_vars = dict()
    while changed:
        changed = False
        for var in vlist.keys():
            try:
                new_value = calc_value(vlist[var], new_vars)
                #print 'calc: ', vlist[var], new_vars.keys()
            except TooEarlyEx:
                continue # TODO: dependenies graph? never heard of this.
            if new_value != None:
                new_vars[var] = new_value
                del vlist[var]
                changed = True
            else:
                pass
        if len(vlist.keys()) > 0 and not changed:
            raise Exception('Cannot resolve variables: ' + ', '.join(vlist.keys()))
    return new_vars


def remove_vars(lines, vlist):
    r = []
    for l in lines:
        if re_vardef.match(l) or re_varadd.match(l):
            continue
        try:
            l = calc_value(l, vlist)
        except TooEarlyEx as ex:
            raise Exception('Unknown variable ' + ex.variable + ' in ' + l)
        if l != None:
            r.append(l)
    return r


def simplify_make(lines):
    vlist = find_vars(lines)
    vlist = calc_vars(vlist)
    # GNU make implicit variables:
    vrs = {'RM': 'rm -f', 'CC': 'cc'}
    vrs.update(vlist)
    #print vlist
    lines = remove_vars(lines, vrs)
    return lines


def tests():
    vlist = dict()
    vlist["LST"] = "1 2 3"
    v = calc_value('$(foreach v, $(LST), -$(v))', vlist)
    assert (v == '-1 -2 -3')
    v = calc_value('$(LST) $(foreach v, $(LST), -$(v)) $(LST)', vlist)
    assert (v == '1 2 3 -1 -2 -3 1 2 3')
    v = calc_value('$(addprefix src/, file.c)', vlist)
    assert (v == 'src/file.c')
    v = calc_value('$(patsubst %.c, %.o, file.c)', vlist)
    assert (v == 'file.o')
    v = calc_value('$(addprefix src/,$(patsubst %.c, %.o, file.c))', vlist)
    assert (v == 'src/file.o')
    v = calc_value('$(addprefix ./,$(patsubst %.c, %.o, $(addprefix src/, file.c)))', vlist)
    assert (v == './src/file.o')
    v = calc_value('$(shell ls /b*n/ls)', vlist)
    assert (v == '/bin/ls')
    #
    lines = []
    lines.append('L0=')
    lines.append('L1=1')
    lines.append('L2=2 3')
    lines.append('V0=$(foreach v, $(L0), -$(v))')
    lines.append('V1=$(foreach v, $(L0) $(L1) $(L2), -$(v))')
    lines.append('all: $(V1) $(foreach v, $(L1), $(v)) $(L2)')
    lines.append('\t$(foreach v, $(L1), $(v))')
    r = simplify_make(lines)
    assert (r[0] == 'all: -1 -2 -3 1 2 3')
    assert (r[1] == '\t1')
    #
    lines = [
        "M = s1 s2 s3",
        "S = src $(addprefix src/,$(M))",
        "all:",
        "\tls $(S)",
    ]
    r = simplify_make(lines)
    assert (r[0] == 'all:')
    assert (r[1] == '\tls src src/s1 src/s2 src/s3')


def main():
    lines = read_input()
    r = simplify_make(lines)
    for l in r:
        print l


tests()

if __name__ == "__main__":
    main()

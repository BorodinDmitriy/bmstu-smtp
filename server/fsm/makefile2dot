#!/usr/bin/python
# -*- coding: utf-8 -*-

import re, sys, os.path

# Default List of meaningfull commands. Exandable via command-line arguments
commands = ['flex', 'bison', 'c99 -c', 'gcc -c', 'c99 ', 'gcc ', 'ld ', 'pdflatex', 'autogen', 'doxygen', 'valgrind', 'check', 'makedot2dot', 'makesimple', 'dot2tex', 'cc -c', 'cc ']

re_task = re.compile('^\s*(.*?)\s*:\s*(.*?)\s*$')
re_rule = re.compile('^\t\s*(.*?)\s*$')


def read_targets():
    targets = dict()
    last_target = None
    try:
        while 1:
            line = raw_input()
            m = re_task.match(line)
            if m:
                last_target = m.group(1)
                list = [l for l in m.group(2).split()]
                re1 = None
                if last_target.find('%') != -1:
                    re1 = re.compile('^' + re.escape(last_target).replace('\%', '(.*?)') + '$')
                targets[last_target] = (list, [], re1)
            elif last_target:
                m = re_rule.match(line)
                if m:
                    targets[last_target][1].append(m.group(1))
    except EOFError:
        return targets


def get_target(file, targets):
    if file in targets.keys():
        return file
    for t in targets.keys():
        re1 = targets[t][2]
        if re1:
            if re1.match(file):
                return t
    return None


def get_rule_name(lines):
    global commands
    name = None
    for l in lines:
        for cmd in commands:
            if l.find(cmd) != -1:
                name = cmd
                break
    if name:
        return name.strip()
    else:
        return '-'


def escape(s):
    return '"' + s.replace('_', '-').replace('.', '-') + '"'


style_file = 'node [shape=ellipse, label="%s"]; '
style_rule = 'node [shape=box, label="%s"]; '
arrow = ' -> '
sep = ';'


def print_rule(id, label):
    print (style_rule % (label, )) + escape(id) + sep


def print_file(id):
    short = os.path.basename(id)
    print (style_file % (short, )) + escape(short) + sep


printed = dict()


def process_rule(target, id, targets):
    global printed
    if not id:
        return
    if target in printed.keys():
        return
    printed[target] = True
    target_id = os.path.basename(target)
    rule_id = target_id + '-rule'
    print_rule(rule_id, get_rule_name(targets[id][1]))
    print escape(rule_id) + arrow + escape(target_id) + sep
    re1 = targets[id][2]
    for pre in targets[id][0]:
        if re1:
            m = re1.match(target)
            pre = pre.replace('%', m.group(1))
        print_file(pre)
        next_id = get_target(pre, targets)
        if next_id:
            process_rule(pre, next_id, targets)
        print  escape(os.path.basename(pre)) + arrow + escape(rule_id) + sep


def print_header():
    print 'digraph test {'
    print 'rankdir=LR;'
    print 'ranksep=0.1; nodesep=0.1; defaultdist = 0.1; len = 0.1;'


def print_footer():
    print '}'


def print_dot(target, targets):
    print_header()
    id = get_target(target, targets)
    if id:
        print_file(target)
        process_rule(target, id, targets)
    print_footer()
    return


def make_dot(target):
    targets = read_targets()
    print_dot(target, targets)


if len(sys.argv) < 2:
    sys.stderr.write('usage: ' + sys.argv[0] + ' <target> [<command 1> <command 2>... ]\n')
    exit(-1)
else:
    commands.extend(sys.argv[2:])
    sys.stderr.write('Known commands: ' + ' '.join(['[' + l + ']' for l in commands]) + '\n')
    make_dot(sys.argv[1])

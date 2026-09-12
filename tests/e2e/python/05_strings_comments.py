# e2e-markers: """Docstring, f"{name}", 'single', True, None
"""Module docstring here."""


def strings(name):
    a = "double"
    b = 'single'
    c = """triple
spanning lines"""
    d = '''also triple'''
    e = f"hello {name}"
    f = f"{name!r:>10}"
    g = f"{data['key']}"
    h = f"{x[1:]}"
    i = rf"C:\path\{name}"
    j = b"bytes"
    k = "escapes \n \t \\ \" \'"
    m = "adjacent" "concat"
    n = 'it\'s'
    o = "say \"hi\""
    doc = """summary.

    details with "quotes" and 'ticks'.
    """
    return o


def literals():
    a = 42
    b = 3.14
    c = 0xFF
    d = 0o17
    e = 0b101
    f = 1_000_000
    g = 2 + 3j
    h = True
    i = False
    j = None
    k = ...
    return a


# a standalone comment

x = 1  # trailing comment


def between():
    a = 1
    # comment between statements
    b = 2
    if a:
        # comment opening a suite
        c = 3
    # comment closing a suite
    return b


def branches_with_comments(x):
    if x:
        a()  # trailing in suite
    # comment before elif
    elif not x:
        b()
    # comment before else
    else:
        c()

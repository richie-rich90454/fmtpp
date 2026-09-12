# e2e-markers: import os, from sys, x: int, yield total, global seen
import os
import re as regex
from sys import argv as av
from os.path import (
    join,
    exists,
)
from . import sibling
from ..pkg import thing
import asyncio, sys


counter = 0
name: str = "fmtpp"
ratio: float = 3.5
pair = single = None
x, y = 1, 2
first, *rest = items
total += 5
count -= 1


def flow():
    global counter
    counter += 1
    assert counter > 0, "must advance"
    yield counter
    del counter
    return counter


def calls():
    print(1, 2, sep=", ", end="\n")
    result = compute(
        alpha,
        beta=2,
        *args,
        **kw,
    )
    chained = (
        foo()
        .bar(1)
        .baz(key="v")
    )
    total = sum(v for v in items if v)
    matrix = [[i * j for j in range(3)] for i in range(3)]
    mapping = {k: v for k, v in pairs if k}
    gen = (x * 2 for x in items)
    f = lambda a, b=a + 1: a + b
    g = lambda: None
    z = x[1:10:2]
    w = data["key"]
    v = "yes" if flag else "no"
    back = do_work() if ready else wait()
    multi = first + second \
        + third
    tup = (1, 2)
    lst = [1, 2, 3]
    st = {1, 2}
    d = {"a": 1}

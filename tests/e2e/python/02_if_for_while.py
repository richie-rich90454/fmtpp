# e2e-markers: if sig, elif other, else:, for i, while busy
def decide(sig, items):
    if sig == "go":
        launch()
    elif sig == "slow" or sig == "wait":
        queue()
    elif not sig:
        raise ValueError("empty")
    else:
        abort()


def loops(items):
    for i, item in enumerate(items):
        if i > 10:
            break
        print(i, item)
    else:
        print("drained")
    busy = True
    while busy:
        busy = poll()
    while True:
        break


def inline(x):
    if x: return 1
    elif not x: return 0
    else: return -1


def nested_chains(a, b, c):
    if a:
        if b:
            if c:
                return 1
            elif not c:
                return 2
        else:
            return 3
    return 4


def conds(x, y, s, d):
    a = 1 if x else 2
    b = "y" if s.startswith("y") else "n"
    if x < y and y <= 10 and y != 0:
        hit()
    if not (x or y):
        miss()
    if d["k"] and s[1:]:
        ok()
    if (f := compute()):
        use(f)

# e2e-markers: try:, except IOError, except ValueError, finally:, with open, match cmd
def guarded(path):
    try:
        risky(path)
    except IOError as e:
        handle(e)
    except (ValueError, TypeError):
        fix()
    except:
        raise
    else:
        ok()
    finally:
        cleanup()


def nested_try():
    try:
        try:
            a()
        except A:
            b()
    except B:
        c()


def resources(path, lock):
    with open(path) as h:
        data = h.read()
    with open("a") as x, open("b") as y:
        join(x, y)
    with lock:
        work()


async def locked(lock):
    async with lock:
        x = 1
    return x


def dispatch(cmd):
    match cmd.split():
        case ["go", target]:
            run(target)
        case ["stop"]:
            halt()
        case str() if len(cmd) > 3:
            other(cmd)
        case {"key": value} | [*items]:
            combo(value, items)
        case _:
            unknown()


def one_liners():
    try: risky()
    except E: fix()
    with a as b: use(b)
    match x:
        case 1: one()

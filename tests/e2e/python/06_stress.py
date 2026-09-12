# e2e-markers: class Outer, class Inner, async def run, match m, try:
class Outer(Base, Mixin, metaclass=Meta):
    """Outer docs."""

    count = 0

    class Inner:
        def deep(self):
            return 1

    def run(self, jobs):
        results = []
        for job in jobs:
            try:
                with job.lock:
                    out = job.execute()
            except JobError as e:
                log(e)
                continue
            else:
                results.append(out)
            match out:
                case {"ok": True}:
                    note("good")
                case _:
                    note("bad")
        while results:
            item = results.pop()
            if item:
                yield item


async def main(urls):
    async with session() as s:
        for url in urls:
            try:
                data = await s.get(url)
            except TimeoutError:
                continue
            else:
                yield data


class Empty:
    pass


def also_empty():
    """Only a docstring."""

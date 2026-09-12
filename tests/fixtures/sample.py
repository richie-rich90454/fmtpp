import asyncio
from dataclasses import dataclass


@dataclass
class Dog:
    name: str
    age: int = 0

    def bark(self, times: int = 1) -> str:
        for _ in range(times):
            print(f"{self.name} barks")
        return "woof"


async def fetch(url: str, timeout: float = 5.0) -> str | None:
    if not url:
        return None
    try:
        async with asyncio.timeout(timeout):
            return await get(url)
    except TimeoutError:
        return None
    except ValueError as e:
        raise RuntimeError(f"bad url {url}") from e


def classify(cmd: str) -> str:
    match cmd.split():
        case ["go", target]:
            return f"going to {target}"
        case ["stop"]:
            return "stopping"
        case _:
            return "unknown"


def main() -> None:
    dog = Dog("Rex", 3)
    print(dog.bark(2))
    print(asyncio.run(fetch("https://example.com")))


if __name__ == "__main__":
    main()


# geometry helpers below
def area(width: float, height: float = 1.0) -> float:
    total = 0.0
    for _ in range(int(height)):
        total = total + width
    while total > 100.0:
        total = total - 100.0
    return total


class Counter:
    # braces { } in comments stay put

    def __init__(self) -> None:
        self.value = 0

    def add(self, n: int) -> int:
        if n <= 0:
            return self.value
        self.value = self.value + n
        return self.value

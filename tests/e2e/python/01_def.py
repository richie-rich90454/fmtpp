# e2e-markers: def greet, async def fetch, class Dog, @dataclass
import asyncio
from dataclasses import dataclass


def greet(name, greeting="hello", *args, **kw):
    """Greet someone warmly."""
    return f"{greeting}, {name}"


async def fetch(url, timeout=5.0):
    if not url:
        return None
    return url


@dataclass
class Dog(Animal):
    name: str
    age: int = 0

    def bark(self, times: int = 1) -> str:
        for _ in range(times):
            print(f"{self.name} barks")
        return "woof"

    @property
    def label(self):
        return f"{self.name} ({self.age})"

    @staticmethod
    def species():
        return "dog"

    @classmethod
    def make(cls, name):
        return cls(name)


def nested():
    def inner(x=(1, 2), y={"a": 1}, z=[1, 2], f=lambda v: v):
        return x, y, z, f
    return inner


def empty():
    pass


def one_liner(): return 42


def typed(a: int, b: str = "x", *args: int, key: str, **kw: str) -> bool:
    return True

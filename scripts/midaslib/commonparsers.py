import functools
from argparse import ArgumentParser


def parse_address(func):
    @functools.wraps(func)
    def wrapper(self, parser: ArgumentParser):
        parser.add_argument("address", type=str)
        func(self, parser)
    return wrapper

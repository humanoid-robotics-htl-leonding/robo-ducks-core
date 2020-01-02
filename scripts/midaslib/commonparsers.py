import functools
from argparse import ArgumentParser


def parse_address(after = False):
    def parse_address_deco(func):
        @functools.wraps(func)
        def wrapper(self, parser: ArgumentParser):
            if after:
                func(self, parser)
            parser.add_argument("address", type=str)
            if not after:
                func(self, parser)
        return wrapper
    return parse_address_deco

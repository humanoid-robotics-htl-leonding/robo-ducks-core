from argparse import ArgumentParser

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address


@command("compile", "c")
class CompileCommand(Command):
    @parse_address
    def define_parser(self, parser: ArgumentParser):
        pass

    def execute(self, args):
        print("Execute: Compile")
        print(args)


if __name__ == '__main__':
    raise RuntimeError("Command class cannot be executed.")

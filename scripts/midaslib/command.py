import abc
import functools
from argparse import ArgumentParser


class Command(abc.ABC):
    help = "Description and Help: TODO"
    def __init__(self):
        self.name = None
        pass

    def handle(self, args):
        self.execute(args)

    def parser(self, parser: ArgumentParser):
        parser.set_defaults(command=self)
        self.define_parser(parser)

    @abc.abstractmethod
    def define_parser(self, parser: ArgumentParser):
        pass

    @abc.abstractmethod
    def execute(self, args):
        pass


def command(name: str, *aliases: str):
    def command_decorator(cmd: Command):
        orig_init = cmd.__init__

        @functools.wraps(cmd.__init__)
        def new_init(self, *args, **kwargs):
            orig_init(self)
            self.name = name
            self.aliases = aliases
        cmd.__init__ = new_init
        return cmd
    return command_decorator

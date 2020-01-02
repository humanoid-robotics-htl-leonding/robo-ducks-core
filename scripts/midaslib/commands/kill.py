from argparse import ArgumentParser

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address
from midaslib.naocom import Nao


@command("kill")
class KillCommand(Command):
    help = "Stop running tuhhNao on nao. (essentially midas run [ip] killall tuhhNao)"

    @parse_address()
    def define_parser(self, parser: ArgumentParser):
        pass

    def execute(self, args):
        nao = Nao(args.address)
        nao.execute("killall tuhhNao")

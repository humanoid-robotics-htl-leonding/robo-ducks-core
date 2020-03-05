from argparse import ArgumentParser

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address
from midaslib.naocom import Nao


@command("start")
class StartCommand(Command):
    help = "Simply start the tuhhNao executable. Essentially midas run /home/nao/naoqi/bin/tuhhNao"

    @parse_address()
    def define_parser(self, parser: ArgumentParser):
        pass

    def execute(self, args):
        nao = Nao(args.address)
        nao.execute(["/home/nao/naoqi/bin/tuhhNao"])

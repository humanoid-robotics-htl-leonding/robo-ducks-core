from argparse import ArgumentParser

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address
from midaslib.naocom import Nao


@command("copy", "cp")
class CopyCommand(Command):
    help = "Copy singular file."

    @parse_address()
    def define_parser(self, parser: ArgumentParser):
        parser.add_argument("source", help="Local file")
        parser.add_argument("destination", help="Remote file")

    def execute(self, args):
        nao = Nao(args.address)
        nao.upload(args.source, args.destination)

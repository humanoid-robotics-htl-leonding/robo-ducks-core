from argparse import ArgumentParser

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address
from midaslib.naocom import Nao


@command("shutdown")
class ShutdownCommand(Command):
    help = "Shutdown or reboot"  # Will be shown when midas -h is run

    @parse_address(True)
    def define_parser(self, parser: ArgumentParser):  # Here you can register the needed arguments
        parser.add_argument("--reboot", "-r", action="store_true", help="Reboot")

    def execute(self, args):
        nao = Nao(args.address)  # Nao Connector
        if args.reboot:
            nao.execute(["systemctl", "reboot"])
        else:
            nao.execute(["systemctl", "poweroff"])

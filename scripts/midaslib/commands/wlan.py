import abc
import functools
import logging
import os
import stat
import time
from argparse import ArgumentParser
from getpass import getpass

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address

import midaslib.naocom as nc


@command("wlan")
class WlanCommand(Command):
    nao: nc.Nao = None
    args = None
    help = "Connect a NAO (currently connected via LAN) to a WLAN"

    @parse_address(True)
    def define_parser(self, parser: ArgumentParser):
        parser.add_argument("ssid", type=str, help="SSID of WLAN")
        pass

    # @touch_step("SSH-Key")
    # def upload_ssh_key(self):
    #     self.nao.copy_ssh_key()

    def execute(self, args):
        print(f"Password for {args.ssid}: ")
        password = getpass()

        self.args = args
        self.nao = nc.Nao(args.address, True)
        self.nao.execute("sudo killall wpa_supplicant".split(' '))

        wpa_supplicant_lines = [
            "ctrl_interface=/var/run/wpa_supplicant",
            "ctrl_interface_group=0",
            "update_config=1"
        ]
        self.nao.execute(["sh", "-c", "echo '' | sudo tee /etc/wpa_supplicant.conf"])
        for line in wpa_supplicant_lines:
            self.nao.execute(["sh", "-c", f"echo '{line}' | sudo tee -a /etc/wpa_supplicant.conf"])

        self.nao.execute(["sh", "-c", f"sudo wpa_passphrase {args.ssid} {password} | sudo tee -a /etc/wpa_supplicant.conf"])
        self.nao.execute("sudo wpa_supplicant -B -D wext -i wlan0 -c /etc/wpa_supplicant.conf".split(' '))
        logging.info("Done... ")
        time.sleep(1)
        self.nao.execute("iw wlan0 link".split(' '))


if __name__ == '__main__':
    raise RuntimeError("Command class cannot be executed.")

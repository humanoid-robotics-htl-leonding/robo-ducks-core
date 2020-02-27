import abc
import functools
import logging
import os
import stat
from argparse import ArgumentParser

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address

import midaslib.naocom as nc


def touch_step(name):
    def touch_decoration(func):
        @functools.wraps(func)
        def touch_wrapper(self):
            logging.info(f" => Step: {name}")
            func(self)
        return touch_wrapper
    return touch_decoration


sysroot_file_names = {
    "nao5": "sysroot-7.3.0-1.tar.bz2",
    "nao6": "sysroot-9.2.0-1.tar.bz2"
}


@command("touch", "t")
class TouchCommand(Command):
    nao: nc.Nao = None
    args = None
    help = "(Former Gammaray) Setup NAO for usage with RoboDucks Code."

    @parse_address(True)
    def define_parser(self, parser: ArgumentParser):
        parser.add_argument("target", choices=["nao5", "nao6"], help="Nao Version to touch")
        parser.add_argument("--no-sysroot", "-n", action="store_true", default=False, help="Nao Version to touch")
        parser.add_argument("--hostname", type=str, default=None, help="Hostname to set the nao to")
        pass

    # @touch_step("SSH-Key")
    # def upload_ssh_key(self):
    #     self.nao.copy_ssh_key()

    def execute(self, args):
        self.args = args
        self.nao = nc.Nao(args.address)

        os.chmod(os.path.join(nc.base_dir, "files", "ssh_key"), stat.S_IRUSR)
        print("Uploading SSHkey...")
        self.nao.copy_ssh_key()
        print("Uploaded SSHkey...")

        if not args.no_sysroot:
            if args.target not in sysroot_file_names:
                raise RuntimeError(f"Unknown nao version: {args.target}.")

            sysroot_path = os.path.join(nc.project_base_dir, "toolchain", sysroot_file_names[args.target])

            if not os.path.exists(sysroot_path):
                raise RuntimeError(f"Please install sysroot for {args.target} as {sysroot_path}")

            print("Uploading Sysroot...")
            self.nao.upload(sysroot_path, "sysroot.tar.bz2")
            print("Extracting Sysroot...")
            self.nao.execute("tar xf sysroot.tar.bz2".split(' '))
            print("Deleting old Sysroot...")
            self.nao.execute("rm sysroot.tar.bz2".split(' '))
            print("Done.")

        if args.hostname:
            print("Setting hostname")
            self.nao.execute(f"hostnamectl set-hostname {args.hostname}".split(' '))

        print("Copying configs and such...")
        self.nao.upload(os.path.join(nc.base_dir, "files", "bin"), ".local")
        self.nao.upload(os.path.join(nc.base_dir, "files", "etc"), ".local")
        self.nao.upload(os.path.join(nc.base_dir, "files", "asound.state"), ".config")
        self.nao.upload(os.path.join(nc.base_dir, "files", ".asoundrc"), "~")
        print("Done.")

        print("Set executable bits")
        self.nao.execute("chmod u+x .local/bin/alsa-restore".split(' '))
        # self.nao.execute("chmod u+x .local/bin/connman-hulks".split(' '))
        self.nao.execute("chmod u+x .local/bin/setNetwork".split(' '))
        print("Done.")

        print("Copy bashrc")
        self.nao.upload(os.path.join(nc.base_dir, "files", "bashrc_v6"), ".profile")
        print("Done.")

        print("Copy services")
        self.nao.execute("mkdir -p .config/systemd/user".split(' '))
        self.nao.upload(os.path.join(nc.base_dir, "files", "alsa-restore.service"), ".config/systemd/user/")
        # self.nao.upload(os.path.join(nc.base_dir, "files", "connman-hulks.service"), ".config/systemd/user/")
        self.nao.upload(os.path.join(nc.base_dir, "files", "hulk.service"), ".config/systemd/user/")
        self.nao.upload(os.path.join(nc.base_dir, "files", "lola-hulks.service"), ".config/systemd/user/")
        print("Done.")

        print("Activating Services")
        self.nao.execute("systemctl --user enable alsa-restore.service".split(' '))
        # self.nao.execute("systemctl --user enable connman-hulks.service".split(' '))
        self.nao.execute("systemctl --user enable hulk.service".split(' '))
        self.nao.execute("systemctl --user enable lola-hulks.service".split(' '))
        print("Done.")

        print("Add robocup.conf")
        self.nao.execute("touch robocup.conf".split(' '))
        print("Done.")

        print("Rebooting now.")
        self.nao.reboot()


if __name__ == '__main__':
    raise RuntimeError("Command class cannot be executed.")

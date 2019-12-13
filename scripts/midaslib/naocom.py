import logging
import os
import subprocess
from enum import Enum


def exit_on_failure(func):
    def wrapper(*args, **kwargs):
        return_code = func(*args, **kwargs)
        if return_code:
            logging.error("Error: " + func.__name__ + " " + str(args) + " " +
                          str(kwargs) + " exited with return code " +
                          str(return_code))
            cont = input("Continue (y/N)? ")
            if cont != 'y':
                exit(1)

    return wrapper


class NaoVersion(Enum):
    V5 = 0
    V6 = 1


base_dir = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
ssh_standard_args = [
    "-o", "UserKnownHostsFile=/dev/null",
    "-o", "StrictHostKeyChecking=no",
    "-o", "LogLevel=quiet",
    "-o", "ConnectTimeout=5",
    "-i", base_dir + "/files/ssh_key"
]


def subprocess_run(command):
    logging.info(" ".join(command))
    return subprocess.run(command).returncode


@exit_on_failure
def copy_ssh_id(password, user, address):
    ssh_copy_id_command = [
        "sshpass", "-p", password, "ssh-copy-id", *ssh_standard_args, user + "@" + address
    ]
    return subprocess_run(ssh_copy_id_command)


@exit_on_failure
def scp(sources, destination):
    if type(sources) != list:
        sources = [sources]

    scp_command = [
        "scp", *ssh_standard_args, "-r", *sources, destination
    ]
    return subprocess_run(scp_command)


@exit_on_failure
def ssh_command(address, user, command):
    command = [
        "ssh", *ssh_standard_args, "-l", user, address, command
    ]
    return subprocess_run(command)


class Nao:
    def __init__(self, address):
        self.address = address
        self.user = "nao"

    def execute(self, command):
        return ssh_command(self.address, self.user, command)

    def upload(self, source, destination):
        return scp(source, f"{self.user}@{self.address}:{destination}")

    def reboot(self):
        self.execute("systemctl reboot")

    def copy_ssh_key(self):
        return copy_ssh_id("nao", self.user, self.address)

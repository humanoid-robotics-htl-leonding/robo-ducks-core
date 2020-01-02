import os
import tempfile

from midaslib.compiler import Nao6CompileTarget
from midaslib.naocom import project_base_dir as base_dir
import midaslib.naocom as nc


def create_build_link(from_path_parts, to_path_parts):
    from_path = os.path.join(*from_path_parts)
    to_path = os.path.join(*to_path_parts)
    if not os.path.exists(from_path):
        raise FileNotFoundError(f"Tried to create link to missing file {from_path}")
    return os.symlink(from_path, to_path, True)


class Uploader:
    def __init__(self, config, address, target, build_type):
        self.with_config = config
        self.address = address
        self.target = target
        self.build_type = build_type

    def upload(self):
        temp_dir = tempfile.TemporaryDirectory("_naoqi", "midas_")
        print(temp_dir.name)
        os.makedirs(os.path.join(temp_dir.name, "naoqi", "lib"))
        os.makedirs(os.path.join(temp_dir.name, "naoqi", "bin"))
        os.makedirs(os.path.join(temp_dir.name, "naoqi", "filetransport_ball_candidates"))

        if self.with_config:
            create_build_link([base_dir, "home", "preferences"], [temp_dir.name, "naoqi", "preferences"])
        create_build_link([base_dir, "home", "configuration"], [temp_dir.name, "naoqi", "configuration"])

        create_build_link([base_dir, "home", "motions"], [temp_dir.name, "naoqi", "motions"])
        create_build_link([base_dir, "home", "poses"], [temp_dir.name, "naoqi", "poses"])

        build_dir = os.path.join(base_dir, "build", self.target.foldername, self.build_type.foldername)
        if self.target != Nao6CompileTarget:
            create_build_link([build_dir, "src", "tuhhsdk", "libtuhhALModule.so"],
                              [temp_dir.name, "naoqi", "lib", "libtuhhALModule.so"])

        create_build_link([build_dir, "src", "tuhhsdk", "tuhhNao"], [temp_dir.name, "naoqi", "bin", "tuhhNao"])

        nao = nc.Nao(self.address)

        nao.rsync(os.path.join(temp_dir.name, "naoqi"), False)

        temp_dir.cleanup()

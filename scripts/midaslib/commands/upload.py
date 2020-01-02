import logging
import os
import tempfile
from argparse import ArgumentParser

from midaslib.command import command, Command
from midaslib.commonparsers import parse_address
from midaslib.compiler import Targets, Compiler, BuildTypes, Nao6CompileTarget

from midaslib.naocom import project_base_dir as base_dir

import midaslib.naocom as nc


def create_build_link(from_path_parts, to_path_parts):
    from_path = os.path.join(*from_path_parts)
    to_path = os.path.join(*to_path_parts)
    if not os.path.exists(from_path):
        raise FileNotFoundError(f"Tried to create link to missing file {from_path}")
    return os.symlink(from_path, to_path, True)


@command("upload", "u")
class UploadCommand(Command):

    @parse_address
    def define_parser(self, parser: ArgumentParser):
        # parser.add_argument("target", choices=Targets.keys(), help="Compile target")
        parser.add_argument("build", choices=BuildTypes.keys(), help="Build type")
        parser.add_argument("target", choices=Targets.keys(), help="Compile target")
        parser.add_argument("--config", "-c", action="store_true", help="Upload Config")

    def execute(self, args):
        build_type = BuildTypes[args.build]
        target = Targets[args.target]
        logging.info(f"Uploading '{build_type.name}' to '{target.name}'")

        temp_dir = tempfile.TemporaryDirectory("_naoqi", "midas_")
        print(temp_dir.name)
        os.makedirs(os.path.join(temp_dir.name, "naoqi", "lib"))
        os.makedirs(os.path.join(temp_dir.name, "naoqi", "bin"))
        os.makedirs(os.path.join(temp_dir.name, "naoqi", "filetransport_ball_candidates"))

        if args.config:
            create_build_link([base_dir, "home", "preferences"], [temp_dir.name, "naoqi", "preferences"])
            create_build_link([base_dir, "home", "configuration"], [temp_dir.name, "naoqi", "configuration"])

        create_build_link([base_dir, "home", "motions"], [temp_dir.name, "naoqi", "motions"])
        create_build_link([base_dir, "home", "poses"], [temp_dir.name, "naoqi", "poses"])

        build_dir = os.path.join(base_dir, "build", target.foldername, build_type.foldername)
        if target != Nao6CompileTarget:
            create_build_link([build_dir, "src", "tuhhsdk", "libtuhhALModule.so"],
                              [temp_dir.name, "naoqi", "lib", "libtuhhALModule.so"])

        create_build_link([build_dir, "src", "tuhhsdk", "tuhhNao"], [temp_dir.name, "naoqi", "bin", "tuhhNao"])

        nao = nc.Nao(args.address)

        nao.rsync(os.path.join(temp_dir.name, "naoqi"), False)

        temp_dir.cleanup()

# # /home/nao/naoqi structure
  # mkdir -p "${TMP_DIR}/naoqi"
  # mkdir -p "${TMP_DIR}/naoqi/lib"
  # mkdir -p "${TMP_DIR}/naoqi/bin"
  # mkdir -p "${TMP_DIR}/naoqi/filetransport_ball_candidates"
  #
  # if ${UPLOAD_CONFIG}; then
  #   ln -s "${BASEDIR}/home/preferences" "${TMP_DIR}/naoqi/preferences"
  #   ln -s "${BASEDIR}/home/configuration" "${TMP_DIR}/naoqi/configuration"
  # fi
  # ln -s "${BASEDIR}/home/motions" "${TMP_DIR}/naoqi/motions"
  # ln -s "${BASEDIR}/home/poses"   "${TMP_DIR}/naoqi/poses"
  # if [[ "${TARGET}" -ne "nao6" ]]; then
  #   ln -s "${BASEDIR}/build/${TARGET}/${BUILD_TYPE}/src/tuhhsdk/libtuhhALModule.so" "${TMP_DIR}/naoqi/lib/libtuhhALModule.so"
  # fi
  # ln -s "${BASEDIR}/build/${TARGET}/${BUILD_TYPE}/src/tuhhsdk/tuhhNao" "${TMP_DIR}/naoqi/bin/tuhhNao"
  #
  # # ssh wants the key permissions to be like that
  # if [ -e "${SSH_KEY}" ]; then
  #   chmod 400 "${SSH_KEY}"
  # fi
  #
  # # ssh connection command with parameters; check also the top config part
  # local SSH_CMD="ssh -q -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -l ${SSH_USERNAME} -i \"${SSH_KEY}\""
  #
  # # parameters for rsync
  # local RSYNC_PARAMETERS="-trzKLP ${RSYNC_EXCLUDE}"
  # if ${DELETE_FILES}; then
  #   RSYNC_PARAMETERS+=" --delete --delete-excluded"
  #   delete_logs $BASEDIR $RSYNC_TARGET
  # fi
  #
  # # run rsync with prepared parameters
  # rsync ${RSYNC_PARAMETERS} --rsh="${SSH_CMD}" "${TMP_DIR}/naoqi" "${RSYNC_TARGET}:"
  # local RSYNC_RESULT=$?
  #
  # # clean temp directory
  # rm -rf "${TMP_DIR}"
  #
  # return $RSYNC_RESULT

        # target = Targets[args.target]
        # logging.info(f"Setting up '{target.name}'")
        # compiler = Compiler(target)
        # compiler.setup()

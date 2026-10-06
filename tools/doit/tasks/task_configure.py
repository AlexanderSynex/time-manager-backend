from doit import task_params
from doit.action import CmdAction
from doit.tools import create_folder, Interactive
from tools.doit.common.environment import BUILD_TYPE, get_project_root, get_project_build_dir, get_expected_build_types
import logging

import shutil


def prepare_build_folder(reconfigure: bool):
    build_path = get_project_build_dir()

    if reconfigure:
        shutil.rmtree(build_path, ignore_errors=True)
    
    create_folder(build_path)


def conan_install(release:bool, debug:bool) -> str:
    flags_positive = release and debug

    if flags_positive:
        logging.error("Flags --release and --debug can not be used at the sametime")
        return 
    
    types = get_expected_build_types(release=release, debug=debug)

    build_system = "Ninja Multi-Config" if CmdAction("command -v ninja",).execute() is None else "Unix Makefiles"

    cmds = []
    for build_type in types:
        conan_install_cmd = " ".join([
            f"conan install {get_project_root()}",
            f"--output-folder={get_project_build_dir()}",
            f"--build=missing",
            f"-s build_type={build_type}",
            f"-c tools.cmake.cmaketoolchain:generator=\"{build_system}\"",
            f"-pr=default"
        ]) 
        cmake_configure_cmd = " ".join([
            f"cmake --preset conan-default"
        ])
        cmds += [" && ".join([conan_install_cmd, cmake_configure_cmd])]

    return " && ".join(cmds)


@task_params([
    {
        'name': 'reconfigure',
        'short': 'f',
        'long': 'force',
        'default': False,
        'type': bool,
        'help': 'Reconfigure project and its dependencies'
    },
    {
        'name': 'release',
        'long': 'release',
        'default': False,
        'type': bool,
        'help': 'Configure only release project'
    },
    {
        'name': 'debug',
        'long': 'debug',
        'default': False,
        'type': bool,
        'help': 'Configure only debug project'
    },
])
def task_configure(reconfigure, release, debug):
    return {
        'actions': [
            prepare_build_folder,
            Interactive(conan_install),
        ],
        'targets': [
            get_project_build_dir() / "conan_toolchain.cmake", 
            *(get_project_build_dir() / build_type for build_type in get_expected_build_types(release, debug))
        ],
        'uptodate': [True and not reconfigure], 
        'doc': 'Configures the project',
    }

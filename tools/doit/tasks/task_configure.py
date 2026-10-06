from doit import task_params
from doit.action import CmdAction
from doit.tools import create_folder, Interactive
from tools.doit.common.environment import get_project_root, get_project_build_dir
import logging

import shutil


def get_expected_types(release: bool, debug: bool):
    all = not release and not debug
    release = release or all
    debug = debug or all
    types = []
    if debug:
        types += ["debug"]
    if release:
        types += ["release"]
    return types


def prepare_build_folder(reconfigure: bool):
    build_path = get_project_build_dir()

    if reconfigure:
        shutil.rmtree(build_path, ignore_errors=True)
    
    create_folder(build_path)


def conan_install(release:bool, debug:bool):
    flags_positive = release and debug

    if flags_positive:
        logging.error("Flags --release and --debug can not be used at the sametime")
        return 
    
    types = get_expected_types(release=release, debug=debug)

    build_system = "Ninja" if CmdAction("command -v ninja",).execute() is None else "Unix Makefiles"

    cmds = []
    for build_type in types:
        print(build_type)
        profile = f"default-{build_type}"
        cmds += [
            f"conan install {get_project_root()} "
            f"--output-folder={get_project_build_dir(build_type=build_type)} "
            f"--build=missing "
            f"-c tools.cmake.cmaketoolchain:generator=\"{build_system}\" "
            f"--profile {profile}"
        ]
    
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
        'targets': [get_project_build_dir(build_type) / "conan_toolchain.cmake" for build_type in get_expected_types(release, debug)],
        'uptodate': [True and not reconfigure], 
        'doc': 'Configures the project',
    }
    
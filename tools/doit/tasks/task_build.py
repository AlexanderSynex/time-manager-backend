from doit import task_params
from doit.tools import Interactive
from tools.doit.common.environment import get_expected_build_types

def run_build(clean: bool, release:bool, debug:bool):
    build_types = get_expected_build_types(release=release, debug=debug)

    cmds = []
    for build_type in build_types:
        cmds += [f'cmake --build --preset conan-{build_type.lower()}']
        if clean:
            cmds[-1] += ' --clean-first'

    return ' && '.join(cmds)


@task_params([
    {
        'name': 'clean',
        'short': 'f',
        'long': 'clean',
        'default': False,
        'type': bool,
        'help': 'Clean rebuild'
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
def task_build(clean: bool, release: bool, debug: bool):
    return {
        'actions': [Interactive(run_build)],
        'doc': 'Builds binary for the app',
        'task_dep': ['configure'],
    }

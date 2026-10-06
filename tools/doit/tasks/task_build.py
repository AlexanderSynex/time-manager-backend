from doit.action import CmdAction

def run_build(is_release: bool):
    return CmdAction('/usr/bin/cmake --build /home/synex/dev/cpp/time-tracker/backend/build --parallel 16 --')

def task_build():
    return {
        'actions': [run_build],
        'doc': 'Builds binary for the app',
        'params': [
            {
                'name': 'is_release',
                'long': 'release',
                'default': False,
                'type': bool,
                'help': 'Build a release version'
            }
        ],
    }

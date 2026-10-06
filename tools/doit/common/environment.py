from pathlib import Path
from functools import lru_cache
from typing import Literal

@lru_cache(maxsize=1)
def get_project_root() -> Path:
    for p in Path(__file__).resolve().parents:
        if (p / '.git').exists() or (p / 'pyproject.toml').exists():
            return p
    return Path(__file__).resolve().parent

@lru_cache(maxsize=1)
def get_project_build_dir(build_type: Literal[None, 'build', 'release'] = None) -> Path:
    path = get_project_root() / "build"
    if build_type is not None:
        path = path / build_type
    return path

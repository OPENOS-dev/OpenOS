# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Replacement module for sysconfig used for CrOS.

Historically, we used to patch sysconfig.py, and many distutils files to handle
cross-compilation correctly.  This proved to be difficult and error prone, and
we'd rather simply provide the correct config data we know for CrOS.
"""

# pylint: disable=redefined-builtin

import os
import platform
import sys


def get_python_version():
    """Get the Python version we're compiling for (via EPYTHON)."""
    epython = os.environ.get("EPYTHON", "")
    if epython.startswith("python"):
        return epython[6:]

    # EPYTHON unset?  Assume we're compiling for this python.
    return f"{sys.version_info.major}.{sys.version_info.minor}"


def _get_abi():
    """Get the ABI environment variable, or a fallback if unavailable."""
    abi = os.environ.get("ABI")
    if abi:
        return abi

    machine = platform.machine().lower()
    is_arm = machine.startswith("arm") or machine.startswith("aarch64")
    is_32bit = platform.architecture()[0] == "32bit"
    if is_arm and is_32bit:
        return "arm"
    elif is_arm:
        return "arm64"
    else:
        return "amd64"


_ABI = _get_abi()
_ABIFULL = {
    "amd64": "x86_64-linux-gnu",
    "arm": "arm-linux-gnueabihf",
    "arm64": "aarch64-linux-gnu",
}.get(_ABI, "unknown")
_SOABI = f"cpython-{get_python_version().replace('.', '')}-{_ABIFULL}"
_LIBDIR_NAME = os.environ.get(
    f"LIBDIR_{_ABI}", "lib64" if "64" in _ABI else "lib"
)
_CC = os.environ.get("CC", "clang")
_CXX = os.environ.get("CXX", "clang++")
_SYSROOT = os.environ.get("SYSROOT", "/").rstrip("/")
_PREFIX = f"{_SYSROOT}/usr"
_BASE_PREFIX = _PREFIX
_EXEC_PREFIX = _PREFIX
_BASE_EXEC_PREFIX = _PREFIX


# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/sysconfig.py#L185C1-L190C1
def _safe_realpath(path):
    try:
        return os.path.realpath(path)
    except OSError:
        return path


# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/sysconfig.py#L191C1-L197C1
if sys.executable:
    _PROJECT_BASE = os.path.dirname(_safe_realpath(sys.executable))
else:
    # sys.executable can be empty if argv[0] has been changed and Python is
    # unable to retrieve the real program name
    _PROJECT_BASE = _safe_realpath(os.getcwd())

# We swap-in this module after the Python build is finished.
_PYTHON_BUILD = False

# Detect and respect venv.
if sys.prefix != sys.base_prefix:
    _PREFIX = sys.prefix


# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/sysconfig.py#L109
def _getuserbase():
    env_base = os.environ.get("PYTHONUSERBASE", None)
    if env_base:
        return env_base
    return os.path.expanduser(os.path.join("~", ".local"))


_CONFIG_VARS = {
    "ABIFLAGS": "",
    "AR": os.environ.get("AR", "llvm-ar"),
    "ARFLAGS": os.environ.get("ARFLAGS", "rcs"),
    "BLDLIBRARY": f"-L. -lpython{get_python_version()}",
    "BLDSHARED": f"{_CC} -shared",
    "BUILDPYTHON": sys.executable,
    "CC": _CC,
    "CCSHARED": "-fPIC",
    "CFLAGS": os.environ.get("CFLAGS", ""),
    "CFLAGSFORSHARED": "-fPIC",
    "CONFIG_ARGS": os.environ.get("CONFIG_ARGS", ""),
    "CONFINCLUDEDIR": f"{_PREFIX}/include",
    "CONFINCLUDEPY": f"{_PREFIX}/include/python{get_python_version()}",
    "CPPFLAGS": os.environ.get("CPPFLAGS", ""),
    "CXX": _CXX,
    "CXXFLAGS": os.environ.get("CXXFLAGS", ""),
    "DESTLIB": f"/usr/lib/python{get_python_version()}",
    "DIRMODE": 755,
    "ENSUREPIP": "no",
    "EXE": "",
    "EXEMODE": 755,
    "EXPAT_LDFLAGS": "-lexpat",
    "EXT_SUFFIX": f".{_SOABI}.so",
    "FILEMODE": 644,
    "INCLUDEDIR": f"{_PREFIX}/include",
    "INCLUDEPY": f"{_PREFIX}/include/python{get_python_version()}",
    "LDCXXSHARED": f"{_CXX} -shared",
    "LDFLAGS": os.environ.get("LDFLAGS", ""),
    "LDLIBRARY": f"libpython{get_python_version()}.so",
    "LDSHARED": f"{_CC} -shared",
    "LDVERSION": get_python_version(),
    "LIBDEST": f"/usr/lib/python{get_python_version()}",
    "LIBDIR": f"{_PREFIX}/{_LIBDIR_NAME}",
    "LIBM": "-lm",
    "LIBRARY": f"libpython{get_python_version()}.a",
    "LIBS": "-ldl",
    "LINKCC": _CC,
    "LN": "ln",
    "MAINCC": _CC,
    "OPENSSL_LIBS": "-lssl -lcrypto",
    "Py_ENABLE_SHARED": 1,
    "SHLIB_SUFFIX": ".so",
    "SOABI": _SOABI,
    "VERSION": get_python_version(),
    # Required for the ensurepip module to work. It contains the path where the
    # dev-python/ensurepip-pip and dev-python/ensurepip-setuptools packages are
    # installed. This config variable is used here:
    # https://github.com/python/cpython/blob/e33b6fccd3969dc2351f6a64d7b0362c36c2be96/Lib/ensurepip/__init__.py#L29
    "WHEEL_PKG_DIR": "/usr/lib/python/ensurepip",
    "abiflags": "",
    "base": _PREFIX,
    "exec_prefix": _PREFIX,
    "installed_base": _PREFIX,
    "installed_platbase": _PREFIX,
    "libdir_name": _LIBDIR_NAME,
    "platbase": _PREFIX,
    "platlibdir": getattr(sys, "platlibdir", "lib"),
    "prefix": _PREFIX,
    "projectbase": os.path.dirname(sys.executable),
    "py_version": get_python_version(),
    "py_version_nodot": get_python_version().replace(".", ""),
    "py_version_nodot_plat": "",
    "py_version_short": get_python_version(),
    # https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/sysconfig.py#L675
    "userbase": _getuserbase(),
}

# Keys for get_config_var() that are never converted to Python integers.
_ALWAYS_STR = {
    "MACOSX_DEPLOYMENT_TARGET",
}

# Required by distutils.
# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/distutils/command/install.py#L44C22-L44C31
# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/sysconfig.py#L26C1-L58C11
_INSTALL_SCHEMES = {
    "posix_prefix": {
        "stdlib": "{installed_base}/{platlibdir}/python{py_version_short}",
        "platstdlib": "{platbase}/{platlibdir}/python{py_version_short}",
        "purelib": "{base}/lib/python{py_version_short}/site-packages",
        # pylint: disable=line-too-long
        "platlib": "{platbase}/{platlibdir}/python{py_version_short}/site-packages",
        # pylint: disable=line-too-long
        "include": "{installed_base}/include/python{py_version_short}{abiflags}",
        # pylint: disable=line-too-long
        "platinclude": "{installed_platbase}/include/python{py_version_short}{abiflags}",
        "scripts": "{base}/bin",
        "data": "{base}",
    },
    "posix_home": {
        "stdlib": "{installed_base}/lib/python",
        "platstdlib": "{base}/lib/python",
        "purelib": "{base}/lib/python",
        "platlib": "{base}/lib/python",
        "include": "{installed_base}/include/python",
        "platinclude": "{installed_base}/include/python",
        "scripts": "{base}/bin",
        "data": "{base}",
    },
    # https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/sysconfig.py#L145
    "posix_user": {
        "stdlib": "{userbase}/{platlibdir}/python{py_version_short}",
        "platstdlib": "{userbase}/{platlibdir}/python{py_version_short}",
        "purelib": "{userbase}/lib/python{py_version_short}/site-packages",
        "platlib": "{userbase}/lib/python{py_version_short}/site-packages",
        "include": "{userbase}/include/python{py_version_short}",
        "scripts": "{userbase}/bin",
        "data": "{userbase}",
    },
    "nt": {
        # These keys are needed by Python 3.11's distutils module, see
        # https://github.com/python/cpython/blob/db85d51d3ea4adfc6147d6af400e167659689eed/Lib/distutils/command/install.py#L42
        # lines 42-52. That said, we can leave their values empty because we
        # will never build for Windows.
        "stdlib": "",
        "platstdlib": "",
        "purelib": "",
        "platlib": "",
        "include": "",
        "platinclude": "",
        "scripts": "",
        "data": "",
    },
}


def get_config_vars(*args):
    if args:
        return [_CONFIG_VARS.get(x) for x in args]
    return _CONFIG_VARS


def get_config_var(name):
    return _CONFIG_VARS.get(name)


def get_platform():
    return os.environ.get("_PYTHON_HOST_PLATFORM", "linux-x86_64")


def get_preferred_scheme(key):
    if key == "prefix" and sys.prefix != sys.base_prefix:
        return "venv"
    return {
        "prefix": "posix_prefix",
        "home": "posix_home",
        "user": "posix_user",
    }[key]


def get_default_scheme():
    return get_preferred_scheme("prefix")


# Meson uses _get_default_scheme :/
_get_default_scheme = get_default_scheme


def get_scheme_names():
    return ("posix_prefix", "posix_home", "posix_user", "posix_venv", "venv")


def get_paths(scheme=None, vars=None, expand=True):
    if scheme in [None, "posix_venv", "venv"]:
        scheme = "posix_prefix"
    paths = _INSTALL_SCHEMES[scheme]

    if not expand:
        return paths
    vars = {**_CONFIG_VARS, **(vars or {})}
    return {k: v.format(**vars) for k, v in paths.items()}


def get_path(name, scheme=None, vars=None, expand=True):
    return get_paths(scheme, vars, expand)[name]


def is_python_build(check_home: bool = False) -> bool:
    # We swap-in this module after the Python build is finished.
    del check_home
    return False


# Required by distutils.
# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/distutils/sysconfig.py#L29
# pylint: disable=unused-argument
def _init_posix(vars):
    pass


# Required by distutils.
# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/distutils/sysconfig.py#L32
# pylint: disable=unused-argument
def _init_non_posix(vars):
    pass


# Required by distutils.
# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/distutils/sysconfig.py#L34
# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/sysconfig.py#L180C1-L182C46
_variable_rx = r"([a-zA-Z][a-zA-Z0-9_]+)\s*=\s*(.*)"
_findvar1_rx = r"\$\(([A-Za-z][A-Za-z0-9_]*)\)"
_findvar2_rx = r"\${([A-Za-z][A-Za-z0-9_]*)}"


# Required by distutils.
# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/distutils/sysconfig.py#L30
# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/sysconfig.py#L553
def parse_config_h(fp, vars=None):
    """Parse a config.h-style file.

    A dictionary containing name/value pairs is returned.  If an
    optional dictionary is passed in as the second argument, it is
    used instead of a new dictionary.
    """
    if vars is None:
        vars = {}
    import re

    define_rx = re.compile("#define ([A-Z][A-Za-z0-9_]+) (.*)\n")
    undef_rx = re.compile("/[*] #undef ([A-Z][A-Za-z0-9_]+) [*]/\n")

    while True:
        line = fp.readline()
        if not line:
            break
        m = define_rx.match(line)
        if m:
            n, v = m.group(1, 2)
            try:
                if n in _ALWAYS_STR:
                    raise ValueError
                v = int(v)
            except ValueError:
                pass
            vars[n] = v
        else:
            m = undef_rx.match(line)
            if m:
                vars[m.group(1)] = 0
    return vars


# Required by distutils.
# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/distutils/sysconfig.py#L38C5-L38C25
# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/sysconfig.py#L803
def expand_makefile_vars(s, vars):
    import re

    while True:
        m = re.search(_findvar1_rx, s) or re.search(_findvar2_rx, s)
        if m:
            (beg, end) = m.span()
            s = s[0:beg] + vars.get(m.group(1)) + s[end:]
        else:
            break
    return s


# Required by distutils.
# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/distutils/sysconfig.py#L40
# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/sysconfig.py#L587
def get_config_h_filename():
    return os.path.join(get_path("platinclude"), "pyconfig.h")


# Required by distutils.
# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/distutils/sysconfig.py#L43
# https://github.com/python/cpython/blob/795f2597a4be988e2bb19b69ff9958e981cb894e/Lib/sysconfig.py#L443
def get_makefile_filename():
    return os.path.join(get_path("stdlib"), "config", "Makefile")


def _main():
    print(f"Platform: {get_platform()!r}")
    print(f"Python version: {get_python_version()!r}")
    print(f"Current installation scheme: {get_default_scheme()!r}")
    print()

    print("Paths:")
    for name, value in sorted(get_paths().items()):
        print(f"\t{name} = {value!r}")
    print()

    print("Variables:")
    for name, value in sorted(_CONFIG_VARS.items()):
        print(f"\t{name} = {value!r}")


if __name__ == "__main__":
    _main()

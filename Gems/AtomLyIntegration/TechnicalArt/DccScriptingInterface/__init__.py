# coding:utf-8
#!/usr/bin/python
#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#
# --------------------------------------------------------------------------
"""! @brief

<DCCsi>/__init__.py

Allow DccScriptingInterface Gem to be the parent python pkg
"""
DCCSI_DOCS_URL = "https://github.com/o3de/o3de/blob/development/Gems/AtomLyIntegration/TechnicalArt/DccScriptingInterface/readme.md"

DCCSI_INFO = {
    'name': 'O3DE_DCCSI_GEM',
    "description": "DccScriptingInterface",
    'status': 'prototype',
    'version': (0, 0, 1),
    'platforms': ({
        'include': ('win'),   # windows
        'exclude': ('darwin', # mac
                    'linux')  # linux, linux_x64
    }),
    "doc_url": DCCSI_DOCS_URL
}
import timeit
_MODULE_START = timeit.default_timer()  # start tracking
# -------------------------------------------------------------------------
# standard imports
import os
import sys
import site
import json
import warnings
from pathlib import Path
import logging as _logging
# -------------------------------------------------------------------------
# global scope
_PACKAGENAME = 'DCCsi'

STR_CROSSBAR = f"{('-' * 74)}"
FRMT_LOG_LONG = "[%(name)s][%(levelname)s] >> %(message)s (%(asctime)s; %(filename)s:%(lineno)d)"
FRMT_LOG_SHRT = "[%(asctime)s][%(name)s][%(levelname)s] >> %(message)s"

# allow package to capture warnings
_logging.captureWarnings(capture=True)

# default loglevel to info unless set
ENVAR_DCCSI_LOGLEVEL = 'DCCSI_LOGLEVEL'
DCCSI_LOGLEVEL = int(os.getenv(ENVAR_DCCSI_LOGLEVEL, _logging.INFO))

# configure basic logger since this is the top-level module
# note: not using a common logger to reduce cyclical imports
_logging.basicConfig(level=DCCSI_LOGLEVEL,
                    format=FRMT_LOG_LONG,
                    datefmt='%m-%d %H:%M')

_LOGGER = _logging.getLogger(_PACKAGENAME)
_LOGGER.debug(STR_CROSSBAR)
_LOGGER.debug(f'Initializing: {_PACKAGENAME}')

__all__ = ['globals', # global state module
           'config', # dccsi core config.py
           'constants', # global dccsi constants
           'foundation', # set up dependancy pkgs for DCC tools
           'return_sys_version', # util
           'azpy', # shared pure python api
           'Editor', # O3DE editor scripts
           'Tools' # DCC and IDE tool integrations
           ]

# be careful when pulling from this __init__.py module
# avoid cyclical imports, this module should not import from sub-modules

# we need to set up basic access to the DCCsi
_MODULE_PATH = Path(__file__)
_LOGGER.debug(f'_MODULE_PATH: {_MODULE_PATH}')
# -------------------------------------------------------------------------



# -------------------------------------------------------------------------
def pathify(path_list):
    """Make list path objects"""
    for i, p in enumerate(path_list):
        p = Path(p)
        path_list[i] = p.resolve()

    return path_list

def add_site_dir(site_dir):
    """Add a site package and moves to front of sys.path"""

    site_dir = Path(site_dir).resolve()

    _prev_sys_paths = pathify(list(sys.path)) # copy

    # if passing a Path object cast to string value
    site.addsitedir(site_dir.as_posix())

    # new entries have precedence, front of sys.path
    for p in pathify(list(sys.path)):
        if p not in _prev_sys_paths:
            sys.path.remove(str(p))
            sys.path.insert(0, str(p))

    return site_dir
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# add gems parent, dccsi lives under:
#  < o3de >\Gems\AtomLyIntegration\TechnicalArt
PATH_O3DE_TECHART_GEMS = _MODULE_PATH.parents[1].resolve()
PATH_O3DE_TECHART_GEMS = add_site_dir(PATH_O3DE_TECHART_GEMS)

# there may be other TechArt gems in the future
# or the dccsi maybe split up
ENVAR_PATH_DCCSIG = 'PATH_DCCSIG'

# < o3de >\Gems\AtomLyIntegration\TechnicalArt\< dccsi >
# the default location
PATH_DCCSIG = _MODULE_PATH.parents[0].resolve()

# this allows the dccsi gem location to be overridden in the external env
PATH_DCCSIG = Path(os.getenv(ENVAR_PATH_DCCSIG,
                              PATH_DCCSIG.as_posix()))
try: # validate (if envar is bad)
    PATH_DCCSIG.resolve(strict=True)
    _LOGGER.debug(f'{ENVAR_PATH_DCCSIG}: {PATH_DCCSIG.as_posix()}')
except NotADirectoryError as e:
    _LOGGER.error(f'{ENVAR_PATH_DCCSIG} failed: {PATH_DCCSIG.as_posix()}',
                  exc_info=True)
    raise NotADirectoryError

# < dccsi >\3rdParty bootstrapping area for installed site-packages
# dcc tools on a different version of python will import from here.
# path string constructor
ENVAR_PATH_DCCSI_PYTHON_LIB = 'PATH_DCCSI_PYTHON_LIB'

# a str path constructor for the dccsi 3rdPary site-dir
STR_DCCSI_PYTHON_LIB = (f'{PATH_DCCSIG.as_posix()}' +
                        f'\\3rdParty\\Python\\Lib' +
                        f'\\{sys.version_info[0]}.x' +
                        f'\\{sys.version_info[0]}.{sys.version_info[1]}.x' +
                        f'\\site-packages')

PATH_DCCSI_PYTHON_LIB = Path(STR_DCCSI_PYTHON_LIB)
try:
    PATH_DCCSI_PYTHON_LIB = Path(PATH_DCCSI_PYTHON_LIB).resolve(strict=True)
    _LOGGER.debug(f'{ENVAR_PATH_DCCSI_PYTHON_LIB} is: {PATH_DCCSI_PYTHON_LIB.as_posix()}')
except NotADirectoryError as e:
    _LOGGER.warning(f'{ENVAR_PATH_DCCSI_PYTHON_LIB} does not exist:' +
                    f'{PATH_DCCSI_PYTHON_LIB.as_Posix()}')
    _LOGGER.error(f'Try using foundation.py', exc_info=True)
    # dependencies are required to exist
    raise NotADirectoryError
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# paths we can retrieve easily from o3de
O3DE_LOG_FOLDER = None # empty containers
O3DE_CACHE = None
PATH_O3DE_PROJECT = None
O3DE_DEV = None
PATH_O3DE_BIN = None

try:
    import azlmbr.paths

    _LOGGER.debug(f'log: {azlmbr.paths.log}')
    O3DE_LOG_FOLDER = Path(azlmbr.paths.log)

    _LOGGER.debug(f'products: {azlmbr.paths.products}')
    O3DE_CACHE = Path(azlmbr.paths.products)

    _LOGGER.debug(f'projectroot: {azlmbr.paths.projectroot}')
    PATH_O3DE_PROJECT = Path(azlmbr.paths.projectroot)

    _LOGGER.debug(f'engroot: {azlmbr.paths.engroot}')
    O3DE_DEV = Path(azlmbr.paths.engroot)
    O3DE_DEV = add_site_dir(O3DE_DEV)

    _LOGGER.debug(f'executableFolder: {azlmbr.paths.executableFolder}')
    PATH_O3DE_BIN = Path(azlmbr.paths.executableFolder)
    PATH_O3DE_BIN = add_site_dir(PATH_O3DE_BIN)

except ImportError as e:
    _LOGGER.warning(f'{e}')
    _LOGGER.warning(f'Not running o3do, no: azlmbr.paths')
    _LOGGER.warning(f'Using fallbacks ...')
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# a developer file for setting envars and overrides
PATH_ENV_DEV = Path(PATH_DCCSIG, 'Tools', 'Dev', 'Windows', 'Env_Dev.bat')

# a local settings file to overrides envars in config.py
DCCSI_SETTINGS_LOCAL_FILENAME = 'setting.local.json'
PATH_DCCSI_SETTINGS_LOCAL = Path(PATH_DCCSIG, DCCSI_SETTINGS_LOCAL_FILENAME)

# the o3de manifest data
TAG_USER_O3DE = '.o3de'
SLUG_MANIFEST_FILENAME = 'o3de_manifest.json'
USER_HOME = Path.home()

_user_home_parts = os.path.split(USER_HOME)

if str(_user_home_parts[1].lower()) == 'documents':
    USER_HOME = _user_home_parts[0]
    _LOGGER.debug(f'user home CORRECTED: {USER_HOME}')

O3DE_MANIFEST_PATH = USER_HOME.joinpath(f'{TAG_USER_O3DE}',
                                        SLUG_MANIFEST_FILENAME)

# {user_home}\.o3de\registry\bootstrap.setreg
SLUG_BOOTSTRAP_FILENAME = 'bootstrap.setreg'
O3DE_BOOTSTRAP_PATH = Path(USER_HOME,
                      TAG_USER_O3DE,
                      'Registry',
                      SLUG_BOOTSTRAP_FILENAME)
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# default o3de engine location
# a suggestion would be for us to refactor from _O3DE_DEV to _O3DE_ROOT
# dev is a legacy Lumberyard concept, as the engine sandbox folder was /dev
ENVAR_O3DE_DEV = 'O3DE_DEV'

if not O3DE_DEV:
    # we could also check the .o3de\o3de_manifest.json?
    # most end users likely only have one engine install?
    try:
        # the manifest doesn't exist until you run o3de.exe for the first time
        O3DE_MANIFEST_PATH.resolve(strict=True)
    except FileExistsError as e:
        _LOGGER.warning(f'O3DE Manifest does not exist: {O3DE_MANIFEST_PATH}')
        _LOGGER.warning(f'Make sure the engine is installed, and run O3DE.exe')
        _LOGGER.warning(f'Or build from source and and register the engine ')
        _LOGGER.warning(f'CMD > c:\\path\\to\\o3de\\scripts\\o3de register --this-engine')
        O3DE_MANIFEST_PATH = None

    O3DE_MANIFEST_DATA = None
    if O3DE_MANIFEST_PATH:
        try:
            with open(O3DE_MANIFEST_PATH, "r") as data:
                O3DE_MANIFEST_DATA = json.load(data)
        except IOError as e:
            _LOGGER.error(f'Cannot read manifest: {O3DE_MANIFEST_PATH} ')
            pass

    if O3DE_MANIFEST_DATA:
        _LOGGER.warning(f'O3DE Manifest found: {O3DE_MANIFEST_PATH}')
        # what if there are multiple "engines_path"s? We don't know which to use
        ENGINES_PATH = O3DE_MANIFEST_DATA['engines_path']

        if len(ENGINES_PATH) == 1: # there can only be one
            O3DE_ENGINENAME = list(ENGINES_PATH.items())[0][0]
            O3DE_DEV = Path(list(ENGINES_PATH.items())[0][1])
        else:
            _LOGGER.warning('--------')
            _LOGGER.warning(f'Manifest defines more then one engine: {O3DE_DEV.as_posix()}')
            _LOGGER.warning(f'Not sure which to use? We suggest...')
            _LOGGER.warning(f"Put 'set {ENVAR_O3DE_DEV}=c:\\path\\to\\o3de' in: {PATH_ENV_DEV}")
            _LOGGER.warning(f"And '{ENVAR_O3DE_DEV}:c:\\path\\to\\o3de' in: {PATH_DCCSI_SETTINGS_LOCAL}")
            _LOGGER.warning('--------')

        try:
            O3DE_DEV.resolve(strict=True) # make sure the found engine exists
            _LOGGER.debug(f'O3DE Manifest {ENVAR_O3DE_DEV} is: {O3DE_DEV.as_posix()}')
        except NotADirectoryError as e:
            _LOGGER.warning('--------')
            _LOGGER.warning(f'Manifest engine does not exist: {O3DE_DEV.as_posix()}')
            _LOGGER.warning(f'Make sure the engine is installed, and run O3DE.exe')
            _LOGGER.warning(f'Or build from source and and register the engine ')
            _LOGGER.warning(f'CMD > c:\\path\\to\\o3de\\scripts\\o3de register --this-engine')
            _LOGGER.warning('--------')
            O3DE_DEV = None

    # obvious fallback 1, just walk up ...
    # unless the dev has the dccsi somewhere else this should work fine
    # if we move the dccsi out of the engine later we can refactor
    if not O3DE_DEV: # fallback 1, assume root from dccsi
        O3DE_DEV = PATH_DCCSIG.parents[3]
        try:
            O3DE_DEV.resolve(strict=True)
            _LOGGER.debug(f'{ENVAR_O3DE_DEV} is: {O3DE_DEV.as_posix()}')
        except NotADirectoryError as e:
            O3DE_DEV = None
            _LOGGER.error(f'{ENVAR_O3DE_DEV} does not exist: {O3DE_DEV.as_posix()}',
                          exc_info=True)
            raise NotADirectoryError

# but it's always best to just be explicit?
# fallback 2, check if a dev set it in env and allow override
O3DE_DEV = os.getenv(ENVAR_O3DE_DEV, O3DE_DEV)
if O3DE_DEV: # could still end up None
    try:
        O3DE_DEV = Path(O3DE_DEV).resolve(strict=True)
        _LOGGER.debug(f'Final {ENVAR_O3DE_DEV} is: {O3DE_DEV.as_posix()}')
        # should we implement a safe way to manage adding paths
        # to make sure we are not adding paths already there?
        O3DE_DEV = add_site_dir(O3DE_DEV)
    except NotADirectoryError as e:
        _LOGGER.warning(f'{ENVAR_O3DE_DEV} does not exist: {O3DE_DEV.as_posix()}')
        _LOGGER.warning(f"Put 'set = c:\\path\\to\\o3de' in: {PATH_ENV_DEV}")
        _LOGGER.warning(f"And '{ENVAR_O3DE_DEV}:c:\\path\\to\\o3de' in: " +
                        f"{PATH_DCCSI_SETTINGS_LOCAL}")
        _LOGGER.warning('--------')
        O3DE_DEV = None
else:
    _LOGGER.error(f'{ENVAR_O3DE_DEV} not defined', exc_info=True)
    raise NotADirectoryError
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# o3de game project
# this is a problem of multiple projects
# we can get the local project root if we are running in a o3de exe
O3DE_PROJECT = None# str name of the project

ENVAR_PATH_O3DE_PROJECT = 'PATH_O3DE_PROJECT' # project path
ENVAR_O3DE_PROJECT = 'O3DE_PROJECT' # project name

# 2, we could also check the .o3de\o3de_manifest.json?
if not PATH_O3DE_PROJECT:
    # but there could be many projects ...
    # Let's assume many users will only have one project
    if O3DE_MANIFEST_DATA:
        PROJECTS = O3DE_MANIFEST_DATA['projects']
        if len(PROJECTS) == 1: # there can only be one
            PATH_O3DE_PROJECT = PROJECTS[0]
        else:
            _LOGGER.warning('--------')
            _LOGGER.warning(f'Manifest defines more then one project')
            _LOGGER.warning(f'Not sure which to use? We suggest...')
            _LOGGER.warning(f"Put 'set {ENVAR_PATH_O3DE_PROJECT}=c:\\path\\to\\o3de\\project' in: {PATH_ENV_DEV}")
            _LOGGER.warning(f"And '{ENVAR_O3DE_DEV}:c:\\path\\to\\o3de\\project' in: {PATH_DCCSI_SETTINGS_LOCAL}")
            _LOGGER.warning('--------')

# 3, we can see if a global project is set as the default
if not PATH_O3DE_PROJECT:
    O3DE_BOOTSTRAP_DATA = None
    if O3DE_BOOTSTRAP_PATH.resolve(strict=True):
        try:
            with open(O3DE_BOOTSTRAP_PATH, "r") as data:
                O3DE_BOOTSTRAP_DATA = json.load(data)
        except IOError as e:
            _LOGGER.error(f'Cannot read bootstrap: {O3DE_BOOTSTRAP_PATH} ')
            pass
    else:
        _LOGGER.debug(f'Bootstrap does not exist: {O3DE_BOOTSTRAP_PATH}')

    if O3DE_BOOTSTRAP_DATA:
        try:
            PATH_O3DE_PROJECT = Path(O3DE_BOOTSTRAP_DATA['Amazon']['AzCore']['Bootstrap']['project_path'])
        except KeyError as e:
            _LOGGER.warning(f'Bootstrap error: {e}')
            PATH_O3DE_PROJECT = None

    if PATH_O3DE_PROJECT: # we got one
        if PATH_O3DE_PROJECT.resolve(strict=True): #validate
            pass
        else:
            PATH_O3DE_PROJECT = None

# 4, we can fallback to the DCCsi as the project
# 5, be explicit, and allow it to be set/overridden from the env
if not PATH_O3DE_PROJECT:
    PATH_O3DE_PROJECT = Path(os.getenv(ENVAR_PATH_O3DE_PROJECT, PATH_DCCSIG))

    # note: if you are a developer, and launching via the dccsi windows .bat files
    # the DccScriptingInterface gets set in the env as the default project
    # which would in this case override the other options above
    # when running from a cli in that env, or a IDE launched in that env

    if not os.environ.get(ENVAR_O3DE_PROJECT):
        _LOGGER.warning('--------')
        _LOGGER.warning(f"envar '{ENVAR_O3DE_PROJECT}' does not exist")
        _LOGGER.warning(f"Put 'set {ENVAR_O3DE_PROJECT}= c:\\path\\to\\o3de' in: {PATH_ENV_DEV}")
        _LOGGER.warning(f"And '{ENVAR_O3DE_PROJECT}:c:\\path\\to\\o3de' in: "
                        + f"{PATH_DCCSI_SETTINGS_LOCAL}")
        _LOGGER.warning('--------')

        try:
            PATH_O3DE_PROJECT.resolve(strict=True)
        except NotADirectoryError as e:
            _LOGGER.warning(f'{e}')
            _LOGGER.warning(f'path is bad: {PATH_O3DE_PROJECT}')
            PATH_O3DE_PROJECT = PATH_DCCSIG # ultimate fallback

# log the final results
_LOGGER.warning(f'Default {ENVAR_PATH_O3DE_PROJECT}: {PATH_O3DE_PROJECT.as_posix()}')

# the projects name, suggestion we should extend this to actually retrieve
# the project name, from a root data file such as project.json or gem.json
O3DE_PROJECT = str(os.getenv(ENVAR_O3DE_PROJECT,
                             PATH_O3DE_PROJECT.name))

_LOGGER.warning(f'Default {ENVAR_O3DE_PROJECT}: {O3DE_PROJECT}')
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# default o3de engine bin folder
ENVAR_PATH_O3DE_BIN = 'PATH_O3DE_BIN'

# suggestion, create a discovery.py module for the dccsi to find o3de

# \bin could vary a lot in location, between end users and developers...
# engine could be built from source, with unknown 'build' folder location
# because users can name that folder and configure cmake in a number of ways

# also could be a project centric build, which is problematic if we don't
# know the project folder location (block above might derive a default fallback)

# if the user is a dev building from source, and the have some kind of 'build'
# folder under the engine root, we can use this search method (but can be slow)
# import  DccScriptingInterface.azpy.config_utils as dccsi_config_utils
# PATH_O3DE_BUILD = dccsi_config_utils.get_o3de_build_path(O3DE_DEV,'CMakeCache.txt')
# and should result in something like: C:\depot\o3de-dev\build
# but pre-built installer builds don't have this file (search fails)

# the next problem is what is the path down from that root to executables?
# the profile executable path for the nightly installer is a default like:
# C:\O3DE\0.0.0.0\bin\Windows\profile\Default

# when I create a build folder, and build engine-centric it's something like:
# C:\depot\o3de-dev\build\bin\profile

# There are a lot of ways we could search and derive.
# We could search and walk down ... what a mess

# o3de provides this method (used above), which standalone tools can't access
# import azlmbr.paths
# _LOGGER.debug(f'engroot: {azlmbr.paths.engroot}')

# fallback 1, # the easiest check is to derive if we are running o3de
if not PATH_O3DE_BIN:

    # executable, such as Editor.exe or MaterialEditor.exe
    O3DE_EDITOR = Path(sys.executable) # executible path

    if O3DE_EDITOR.stem.lower() in {"editor",
                                    "materialeditor",
                                    "materialcanvas",
                                    "assetprocessor",
                                    "assetbuilder"}:
        PATH_O3DE_BIN = O3DE_EDITOR.parent
        # honestly though, I could see this failing in edge cases?
        # good enough for now I think...
        PATH_O3DE_BIN = add_site_dir(PATH_O3DE_BIN)

    elif O3DE_EDITOR.stem.lower() == "python":
        pass # we don't know which python, could be a DCC python?

    else:
        pass # anything else we don't handle here
        # (because we are looking for known o3de things)

# all this extra work, is because we want to support the creation
# of standalone external Qt/PySide tools. So we want access to the build
# to load Qt DLLs and such (another example is compiled openimageio)

# A second way would be to check the known default installer location
# Could that still result in multiple installs for developers?
# Projects are usually coded against an engine version
# nightly:     C:\O3DE\0.0.0.0
# last stable: C:\O3DE\22.05.0
# pass, leave that to discovery.py (grep a pattern?)

# A third way would be to check the OS registry for custom install location
# this might be the best robust solution for multi-platform?
# pass, this is a prototype leave that to discovery.py
# still has the multi-install/multi-engine problem

# A fourth way, would be to look at o3de data
# if we know the project, check the project.json
# the project.json can define which registered engine to use
# such as 'o3de' versus 'o3de-sdk', providing multi-engine support
# check the .o3de\o3de_manifest.json and retreive engine path?

# the next way, for now the best way, is to just explicitly set it somewhere
# fallback 2, check if a dev set it in env and allow override
# this works well for a dev in IDE on windows, by setting in Env_Dev.bat
# "DccScriptingInterface\Tools\Dev\Windows\Env_Dev.bat" or
# "DccScriptingInterface\Tools\IDE\Wing\Env_Dev.bat"
if not PATH_O3DE_BIN:
    PATH_O3DE_BIN = os.environ.get(ENVAR_PATH_O3DE_BIN)

# if nothing else worked, try using the slow search method
if not PATH_O3DE_BIN and O3DE_DEV:
    import DccScriptingInterface.azpy.config_utils as dccsi_config_utils
    PATH_O3DE_BUILD = dccsi_config_utils.get_o3de_build_path(O3DE_DEV,'CMakeCache.txt')

if PATH_O3DE_BIN:
    PATH_O3DE_BIN = Path(PATH_O3DE_BIN)
    try:
        PATH_O3DE_BIN = PATH_O3DE_BIN.resolve(strict=True)
        _LOGGER.debug(f'Final {ENVAR_PATH_O3DE_BIN} is: {PATH_O3DE_BIN.as_posix()}')
        # should we implement a safe way to manage adding paths
        # to make sure we are not adding paths already there?
        add_site_dir(PATH_O3DE_BIN)
    except NotADirectoryError as e:
        _LOGGER.warning(f'{ENVAR_PATH_O3DE_BIN} does not exist: {PATH_O3DE_BIN.as_posix()}')
        _LOGGER.warning(f"Put 'set {ENVAR_PATH_O3DE_BIN}= c:\\path\\to\\o3de' in: {PATH_ENV_DEV}")
        _LOGGER.warning(f"And '{ENVAR_PATH_O3DE_BIN}:c:\\path\\to\\o3de' in: " +
                        f"{PATH_DCCSI_SETTINGS_LOCAL}")
        _LOGGER.warning('--------')
        PATH_O3DE_BIN = None
else:
    _LOGGER.error(f'{ENVAR_PATH_O3DE_BIN} not defined', exc_info=True)
    #raise NotADirectoryError

_MODULE_END = timeit.default_timer() - _MODULE_START
_LOGGER.debug(f'DCCsi init complete')
_LOGGER.debug(f'{_PACKAGENAME}.init took: {_MODULE_END} sec')
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# will probably deprecate this whole block to avoid cyclical imports
# this doesn't need to be an entrypoint or debugged from cli

# from DccScriptingInterface.globals import *
# from azpy.config_utils import attach_debugger
# from azpy import test_imports

# # suggestion would be to turn this into a method to reduce boilerplate
# # but where to put it that makes sense?
# if DCCSI_DEV_MODE:
#     # if dev mode, this will attempt to auto-attach the debugger
#     # at the earliest possible point in this module
#     attach_debugger(debugger_type=DCCSI_GDEBUGGER)
#
#     _LOGGER.debug(f'Testing Imports from {_PACKAGENAME}')
#
#     # If in dev mode and test is flagged this will force imports of __all__
#     # although slower and verbose, this can help detect cyclical import
#     # failure and other issues
#
#     # the DCCSI_TESTS flag needs to be properly added in .bat env
#     if DCCSI_TESTS:
#         test_imports(_all=__all__,
#                      _pkg=_PACKAGENAME,
#                      _logger=_LOGGER)
# -------------------------------------------------------------------------

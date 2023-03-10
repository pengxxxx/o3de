#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

import fnmatch
import os
import pathlib
from typing import Type, List

from commit_validation.commit_validation import Commit, CommitValidator, EXCLUDED_VALIDATION_PATTERNS, VERBOSE

# Disallowed File Name Patterns
# these must be LOWER CASE since they will be compared after a 'lower'
AZ_CODEGEN_FILENAME_PATTERN = "*.generated*"
QT_COMPILER_PATTERNS = ["moc_*", "qrc_*", "ui_*.h"]
CMAKE_CACHE_FILENAME_PATTERNS = ["cmakecache.txt", "cmake_install.cmake", "ctesttestfile.cmake", "ctestcostdata.txt"]
OTHER_GENERATED_FILES = ["appxmanifest.xml"]

# Disallowed File Extensions
CMAKE_CACHE_EXTENSIONS = [".rule", ".stamp", ".depend"]
TEMPFILE_EXTENSIONS = [".backup", ".bak", ".bkup", ".temp", ".tempfile", ".temporary", ".tmp"]
EXECUTION_ARTIFACT_EXTENSIONS = [".cache", ".dmp", ".log", ".pyc"]
CLANG_FILE_EXTENSIONS = [".o", ".s"]
MSVS_FILE_EXTENSIONS = [
    ".aps",
    ".csproj",
    ".exp",
    ".filters",
    ".idb",
    ".ilk",
    ".lastbuildstate",
    ".ncb",
    ".obj",
    ".opensdf",
    ".pch",
    ".pdb",
    ".pgc",
    ".pgd",
    ".psess",
    ".rsp",
    ".sbr",
    ".sdf",
    ".sln",
    ".suo",
    ".tlb",
    ".tlh",
    ".tli",
    ".tlog",
    ".vap",
    ".vbg",
    ".vbproj",
    ".vcxitems",
    ".vcxproj",
    ".vdproj",
    ".vmx",
    ".vsp",
    ".vspscc",
    ".vspx",
    ".vssscc",
    ".vup",
]


class GeneratedFilesValidator(CommitValidator):
    """A file-level validator to check if a file is a true source file, and not generated during build or execution"""

    def run(self, commit: Commit, errors: List[str]) -> bool:
        for file_name in commit.get_files():
            file_identifier = f"{file_name}::{self.__class__.__name__}"
            for pattern in EXCLUDED_VALIDATION_PATTERNS:
                if fnmatch.fnmatch(file_name, pattern):
                    if VERBOSE: print(f'{file_identifier} SKIPPED - Validation pattern excluded on path.')
                    break
            else:
                file_path_lower = pathlib.Path(file_name.lower())
                extension = file_path_lower.suffix
                if extension in CMAKE_CACHE_EXTENSIONS:
                    error_message = str(f'{file_identifier} FAILED - Autogenerated CMake file detected by extension.')
                    if VERBOSE: print(error_message)
                    errors.append(error_message)
                    continue
                if extension in MSVS_FILE_EXTENSIONS:
                    error_message = str(f'{file_identifier} FAILED - Autogenerated Visual Studio file detected by extension.')
                    if VERBOSE: print(error_message)
                    errors.append(error_message)
                    continue
                if extension in CLANG_FILE_EXTENSIONS:
                    error_message = str(f'{file_identifier} FAILED - Autogenerated Clang file detected by extension.')
                    if VERBOSE: print(error_message)
                    errors.append(error_message)
                    continue
                if extension in TEMPFILE_EXTENSIONS:
                    error_message = str(f'{file_identifier} FAILED - Autogenerated temporary file detected by extension.')
                    if VERBOSE: print(error_message)
                    errors.append(error_message)
                    continue
                if extension in EXECUTION_ARTIFACT_EXTENSIONS:
                    error_message = str(f'{file_identifier} FAILED - Execution artifact detected by extension.')
                    if VERBOSE: print(error_message)
                    errors.append(error_message)
                    continue

                for cmake_pattern in QT_COMPILER_PATTERNS:
                    if fnmatch.fnmatch(file_path_lower.name, cmake_pattern):
                        error_message = str(f'{file_identifier} FAILED - Autogenerated QT file detected by pattern.')
                        if VERBOSE: print(error_message)
                        errors.append(error_message)
                        break
                for cmake_pattern in CMAKE_CACHE_FILENAME_PATTERNS:
                    if fnmatch.fnmatch(file_path_lower.name, cmake_pattern):
                        error_message = str(f'{file_identifier} FAILED - Autogenerated CMake file detected by name.')
                        if VERBOSE: print(error_message)
                        errors.append(error_message)
                        break
                if fnmatch.fnmatch(file_path_lower.name, AZ_CODEGEN_FILENAME_PATTERN):
                    error_message = str(f'{file_identifier} FAILED - Autogenerated AzCodeGen file detected by name.')
                    if VERBOSE: print(error_message)
                    errors.append(error_message)
                    continue
                if file_path_lower.name in OTHER_GENERATED_FILES:
                    error_message = str(f'{file_identifier} FAILED - Generated file (others).')
                    if VERBOSE: print(error_message)
                    errors.append(error_message)
                    continue

        return (not errors)


def get_validator() -> Type[GeneratedFilesValidator]:
    """Returns the validator class for this module"""
    return GeneratedFilesValidator

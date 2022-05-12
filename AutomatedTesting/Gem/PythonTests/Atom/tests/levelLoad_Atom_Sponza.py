"""
Copyright (c) Contributors to the Open 3D Engine Project.
For complete copyright and license terms please see the LICENSE at the root of this distribution.

SPDX-License-Identifier: Apache-2.0 OR MIT
"""

class Tests:
    sponza_level_load = (
        "Sponza level loaded",
        "P0: Sponza level failed to load")
    sponza_enter_game_mode = (
        "Sponza entered gameplay successfully",
        "P0: Sponza failed to enter gameplay")
    sponza_exit_game_mode = (
        "Sponza exited gameplay successfully",
        "P0: Sponza failed to exit gameplay")
    empty_level_load = (
        "Empty level loaded successfully, Editor remains stable",
        "P0: Empty level fails to load, editor is either hung or crashed")


def Atom_Editor_LevelLoad_Sponza():
    """
    Summary:
    Loads the "Sponza" level within an instance of Editor.exe using the null renderer. Test verifies that
    the level loads, can enter/exit gameplay, and that Editor.exe remains stable throughout this process.

    Test setup:
    - Launch Editor.exe

    Expected Behavior:
    Test verifies that level loads, enters/exits game mode, and editor remains stable.

    Test Steps:
    1) Load level, confirm that correct level is loaded, and report results
    2) Validate that editor can enter gameplay successfully
    3) Validate that editor can exit gameplay successfully
    4) Open an empty level to verify editor remains stable
    5) Look for errors or asserts.

    :return: None
    """

    import azlmbr.legacy.general as general

    from editor_python_test_tools.utils import Report, Tracer, TestHelper

    with Tracer() as error_tracer:

        # 1. Load level Sponza, confirm that correct level is loaded, and report results
        TestHelper.init_idle()
        TestHelper.open_level("Graphics", "Sponza")
        Report.result(Tests.sponza_level_load, "Sponza" == general.get_current_level_name())

        # 2. Validate that editor can enter gameplay successfully.
        TestHelper.enter_game_mode(Tests.sponza_enter_game_mode)
        general.idle_wait_frames(1)

        # 3. Validate that editor can exit gameplay successfully.
        TestHelper.exit_game_mode(Tests.sponza_exit_game_mode)
        general.idle_wait_frames(1)

        # 4. Open an empty level to verify editor remains stable.
        TestHelper.open_level("Graphics", "base_empty")
        Report.result(Tests.empty_level_load, "base_empty" == general.get_current_level_name())

        # 5. Look for errors or asserts.
        TestHelper.wait_for_condition(lambda: error_tracer.has_errors or error_tracer.has_asserts, 1.0)
        for error_info in error_tracer.errors:
            Report.info(f"Error: {error_info.filename} {error_info.function} | {error_info.message}")
        for assert_info in error_tracer.asserts:
            Report.info(f"Assert: {assert_info.filename} {assert_info.function} | {assert_info.message}")


if __name__ == "__main__":
    from editor_python_test_tools.utils import Report
    Report.start_test(Atom_Editor_LevelLoad_Sponza)

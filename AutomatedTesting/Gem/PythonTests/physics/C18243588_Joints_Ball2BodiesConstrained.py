"""
Copyright (c) Contributors to the Open 3D Engine Project.
For complete copyright and license terms please see the LICENSE at the root of this distribution.

SPDX-License-Identifier: Apache-2.0 OR MIT
"""

# Test case ID : C18243588
# Test Case Title : Check that ball joint constrains 2 bodies within cone limits

# fmt: off
class Tests:
    enter_game_mode                 = ("Entered game mode",                         "Failed to enter game mode")
    exit_game_mode                  = ("Exited game mode",                          "Couldn't exit game mode")
    lead_found                      = ("Found lead",                                "Did not find lead")
    follower_found                  = ("Found follower",                            "Did not find follower")
    check_lead_position             = ("Lead stays still",                          "Lead moved")
    check_follower_position         = ("Follower moved in X, Y and Z directions",   "Follower did not move in X, Y and Z directions")
    check_follower_below_lead       = ("Follower remains below lead",               "Follower moved above lead")
# fmt: on


def C18243588_Joints_Ball2BodiesConstrained():
    """
    Summary: Check that ball joint constrains 2 bodies within cone limits

    Level Description:
    lead - Starts above follower entity
    follower - Starts below lead entity. Constrained to lead entity with ball joint. Starts with initial velocity of (5, 2, 0).

    Expected Behavior: 
    The follower entity moved in the positive X, Y and Z directions.
    The position of the lead entity does not change much.
    The follower entity did not move above the lead entity.

    Test Steps:
    1) Open Level
    2) Enter Game Mode
    3) Create and Validate Entities
    4) Wait for several seconds
    5) Check to see if lead and follower behaved as expected
    6) Exit Game Mode
    7) Close Editor

    Note:
    - This test file must be called from the Open 3D Engine Editor command terminal
    - Any passed and failed tests are written to the Editor.log file.
            Parsing the file or running a log_monitor are required to observe the test results.

    :return: None
    """
    import os
    import sys

    import ImportPathHelper as imports

    imports.init()

    from editor_python_test_tools.utils import Report
    from editor_python_test_tools.utils import TestHelper as helper

    import azlmbr.legacy.general as general
    import azlmbr.bus

    import JointsHelper
    from JointsHelper import JointEntity

    # Constants
    FLOAT_EPSILON = 0.1 # Negligible float value for comparing with translation vectors. Values smaller than this are considered zero.

    # Helper Entity class
    class Entity(JointEntity):
        def criticalEntityFound(self): # Override function to use local Test dictionary
            Report.critical_result(Tests.__dict__[self.name + "_found"], self.id.isValid())

    # Main Script
    helper.init_idle()

    # 1) Open Level
    helper.open_level("Physics", "C18243588_Joints_Ball2BodiesConstrained")

    # 2) Enter Game Mode
    helper.enter_game_mode(Tests.enter_game_mode)

    # 3) Create and Validate Entities
    lead = Entity("lead")
    follower = Entity("follower")
    Report.info_vector3(lead.position, "lead initial position:")
    Report.info_vector3(follower.position, "follower initial position:")
    leadInitialPosition = lead.position
    followerInitialPosition = follower.position

    # 4) Wait for several seconds
    general.idle_wait(1.0) # wait for lead and follower to move

    # 5) Check to see if lead and follower behaved as expected
    Report.info_vector3(lead.position, "lead position after 1 second:")
    Report.info_vector3(follower.position, "follower position after 1 second:")

    leadPositionDelta = lead.position.Subtract(leadInitialPosition)
    leadRemainedStill = JointsHelper.vector3SmallerThanScalar(leadPositionDelta, FLOAT_EPSILON)
    Report.critical_result(Tests.check_lead_position, leadRemainedStill)

    followerPositionDelta = follower.position.Subtract(followerInitialPosition)
    followerMovedInXAndZOnly = JointsHelper.vector3LargerThanScalar(followerPositionDelta, FLOAT_EPSILON)
    Report.critical_result(Tests.check_follower_position, followerMovedInXAndZOnly)

    followerBelowLead = follower.position.z < lead.position.z
    Report.critical_result(Tests.check_follower_below_lead, followerBelowLead)

    # 6) Exit Game Mode
    helper.exit_game_mode(Tests.exit_game_mode)



if __name__ == "__main__":
    import ImportPathHelper as imports
    imports.init()

    from editor_python_test_tools.utils import Report
    Report.start_test(C18243588_Joints_Ball2BodiesConstrained)

#
# Copyright (c) Contributors to the Open 3D Engine Project
# 
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#


set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_OSX_ARCHITECTURES arm64)


set(LY_IOS_CODE_SIGNING_IDENTITY "iPhone Developer" CACHE STRING "iPhone Developer")
set(LY_IOS_DEPLOYMENT_TARGET "13.0" CACHE STRING "iOS Deployment Target")
set(LY_IOS_DEVELOPMENT_TEAM "CF9TGN983S" CACHE STRING "The development team ID")


# PAL variables
set(PAL_PLATFORM_NAME iOS)
string(TOLOWER ${PAL_PLATFORM_NAME} PAL_PLATFORM_NAME_LOWERCASE)

include(${CMAKE_CURRENT_LIST_DIR}/SDK_ios.cmake)

set(CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")
set(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${LY_IOS_DEPLOYMENT_TARGET})
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE NO)

set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED YES)
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${LY_IOS_CODE_SIGNING_IDENTITY})
set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${LY_IOS_DEVELOPMENT_TEAM})

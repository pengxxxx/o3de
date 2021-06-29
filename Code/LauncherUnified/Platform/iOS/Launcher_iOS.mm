/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#import <UIKit/UIKit.h>


int main(int argc, char* argv[])
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    UIApplicationMain(argc,
                      argv,
                      @"O3DEApplication_iOS",
                      @"O3DEApplicationDelegate_iOS");
    [pool release];
    return 0;
}

void CVar_OnViewportPosition([[maybe_unused]] const AZ::Vector2& value) {}

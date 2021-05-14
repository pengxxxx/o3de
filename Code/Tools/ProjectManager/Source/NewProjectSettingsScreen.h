/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */
#pragma once

#if !defined(Q_MOC_RUN)
#include <ScreenWidget.h>
#endif

namespace O3DE::ProjectManager
{
    class NewProjectSettingsScreen
        : public ScreenWidget
    {
    public:
        explicit NewProjectSettingsScreen(QWidget* parent = nullptr);
        ~NewProjectSettingsScreen() = default;
        ProjectManagerScreen GetScreenEnum() override;
        QString GetNextButtonText() override;
    };

} // namespace O3DE::ProjectManager

/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once


#if !defined(Q_MOC_RUN)
#include <QDialog>
#endif

namespace O3DE::ProjectManager
{
    QT_FORWARD_DECLARE_CLASS(FormLineEditWidget)

    class ShowBuildLogScreen : public QDialog
    {
    public:
        explicit ShowBuildLogScreen(QWidget* parent = nullptr);
        ~ShowBuildLogScreen() = default;

    private:
        FormLineEditWidget* m_repoPath = nullptr;
    };
} // namespace O3DE::ProjectManager

/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <ScriptCanvas/Core/Node.h>

#include <ScriptCanvas/Internal/Nodes/ExpressionNodeBase.h>

#include <Include/ScriptCanvas/Libraries/Math/MathExpression.generated.h>

namespace ScriptCanvas
{
    namespace Nodes
    {
        namespace Internal
        {
            class ExpressionNodeBase;
        }

        namespace Math
        {
            //! Provides a node that can take in a mathematical expression and convert it into a single node.
            class MathExpression
                : public Internal::ExpressionNodeBase
            {

                SCRIPTCANVAS_NODE(MathExpression);

            protected:

                ConstSlotsOutcome GetSlotsInExecutionThreadByTypeImpl(const Slot&, CombinedSlotType targetSlotType, const Slot*) const override
                {
                    return AZ::Success(GetSlotsByType(targetSlotType));
                }

                void OnResult(const ExpressionEvaluation::ExpressionResult& result) override;
                ExpressionEvaluation::ParseOutcome ParseExpression(const AZStd::string& formatString) override;

                AZStd::string GetExpressionSeparator() const override;
            };
        }
    }
}


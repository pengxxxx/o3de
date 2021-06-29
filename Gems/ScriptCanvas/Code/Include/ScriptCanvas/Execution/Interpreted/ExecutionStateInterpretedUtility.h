/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Outcome/Outcome.h>
#include <ScriptCanvas/Core/Datum.h>

struct lua_State;

namespace ScriptCanvas
{
    namespace Grammar
    {
        struct DebugSymbolMap;
    }

    namespace Execution
    {
        void InitializeFromLuaStackFunctions(Grammar::DebugSymbolMap& debugMap);

        bool IsLuaValueType(Data::eType etype);
    } 

} 

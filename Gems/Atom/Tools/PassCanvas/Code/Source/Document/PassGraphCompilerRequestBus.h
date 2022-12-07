/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <GraphCanvas/Editor/EditorTypes.h>
#include <GraphModel/Model/DataType.h>

namespace PassCanvas
{
    class PassGraphCompilerRequests : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        typedef AZ::Uuid BusIdType;

        // Get a list of all of the generated files from the last time this graph was compiled.
        virtual const AZStd::vector<AZStd::string>& GetGeneratedFilePaths() const = 0;

        // Evaluate the graph nodes, slots, values, and settings to generate and export shaders, pass types, and passes.
        virtual bool CompileGraph() = 0;

        // Schedule the graph to be compiled on the next system tick.
        virtual void QueueCompileGraph() = 0;

        // Returns true if graph compilation has already been scheduled.
        virtual bool IsCompileGraphQueued() const = 0;
    };

    using PassGraphCompilerRequestBus = AZ::EBus<PassGraphCompilerRequests>;
} // namespace PassCanvas

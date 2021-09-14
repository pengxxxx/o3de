/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

/** @file
 * Header file for buses that dispatch tick notification events  
 * and receive tick-related requests.
 * A tick is a unit of time generated by the application. 
 */

#ifndef AZCORE_COMPONENT_TICK_BUS_H
#define AZCORE_COMPONENT_TICK_BUS_H

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Debug/AssetTracking.h>
#include <AzCore/std/chrono/chrono.h>
#include <AzCore/std/parallel/mutex.h> // For TickBus thread events.
#include <AzCore/Script/ScriptTimePoint.h>

namespace AZ
{
    /**
     * Values to help you set when a particular handler is notified of ticks.
     */
    enum ComponentTickBus
    {
        TICK_FIRST          = 0,       ///< First position in the tick handler order.

        TICK_PLACEMENT      = 50,      ///< Suggested tick handler position for components that need to be early in the tick order.

        TICK_INPUT          = 75,      ///< Suggested tick handler position for input components.

        TICK_GAME           = 80,      ///< Suggested tick handler for game-related components.

        TICK_ANIMATION      = 100,     ///< Suggested tick handler position for animation components.

        TICK_PHYSICS_SYSTEM = 200,     ///< Suggested tick handler position for physics systems. Note: This should only be used for the Physics System.

        TICK_PHYSICS        = TICK_PHYSICS_SYSTEM + 1,  ///< Suggested tick handler position for physics components

        TICK_ATTACHMENT     = 500,     ///< Suggested tick handler position for attachment components.

        TICK_PRE_RENDER     = 750,     ///< Suggested tick handler position to update render-related data.

        TICK_RENDER         = 800,     ///< Suggested tick handler position for rendering.

        TICK_DEFAULT        = 1000,    ///< Default tick handler position when the handler is constructed.

        TICK_UI             = 2000,    ///< Suggested tick handler position for UI components.

        TICK_LAST           = 100000,  ///< Last position in the tick handler order.
    };

        
    /**
     * Interface for AZ::TickBus, which is the EBus that dispatches tick events.
     * These tick events are executed on the main game thread. In games, AZ::TickBus
     * dispatches ticks even if the application is not in focus. In tools, AZ::TickBus 
     * can become inactive when the tool loses focus.
     * @note Do not add a mutex to TickEvents. It is unnecessary and typically degrades performance.
     */
    class TickEvents
        : public AZ::EBusTraits
    {
    public:
        AZ_RTTI(TickEvents, "{DF79B555-D9E9-489A-8A00-AD39C564E258}");

        /**
         * Creates an instance of the class and sets the tick order of the 
         * handler to the default value.
         */
        TickEvents()
            : m_tickOrder(TICK_DEFAULT) {}

        /**
         * Destroys the instance of the class.
         */
        virtual ~TickEvents() {}

        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides - application is a singleton
        
        /**
         * Overrides the default AZ::EBusTraits handler policy so that multiple 
         * handlers can connect to the bus. This bus has one address because it  
         * uses the default EBusTraits address policy. At the address, handlers 
         * receive events based on the order in which the components are initialized, 
         * unless a handler explicitly sets its TickEvents::m_tickOrder.
         */
        static const AZ::EBusHandlerPolicy HandlerPolicy = EBusHandlerPolicy::MultipleAndOrdered;  
             
        /**
         * Enables the event queue, which you can use to execute actions just before the OnTick event.
         */
        static const bool EnableEventQueue = true;        
        
        /**
         * Specifies the mutex that is used when adding and removing events from the event queue.
         * This mutex is for the event queue, not TickEvents. Do not add a mutex to TickEvents.
         */
        typedef AZStd::recursive_mutex EventQueueMutexType; 
        
        /**
         * Determines the order in which handlers receive tick events. 
         * Handlers receive events based on the order in which the components are initialized, 
         * unless a handler explicitly sets its position.
         */
        struct BusHandlerOrderCompare
        {
            AZ_FORCE_INLINE bool operator()(TickEvents* left, TickEvents* right) const { return left->GetTickOrder() < right->GetTickOrder(); }
        };

        /**
        * Enable tick bus to work with the AssetTracking
        */
        using EventProcessingPolicy = Debug::AssetTrackingEventProcessingPolicy<>;
        //////////////////////////////////////////////////////////////////////////

        /**
         * Signals that the application has issued a tick.
         * @param deltaTime The delta (in seconds) from the previous tick and the current time.
         * @param time The current time.
         */
        virtual void    OnTick(float deltaTime, ScriptTimePoint time) = 0;

        /**
         * Specifies The order in which a handler receives tick events relative to other handlers.
         * This value should not be changed while the handler is connected.
         * See the ComponentTickBus enum for recommended values.
         * @return a value specifying this handler's relative order.
         */
        virtual int     GetTickOrder()
        {
            // m_tickOrder is deprecated, respect it for the time being but warn if it's used.
            AZ_Warning("TickBus", m_tickOrder == TICK_DEFAULT, "TickBus::Handler::m_tickOrder has been deprecated, implement GetTickOrder() instead.");
            return m_tickOrder;
        }

    protected:
        // Only the component application is allowed to issue ticks.
        friend class ComponentApplication;

        /**
         * Deprecated.
         * @deprecated Override GetTickOrder() to specify the order in which the handler receives tick events.
         */
        int     m_tickOrder;
    };

    /**
     * The EBus for tick notification events.
     * The events are defined in the AZ::TickEvents class.
     */
    typedef AZ::EBus<TickEvents>    TickBus;
    
    /**
     * Interface for AZ::TickRequestBus, which components use to make tick-related 
     * requests.
     * Available requests are to get the time between ticks or the current time in seconds.
     */
    class TickRequests
        : public AZ::EBusTraits
    {
    public:

        /**
         * Gets the latest time between ticks.
         */
        virtual float GetTickDeltaTime() = 0;

        /**
         * Gets the time in seconds since the epoch.
         */
        virtual ScriptTimePoint GetTimeAtCurrentTick() = 0;
    };

    /**
     * The EBus for tick-related requests.
     * The events are defined in the AZ::TickRequests class.
     */
    typedef AZ::EBus<TickRequests> TickRequestBus;


    /**
     * Interface for AZ::SystemTickBus, which is the EBus that dispatches system tick events.
     * System tick events are dispatched at some interval of a small number of milliseconds,
     * even when the host application does not have focus. It can be be used for anything that needs to be serviced
     * regularly, such as network or asset processor polling.
     * Note that it does not necessarily occur at a consistent interval. In some tools, such as the Editor, 
     * OnSystemTick() can be called more often than the regular interval.
     * If timing matters, use TickEvents::OnTick() instead.
     * @note Do not add a mutex to SystemTickEvents. It is unnecessary and typically degrades performance.
     */
    class SystemTickEvents : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides - application is a singleton

        /**
         * Overrides the default AZ::EBusTraits handler policy so that multiple
         * handlers can connect to the bus. This bus has one address because it
         * uses the default EBusTraits address policy. 
         */
        static const AZ::EBusHandlerPolicy HandlerPolicy = EBusHandlerPolicy::Multiple; 

        /**
         * Enables the event queue, which you can use to execute actions just before the OnSystemTick event.
         */
        static const bool EnableEventQueue = true; 

        /**
         * Specifies the mutex that is used when adding and removing events from the event queue.
         * This mutex is for the event queue, not SystemTickEvents. Do not add a mutex to SystemTickEvents.
         */
        typedef AZStd::mutex EventQueueMutexType; 

        /**
        * Enable tick bus to work with the AssetTracking
        */
        using EventProcessingPolicy = Debug::AssetTrackingEventProcessingPolicy<>;
        //////////////////////////////////////////////////////////////////////////

        /**
         * Signals that the application has issued a system tick.
         */
        virtual void OnSystemTick() = 0;

    };

    /**
     * The EBus for system tick notification events.
     * The events are defined in the AZ::SystemTickEvents class.
     */
    using SystemTickBus = AZ::EBus<SystemTickEvents>;
}

#endif // AZCORE_COMPONENT_TICK_BUS_H
#pragma once

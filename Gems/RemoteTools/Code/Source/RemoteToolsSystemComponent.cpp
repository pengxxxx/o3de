
#include <RemoteToolsSystemComponent.h>

#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/ObjectStream.h>

#include <AzNetworking/Framework/INetworking.h>
#include <AzNetworking/Utilities/CidrAddress.h>

#include <Source/AutoGen/RemoteTools.AutoPackets.h>
#include <Source/AutoGen/RemoteTools.AutoPacketDispatcher.h>

namespace RemoteTools
{
    static const AzNetworking::CidrAddress ToolingCidrFilter = AzNetworking::CidrAddress();

    void RemoteToolsSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<RemoteToolsSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<RemoteToolsSystemComponent>("RemoteTools", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void RemoteToolsSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("RemoteToolsService"));
    }

    void RemoteToolsSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("RemoteToolsService"));
    }

    void RemoteToolsSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("NetworkingService"));
    }

    void RemoteToolsSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        ;
    }

    RemoteToolsSystemComponent::RemoteToolsSystemComponent()
    {
        if (AzFramework::RemoteToolsInterface::Get() == nullptr)
        {
            AzFramework::RemoteToolsInterface::Register(this);
        }
    }

    RemoteToolsSystemComponent::~RemoteToolsSystemComponent()
    {
        if (AzFramework::RemoteToolsInterface::Get() == this)
        {
            AzFramework::RemoteToolsInterface::Unregister(this);
        }
    }

    void RemoteToolsSystemComponent::Init()
    {
    }

    void RemoteToolsSystemComponent::Activate()
    {
        AZ::SystemTickBus::Handler::BusConnect();
    }

    void RemoteToolsSystemComponent::Deactivate()
    {
        AZ::SystemTickBus::Handler::BusDisconnect();
    }

    void RemoteToolsSystemComponent::OnSystemTick()
    {
#if !defined(AZ_RELEASE_BUILD)
        // If we're not the host and not connected to one, attempt to connect on a fixed interval

#endif
    }

    void RemoteToolsSystemComponent::RegisterToolingService(AZ::Crc32 key, AZ::Name name, uint16_t port)
    {
        m_entryRegistry[key] = RemoteToolsRegistryEntry();
        m_entryRegistry[key].m_name = name;
        m_entryRegistry[key].m_port = port;
    }

    const AzFramework::ReceivedRemoteToolsMessages* RemoteToolsSystemComponent::GetReceivedMessages(AZ::Crc32 key) const
    {
        if (m_inbox.contains(key))
        {
            return &m_inbox.at(key);
        }
        return nullptr;
    }

    void RemoteToolsSystemComponent::ClearReceivedMessages(AZ::Crc32 key)
    {
        if (m_inbox.contains(key))
        {
            m_inbox.at(key).clear();
        }
    }

    void RemoteToolsSystemComponent::RegisterRemoteToolsEndpointJoinedHandler(
        AZ::Crc32 key, AzFramework::RemoteToolsEndpointStatusEvent::Handler handler)
    {
        handler.Connect(m_entryRegistry[key].m_endpointJoinedEvent);
    }

    void RemoteToolsSystemComponent::RegisterRemoteToolsEndpointLeftHandler(
        AZ::Crc32 key, AzFramework::RemoteToolsEndpointStatusEvent::Handler handler)
    {
        handler.Connect(m_entryRegistry[key].m_endpointLeftEvent);
    }

    void RemoteToolsSystemComponent::RegisterRemoteToolsEndpointConnectedHandler(
        AZ::Crc32 key, AzFramework::RemoteToolsEndpointConnectedEvent::Handler handler)
    {
        handler.Connect(m_entryRegistry[key].m_endpointConnectedEvent);
    }

    void RemoteToolsSystemComponent::RegisterRemoteToolsEndpointChangedHandler(
        AZ::Crc32 key, AzFramework::RemoteToolsEndpointChangedEvent::Handler handler)
    {
        handler.Connect(m_entryRegistry[key].m_endpointChangedEvent);
    }

    void RemoteToolsSystemComponent::EnumTargetInfos(AZ::Crc32 key, AzFramework::RemoteToolsEndpointContainer& infos)
    {
        if (m_entryRegistry.contains(key))
        {
            infos = m_entryRegistry[key].m_availableTargets;
        }
        else
        {
            infos = AzFramework::RemoteToolsEndpointContainer();
        }
    }

    void RemoteToolsSystemComponent::SetDesiredEndpoint(AZ::Crc32 key, AZ::u32 desiredTargetID)
    {
        AZ_TracePrintf("RemoteToolsSystemComponent", "Set Target - %u", desiredTargetID);
        if (m_entryRegistry.contains(key))
        {
            RemoteToolsRegistryEntry& entry = m_entryRegistry[key];
            if (desiredTargetID != entry.m_lastTarget.GetPersistentId())
            {
                AzFramework::RemoteToolsEndpointInfo ti = GetEndpointInfo(key, desiredTargetID);
                AZ::u32 oldTargetID = entry.m_lastTarget.GetPersistentId();
                entry.m_lastTarget = ti;
                entry.m_tmpInboundBuffer.clear();
                entry.m_tmpInboundBufferPos = 0;

                 m_entryRegistry[key].m_endpointChangedEvent.Signal(desiredTargetID, oldTargetID);


                if (ti.IsValid() && ti.IsOnline())
                {
                    m_entryRegistry[key].m_endpointConnectedEvent.Signal(true);
                }
                else
                {
                    m_entryRegistry[key].m_endpointConnectedEvent.Signal(false);
                }
            }
        }
    }

    void RemoteToolsSystemComponent::SetDesiredEndpointInfo(AZ::Crc32 key, const AzFramework::RemoteToolsEndpointInfo& targetInfo)
    {
        SetDesiredEndpoint(key, targetInfo.GetPersistentId());
    }

    AzFramework::RemoteToolsEndpointInfo RemoteToolsSystemComponent::GetDesiredEndpoint(AZ::Crc32 key)
    {
        if (m_entryRegistry.contains(key))
        {
            return m_entryRegistry[key].m_lastTarget;
        }

        return AzFramework::RemoteToolsEndpointInfo(); // return an invalid target info.
    }

    AzFramework::RemoteToolsEndpointInfo RemoteToolsSystemComponent::GetEndpointInfo(AZ::Crc32 key, AZ::u32 desiredTargetID)
    {
        if (m_entryRegistry.contains(key))
        {
            AzFramework::RemoteToolsEndpointContainer container = m_entryRegistry[key].m_availableTargets;
            AzFramework::RemoteToolsEndpointContainer::const_iterator finder = container.find(desiredTargetID);
            if (finder != container.end())
            {
                return finder->second;
            }
        }

        return AzFramework::RemoteToolsEndpointInfo(); // return an invalid target info.
    }

    bool RemoteToolsSystemComponent::IsEndpointOnline(AZ::Crc32 key, AZ::u32 desiredTargetID)
    {
        if (m_entryRegistry.contains(key))
        {
            AzFramework::RemoteToolsEndpointContainer container = m_entryRegistry[key].m_availableTargets;
            AzFramework::RemoteToolsEndpointContainer::const_iterator finder = container.find(desiredTargetID);
            if (finder != container.end())
            {
                return finder->second.IsOnline();
            }
        }

        return false;
    }

    void RemoteToolsSystemComponent::SendRemoteToolsMessage(
        const AzFramework::RemoteToolsEndpointInfo& target, const AzFramework::RemoteToolsMessage& msg)
    {
        AZ::SerializeContext* serializeContext;
        EBUS_EVENT_RESULT(serializeContext, AZ::ComponentApplicationBus, GetSerializeContext);

        // Messages targeted at our own application just transfer right over to the inbox.
        if (target.IsSelf())
        {
            AzFramework::RemoteToolsMessage* inboxMessage = static_cast<AzFramework::RemoteToolsMessage*>(
                serializeContext->CloneObject(static_cast<const void*>(&msg), msg.RTTI_GetType()));
            AZ_Assert(inboxMessage, "Failed to clone local loopback message.");
            inboxMessage->SetSenderTargetId(target.GetPersistentId());

            if (msg.GetCustomBlobSize() > 0)
            {
                void* blob = azmalloc(msg.GetCustomBlobSize(), 16, AZ::OSAllocator);
                memcpy(blob, msg.GetCustomBlob(), msg.GetCustomBlobSize());
                inboxMessage->AddCustomBlob(blob, msg.GetCustomBlobSize(), true);
            }

            m_inboxMutex.lock();
            m_inbox[msg.GetSenderTargetId()].push_back(inboxMessage);
            m_inboxMutex.unlock();

            return;
        }

        AZStd::vector<char, AZ::OSStdAllocator> msgBuffer;
        AZ::IO::ByteContainerStream<AZStd::vector<char, AZ::OSStdAllocator>> outMsg(&msgBuffer);

        AZ::ObjectStream* objStream = AZ::ObjectStream::Create(&outMsg, *serializeContext, AZ::ObjectStream::ST_BINARY);
        objStream->WriteClass(&msg);
        if (!objStream->Finalize())
        {
            AZ_Assert(false, "ObjectStream failed to serialize outbound TmMsg!");
        }

        size_t customBlobSize = msg.GetCustomBlobSize();
        if (msg.GetCustomBlobSize() > 0)
        {
            outMsg.Write(customBlobSize, msg.GetCustomBlob());
        }

        AzNetworking::INetworkInterface* networkInterface =
            AZ::Interface<AzNetworking::INetworking>::Get()->RetrieveNetworkInterface(
                AZ::Name(AZStd::string::format("%d", target.GetPersistentId())));

        OutboundToolingDatum datum;
        datum.first = target.GetPersistentId();
        datum.second.swap(msgBuffer);
        m_outboxThread->PushOutboxMessage(
            networkInterface, static_cast<AzNetworking::ConnectionId>(target.GetNetworkId()), AZStd::move(datum));
    }

    void RemoteToolsSystemComponent::OnMessageParsed(
        AzFramework::RemoteToolsMessage** ppMsg, void* classPtr, const AZ::Uuid& classId, const AZ::SerializeContext* sc)
    {
        // Check that classPtr is a RemoteToolsMessage
        AZ_Assert(*ppMsg == nullptr, "ppMsg is already set! are we deserializing multiple messages in one call?");
        *ppMsg = sc->Cast<AzFramework::RemoteToolsMessage*>(classPtr, classId);
        AZ_Assert(*ppMsg, "Failed to downcast msg pointer to a TmMsg. Is RTTI and reflection set up properly?");
    }

    bool RemoteToolsSystemComponent::HandleRequest(
        AzNetworking::IConnection* connection,
        [[maybe_unused]] const AzNetworking::IPacketHeader& packetHeader,
        const RemoteToolsPackets::RemoteToolsConnect& packet)
    {
        AzNetworking::ByteOrder byteOrder = connection->GetConnectionRole() == AzNetworking::ConnectionRole::Acceptor
            ? AzNetworking::ByteOrder::Host
            : AzNetworking::ByteOrder::Network;
        AZ::Crc32 key = connection->GetRemoteAddress().GetPort(byteOrder);

        if (m_entryRegistry.contains(key))
        {
            const uint32_t persistentId = packet.GetPersistentId();
            AzFramework::RemoteToolsEndpointContainer::pair_iter_bool ret =
                m_entryRegistry[key].m_availableTargets.insert_key(persistentId);
            AzFramework::RemoteToolsEndpointInfo& ti = ret.first->second;
            ti.SetInfo(packet.GetDisplayName(), persistentId, static_cast<uint32_t>(connection->GetConnectionId()));
            m_entryRegistry[key].m_endpointJoinedEvent.Signal(ti);
        }
        return true;
    }

    bool RemoteToolsSystemComponent::HandleRequest(
        AzNetworking::IConnection* connection,
        [[maybe_unused]] const AzNetworking::IPacketHeader& packetHeader,
        [[maybe_unused]] const RemoteToolsPackets::RemoteToolsPacket& packet)
    {
        AZ::Crc32 key = packet.GetPersistentId();

        // Receive
        if (connection->GetConnectionRole() == AzNetworking::ConnectionRole::Acceptor
            && static_cast<uint32_t>(connection->GetConnectionId()) != m_entryRegistry[key].m_lastTarget.GetNetworkId())
        {
            // Listener should route traffic based on selected target
            return true;
        }
      
        if (!m_entryRegistry.contains(key))
        {
            m_entryRegistry[key] = RemoteToolsRegistryEntry();
        }

        // If we're a client, set the host to our desired target
        if (connection->GetConnectionRole() == AzNetworking::ConnectionRole::Connector)
        {
            if (GetEndpointInfo(key, packet.GetPersistentId()).GetPersistentId() == 0)
            {
                AzFramework::RemoteToolsEndpointContainer::pair_iter_bool ret =
                    m_entryRegistry[key].m_availableTargets.insert_key(packet.GetPersistentId());

                AzFramework::RemoteToolsEndpointInfo& ti = ret.first->second;
                ti.SetInfo("Host", packet.GetPersistentId(), static_cast<uint32_t>(connection->GetConnectionId()));
                m_entryRegistry[key].m_endpointJoinedEvent.Signal(ti);
            }

            if (GetDesiredEndpoint(key).GetPersistentId() != packet.GetPersistentId())
            {
                SetDesiredEndpoint(key, packet.GetPersistentId());
            }
        }

        const uint32_t totalBufferSize = packet.GetSize();

        // Messages can be larger than the size of a packet so reserve a buffer for the total message size
        if (m_entryRegistry[key].m_tmpInboundBufferPos == 0)
        {
            m_entryRegistry[key].m_tmpInboundBuffer.reserve(totalBufferSize);
        }

        // Read as much data as the packet can include and append it to the buffer
        const uint32_t readSize = AZStd::min(totalBufferSize - m_entryRegistry[key].m_tmpInboundBufferPos, RemoteToolsBufferSize);
        memcpy(
            m_entryRegistry[key].m_tmpInboundBuffer.begin() + m_entryRegistry[key].m_tmpInboundBufferPos,
            packet.GetMessageBuffer().GetBuffer(), readSize);
        m_entryRegistry[key].m_tmpInboundBufferPos += readSize;
        if (m_entryRegistry[key].m_tmpInboundBufferPos == totalBufferSize)
        {
            AZ::SerializeContext* serializeContext;
            EBUS_EVENT_RESULT(serializeContext, AZ::ComponentApplicationBus, GetSerializeContext);

            // Deserialize the complete buffer
            AZ::IO::MemoryStream msgBuffer(m_entryRegistry[key].m_tmpInboundBuffer.data(), totalBufferSize, totalBufferSize);
            AzFramework::RemoteToolsMessage* msg = nullptr;
            AZ::ObjectStream::ClassReadyCB readyCB(AZStd::bind(
                &RemoteToolsSystemComponent::OnMessageParsed, this, &msg, AZStd::placeholders::_1, AZStd::placeholders::_2,
                AZStd::placeholders::_3));
            AZ::ObjectStream::LoadBlocking(
                &msgBuffer, *serializeContext, readyCB,
                AZ::ObjectStream::FilterDescriptor(nullptr, AZ::ObjectStream::FILTERFLAG_IGNORE_UNKNOWN_CLASSES));

            // Append to the inbox for handling
            if (msg)
            {
                if (msg->GetCustomBlobSize() > 0)
                {
                    void* blob = azmalloc(msg->GetCustomBlobSize(), 1, AZ::OSAllocator, "TmMsgBlob");
                    msgBuffer.Read(msg->GetCustomBlobSize(), blob);
                    msg->AddCustomBlob(blob, msg->GetCustomBlobSize(), true);
                }
                msg->SetSenderTargetId(packet.GetPersistentId());

                m_inboxMutex.lock();
                m_inbox[msg->GetSenderTargetId()].push_back(msg);
                m_inboxMutex.unlock();
                m_entryRegistry[key].m_tmpInboundBuffer.clear();
                m_entryRegistry[key].m_tmpInboundBufferPos = 0;
            }
        }

        return true;
    }

    AzNetworking::ConnectResult RemoteToolsSystemComponent::ValidateConnect(
        [[maybe_unused]] const AzNetworking::IpAddress& remoteAddress,
        [[maybe_unused]] const AzNetworking::IPacketHeader& packetHeader,
        [[maybe_unused]] AzNetworking::ISerializer& serializer)
    {
        return AzNetworking::ConnectResult::Accepted;
    }

    void RemoteToolsSystemComponent::OnConnect([[maybe_unused]] AzNetworking::IConnection* connection)
    {
        // Invoked when a tooling connection is established, handshake logic is handled via ToolingConnect message
        ;
    }

    AzNetworking::PacketDispatchResult RemoteToolsSystemComponent::OnPacketReceived(
        AzNetworking::IConnection* connection,
        [[maybe_unused]] const AzNetworking::IPacketHeader& packetHeader,
        [[maybe_unused]] AzNetworking::ISerializer& serializer)
    {
        if (!ToolingCidrFilter.IsMatch(connection->GetRemoteAddress()))
        {
            // Only IPs within the Cidr filter is valid
            return AzNetworking::PacketDispatchResult::Skipped;
        }

        return RemoteToolsPackets::DispatchPacket(connection, packetHeader, serializer, *this);
    }

    void RemoteToolsSystemComponent::OnPacketLost(
        [[maybe_unused]] AzNetworking::IConnection* connection, [[maybe_unused]] AzNetworking::PacketId packetId)
    {
        ;
    }

    void RemoteToolsSystemComponent::OnDisconnect(
        [[maybe_unused]] AzNetworking::IConnection* connection,
        [[maybe_unused]] AzNetworking::DisconnectReason reason,
        [[maybe_unused]] AzNetworking::TerminationEndpoint endpoint)
    {
        // If our desired target has left the network, flag it and notify listeners
        if (reason != AzNetworking::DisconnectReason::ConnectionRejected)
        {
            for (auto registryIt = m_entryRegistry.begin(); registryIt != m_entryRegistry.end(); ++registryIt)
            {
                AzFramework::RemoteToolsEndpointContainer& container = registryIt->second.m_availableTargets;
                for (auto endpointIt = container.begin(); endpointIt != container.end(); ++endpointIt)
                {
                    if (endpointIt->second.GetNetworkId() == static_cast<AZ::u32>(connection->GetConnectionId()))
                    {
                        AzFramework::RemoteToolsEndpointInfo ti = endpointIt->second;
                        container.erase(endpointIt);
                        registryIt->second.m_endpointLeftEvent.Signal(ti);
                    }
                }
            }
        }
    }

} // namespace RemoteTools

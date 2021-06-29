#
# Copyright (c) Contributors to the Open 3D Engine Project
# 
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    BuildInfo.h
    EBus.h
    Docs.h
    GridMate.cpp
    GridMate.h
    GridMateEventsBus.h
    GridMateService.h
    MathUtils.h
    Memory.h
    Types.h
    Version.h
    Carrier/Carrier.cpp
    Carrier/Carrier.h
    Carrier/Compressor.h
    Carrier/Cripter.h
    Carrier/DefaultHandshake.cpp
    Carrier/DefaultHandshake.h
    Carrier/DefaultSimulator.cpp
    Carrier/DefaultSimulator.h
    Carrier/DefaultTrafficControl.cpp
    Carrier/DefaultTrafficControl.h
    Carrier/Driver.h
    Carrier/DriverEvents.h
    Carrier/Handshake.h
    Carrier/Simulator.h
    Carrier/SocketDriver.cpp
    Carrier/SocketDriver.h
    Carrier/StreamSocketDriver.cpp
    Carrier/StreamSocketDriver.h
    Carrier/TrafficControl.h
    Carrier/Utils.h
    Containers/list.h
    Containers/queue.h
    Containers/set.h
    Containers/slist.h
    Containers/unordered_map.h
    Containers/unordered_set.h
    Containers/vector.h
    Drillers/CarrierDriller.cpp
    Drillers/CarrierDriller.h
    Drillers/ReplicaDriller.cpp
    Drillers/ReplicaDriller.h
    Drillers/SessionDriller.cpp
    Drillers/SessionDriller.h
    Online/OnlineUtilityThread.h
    Online/UserServiceTypes.h
    Replica/BasicHostChunkDescriptor.h
    Replica/DeltaCompressedDataSet.h
    Replica/DataSet.cpp
    Replica/DataSet.h
    Replica/Interpolators.h
    Replica/MigrationSequence.cpp
    Replica/MigrationSequence.h
    Replica/RemoteProcedureCall.cpp
    Replica/RemoteProcedureCall.h
    Replica/Replica.cpp
    Replica/Replica.h
    Replica/ReplicaChunk.cpp
    Replica/ReplicaChunk.h
    Replica/ReplicaChunkDescriptor.cpp
    Replica/ReplicaChunkDescriptor.h
    Replica/ReplicaChunkInterface.h
    Replica/ReplicaCommon.h
    Replica/ReplicaDefs.h
    Replica/ReplicaDrillerEvents.h
    Replica/ReplicaFunctions.h
    Replica/ReplicaFunctions.inl
    Replica/ReplicaInline.inl
    Replica/ReplicaMgr.cpp
    Replica/ReplicaMgr.h
    Replica/ReplicaStatus.cpp
    Replica/ReplicaStatus.h
    Replica/ReplicaStatusInterface.h
    Replica/ReplicaTarget.cpp
    Replica/ReplicaTarget.h
    Replica/ReplicaUtils.h
    Replica/SystemReplicas.cpp
    Replica/SystemReplicas.h
    Replica/Throttles.h
    Replica/Tasks/ReplicaMarshalTasks.cpp
    Replica/Tasks/ReplicaMarshalTasks.h
    Replica/Tasks/ReplicaTask.h
    Replica/Tasks/ReplicaTaskManager.h
    Replica/Tasks/ReplicaUpdateTasks.cpp
    Replica/Tasks/ReplicaUpdateTasks.h
    Replica/Tasks/ReplicaProcessPolicy.cpp
    Replica/Tasks/ReplicaProcessPolicy.h
    Replica/Tasks/ReplicaPriorityPolicy.h
    Replica/Interest/BitmaskInterestHandler.cpp
    Replica/Interest/BitmaskInterestHandler.h
    Replica/Interest/InterestDefs.h
    Replica/Interest/InterestManager.cpp
    Replica/Interest/InterestManager.h
    Replica/Interest/InterestQueryResult.h
    Replica/Interest/RulesHandler.h
    Serialize/Buffer.cpp
    Serialize/Buffer.h
    Serialize/PackedSize.h
    Serialize/CompressionMarshal.cpp
    Serialize/CompressionMarshal.h
    Serialize/ContainerMarshal.h
    Serialize/DataMarshal.h
    Serialize/MarshalerTypes.h
    Serialize/MathMarshal.h
    Serialize/UtilityMarshal.h
    Serialize/UuidMarshal.h
    Session/LANSession.cpp
    Session/LANSession.h
    Session/LANSessionServiceTypes.h
    Session/LANSessionServiceBus.h
    Session/Session.cpp
    Session/Session.h
    Session/SessionServiceBus.h
    String/string.h
    VoiceChat/VoiceChatServiceBus.h
)

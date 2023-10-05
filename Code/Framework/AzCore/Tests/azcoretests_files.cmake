#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Main.cpp
    Asset/AssetCommon.cpp
    Asset/AssetDataStreamTests.cpp
    Asset/AssetManagerLoadingTests.cpp
    Asset/AssetManagerStreamingTests.cpp
    Asset/BaseAssetManagerTest.cpp
    Asset/BaseAssetManagerTest.h
    Asset/MockLoadAssetCatalogAndHandler.h
    Asset/TestAssetTypes.h
    AssetJsonSerializerTests.cpp
    AZStd/Algorithms.cpp
    AZStd/Allocators.cpp
    AZStd/Atomics.cpp
    AZStd/Any.cpp
    AZStd/Bitset.cpp
    AZStd/Charconv.cpp
    AZStd/ConceptsTests.cpp
    AZStd/CreateDestroy.cpp
    AZStd/ConcurrentAllocators.cpp
    AZStd/ConcurrentContainers.cpp
    AZStd/ChronoTests.cpp
    AZStd/DequeAndSimilar.cpp
    AZStd/Examples.cpp
    AZStd/ExpectedTests.cpp
    AZStd/FunctionalBasic.cpp
    AZStd/FunctorsBind.cpp
    AZStd/Hashed.cpp
    AZStd/Invoke.cpp
    AZStd/Iterators.cpp
    AZStd/Lists.cpp
    AZStd/ListsFixed.cpp
    AZStd/ListsIntrusive.cpp
    AZStd/LockFreeQueues.cpp
    AZStd/LockFreeStacks.cpp
    AZStd/LockTests.cpp
    AZStd/Math.cpp
    AZStd/Numeric.cpp
    AZStd/Ordered.cpp
    AZStd/Optional.cpp
    AZStd/Pair.cpp
    AZStd/Parallel.cpp
    AZStd/RangesAlgorithmBenchmarks.cpp
    AZStd/RangesAlgorithmTests.cpp
    AZStd/RangesTests.cpp
    AZStd/RangesUtilityTests.cpp
    AZStd/RangesViewTests.cpp
    AZStd/ScopedLockTests.cpp
    AZStd/SetsIntrusive.cpp
    AZStd/SmartPtr.cpp
    AZStd/SpanTests.cpp
    AZStd/String.cpp
    AZStd/TypeTraits.cpp
    AZStd/Tuple.cpp
    AZStd/UserTypes.h
    AZStd/Variant.cpp
    AZStd/VariantSerialization.cpp
    AZStd/VectorAndArray.cpp
    AssetManager.cpp
    AttributeDomInteropTests.cpp
    TestCatalog.h
    TestCatalog.cpp
    AssetSerializerTests.cpp
    BehaviorContext.cpp
    BehaviorContextFixture.h
    Components.cpp
    Console/LoggerSystemComponentTests.cpp
    Console/ConsoleTests.cpp
    Date/DateFormatTests.cpp
    Debug/PerformanceCollectorTests.cpp
    Debug/Trace.cpp
    Debug.cpp
    DLL.cpp
    DOM/DomFixtures.cpp
    DOM/DomFixtures.h
    DOM/DomJsonTests.cpp
    DOM/DomJsonBenchmarks.cpp
    DOM/DomPathTests.cpp
    DOM/DomPathBenchmarks.cpp
    DOM/DomPatchTests.cpp
    DOM/DomPatchBenchmarks.cpp
    DOM/DomValueTests.cpp
    DOM/DomValueBenchmarks.cpp
    DOM/DomPrefixTreeTests.cpp
    DOM/DomPrefixTreeBenchmarks.cpp
    EBus/EBusSharedDispatchMutexTests.cpp
    EBus/ScheduledEventTests.cpp
    EBus.cpp
    EntityIdTests.cpp
    EntityTests.cpp
    EnumTests.cpp
    EventTests.cpp
    FileIOBaseTestTypes.h
    Geometry2DUtils.cpp
    Interface.cpp
    IO/FileReaderTests.cpp
    IO/Path/PathReflectTests.cpp
    IO/Path/PathTests.cpp
    IPC.cpp
    Jobs.cpp
    JSON.cpp
    FixedWidthIntegers.cpp
    GenericStreamMock.h
    GenericStreamTests.cpp
    Math/AabbTests.cpp
    Math/CapsuleTests.cpp
    Math/ColorTests.cpp
    Math/CrcTests.cpp
    Math/CrcTestsCompileTimeLiterals.h
    Math/FrustumTests.cpp
    Math/FrustumPerformanceTests.cpp
    Math/Geometry3DUtilsTests.cpp
    Math/HemisphereTests.cpp
    Math/IntersectionPerformanceTests.cpp
    Math/IntersectionTestHelpers.cpp
    Math/IntersectionTestHelpers.h
    Math/IntersectionTests.cpp
    Math/MathIntrinsicsTests.cpp
    Math/IntersectPointTest.cpp
    Math/MathStringsTests.cpp
    Math/MathUtilsTests.cpp
    Math/Matrix3x3PerformanceTests.cpp
    Math/Matrix3x3Tests.cpp
    Math/Matrix3x4PerformanceTests.cpp
    Math/Matrix3x4Tests.cpp
    Math/Matrix4x4PerformanceTests.cpp
    Math/Matrix4x4Tests.cpp
    Math/MatrixMxNTests.cpp
    Math/MatrixMxNPerformanceTests.cpp
    Math/MatrixUtilsTests.cpp
    Math/MathTest.h
    Math/MathTestData.h
    Math/ObbPerformanceTests.cpp
    Math/ObbTests.cpp
    Math/PlanePerformanceTests.cpp
    Math/PlaneTests.cpp
    Math/QuaternionPerformanceTests.cpp
    Math/QuaternionTests.cpp
    Math/RandomTests.cpp
    Math/ShapeIntersectionPerformanceTests.cpp
    Math/ShapeIntersectionTests.cpp
    Math/SfmtTests.cpp
    Math/SimdMathTests.cpp
    Math/SphereTests.cpp
    Math/RayTests.cpp
    Math/LineSegmentTests.cpp
    Math/SplineTests.cpp
    Math/TransformPerformanceTests.cpp
    Math/TransformTests.cpp
    Math/Vector2PerformanceTests.cpp
    Math/Vector2Tests.cpp
    Math/Vector3PerformanceTests.cpp
    Math/Vector3Tests.cpp
    Math/Vector4PerformanceTests.cpp
    Math/Vector4Tests.cpp
    Math/VectorNTests.cpp
    Math/VectorNPerformanceTests.cpp
    Memory/AllocatorBenchmarks.cpp
    Memory/HphaAllocator.cpp
    Memory/HphaAllocatorErrorDetection.cpp
    Memory/LeakDetection.cpp
    Memory.cpp
    Metrics/EventLoggerFactoryTests.cpp
    Metrics/EventLoggerReflectUtilsTests.cpp
    Metrics/EventLoggerUtilsTests.cpp
    Metrics/JsonTraceEventLoggerTests.cpp
    Module.cpp
    ModuleTestBus.h
    Name/NameJsonSerializerTests.cpp
    Name/NameBenchmarks.cpp
    Name/NameTests.cpp
    OrderedEventBenchmarks.cpp
    OrderedEventTests.cpp
    OutcomeTests.cpp
    Patching.cpp
    RemappableId.cpp
    RTTI/IsTypeofBenchmarks.cpp
    RTTI/TypeSafeIntegralTests.cpp
    Rtti.cpp
    Script.cpp
    ScriptMath.cpp
    Serialization/Json/ArraySerializerTests.cpp
    Serialization/Json/AnySerializerTests.cpp
    Serialization/Json/BaseJsonSerializerFixture.h
    Serialization/Json/BaseJsonSerializerTests.cpp
    Serialization/Json/BasicContainerSerializerTests.cpp
    Serialization/Json/BoolSerializerTests.cpp
    Serialization/Json/ByteStreamSerializerTests.cpp
    Serialization/Json/ColorSerializerTests.cpp
    Serialization/Json/DoubleSerializerTests.cpp
    Serialization/Json/IntSerializerTests.cpp
    Serialization/Json/JsonRegistrationContextTests.cpp
    Serialization/Json/JsonSerializationMetadataTests.cpp
    Serialization/Json/JsonSerializationResultTests.cpp
    Serialization/Json/JsonSerializationTests.h
    Serialization/Json/JsonSerializationTests.cpp
    Serialization/Json/JsonSerializationUtilsTests.cpp
    Serialization/Json/JsonSerializerConformityTests.h
    Serialization/Json/JsonSerializerMock.h
    Serialization/Json/MapSerializerTests.cpp
    Serialization/Json/MathVectorSerializerTests.cpp
    Serialization/Json/MathMatrixSerializerTests.cpp
    Serialization/Json/PathSerializerTests.cpp
    Serialization/Json/SmartPointerSerializerTests.cpp
    Serialization/Json/StringSerializerTests.cpp
    Serialization/Json/TestCases.h
    Serialization/Json/TestCases_Base.h
    Serialization/Json/TestCases_Base.cpp
    Serialization/Json/TestCases_Classes.h
    Serialization/Json/TestCases_Classes.cpp
    Serialization/Json/TestCases_Compare.cpp
    Serialization/Json/TestCases_Enum.cpp
    Serialization/Json/TestCases_Importing.cpp
    Serialization/Json/TestCases_Patching.cpp
    Serialization/Json/TestCases_Pointers.h
    Serialization/Json/TestCases_Pointers.cpp
    Serialization/Json/TestCases_TypeId.cpp
    Serialization/Json/TransformSerializerTests.cpp
    Serialization/Json/TupleSerializerTests.cpp
    Serialization/Json/UnorderedSetSerializerTests.cpp
    Serialization/Json/UnsupportedTypesSerializerTests.cpp
    Serialization/Json/UuidSerializerTests.cpp
    Serialization.cpp
    SerializeContextFixture.h
    Settings/CommandLineTests.cpp
    Settings/ConfigParserTests.cpp
    Settings/ConfigurableStackTests.cpp
    Settings/SettingsRegistryTests.cpp
    Settings/SettingsRegistryConsoleUtilsTests.cpp
    Settings/SettingsRegistryMergeUtilsTests.cpp
    Settings/SettingsRegistryOriginTrackerTests.cpp
    Settings/SettingsRegistryScriptUtilsTests.cpp
    Settings/SettingsRegistryVisitorUtilsTests.cpp
    Settings/TextParserTests.cpp
    Slice.cpp
    State.cpp
    Statistics.cpp
    StatisticalProfilerBenchmarks.cpp
    StatisticalProfilerHelpers.h
    StatisticalProfilerTests.cpp
    Streamer/BlockCacheTests.cpp
    Streamer/DedicatedCacheTests.cpp
    Streamer/FullDecompressorTests.cpp
    Streamer/IStreamerMock.h
    Streamer/IStreamerTypesMock.h
    Streamer/ReadSplitterTests.cpp
    Streamer/SchedulerTests.cpp
    Streamer/StreamStackEntryConformityTests.h
    Streamer/StreamStackEntryMock.h
    Streamer/StreamStackEntryTests.cpp
    StreamerTests.cpp
    StringFunc.cpp
    SystemFileTest.cpp
    SystemFileStreamTest.cpp
    TaskTests.cpp
    TickBusTest.cpp
    Time/TimeTests.cpp
    UUIDTests.cpp
    XML.cpp
)

# Prevent the following files from being grouped in UNITY builds
set(SKIP_UNITY_BUILD_INCLUSION_FILES
    EnumTests.cpp
    Memory/HphaAllocator.cpp
    Memory/HphaAllocatorErrorDetection.cpp
)

#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    testdata/config_broken_badplatform/AssetProcessorPlatformConfig.setreg
    testdata/config_broken_noplatform/AssetProcessorPlatformConfig.setreg
    testdata/config_broken_noscans/AssetProcessorPlatformConfig.setreg
    testdata/config_broken_recognizers/AssetProcessorPlatformConfig.setreg
    testdata/config_regular/AssetProcessorPlatformConfig.setreg
    testdata/config_metadata/AssetProcessorPlatformConfig.setreg
    testdata/config_regular_platform_scanfolder/AssetProcessorPlatformConfig.setreg
    testdata/EmptyDummyProject/AssetProcessorGamePlatformConfig.setreg
    testdata/DummyProject/AssetProcessorGamePlatformConfig.setreg
    native/tests/AssetProcessorTest.h
    native/tests/AssetProcessorTest.cpp
    native/tests/BaseAssetProcessorTest.h
    native/tests/assetdatabase/AssetDatabaseTest.cpp
    native/tests/resourcecompiler/RCControllerTest.cpp
    native/tests/resourcecompiler/RCControllerTest.h
    native/tests/resourcecompiler/RCJobTest.cpp
    native/tests/assetBuilderSDK/assetBuilderSDKTest.h
    native/tests/assetBuilderSDK/assetBuilderSDKTest.cpp
    native/tests/assetBuilderSDK/JobProductTests.cpp
    native/tests/assetBuilderSDK/SerializationDependenciesTests.cpp
    native/tests/assetmanager/AssetProcessorManagerTest.cpp
    native/tests/assetmanager/AssetProcessorManagerTest.h
    native/tests/assetmanager/JobDependencySubIdTests.cpp
    native/tests/assetmanager/JobDependencySubIdTests.h
    native/tests/assetmanager/ModtimeScanningTests.cpp
    native/tests/assetmanager/ModtimeScanningTests.h
    native/tests/assetmanager/MockAssetProcessorManager.cpp
    native/tests/assetmanager/MockAssetProcessorManager.h
    native/tests/assetmanager/MockFileProcessor.h
    native/tests/assetmanager/MockFileProcessor.cpp
    native/tests/assetmanager/TestEventSignal.cpp
    native/tests/assetmanager/TestEventSignal.h
    native/tests/assetmanager/AssetManagerTestingBase.cpp
    native/tests/assetmanager/AssetManagerTestingBase.h
    native/tests/assetmanager/IntermediateAssetTests.cpp
    native/tests/assetmanager/IntermediateAssetTests.h
    native/tests/assetmanager/Validators/LfsPointerFileValidatorTests.cpp
    native/tests/assetmanager/Validators/LfsPointerFileValidatorTests.h
    native/tests/utilities/assetUtilsTest.cpp
    native/tests/platformconfiguration/platformconfigurationtests.cpp
    native/tests/platformconfiguration/platformconfigurationtests.h
    native/tests/utilities/JobModelTest.cpp
    native/tests/utilities/JobModelTest.h
    native/tests/utilities/StatsCaptureTest.cpp
    native/tests/AssetCatalog/AssetCatalogUnitTests.cpp
    native/tests/assetscanner/AssetScannerTests.h
    native/tests/assetscanner/AssetScannerTests.cpp
    native/tests/BuilderConfiguration/BuilderConfigurationTests.cpp
    native/tests/FileProcessor/FileProcessorTests.h
    native/tests/FileProcessor/FileProcessorTests.cpp
    native/tests/FileStateCache/FileStateCacheTests.h
    native/tests/FileStateCache/FileStateCacheTests.cpp
    native/tests/InternalBuilders/SettingsRegistryBuilderTests.cpp
    native/tests/MissingDependencyScannerTests.cpp
    native/tests/SourceFileRelocatorTests.cpp
    native/tests/PathDependencyManagerTests.cpp
    native/tests/AssetProcessorMessagesTests.cpp
    native/tests/ApplicationManagerTests.cpp
    native/tests/ApplicationManagerTests.h
    native/tests/BuilderManagerTests.cpp
    native/tests/BuilderManagerTests.h
    native/tests/SourceAssetReferenceTests.cpp
    native/unittests/AssetCacheServerUnitTests.cpp
    native/unittests/AssetProcessingStateDataUnitTests.cpp
    native/unittests/AssetProcessingStateDataUnitTests.h
    native/unittests/AssetProcessorManagerUnitTests.cpp
    native/unittests/AssetProcessorManagerUnitTests.h
    native/unittests/AssetProcessorServerUnitTests.cpp
    native/unittests/AssetProcessorServerUnitTests.h
    native/unittests/AssetScannerUnitTests.cpp
    native/unittests/AssetScannerUnitTests.h
    native/unittests/ConnectionUnitTests.cpp
    native/unittests/ConnectionUnitTests.h
    native/unittests/ConnectionManagerUnitTests.cpp
    native/unittests/ConnectionManagerUnitTests.h
    native/unittests/FileWatcherUnitTests.cpp
    native/unittests/FileWatcherUnitTests.h
    native/unittests/PlatformConfigurationUnitTests.cpp
    native/unittests/PlatformConfigurationUnitTests.h
    native/unittests/RCcontrollerUnitTests.cpp
    native/unittests/RCcontrollerUnitTests.h
    native/unittests/UnitTestRunner.cpp
    native/unittests/UnitTestRunner.h
    native/unittests/UtilitiesUnitTests.cpp
    native/unittests/UtilitiesUnitTests.h
    native/unittests/AssetRequestHandlerUnitTests.cpp
    native/unittests/AssetRequestHandlerUnitTests.h
    native/unittests/MockConnectionHandler.h
    native/unittests/MockApplicationManager.cpp
    native/unittests/MockApplicationManager.h
    native/unittests/BuilderSDKUnitTests.cpp
    native/utilities/UnitTestShaderCompilerServer.cpp
    native/utilities/UnitTestShaderCompilerServer.h
    native/tests/test_main.cpp
    native/tests/UnitTestUtilities.h
    native/tests/UnitTestUtilities.cpp
)

set(SKIP_UNITY_BUILD_INCLUSION_FILES
    native/tests/utilities/JobModelTest.cpp
)

#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    native/AssetDatabase/AssetDatabase.cpp
    native/AssetDatabase/AssetDatabase.h
    native/AssetManager/AssetCatalog.cpp
    native/AssetManager/AssetCatalog.h
    native/AssetManager/assetProcessorManager.cpp
    native/AssetManager/assetProcessorManager.h
    native/AssetManager/ProductAsset.h
    native/AssetManager/ProductAsset.cpp
    native/AssetManager/AssetRequestHandler.cpp
    native/AssetManager/AssetRequestHandler.h
    native/AssetManager/assetScanFolderInfo.h
    native/AssetManager/assetScanFolderInfo.cpp
    native/AssetManager/assetScanner.cpp
    native/AssetManager/assetScanner.h
    native/AssetManager/assetScannerWorker.cpp
    native/AssetManager/assetScannerWorker.h
    native/AssetManager/FileStateCache.cpp
    native/AssetManager/FileStateCache.h
    native/AssetManager/Validators/LfsPointerFileValidator.cpp
    native/AssetManager/Validators/LfsPointerFileValidator.h
    native/AssetManager/PathDependencyManager.cpp
    native/AssetManager/PathDependencyManager.h
    native/AssetManager/SourceFileRelocator.cpp
    native/AssetManager/SourceFileRelocator.h
    native/AssetManager/ControlRequestHandler.cpp
    native/AssetManager/ControlRequestHandler.h
    native/AssetManager/ExcludedFolderCache.cpp
    native/AssetManager/ExcludedFolderCache.h
    native/AssetManager/ExcludedFolderCacheInterface.h
    native/AssetManager/SourceAssetReference.h
    native/AssetManager/SourceAssetReference.cpp
    native/assetprocessor.h
    native/connection/connection.cpp
    native/connection/connection.h
    native/connection/connectionManager.cpp
    native/connection/connectionManager.h
    native/connection/connectionMessages.h
    native/connection/connectionworker.cpp
    native/connection/connectionworker.h
    native/FileProcessor/FileProcessor.cpp
    native/FileProcessor/FileProcessor.h
    native/FileWatcher/FileWatcher.cpp
    native/FileWatcher/FileWatcher.h
    native/FileWatcher/FileWatcherBase.h
    native/InternalBuilders/SettingsRegistryBuilder.cpp
    native/InternalBuilders/SettingsRegistryBuilder.h
    native/resourcecompiler/JobsModel.cpp
    native/resourcecompiler/JobsModel.h
    native/resourcecompiler/RCBuilder.cpp
    native/resourcecompiler/RCBuilder.h
    native/resourcecompiler/RCCommon.cpp
    native/resourcecompiler/RCCommon.h
    native/resourcecompiler/rccontroller.cpp
    native/resourcecompiler/rccontroller.h
    native/resourcecompiler/rcjob.cpp
    native/resourcecompiler/rcjob.h
    native/resourcecompiler/rcjoblistmodel.cpp
    native/resourcecompiler/rcjoblistmodel.h
    native/resourcecompiler/RCJobSortFilterProxyModel.cpp
    native/resourcecompiler/RCJobSortFilterProxyModel.h
    native/resourcecompiler/RCQueueSortModel.cpp
    native/resourcecompiler/RCQueueSortModel.h
    native/utilities/ApplicationManagerAPI.h
    native/utilities/ApplicationManager.cpp
    native/utilities/ApplicationManager.h
    native/utilities/ApplicationManagerBase.cpp
    native/utilities/ApplicationManagerBase.h
    native/utilities/ApplicationServer.cpp
    native/utilities/ApplicationServer.h
    native/utilities/AssetBuilderInfo.cpp
    native/utilities/AssetBuilderInfo.h
    native/utilities/AssetServerHandler.cpp
    native/utilities/AssetServerHandler.h
    native/utilities/AssetUtilEBusHelper.h
    native/utilities/assetUtils.cpp
    native/utilities/assetUtils.h
    native/utilities/BuilderConfigurationBus.h
    native/utilities/BuilderConfigurationManager.cpp
    native/utilities/BuilderConfigurationManager.h
    native/utilities/Builder.cpp
    native/utilities/Builder.h
    native/utilities/BuilderList.cpp
    native/utilities/BuilderList.h
    native/utilities/BuilderManager.cpp
    native/utilities/BuilderManager.h
    native/utilities/BuilderManager.inl
    native/utilities/ByteArrayStream.cpp
    native/utilities/ByteArrayStream.h
    native/utilities/IniConfiguration.cpp
    native/utilities/IniConfiguration.h
    native/utilities/JobDiagnosticTracker.cpp
    native/utilities/JobDiagnosticTracker.h
    native/utilities/LineByLineDependencyScanner.cpp
    native/utilities/LineByLineDependencyScanner.h
    native/utilities/MissingDependencyScanner.cpp
    native/utilities/MissingDependencyScanner.h
    native/utilities/PlatformConfiguration.cpp
    native/utilities/PlatformConfiguration.h
    native/utilities/PotentialDependencies.h
    native/utilities/StatsCapture.cpp
    native/utilities/StatsCapture.h
    native/utilities/SpecializedDependencyScanner.h
    native/utilities/ThreadHelper.cpp
    native/utilities/ThreadHelper.h
    native/utilities/IPathConversion.h
)

set(SKIP_UNITY_BUILD_INCLUSION_FILES
    native/resourcecompiler/JobsModel.cpp
)

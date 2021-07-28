/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#define AZ_TRAIT_AUDIOSYSTEM_ATL_POOL_SIZE 8 << 10 /* 8 MiB */
#define AZ_TRAIT_AUDIOSYSTEM_ATL_POOL_SIZE_DEFAULT_TEXT "8192 (8 MiB)"
#define AZ_TRAIT_AUDIOSYSTEM_AUDIO_EVENT_POOL_SIZE 256
#define AZ_TRAIT_AUDIOSYSTEM_AUDIO_EVENT_POOL_SIZE_DEFAULT_TEXT "256"
#define AZ_TRAIT_AUDIOSYSTEM_AUDIO_OBJECT_POOL_SIZE 512
#define AZ_TRAIT_AUDIOSYSTEM_AUDIO_OBJECT_POOL_SIZE_DEFAULT_TEXT "512"
#define AZ_TRAIT_AUDIOSYSTEM_AUDIO_THREAD_AFFINITY AFFINITY_MASK_ALL
#define AZ_TRAIT_AUDIOSYSTEM_FILE_CACHE_MANAGER_SIZE 384 << 10 /* 384 MiB */
#define AZ_TRAIT_AUDIOSYSTEM_FILE_CACHE_MANAGER_SIZE_DEFAULT_TEXT "393216 (384 MiB)"

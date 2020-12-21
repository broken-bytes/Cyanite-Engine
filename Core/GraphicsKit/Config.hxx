#pragma once
#include <cstdint>

constexpr uint8_t Frames = 2;
constexpr uint8_t Contexts = 2;
constexpr uint8_t CommandLists = 3;
constexpr uint8_t CommandQueues = 4;


constexpr uint8_t CommandListPre = 0;
constexpr uint8_t CommandListMid = 1;
constexpr uint8_t CommandListPost = 2;
constexpr uint8_t CommandListCount = 3;

constexpr uint64_t GPU_VIRTUAL_ADDRESS_UNDEF = static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(0);

constexpr uint64_t GPU_VIRTUAL_ADDRESS_UNKNOWN = static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(-1);

constexpr D3D_FEATURE_LEVEL MIN_D3D_LVL = D3D_FEATURE_LEVEL_12_0;
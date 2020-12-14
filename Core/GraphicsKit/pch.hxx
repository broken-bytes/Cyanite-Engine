#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

// WIN
#include <windows.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>

// DX12
#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <D3Dcompiler.h>
#include <DirectXMath.h>
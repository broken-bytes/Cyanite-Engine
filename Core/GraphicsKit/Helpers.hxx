#pragma once
#include "pch.hxx"


const auto CPUs = std::thread::hardware_concurrency();

// Assign a name to the object to aid with debugging.
#if defined(_DEBUG) || defined(DBG)
inline void SetName(ID3D12Object* object, LPCWSTR name)
{
    object->SetName(name);
}
inline void SetNameIndexed(ID3D12Object* object, LPCWSTR name, UINT index)
{
    WCHAR fullName[50];
    if (swprintf_s(fullName, L"%s[%u]", name, index) > 0)
    {
        object->SetName(fullName);
    }
}
#else
inline void SetName(ID3D12Object*, LPCWSTR)
{
}
inline void SetNameIndexed(ID3D12Object*, LPCWSTR, UINT)
{
}
#endif

inline auto NameD3D12Obj(ID3D12Object* object, const wchar_t* name) {
    object->SetName(name);
}

inline auto IndexD3D12Obj(
    ID3D12Object* object,
    const wchar_t* name,
    uint64_t id
) {
    object->SetName(name + id);
}
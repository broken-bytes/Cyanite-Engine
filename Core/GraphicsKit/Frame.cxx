#include "pch.hxx"
#include "Frame.hxx"

namespace Cyanite::GraphicsKit {
    Frame::Frame(winrt::com_ptr<ID3D12Device> device,
        winrt::com_ptr<ID3D12PipelineState> pso,
        winrt::com_ptr<ID3D12PipelineState> shadowsPso,
        winrt::com_ptr<ID3D12DescriptorHeap> dspHeap,
        winrt::com_ptr<ID3D12DescriptorHeap> srcHeap,
        winrt::com_ptr<D3D12_VIEWPORT> pViewport,
        UINT frameValue) :
        _value(frameValue),
        _pipelineState(pso),
        _pipelineStateShadowMap(shadowsPso)
    {
    
          
    }

    auto Frame::Commands() const -> std::array<winrt::com_ptr<ID3D12CommandList>, Contexts * 2 + CommandListCount> {
            return _batchList; 
    }
    auto Frame::GraphicsCommands() const -> std::array<winrt::com_ptr<ID3D12GraphicsCommandList>, CommandListCount> {
        return _commandLists;
    }

    auto Frame::ShadowCommands() const -> std::array<winrt::com_ptr<ID3D12GraphicsCommandList>, CommandListCount> {
        return _shadowCommandLists;
    }

    auto Frame::SceneCommands() const -> std::array<winrt::com_ptr<ID3D12GraphicsCommandList>, CommandListCount> {
        return _sceneCommandLists;
    }

    Frame::~Frame()
    {
        for (int i = 0; i < CommandListCount; i++)
        {
            _commandAllocators[i] = nullptr;
            _commandLists[i] = nullptr;
        }

        _shadowConstantBuffer = nullptr;
        _sceneConstantBuffer = nullptr;

        for (int i = 0; i < Contexts; i++)
        {
            _shadowCommandLists[i] = nullptr;
            _shadowCommandAllocators[i] = nullptr;

            _sceneCommandLists[i] = nullptr;
            _sceneCommandAllocators[i] = nullptr;
        }

        _shadowTexture = nullptr;
    }
}

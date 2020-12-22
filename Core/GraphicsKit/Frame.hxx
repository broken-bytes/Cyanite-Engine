#pragma once
#pragma once

#include "pch.hxx"


#include "GraphicsHandler.hxx"
#include "Scene.hxx"

struct Camera;

namespace Cyanite::GraphicsKit {
	struct LightState;

	class Frame {
    public:
        ID3D12CommandList* _batchSubmit[Contexts * 2 + CommandListCount];

        winrt::com_ptr<ID3D12CommandAllocator> _commandAllocators[CommandListCount];
        winrt::com_ptr<ID3D12GraphicsCommandList> _commandLists[CommandListCount];

        winrt::com_ptr<ID3D12CommandAllocator> _shadowCommandAllocators[Contexts];
        winrt::com_ptr<ID3D12GraphicsCommandList> _shadowCommandLists[Contexts];

        winrt::com_ptr<ID3D12CommandAllocator> _sceneCommandAllocators[Contexts];
        winrt::com_ptr<ID3D12GraphicsCommandList> _sceneCommandLists[Contexts];

        UINT64 _fenceValue;

        Frame(
            winrt::com_ptr < ID3D12Device> device,
            winrt::com_ptr<ID3D12PipelineState> pso,
            winrt::com_ptr<ID3D12PipelineState> shadowMapPso,
            winrt::com_ptr<ID3D12DescriptorHeap> dsvHeap,
            winrt::com_ptr<ID3D12DescriptorHeap> cbvSrvHeap,
            D3D12_VIEWPORT* viewport,
            uint64_t frameResourceIndex);
        ~Frame();

        void Bind(ID3D12GraphicsCommandList* pCommandList, BOOL scenePass, D3D12_CPU_DESCRIPTOR_HANDLE* pRtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE* pDsvHandle);
        void Init();
        void SwapBarriers();
        void Finish();
        void WriteConstantBuffers(D3D12_VIEWPORT* pViewport, Camera* pSceneCamera, Camera lightCams[1], LightState lights[1]);


    private:
        winrt::com_ptr<ID3D12PipelineState> _pipelineState;
        winrt::com_ptr<ID3D12PipelineState> _pipelineStateShadowMap;
        winrt::com_ptr<ID3D12Resource> _shadowTexture;
        D3D12_CPU_DESCRIPTOR_HANDLE _shadowDepthView;
        winrt::com_ptr<ID3D12Resource> _shadowConstantBuffer;
        winrt::com_ptr<ID3D12Resource> _sceneConstantBuffer;
        SceneConstantBuffer* _shadowConstantBufferWO;        // WRITE-ONLY pointer to the shadow pass constant buffer.
        SceneConstantBuffer* _sceneConstantBufferWO;        // WRITE-ONLY pointer to the scene pass constant buffer.
        D3D12_GPU_DESCRIPTOR_HANDLE _nullSrvHandle;    // Null SRV for out of bounds behavior.
        D3D12_GPU_DESCRIPTOR_HANDLE _shadowDepthHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE _shadowCbvHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE _sceneCbvHandle;
	};
}

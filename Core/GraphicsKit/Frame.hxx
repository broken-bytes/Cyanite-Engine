#pragma once
#pragma once

#include "pch.hxx"

#include "Scene.hxx"

struct Camera;

namespace Cyanite::GraphicsKit {
	class Frame {

	public:
		Frame(
			winrt::com_ptr<ID3D12Device> device, 
			winrt::com_ptr<ID3D12PipelineState> pso,
			winrt::com_ptr<ID3D12PipelineState> shadowsPso,
			winrt::com_ptr<ID3D12DescriptorHeap> dspHeap,
			winrt::com_ptr<ID3D12DescriptorHeap> srcHeap,
			winrt::com_ptr<D3D12_VIEWPORT> pViewport,
			UINT frameValue);
		~Frame();

		auto Commands() const->std::array<
			winrt::com_ptr<ID3D12CommandList>,
			Contexts * 2 + CommandListCount
		>;

		auto GraphicsCommands() const->std::array<
			winrt::com_ptr<ID3D12GraphicsCommandList>,
			CommandListCount
		>;

		auto ShadowCommands() const->std::array<
			winrt::com_ptr<ID3D12GraphicsCommandList>,
			CommandListCount
		>;

		auto SceneCommands() const->std::array<
			winrt::com_ptr<ID3D12GraphicsCommandList>,
			CommandListCount
		>;

		
		

		void Bind(ID3D12GraphicsCommandList* pCommandList, BOOL scenePass, D3D12_CPU_DESCRIPTOR_HANDLE* pRtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE* pDsvHandle);
		void Init();
		void SwapBarriers();
		void Finish();
		void WriteConstantBuffers(D3D12_VIEWPORT* pViewport, Camera* sceneCamera, std::vector<Camera> lightCams, std::vector<Components::Light> lights);


	private:
		std::array <
			winrt::com_ptr<ID3D12CommandList>,
			Contexts * 2 + CommandListCount
		> _batchList;

		std::array<
			winrt::com_ptr<ID3D12CommandAllocator>,
			CommandListCount
		> _commandAllocators;

		std::array<
			winrt::com_ptr<ID3D12GraphicsCommandList>,
			CommandListCount
		> _commandLists;

		std::array<
			winrt::com_ptr<ID3D12CommandAllocator>,
			CommandListCount
		>_shadowCommandAllocators;

		std::array<
			winrt::com_ptr<ID3D12GraphicsCommandList>,
			CommandListCount
		> _shadowCommandLists;

		std::array<
			winrt::com_ptr<ID3D12CommandAllocator>,
			CommandListCount
		> _sceneCommandAllocators;

		std::array<
			winrt::com_ptr<ID3D12GraphicsCommandList>,
			CommandListCount
		> _sceneCommandLists;

		UINT64 _value;

		winrt::com_ptr<ID3D12PipelineState> _pipelineState;
		winrt::com_ptr<ID3D12PipelineState> _pipelineStateShadowMap;
		winrt::com_ptr<ID3D12Resource> _shadowTexture;
		D3D12_CPU_DESCRIPTOR_HANDLE _shadowDepthView;
		winrt::com_ptr<ID3D12Resource> _shadowConstantBuffer;
		winrt::com_ptr<ID3D12Resource> _sceneConstantBuffer;
		Components::Scene* _shadowConstantBufferWO;        // WRITE-ONLY pointer to the shadow pass constant buffer.
		Components::Scene* _sceneConstantBufferWO;        // WRITE-ONLY pointer to the scene pass constant buffer.
		D3D12_GPU_DESCRIPTOR_HANDLE _nullSrvHandle;    // Null SRV for out of bounds behavior.
		D3D12_GPU_DESCRIPTOR_HANDLE _shadowDepthHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE _shadowCbvHandle;
	};
}

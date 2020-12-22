#include "pch.hxx"
#include "Frame.hxx"

namespace Cyanite::GraphicsKit {
	Frame::Frame(
		winrt::com_ptr<ID3D12Device> device,
		winrt::com_ptr<ID3D12PipelineState> pso,
		winrt::com_ptr<ID3D12PipelineState> shadowMapPso,
		winrt::com_ptr<ID3D12DescriptorHeap> dsvHeap,
		winrt::com_ptr<ID3D12DescriptorHeap> cbvSrvHeap,
		D3D12_VIEWPORT* viewport,
		uint64_t frameResourceIndex) :
		_fenceValue(0),
		_pipelineState(pso),
		_pipelineStateShadowMap(shadowMapPso)
	{
		for (UINT i = 0; i < CommandListCount; i++)
		{
			winrt::check_hresult(
				device->CreateCommandAllocator(
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					IID_PPV_ARGS(_commandAllocators[i].put())
				)
			);
			winrt::check_hresult(
				device->CreateCommandList(
					0,
					D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocators[i].get(),
					_pipelineState.get(),
					IID_PPV_ARGS(_commandLists[i].put())
				)
			);

			IndexD3D12Obj(_commandLists->get(), L"CMD List", i);

			// Close these command lists; don't record into them for now.
			winrt::check_hresult(_commandLists[i]->Close());
		}

		for (UINT i = 0; i < Contexts; i++)
		{
			// Create command list allocators for worker threads. One alloc is 
			// for the shadow pass command list, and one is for the scene pass.
			winrt::check_hresult(
				device->CreateCommandAllocator(
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					IID_PPV_ARGS(
						_shadowCommandAllocators[i].put()
					)
				)
			);
			winrt::check_hresult(
				device->CreateCommandAllocator(
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					IID_PPV_ARGS(
						_sceneCommandAllocators[i].put()
					)
				)
			);

			winrt::check_hresult(
				device->CreateCommandList(
					0,
					D3D12_COMMAND_LIST_TYPE_DIRECT, _shadowCommandAllocators[i].get(), _pipelineStateShadowMap.get(),
					IID_PPV_ARGS(
						_shadowCommandLists[i].put()
					)
				)
			);
			winrt::check_hresult(
				device->CreateCommandList(
					0,
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					_sceneCommandAllocators[i].get(),
					_pipelineState.get(),
					IID_PPV_ARGS(
						_sceneCommandLists[i].put()
					)
				)
			);

			IndexD3D12Obj(
				_shadowCommandLists->get(),
				L"Shadow lists",
				i
			);
			IndexD3D12Obj(
				_sceneCommandLists->get(),
				L"Scene Lists",
				i
			);

			// Close these command lists; don't record into them for now. We will 
			// reset them to a recording state when we start the render loop.
			winrt::check_hresult(_shadowCommandLists[i]->Close());
			winrt::check_hresult(_sceneCommandLists[i]->Close());
		}

		// Describe and create the shadow map texture.
		CD3DX12_RESOURCE_DESC shadowTexDesc(
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			0,
			static_cast<UINT>(viewport->Width),
			static_cast<UINT>(viewport->Height),
			1,
			1,
			DXGI_FORMAT_R32_TYPELESS,
			1,
			0,
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		D3D12_CLEAR_VALUE clearValue;        // Performance tip: Tell the runtime at resource creation the desired clear value.
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		winrt::check_hresult(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&shadowTexDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(_shadowTexture.put())));

		NameD3D12Obj(_shadowTexture.get(), L"ShadowText");

		// Get a handle to the start of the descriptor heap then offset 
		// it based on the frame resource index.
		const UINT dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart(), 1 + frameResourceIndex, dsvDescriptorSize); // + 1 for the shadow map.

		// Describe and create the shadow depth view and cache the CPU 
		// descriptor handle.
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		device->CreateDepthStencilView(_shadowTexture.get(), &depthStencilViewDesc, depthHandle);
		_shadowDepthView = depthHandle;

		// Get a handle to the start of the descriptor heap then offset it 
		// based on the existing textures and the frame resource index. Each 
		// frame has 1 SRV (shadow tex) and 2 CBVs.
		const UINT nullSrvCount = 2;                                // Null descriptors at the start of the heap.
		const UINT textureCount = _countof(SampleAssets::Textures);    // Diffuse + normal textures near the start of the heap.  Ideally, track descriptor heap contents/offsets at a higher level.
		const UINT cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle(cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
		CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle(cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());
		_nullSrvHandle = cbvSrvGpuHandle;
		cbvSrvCpuHandle.Offset(nullSrvCount + textureCount + (frameResourceIndex * Frames), cbvSrvDescriptorSize);
		cbvSrvGpuHandle.Offset(nullSrvCount + textureCount + (frameResourceIndex * Frames), cbvSrvDescriptorSize);

		// Describe and create a shader resource view (SRV) for the shadow depth 
		// texture and cache the GPU descriptor handle. This SRV is for sampling 
		// the shadow map from our shader. It uses the same texture that we use 
		// as a depth-stencil during the shadow pass.
		D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
		shadowSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		shadowSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shadowSrvDesc.Texture2D.MipLevels = 1;
		shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		device->CreateShaderResourceView(_shadowTexture.get(), &shadowSrvDesc, cbvSrvCpuHandle);
		_shadowDepthHandle = cbvSrvGpuHandle;

		// Increment the descriptor handles.
		cbvSrvCpuHandle.Offset(cbvSrvDescriptorSize);
		cbvSrvGpuHandle.Offset(cbvSrvDescriptorSize);

		// Create the constant buffers.
		const UINT constantBufferSize = (sizeof(SceneConstantBuffer) + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1); // must be a multiple 256 bytes
		winrt::check_hresult(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(_shadowConstantBuffer.put())));
		winrt::check_hresult(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(_sceneConstantBuffer.put())));

		// Map the constant buffers and cache their heap pointers.
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		winrt::check_hresult(_shadowConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&_shadowConstantBufferWO)));
		winrt::check_hresult(_sceneConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&_sceneConstantBufferWO)));

		// Create the constant buffer views: one for the shadow pass and
		// another for the scene pass.
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.SizeInBytes = constantBufferSize;

		// Describe and create the shadow constant buffer view (CBV) and 
		// cache the GPU descriptor handle.
		cbvDesc.BufferLocation = _shadowConstantBuffer->GetGPUVirtualAddress();
		device->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
		_shadowCbvHandle = cbvSrvGpuHandle;

		// Increment the descriptor handles.
		cbvSrvCpuHandle.Offset(cbvSrvDescriptorSize);
		cbvSrvGpuHandle.Offset(cbvSrvDescriptorSize);

		// Describe and create the scene constant buffer view (CBV) and 
		// cache the GPU descriptor handle.
		cbvDesc.BufferLocation = _sceneConstantBuffer->GetGPUVirtualAddress();
		device->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
		_sceneCbvHandle = cbvSrvGpuHandle;

		// Batch up command lists for execution later.
		const UINT batchSize = _countof(_sceneCommandLists) + _countof(_shadowCommandLists) + 3;
		_batchSubmit[0] = _commandLists[CommandListPre].get();
		memcpy(_batchSubmit + 1, _shadowCommandLists, _countof(_shadowCommandLists) * sizeof(ID3D12CommandList*));
		_batchSubmit[_countof(_shadowCommandLists) + 1] = _commandLists[CommandListMid].get();
		memcpy(_batchSubmit + _countof(_shadowCommandLists) + 2, _sceneCommandLists, _countof(_sceneCommandLists) * sizeof(ID3D12CommandList*));
		_batchSubmit[batchSize - 1] = _commandLists[CommandListPost].get();
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

	// Builds and writes constant buffers from scratch to the proper slots for 
	// this frame resource.
	void Frame::WriteConstantBuffers(D3D12_VIEWPORT* pViewport, Camera* pSceneCamera, Camera lightCams[], LightState lights[NumLights])
	{
		SceneConstantBuffer sceneConsts = {};
		SceneConstantBuffer shadowConsts = {};

		// Scale down the world a bit.
		XMStoreFloat4x4(&sceneConsts.model, XMMatrixScaling(0.1f, 0.1f, 0.1f));
		XMStoreFloat4x4(&shadowConsts.model, XMMatrixScaling(0.1f, 0.1f, 0.1f));

		// The scene pass is drawn from the camera.
		pSceneCamera->Get3DViewProjMatrices(&sceneConsts.view, &sceneConsts.projection, 90.0f, pViewport->Width, pViewport->Height);

		// The light pass is drawn from the first light.
		lightCams[0].Get3DViewProjMatrices(&shadowConsts.view, &shadowConsts.projection, 90.0f, pViewport->Width, pViewport->Height);

		for (int i = 0; i < NumLights; i++)
		{
			memcpy(&sceneConsts.lights[i], &lights[i], sizeof(LightState));
			memcpy(&shadowConsts.lights[i], &lights[i], sizeof(LightState));
		}

		// The shadow pass won't sample the shadow map, but rather write to it.
		shadowConsts.sampleShadowMap = FALSE;

		// The scene pass samples the shadow map.
		sceneConsts.sampleShadowMap = TRUE;

		shadowConsts.ambientColor = sceneConsts.ambientColor = { 0.1f, 0.2f, 0.3f, 1.0f };

		memcpy(_sceneConstantBufferWO, &sceneConsts, sizeof(SceneConstantBuffer));
		memcpy(_shadowConstantBufferWO, &shadowConsts, sizeof(SceneConstantBuffer));
	}

	void Frame::Init()
	{
		// Reset the command allocators and lists for the main thread.
		for (int i = 0; i < CommandListCount; i++)
		{
			winrt::check_hresult(_commandAllocators[i]->Reset());
			winrt::check_hresult(_commandLists[i]->Reset(_commandAllocators[i].get(), _pipelineState.get()));
		}

		// Clear the depth stencil buffer in preparation for rendering the shadow map.
		_commandLists[CommandListPre]->ClearDepthStencilView(_shadowDepthView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// Reset the worker command allocators and lists.
		for (int i = 0; i < Contexts; i++)
		{
			winrt::check_hresult(_shadowCommandAllocators[i]->Reset());
			winrt::check_hresult(_shadowCommandLists[i]->Reset(_shadowCommandAllocators[i].get(), _pipelineStateShadowMap.get()));

			winrt::check_hresult(_sceneCommandAllocators[i]->Reset());
			winrt::check_hresult(_sceneCommandLists[i]->Reset(_sceneCommandAllocators[i].get(), _pipelineState.get()));
		}
	}

	void Frame::SwapBarriers()
	{
		// Transition the shadow map from writeable to readable.
		_commandLists[CommandListMid]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_shadowTexture.get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}

	void Frame::Finish()
	{
		_commandLists[CommandListPost]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_shadowTexture.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	}

	// Sets up the descriptor tables for the worker command list to use 
	// resources provided by frame resource.
	void Frame::Bind(ID3D12GraphicsCommandList* pCommandList, BOOL scenePass, D3D12_CPU_DESCRIPTOR_HANDLE* pRtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE* pDsvHandle)
	{
		if (scenePass)
		{
			// Scene pass. We use constant buf #2 and depth stencil #2
			// with rendering to the render target enabled.
			pCommandList->SetGraphicsRootDescriptorTable(2, _shadowDepthHandle);        // Set the shadow texture as an SRV.
			pCommandList->SetGraphicsRootDescriptorTable(1, _sceneCbvHandle);

			assert(pRtvHandle != nullptr);
			assert(pDsvHandle != nullptr);

			pCommandList->OMSetRenderTargets(1, pRtvHandle, FALSE, pDsvHandle);
		}
		else
		{
			// Shadow pass. We use constant buf #1 and depth stencil #1
			// with rendering to the render target disabled.
			pCommandList->SetGraphicsRootDescriptorTable(2, _nullSrvHandle);            // Set a null SRV for the shadow texture.
			pCommandList->SetGraphicsRootDescriptorTable(1, _shadowCbvHandle);

			pCommandList->OMSetRenderTargets(0, nullptr, FALSE, &_shadowDepthView);    // No render target needed for the shadow pass.
		}
	}
}

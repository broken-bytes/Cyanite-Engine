#include "pch.hxx"
#include "GraphicsHandler.hxx"
#include "DeviceHandler.hxx"
#include "CommandQueueHandler.hxx"
#include "FactoryHandler.hxx"
#include "SwapChainHandler.hxx"
#include "../AssetKit/AssetHandler.hxx"
namespace Cyanite::GraphicsKit {

	using namespace DirectX;
	using namespace winrt;

	GraphicsHandler::GraphicsHandler(HWND window) {
		this->_window = window;
	}

	GraphicsHandler::~GraphicsHandler() {
		Deinitialize();
	}

	auto GraphicsHandler::Initialize() -> void
	{
		LoadPipeline();
		LoadAssets();
	}
	auto GraphicsHandler::Deinitialize() -> void
	{
		{
			const UINT64 fence = _fenceValue;
			const UINT64 lastCompletedFence = _fence->GetCompletedValue();

			// Signal and increment the fence value.
			winrt::check_hresult(
				_commandQueue->Signal(
					_fence.get(),
					_fenceValue
				)
			);
			_fenceValue++;

			// Wait until the previous frame is finished.
			if (lastCompletedFence < fence)
			{
				winrt::check_hresult(
					_fence->SetEventOnCompletion(
						fence,
						_fenceEvent
					)
				);
				WaitForSingleObject(_fenceEvent, INFINITE);
			}
			CloseHandle(_fenceEvent);
		}

		// Close thread events and thread handles.
		for (int x = 0; x < Contexts; x++)
		{
			CloseHandle(_workerBeginRenderFrame[x]);
			CloseHandle(_workerFinishShadowPass[x]);
			CloseHandle(_workerFinishedRenderFrame[x]);
			CloseHandle(_threadHandles[x]);
		}

		for (int x = 0; x < _countof(_frameResources); x++)
		{
			delete _frameResources[x];
		}
	}
	auto GraphicsHandler::Update() -> void
	{
	}
	auto GraphicsHandler::Render() -> void
	{
		// Record all the commands we need to render the scene into the command list.
		PopulateCommandList();

		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] = { _commandList.get() };
		_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Present the frame.
		winrt::check_hresult(_swapChain->Present(1, 0));

		WaitForPreviousFrame();
	}
	auto GraphicsHandler::Resize(uint32_t width, uint32_t height) -> void
	{
		if (width == _width && _height == height) {
			return;
		}

		_width = width;
		_height = height;
		_aspectRatio = static_cast<float>(width) / static_cast<float>(height);

		for (uint8_t x = 0; x < Frames; x++) {
			_renderTargets[x]->Release();
			_frameResources[x]->Finish();
		}

		DXGI_SWAP_CHAIN_DESC desc{};
		_swapChain->GetDesc(&desc);
		_swapChain->ResizeBuffers(
			2,
			width,
			height,
			desc.BufferDesc.Format,
			desc.Flags
		);
		_frameIndex = _swapChain->GetCurrentBackBufferIndex();
	}

	auto GraphicsHandler::Flush(
		com_ptr<ID3D12CommandQueue> commandQueue,
		com_ptr<ID3D12Fence> fence,
		uint64_t& fenceValue,
		HANDLE fenceEvent
	) -> void
	{
		WaitForPreviousFrame();
	}

	auto GraphicsHandler::FrameStart() -> void {
		_currentFrameResource->Init();

		// Indicate that the back buffer will be used as a render target.
		_currentFrameResource->GraphicsCommands()[CommandListPre].get()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[_frameIndex].get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		// Clear the render target and depth stencil.
		const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvHeap->GetCPUDescriptorHandleForHeapStart(), _frameIndex, _rtvDescriptorSize);
		_currentFrameResource->GraphicsCommands()[CommandListPre]->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		_currentFrameResource->GraphicsCommands()[CommandListPre]->ClearDepthStencilView(_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		winrt::check_hresult(_currentFrameResource->GraphicsCommands()[CommandListPre]->Close());
	}

	auto GraphicsHandler::FrameMid() const -> void {
		_currentFrameResource->SwapBarriers();

		winrt::check_hresult(_currentFrameResource->GraphicsCommands()[CommandListMid]->Close());
	}
	auto GraphicsHandler::FrameEnd() -> void {
		_currentFrameResource->Finish();

		// Indicate that the back buffer will now be used to present.
		_currentFrameResource->GraphicsCommands()[CommandListPost]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[_frameIndex].get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		winrt::check_hresult(_currentFrameResource->GraphicsCommands()[CommandListPost]->Close());
	}

	auto GraphicsHandler::SetDefaultPipeline(winrt::com_ptr<ID3D12GraphicsCommandList> commands)-> void {
		commands->SetGraphicsRootSignature(_rootSignature.get());

		ID3D12DescriptorHeap* heaps[] = {
			_cbvSrvHeap.get(),
			_samplerHeap.get()
		};
		commands->SetDescriptorHeaps(_countof(heaps), heaps);

		commands->RSSetViewports(1, &_viewport);
		commands->RSSetScissorRects(1, &_scissorRect);
		commands->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commands->IASetVertexBuffers(0, 1, &_vertexBufferView);
		commands->IASetIndexBuffer(&_indexBufferView);
		commands->SetGraphicsRootDescriptorTable(
			3,
			_samplerHeap->GetGPUDescriptorHandleForHeapStart()
		);
		commands->OMSetStencilRef(0);
	}

	auto GraphicsHandler::LoadPipeline() -> void
	{
#if defined(_DEBUG)
		// Enable the D3D12 debug layer.
		{

			com_ptr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
			}
		}
#endif

		_factory = FactoryHandler::CreateFactory();

		if (_useWarpDevice)
		{
			com_ptr<IDXGIAdapter> warpAdapter;
			winrt::check_hresult(_factory->EnumWarpAdapter(IID_PPV_ARGS(warpAdapter.put())));


			winrt::check_hresult(D3D12CreateDevice(
				warpAdapter.get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(_device.put())
			));
		}
		else
		{
			winrt::check_hresult(D3D12CreateDevice(
				DeviceHandler::QueryAdapters().get(),
				MIN_D3D_LVL,
				IID_PPV_ARGS(_device.put())
			));
		}

		_swapChain = SwapChainHandler::CreateSwapChainFor(
			_window,
			_commandQueue
		);

		winrt::check_hresult(_factory->MakeWindowAssociation(_window, DXGI_MWA_NO_ALT_ENTER));

		_frameIndex = _swapChain->GetCurrentBackBufferIndex();

		// Create descriptor heaps.
		{
			// Describe and create a render target view (RTV) descriptor heap.
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = Frames;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			winrt::check_hresult(_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&_rtvHeap)));

			// Describe and create a depth stencil view (DSV) descriptor heap.
			// Each frame has its own depth stencils (to write shadows onto) 
			// and then there is one for the scene itself.
			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
			dsvHeapDesc.NumDescriptors = 1 + Frames * 1;
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			winrt::check_hresult(_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_dsvHeap)));

			// Describe and create a shader resource view (SRV) and constant 
			// buffer view (CBV) descriptor heap.  Heap layout: null views, 
			// object diffuse + normal textures views, frame 1's shadow buffer, 
			// frame 1's 2x constant buffer, frame 2's shadow buffer, frame 2's 
			// 2x constant buffers, etc...
			const UINT nullSrvCount = 2;        // Null descriptors are needed for out of bounds behavior reads.
			const UINT cbvCount = Frames * 2;
			const UINT srvCount = _countof(SampleAssets::Textures) + (Frames * 1);
			D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
			cbvSrvHeapDesc.NumDescriptors = nullSrvCount + cbvCount + srvCount;
			cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			winrt::check_hresult(_device->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&_cbvSrvHeap)));
			NameD3D12Obj(_cbvSrvHeap.get(), L"SrcHeap");

			// Describe and create a sampler descriptor heap.
			D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
			samplerHeapDesc.NumDescriptors = 2;        // One clamp and one wrap sampler.
			samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			winrt::check_hresult(_device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&_samplerHeap)));
			NameD3D12Obj(_samplerHeap.get(), L"SamplerHeap");

			_rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

		winrt::check_hresult(_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator)));
	}

	auto GraphicsHandler::LoadAssets() -> void {
		CreateRootSignature();
		CreatePipelineState();

		// Create temporary command list for initial GPU setup.
		winrt::com_ptr<ID3D12GraphicsCommandList> commandList;
		winrt::check_hresult(
			_device->CreateCommandList(
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				_commandAllocator.get(),
				_pipelineState.get(),
				IID_PPV_ARGS(commandList.put())
			)
		);

		CreateRenderTargetViews();
		CreateDepthStencil();

		// Load scene assets.
		UINT fileSize = 0;
		UINT8* pAssetData;
		winrt::check_hresult(ReadDataFromFile(GetAssetFullPath(SampleAssets::DataFileName).c_str(), &pAssetData, &fileSize));

		CreateVertexBuffer();
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = pAssetData + SampleAssets::VertexDataOffset;
		vertexData.RowPitch = SampleAssets::VertexDataSize;
		vertexData.SlicePitch = vertexData.RowPitch;
		CopyToVertexBuffer(commandList, vertexData);

		CreateIndexBuffer();
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = pAssetData + SampleAssets::VertexDataOffset;
		indexData.RowPitch = SampleAssets::VertexDataSize;
		indexData.SlicePitch = indexData.RowPitch;
		CopyToIndexBuffer(commandList, indexData);

		CreateShaderResource();
		free(pAssetData);

		// Create the samplers.
		{
			// Get the sampler descriptor size for the current device.
			const UINT samplerDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

			// Get a handle to the start of the descriptor heap.
			CD3DX12_CPU_DESCRIPTOR_HANDLE samplerHandle(m_samplerHeap->GetCPUDescriptorHandleForHeapStart());

			// Describe and create the wrapping sampler, which is used for 
			// sampling diffuse/normal maps.
			D3D12_SAMPLER_DESC wrapSamplerDesc = {};
			wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			wrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			wrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			wrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			wrapSamplerDesc.MinLOD = 0;
			wrapSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
			wrapSamplerDesc.MipLODBias = 0.0f;
			wrapSamplerDesc.MaxAnisotropy = 1;
			wrapSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			wrapSamplerDesc.BorderColor[0] = wrapSamplerDesc.BorderColor[1] = wrapSamplerDesc.BorderColor[2] = wrapSamplerDesc.BorderColor[3] = 0;
			m_device->CreateSampler(&wrapSamplerDesc, samplerHandle);

			// Move the handle to the next slot in the descriptor heap.
			samplerHandle.Offset(samplerDescriptorSize);

			// Describe and create the point clamping sampler, which is 
			// used for the shadow map.
			D3D12_SAMPLER_DESC clampSamplerDesc = {};
			clampSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			clampSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			clampSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			clampSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			clampSamplerDesc.MipLODBias = 0.0f;
			clampSamplerDesc.MaxAnisotropy = 1;
			clampSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			clampSamplerDesc.BorderColor[0] = clampSamplerDesc.BorderColor[1] = clampSamplerDesc.BorderColor[2] = clampSamplerDesc.BorderColor[3] = 0;
			clampSamplerDesc.MinLOD = 0;
			clampSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
			_device->CreateSampler(&clampSamplerDesc, samplerHandle);
		}

		// Create lights.
		for (int i = 0; i < NumLights; i++)
		{
			// Set up each of the light positions and directions (they all start 
			// in the same place).
			m_lights[i].position = { 0.0f, 15.0f, -30.0f, 1.0f };
			m_lights[i].direction = { 0.0, 0.0f, 1.0f, 0.0f };
			m_lights[i].falloff = { 800.0f, 1.0f, 0.0f, 1.0f };
			m_lights[i].color = { 0.7f, 0.7f, 0.7f, 1.0f };

			XMVECTOR eye = XMLoadFloat4(&m_lights[i].position);
			XMVECTOR at = XMVectorAdd(eye, XMLoadFloat4(&m_lights[i].direction));
			XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

			m_lightCameras[i].Set(eye, at, up);
		}

		// Close the command list and use it to execute the initial GPU setup.
		ThrowIfFailed(commandList->Close());
		ID3D12CommandList* ppCommandLists[] = { commandList.get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Create frame resources.
		for (int i = 0; i < FrameCount; i++)
		{
			m_frameResources[i] = new FrameResource(m_device.Get(), m_pipelineState.Get(), m_pipelineStateShadowMap.Get(), m_dsvHeap.Get(), m_cbvSrvHeap.Get(), &m_viewport, i);
			m_frameResources[i]->WriteConstantBuffers(&m_viewport, &m_camera, m_lightCameras, m_lights);
		}
		m_currentFrameResourceIndex = 0;
		m_pCurrentFrameResource = m_frameResources[m_currentFrameResourceIndex];

		// Create synchronization objects and wait until assets have been uploaded to the GPU.
		{
			ThrowIfFailed(m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
			m_fenceValue++;

			// Create an event handle to use for frame synchronization.
			m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (m_fenceEvent == nullptr)
			{
				ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
			}

			// Wait for the command list to execute; we are reusing the same command 
			// list in our main loop but for now, we just want to wait for setup to 
			// complete before continuing.

			// Signal and increment the fence value.
			const UINT64 fenceToWaitFor = m_fenceValue;
			ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fenceToWaitFor));
			m_fenceValue++;

			// Wait until the fence is completed.
			ThrowIfFailed(m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent));
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	auto GraphicsHandler::CreateRootSignature() -> void {
		// Create the root signature.
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[4]; // Perfomance TIP: Order from most frequent to least frequent.
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);    // 2 frequently changed diffuse + normal textures - using registers t1 and t2.
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);    // 1 frequently changed constant buffer.
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);                                                // 1 infrequently changed shadow texture - starting in register t0.
		ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 2, 0);                                            // 2 static samplers.

		CD3DX12_ROOT_PARAMETER1 rootParameters[4];
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[3].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		winrt::com_ptr<ID3DBlob> signature;
		winrt::com_ptr<ID3DBlob> error;
		winrt::check_hresult(
			D3DX12SerializeVersionedRootSignature(
				&rootSignatureDesc,
				featureData.HighestVersion,
				signature.put(),
				error.put()
			)
		);
		winrt::check_hresult(_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&_rootSignature)));
		NameD3D12Obj(_rootSignature.get(), L"Root Signature");

	}

	auto GraphicsHandler::CreatePipelineState() -> void {
		winrt::com_ptr<ID3DBlob> vertexShader;
		winrt::com_ptr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

		winrt::check_hresult(
			D3DCompileFromFile(
				AssetKit::AssetHandler::GetAssetPath(
					L"shaders.hlsl"
				).c_str(),
				nullptr,
				nullptr,
				"VSMain",
				"vs_5_0",
				compileFlags,
				0,
				vertexShader.put(),
				nullptr
			)
		);
		winrt::check_hresult(
			D3DCompileFromFile(
				AssetKit::AssetHandler::GetAssetPath(
					L"shaders.hlsl"
				).c_str(),
				nullptr,
				nullptr,
				"PSMain",
				"ps_5_0",
				compileFlags,
				0,
				pixelShader.put(),
				nullptr
			)
		);

		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
		inputLayoutDesc.pInputElementDescs = SampleAssets::StandardVertexDescription;
		inputLayoutDesc.NumElements = _countof(SampleAssets::StandardVertexDescription);

		CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		depthStencilDesc.StencilEnable = FALSE;

		// Describe and create the PSO for rendering the scene.
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = inputLayoutDesc;
		psoDesc.pRootSignature = _rootSignature.get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = depthStencilDesc;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;

		winrt::check_hresult(
			_device->CreateGraphicsPipelineState(
				&psoDesc,
				IID_PPV_ARGS(&_pipelineState)
			)
		);
		NameD3D12Obj(_pipelineState.get(), L"Pipeline State");

		// Alter the description and create the PSO for rendering
		// the shadow map.  The shadow map does not use a pixel
		// shader or render targets.
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(0, 0);
		psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		psoDesc.NumRenderTargets = 0;

		winrt::check_hresult(
			_device->CreateGraphicsPipelineState(
				&psoDesc,
				IID_PPV_ARGS(&_pipelineStateShadowMap)
			)
		);
		NameD3D12Obj(
			_pipelineStateShadowMap.get(),
			L"Pipeline Shadow Map"
		);
	}

	auto GraphicsHandler::CreateRenderTargetViews() -> void {
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
			_rtvHeap->GetCPUDescriptorHandleForHeapStart()
		);
		for (uint32_t x = 0; x < Frames; x++)
		{
			winrt::check_hresult(
				_swapChain->GetBuffer(
					x,
					IID_PPV_ARGS(_renderTargets[x].put())
				)
			);
			_device->CreateRenderTargetView(
				_renderTargets[x].get(),
				nullptr,
				rtvHandle
			);
			rtvHandle.Offset(1, _rtvDescriptorSize);

			IndexD3D12Obj(
				_renderTargets[x].get(),
				L"Render Targets",
				x
			);
		}
	}

	auto GraphicsHandler::CreateDepthStencil() -> void {
		CD3DX12_RESOURCE_DESC shadowTextureDesc(
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			0,
			static_cast<UINT>(_viewport.Width),
			static_cast<UINT>(_viewport.Height),
			1,
			1,
			DXGI_FORMAT_D32_FLOAT,
			1,
			0,
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

		D3D12_CLEAR_VALUE clearValue;    // Performance tip: Tell the runtime at resource creation the desired clear value.
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		winrt::check_hresult(
			_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&shadowTextureDesc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&clearValue,
				IID_PPV_ARGS(_depthStencil.put())
			)
		);

		NameD3D12Obj(_depthStencil.get(), L"Depth Stencil");

		// Create the depth stencil view.
		_device->CreateDepthStencilView(
			_depthStencil.get(),
			nullptr,
			_dsvHeap->GetCPUDescriptorHandleForHeapStart()
		);
	}

	auto GraphicsHandler::CreateVertexBuffer() -> void {
		winrt::check_hresult(
			_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::VertexDataSize),
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(_vertexBuffer.put())));

		NameD3D12Obj(_vertexBuffer.get(), L"Vertex Buffer");
		winrt::check_hresult(
			_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::VertexDataSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(_vertexBufferUpload.put())
			)
		);
	}

	auto GraphicsHandler::CopyToVertexBuffer(
		winrt::com_ptr<ID3D12GraphicsCommandList> list,
		D3D12_SUBRESOURCE_DATA vertexData
	) -> void {
		PIXBeginEvent(list.get(), 0, L"Copy vertex buffer data to default resource...");

		UpdateSubresources<1>(
			list.get(),
			_vertexBuffer.get(),
			_vertexBufferUpload.get(),
			0,
			0,
			1,
			&vertexData
			);
		list->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				_vertexBuffer.get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
			)
		);
		PIXEndEvent(list.get());

		// Initialize the vertex buffer view.
		_vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
		_vertexBufferView.SizeInBytes = SampleAssets::VertexDataSize;
		_vertexBufferView.StrideInBytes = SampleAssets::StandardVertexStride;
	}

	auto GraphicsHandler::CreateIndexBuffer() -> void {
		winrt::check_hresult(
			_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::IndexDataSize),
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(_indexBuffer.put())
			)
		);

		NameD3D12Obj(_indexBuffer.get(), L"Index Buffer");
		winrt::check_hresult(
			_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::IndexDataSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(_indexBufferUpload.put())
			)
		);
	}

	auto GraphicsHandler::CopyToIndexBuffer(winrt::com_ptr<ID3D12GraphicsCommandList> list,
		D3D12_SUBRESOURCE_DATA indexData) -> void {
		PIXBeginEvent(list.get(), 0, L"Copy index buffer data to default resource...");

		UpdateSubresources<1>(
			list.get(),
			_indexBuffer.get(),
			_indexBufferUpload.get(),
			0,
			0,
			1,
			&indexData
			);
		list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_indexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

		PIXEndEvent(list.get());
	}

	auto GraphicsHandler::CreateShaderResource() -> void {
		// Get the CBV SRV descriptor size for the current device.
		const UINT cbvSrvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Get a handle to the start of the descriptor heap.
		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvHandle(_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());

		// Describe and create 2 null SRVs. Null descriptors are needed in order 
		// to achieve the effect of an "unbound" resource.
		D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
		nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		nullSrvDesc.Texture2D.MipLevels = 1;
		nullSrvDesc.Texture2D.MostDetailedMip = 0;
		nullSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		_device->CreateShaderResourceView(nullptr, &nullSrvDesc, cbvSrvHandle);
		cbvSrvHandle.Offset(cbvSrvDescriptorSize);

		_device->CreateShaderResourceView(nullptr, &nullSrvDesc, cbvSrvHandle);
		cbvSrvHandle.Offset(cbvSrvDescriptorSize);
		// Create each texture and SRV descriptor.

	}

	auto GraphicsHandler::UploadTextures(std::vector<Components::Texture> textures) -> void {
		auto handle = static_cast<CD3DX12_CPU_DESCRIPTOR_HANDLE>(
			_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart()
		);
		const uint32_t srvCount = textures.size();
		PIXBeginEvent(list.get(), 0, L"Copy diffuse and normal texture data to default resources...");
		for (uint32_t x = 0; x < srvCount; x++)
		{
			// Describe and create a Texture2D.
			const SampleAssets::TextureResource& tex = SampleAssets::Textures[x];
			CD3DX12_RESOURCE_DESC texDesc(
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				tex.Width,
				tex.Height,
				1,
				static_cast<UINT16>(tex.MipLevels),
				tex.Format,
				1,
				0,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				D3D12_RESOURCE_FLAG_NONE);

			winrt::check_hresult(_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&texDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(_textures[x].put())));

			IndexD3D12Obj(_textures->get(), L"Textures", x);

			const UINT subresourceCount = texDesc.DepthOrArraySize * texDesc.MipLevels;
			UINT64 uploadBufferSize = GetRequiredIntermediateSize(_textures[x].get(), 0, subresourceCount);
			winrt::check_hresult(_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(_textureUploads[x].put())));

			// Copy data to the intermediate upload heap and then schedule a copy
			// from the upload heap to the Texture2D.
			D3D12_SUBRESOURCE_DATA textureData = {};
			textureData.pData = pAssetData + tex.Data->Offset;
			textureData.RowPitch = tex.Data->Pitch;
			textureData.SlicePitch = tex.Data->Size;

			UpdateSubresources(list.get(), _textures[x].get(), _textureUploads[x].get(), 0, 0, subresourceCount, &textureData);
			list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_textures[x].get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

			// Describe and create an SRV.
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = tex.Format;
			srvDesc.Texture2D.MipLevels = tex.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			_device->CreateShaderResourceView(_textures[x].get(), &srvDesc, handle);

			// Move to the next descriptor slot.
			handle.Offset(cbvSrvDescriptorSize);
		}
		PIXEndEvent(commandList.Get());
	}

	auto GraphicsHandler::PopulateCommandList() -> void
	{
		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		winrt::check_hresult(_commandAllocator->Reset());

		// However, when ExecuteCommandList() is called on a particular command 
		// list, that command list can then be reset at any time and must be before 
		// re-recording.
		winrt::check_hresult(_commandList->Reset(_commandAllocator.get(), _pipelineState.get()));

		// Set necessary state.
		_commandList->SetGraphicsRootSignature(_rootSignature.get());
		_commandList->RSSetViewports(1, &_viewport);
		_commandList->RSSetScissorRects(1, &_scissorRect);

		// TODO FIX CRASH
		auto trans =
			CD3DX12_RESOURCE_BARRIER::Transition(
				_renderTargets[_frameIndex].get(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			);
		// Indicate that the back buffer will be used as a render target.
		_commandList->ResourceBarrier(1, &trans);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
			_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
			_frameIndex,
			_rtvDescriptorSize
		);
		_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		// Record commands.
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
		_commandList->DrawInstanced(3, 1, 0, 0);

		trans =
			CD3DX12_RESOURCE_BARRIER::Transition(
				_renderTargets[_frameIndex].get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT
			);
		// Indicate that the back buffer will now be used to present.
		_commandList->ResourceBarrier(1, &trans);

		winrt::check_hresult(_commandList->Close());
	}

	auto GraphicsHandler::AwaitGpuCompletion() -> void {
		// Schedule a Signal command in the queue.
		winrt::check_hresult(
			_commandQueue->Signal(
				_fence.get(),
				_fenceValue
			)
		);

		// Wait until the fence has been processed.
		winrt::hresult(
			_fence->SetEventOnCompletion(
				_fenceValue,
				_fenceEvent
			)
		);
		WaitForSingleObjectEx(_fenceEvent, INFINITE, FALSE);
	}

	auto GraphicsHandler::Worker(uint8_t id) -> void {
		assert(id >= 0);
		assert(id < Contexts);

		while (id >= 0 && id < Contexts)
		{
			// Wait for main thread to tell us to draw.

			WaitForSingleObject(_workerBeginRenderFrame[id], INFINITE);
			winrt::com_ptr<ID3D12GraphicsCommandList> shadowCommandList =
				_currentFrameResource->ShadowCommands()[id];
			winrt::com_ptr <ID3D12GraphicsCommandList> sceneCommandList = _currentFrameResource->SceneCommands()[id];

			SetDefaultPipeline(shadowCommandList);
			_currentFrameResource->Bind(sceneCommandList.get(), FALSE, nullptr, nullptr);    // No need to pass RTV or DSV descriptor heap.

			// Set null SRVs for the diffuse/normal textures.
			shadowCommandList->SetGraphicsRootDescriptorTable(0, _cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());

			// Distribute objects over threads by drawing only 1/NumContexts 
			// objects per worker (i.e. every object such that objectnum % 
			// NumContexts == threadIndex).
			PIXBeginEvent(shadowCommandList, 0, L"Worker drawing shadow pass...");

			for (int x = id; x < _countof(SampleAssets::Draws); x += Contexts)
			{
				SampleAssets::DrawParameters drawArgs = SampleAssets::Draws[x];

				shadowCommandList->DrawIndexedInstanced(drawArgs.IndexCount, 1, drawArgs.IndexStart, drawArgs.VertexBase, 0);
			}

			PIXEndEvent(shadowCommandList.get());

			winrt::check_hresult(shadowCommandList->Close());

			// Submit shadow pass.
			SetEvent(_workerFinishShadowPass[id]);

			//
			// Scene pass
			// 

			// Populate the command list.  These can only be sent after the shadow 
			// passes for this frame have been submitted.
			SetDefaultPipeline(sceneCommandList);
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvHeap->GetCPUDescriptorHandleForHeapStart(), _frameIndex, _rtvDescriptorSize);
			CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(_dsvHeap->GetCPUDescriptorHandleForHeapStart());
			_currentFrameResource->Bind(sceneCommandList.get(), TRUE, &rtvHandle, &dsvHandle);

			PIXBeginEvent(sceneCommandList.get(), 0, L"Worker drawing scene pass...");

			D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvHeapStart = _cbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
			const UINT cbvSrvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			const UINT nullSrvCount = 2;
			for (int x = id; x < _countof(SampleAssets::Draws); x += Contexts)
			{
				SampleAssets::DrawParameters drawArgs = SampleAssets::Draws[x];

				// Set the diffuse and normal textures for the current object.
				CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvHandle(cbvSrvHeapStart, nullSrvCount + drawArgs.DiffuseTextureIndex, cbvSrvDescriptorSize);
				sceneCommandList->SetGraphicsRootDescriptorTable(0, cbvSrvHandle);

				sceneCommandList->DrawIndexedInstanced(drawArgs.IndexCount, 1, drawArgs.IndexStart, drawArgs.VertexBase, 0);
			}

			PIXEndEvent(sceneCommandList.get());
			winrt::check_hresult(sceneCommandList->Close());

			// Tell main thread that we are done.
			SetEvent(_workerFinishedRenderFrame[id]);
		}
	}
}

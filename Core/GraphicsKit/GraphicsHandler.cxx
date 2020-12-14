#include "pch.hxx"
#include "GraphicsHandler.hxx"
#include "DeviceHandler.hxx"
#include "../AssetKit/AssetHandler.hxx"
namespace Cyanide::GraphicsKit {

	using namespace DirectX;
	using namespace winrt;

	GraphicsHandler::GraphicsHandler(HWND window) {
		this->_window = window;
	}

	auto GraphicsHandler::Initialize() -> void
	{
		LoadPipeline();
		LoadAssets();
	}
	auto GraphicsHandler::Deinitialize() -> void
	{
		// Wait for the GPU to be done with all resources.
		WaitForPreviousFrame();

		CloseHandle(_fenceEvent);
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
		_width = width;
		_height = height;

		_aspectRatio = static_cast<float>(width) / height;
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

		com_ptr<IDXGIFactory4> factory;
		winrt::check_hresult(CreateDXGIFactory1(IID_PPV_ARGS(factory.put())));

		if (_useWarpDevice)
		{
			com_ptr<IDXGIAdapter> warpAdapter;
			winrt::check_hresult(factory->EnumWarpAdapter(IID_PPV_ARGS(warpAdapter.put())));

			winrt::check_hresult(D3D12CreateDevice(
				warpAdapter.get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(_device.put())
			));
		}
		else
		{
			com_ptr<IDXGIAdapter1> hardwareAdapter;
			DeviceHandler::QueryAdapters();

			winrt::check_hresult(D3D12CreateDevice(
				hardwareAdapter.get(),
				MIN_D3D_LVL,
				IID_PPV_ARGS(_device.put())
			));
		}

		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		winrt::check_hresult(_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(_commandQueue.put())));

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = 0;
		swapChainDesc.Height = 0;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_STRAIGHT;
		swapChainDesc.Flags = 0;

		com_ptr<IDXGISwapChain1> swapChain;
		winrt::check_hresult(factory->CreateSwapChainForHwnd(
			_commandQueue.get(),
			_window,
			&swapChainDesc,
			nullptr,
			nullptr,
			swapChain.put()
		)
		);

		swapChain.as(_swapChain);

		// This sample does not support fullscreen transitions.
		winrt::check_hresult(factory->MakeWindowAssociation(_window, DXGI_MWA_NO_ALT_ENTER));

		_frameIndex = _swapChain->GetCurrentBackBufferIndex();

		// Create descriptor heaps.
		{
			// Describe and create a render target view (RTV) descriptor heap.
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = FrameCount;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			winrt::check_hresult(_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(_rtvHeap.put())));

			_rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

		// Create frame resources.
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvHeap->GetCPUDescriptorHandleForHeapStart());

			// Create a RTV for each frame.
			for (UINT n = 0; n < FrameCount; n++)
			{
				winrt::check_hresult(_swapChain->GetBuffer(n, IID_PPV_ARGS(_renderTargets[n].put())));
				_device->CreateRenderTargetView(_renderTargets[n].get(), nullptr, rtvHandle);
				rtvHandle.Offset(1, _rtvDescriptorSize);
			}
		}

		winrt::check_hresult(_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator)));
	}

	auto GraphicsHandler::LoadAssets() -> void {
		{
			// Create an empty root signature.
			{
				CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
				rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

				com_ptr<ID3DBlob> signature;
				com_ptr<ID3DBlob> error;
				winrt::check_hresult(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.put(), error.put()));
				winrt::check_hresult(_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&_rootSignature)));
			}

			// Create the pipeline state, which includes compiling and loading shaders.
			{
				com_ptr<ID3DBlob> vertexShader;
				com_ptr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
				// Enable better shader debugging with the graphics debugging tools.
				UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
				UINT compileFlags = 0;
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

				// Define the vertex input layout.
				D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
				};

				// Describe and create the graphics pipeline state object (PSO).
				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
				psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
				psoDesc.pRootSignature = _rootSignature.get();
				psoDesc.VS = { static_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
				psoDesc.PS = { static_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
				psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
				psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
				psoDesc.DepthStencilState.DepthEnable = FALSE;
				psoDesc.DepthStencilState.StencilEnable = FALSE;
				psoDesc.SampleMask = UINT_MAX;
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				psoDesc.NumRenderTargets = 1;
				psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
				psoDesc.SampleDesc.Count = 1;
				winrt::check_hresult(_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(_pipelineState.put())));
			}

			// Create the command list.
			winrt::check_hresult(_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator.get(), _pipelineState.get(), IID_PPV_ARGS(_commandList.put())));

			// Command lists are created in the recording state, but there is nothing
			// to record yet. The main loop expects it to be closed, so close it now.
			winrt::check_hresult(_commandList->Close());

			// Create the vertex buffer.
			{
				// Define the geometry for a triangle.
				Vertex triangleVertices[] =
				{
					{ { 0.0f, 0.25f * _aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
					{ { 0.25f, -0.25f * _aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
					{ { -0.25f, -0.25f * _aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
				};

				const UINT vertexBufferSize = sizeof(triangleVertices);

				// Note: using upload heaps to transfer static data like vert buffers is not 
				// recommended. Every time the GPU needs it, the upload heap will be marshalled 
				// over. Please read up on Default Heap usage. An upload heap is used here for 
				// code simplicity and because there are very few verts to actually transfer.
				CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_UPLOAD };
				auto buff = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
				winrt::check_hresult(
					_device->CreateCommittedResource(
						&heapProps,
						D3D12_HEAP_FLAG_NONE,
						&buff,
						D3D12_RESOURCE_STATE_GENERIC_READ,
						nullptr,
						IID_PPV_ARGS(_vertexBuffer.put())
					)
				);

				// Copy the triangle data to the vertex buffer.
				UINT8* pVertexDataBegin;
				CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
				winrt::check_hresult(_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
				memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
				_vertexBuffer->Unmap(0, nullptr);

				// Initialize the vertex buffer view.
				_vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
				_vertexBufferView.StrideInBytes = sizeof(Vertex);
				_vertexBufferView.SizeInBytes = vertexBufferSize;
			}

			// Create synchronization objects and wait until assets have been uploaded to the GPU.
			{
				winrt::check_hresult(_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.put())));
				_fenceValue = 1;

				// Create an event handle to use for frame synchronization.
				_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
				if (_fenceEvent == nullptr)
				{
					winrt::check_hresult(HRESULT_FROM_WIN32(GetLastError()));
				}

				// Wait for the command list to execute; we are reusing the same command 
				// list in our main loop but for now, we just want to wait for setup to 
				// complete before continuing.
				WaitForPreviousFrame();
			}
		}
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

		auto trans =
			CD3DX12_RESOURCE_BARRIER::Transition(
				_renderTargets[_frameIndex].get(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			);
		// Indicate that the back buffer will be used as a render target.
		_commandList->ResourceBarrier(1, &trans);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvHeap->GetCPUDescriptorHandleForHeapStart(), _frameIndex, _rtvDescriptorSize);
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

	auto GraphicsHandler::WaitForPreviousFrame() -> void
	{
		// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
		// This is code implemented as such for simplicity. More advanced samples 
		// illustrate how to use fences for efficient resource usage.

		// Signal and increment the fence value.
		const UINT64 fence = _fenceValue;
		winrt::check_hresult(_commandQueue->Signal(_fence.get(), fence));
		_fenceValue++;

		// Wait until the previous frame is finished.
		if (_fence->GetCompletedValue() < fence)
		{
			winrt::check_hresult(_fence->SetEventOnCompletion(fence, _fenceEvent));
			WaitForSingleObject(_fenceEvent, INFINITE);
		}

		_frameIndex = _swapChain->GetCurrentBackBufferIndex();
	}
}

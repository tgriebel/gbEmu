#include "renderer_d3d12.h"
#include "audioEngine.h"

void wtRenderer::WaitForGpu()
{
	ThrowIfFailed( m_cmd.d3d12CommandQueue->Signal( sync.fence.Get(), sync.fenceValues[ currentFrameIx ] ) );

	ThrowIfFailed( sync.fence->SetEventOnCompletion( sync.fenceValues[ currentFrameIx ], sync.fenceEvent ) );
	WaitForSingleObjectEx( sync.fenceEvent, INFINITE, FALSE );

	sync.fenceValues[ currentFrameIx ]++;
}


void wtRenderer::AdvanceNextFrame()
{
	const UINT64 currentFence = sync.fenceValues[ currentFrameIx ];
	ThrowIfFailed( m_cmd.d3d12CommandQueue->Signal( sync.fence.Get(), currentFence ) );

	currentFrameIx = m_swapChain.dxgi->GetCurrentBackBufferIndex();

	if ( sync.fence->GetCompletedValue() < sync.fenceValues[ currentFrameIx ] )
	{
		ThrowIfFailed( sync.fence->SetEventOnCompletion( sync.fenceValues[ currentFrameIx ], sync.fenceEvent ) );
		WaitForSingleObject( sync.fenceEvent, INFINITE );
	//	PrintLog( "GPU WAIT.\n" );
	}

	sync.fenceValues[ currentFrameIx ] = currentFence + 1;
	frameNumber = sync.fenceValues[ currentFrameIx ];
}


uint32_t wtRenderer::InitD3D12()
{
	view.width = view.defaultWidth;
	view.height = view.defaultHeight;
	view.viewport = CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>( view.width ), static_cast<float>( view.height ) );
	view.scissorRect = CD3DX12_RECT( 0, view.overscanY0, view.defaultWidth, view.overscanY1 );

	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController;
	if ( SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( &debugController ) ) ) )
	{
		debugController->EnableDebugLayer();
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	ThrowIfFailed( CreateDXGIFactory2( dxgiFactoryFlags, IID_PPV_ARGS( &m_dxgiFactory ) ) );
	GetHardwareAdapter( m_dxgiFactory.Get(), &m_dxgiAdapter );
	ThrowIfFailed( D3D12CreateDevice( m_dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &m_d3d12device ) ) );

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed( m_d3d12device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &m_cmd.d3d12CommandQueue ) ) );

	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	ThrowIfFailed( m_d3d12device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &m_cmd.d3d12CopyQueue ) ) );

	m_cmd.d3d12CopyQueue->SetName( L"Copy Queue" );
	m_cmd.d3d12CommandQueue->SetName( L"Draw Queue" );

	CreateFrameBuffers();

	D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
	cbvSrvUavHeapDesc.NumDescriptors = MaxTextures + 1;
	cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed( m_d3d12device->CreateDescriptorHeap( &cbvSrvUavHeapDesc, IID_PPV_ARGS( &m_pipeline.cbvSrvUavHeap ) ) );
	m_pipeline.cbvSrvUavDescStride = m_d3d12device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

	for ( uint32_t i = 0; i < FrameCount; ++i )
	{
		ThrowIfFailed( m_d3d12device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_cmd.commandAllocator[ i ] ) ) );
		ThrowIfFailed( m_d3d12device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS( &m_cmd.cpyCommandAllocator[ i ] ) ) );
		ThrowIfFailed( m_d3d12device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_cmd.imguiCommandAllocator[ i ] ) ) );
	}

	InitImgui();
	CreateD3D12Pipeline();
	initD3D12 = true;

	return 0;
}


void wtRenderer::CreatePSO()
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if ( FAILED( m_d3d12device->CheckFeatureSupport( D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof( featureData ) ) ) )
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	const uint32_t srvCnt = 1; // Only 1 texture is visible to ImGui at a time
	CD3DX12_DESCRIPTOR_RANGE1 ranges[ 2 ] = {};
	ranges[ 0 ].Init( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srvCnt, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE );
	ranges[ 1 ].Init( D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE );

	CD3DX12_ROOT_PARAMETER1 rootParameters[ 2 ] = {};
	rootParameters[ 0 ].InitAsDescriptorTable( 1, &ranges[ 0 ], D3D12_SHADER_VISIBILITY_PIXEL );
	rootParameters[ 1 ].InitAsDescriptorTable( 1, &ranges[ 1 ], D3D12_SHADER_VISIBILITY_PIXEL );

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1( _countof( rootParameters ), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT );

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed( D3DX12SerializeVersionedRootSignature( &rootSignatureDesc, featureData.HighestVersion, &signature, &error ) );
	ThrowIfFailed( m_d3d12device->CreateRootSignature( 0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS( &m_pipeline.rootSig ) ) );

	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ThrowIfFailed( D3DCompileFromFile( GetAssetFullPath( L"crt.hlsl" ).c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr ) );
	ThrowIfFailed( D3DCompileFromFile( GetAssetFullPath( L"crt.hlsl" ).c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr ) );

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,	0 },
		{ "COLOR",		0,	DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,	0 },
		{ "TEXCOORD",	0,	DXGI_FORMAT_R32G32_FLOAT,		0,	D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,	0 }
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof( inputElementDescs ) };
	psoDesc.pRootSignature = m_pipeline.rootSig.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE( vertexShader.Get() );
	psoDesc.PS = CD3DX12_SHADER_BYTECODE( pixelShader.Get() );
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
	psoDesc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed( m_d3d12device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &m_pipeline.pso ) ) );
}


void wtRenderer::CreateCommandLists()
{
	for ( uint32_t i = 0; i < FrameCount; ++i )
	{
		ThrowIfFailed( m_d3d12device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmd.commandAllocator[ i ].Get(), m_pipeline.pso.Get(), IID_PPV_ARGS( &m_cmd.commandList[ i ] ) ) );
		ThrowIfFailed( m_d3d12device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmd.imguiCommandAllocator[ i ].Get(), m_pipeline.pso.Get(), IID_PPV_ARGS( &m_cmd.imguiCommandList[ i ] ) ) );
		ThrowIfFailed( m_d3d12device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_COPY, m_cmd.cpyCommandAllocator[ i ].Get(), m_pipeline.pso.Get(), IID_PPV_ARGS( &m_cmd.cpyCommandList[ i ] ) ) );

		ThrowIfFailed( m_cmd.commandList[ i ]->Close() );
		ThrowIfFailed( m_cmd.imguiCommandList[ i ]->Close() );
		ThrowIfFailed( m_cmd.cpyCommandList[ i ]->Close() );
	}
}


void wtRenderer::CreateConstantBuffers()
{
	const UINT constantBufferDataSize = ( sizeof( DisplayConstantBuffer ) + 255 ) & ~255;

	D3D12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer( constantBufferDataSize );

	ThrowIfFailed( m_d3d12device->CreateCommittedResource(
		&props,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS( &m_pipeline.cb ) ) );

	NAME_D3D12_OBJECT( m_pipeline.cb );

	m_pipeline.shaderData.hardPix = -3.0f;
	m_pipeline.shaderData.hardScan = -8.0f;
	m_pipeline.shaderData.maskDark = 0.9f;
	m_pipeline.shaderData.maskLight = 1.1f;
	m_pipeline.shaderData.warp = { 0.0f, 0.002f };
	m_pipeline.shaderData.imageDim = { 0.0f, 0.0f, 256.0f, 256.0f };
	m_pipeline.shaderData.destImageDim = { 0.0f, 0.0f, 256.0f, 256.0f };
	m_pipeline.shaderData.enable = false;

	m_pipeline.cbvSrvCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE( m_pipeline.cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), MaxTextures, m_pipeline.cbvSrvUavDescStride );
	m_pipeline.cbvSrvGpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE( m_pipeline.cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), MaxTextures, m_pipeline.cbvSrvUavDescStride );

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_pipeline.cb->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = constantBufferDataSize;
	m_d3d12device->CreateConstantBufferView( &cbvDesc, m_pipeline.cbvSrvCpuHandle );

	CD3DX12_RANGE readRange( 0, 0 );
	ThrowIfFailed( m_pipeline.cb->Map( 0, &readRange, reinterpret_cast<void**>( &m_pipeline.pCbvDataBegin ) ) );
	memcpy( m_pipeline.pCbvDataBegin, &m_pipeline.shaderData, sizeof( m_pipeline.cb ) );
}


void wtRenderer::CreateSyncObjects()
{
	ThrowIfFailed( m_d3d12device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &sync.fence ) ) );
	ThrowIfFailed( m_d3d12device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &sync.cpyFence ) ) );
	sync.fenceValues[ currentFrameIx ]++;

	sync.fenceEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );
	if ( sync.fenceEvent == nullptr )
	{
		ThrowIfFailed( HRESULT_FROM_WIN32( GetLastError() ) );
	}
}


void wtRenderer::CreateD3D12Pipeline()
{
	CreatePSO();
	CreateCommandLists();
	CreateVertexBuffers();
	CreateTextureResources();
	CreateConstantBuffers();
	CreateSyncObjects();
	WaitForGpu();
}


void wtRenderer::CreateVertexBuffers()
{
	const float x0 = NormalizeCoordinate( 0, view.defaultWidth );
	const float x1 = NormalizeCoordinate( 256, view.defaultWidth );
	const float y0 = NormalizeCoordinate( 0, view.defaultHeight );
	const float y1 = NormalizeCoordinate( 256, view.defaultHeight );

	const XMFLOAT4 tintColor = { 1.0f, 0.0f, 1.0f, 1.0f };

	Vertex triangleVertices[] =
	{
		{ { x0, y0, 0.0f }, tintColor, { 0.0f, 1.0f } },
		{ { x0, y1, 0.0f }, tintColor, { 0.0f, 0.0f } },
		{ { x1, y1, 0.0f }, tintColor, { 1.0f, 0.0f } },

		{ { x0, y0, 0.0f }, tintColor, { 0.0f, 1.0f } },
		{ { x1, y1, 0.0f }, tintColor, { 1.0f, 0.0f } },
		{ { x1, y0, 0.0f }, tintColor, { 1.0f, 1.0f } },
	};

	const UINT vertexBufferSize = sizeof( triangleVertices );

	D3D12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer( vertexBufferSize );

	ThrowIfFailed( m_d3d12device->CreateCommittedResource(
		&props,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS( &m_pipeline.vb ) ) );

	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange( 0, 0 );
	ThrowIfFailed( m_pipeline.vb->Map( 0, &readRange, reinterpret_cast<void**>( &pVertexDataBegin ) ) );
	memcpy( pVertexDataBegin, triangleVertices, sizeof( triangleVertices ) );
	m_pipeline.vb->Unmap( 0, nullptr );

	m_pipeline.vbView.BufferLocation = m_pipeline.vb->GetGPUVirtualAddress();
	m_pipeline.vbView.StrideInBytes = sizeof( Vertex );
	m_pipeline.vbView.SizeInBytes = vertexBufferSize;
}


void wtRenderer::BuildDrawCommandList()
{
	ThrowIfFailed( m_cmd.commandAllocator[ currentFrameIx ]->Reset() );
	ThrowIfFailed( m_cmd.commandList[ currentFrameIx ]->Reset( m_cmd.commandAllocator[ currentFrameIx ].Get(), m_pipeline.pso.Get() ) );

	m_cmd.commandList[ currentFrameIx ]->SetGraphicsRootSignature( m_pipeline.rootSig.Get() );

	ID3D12DescriptorHeap* ppHeaps[] = { m_pipeline.cbvSrvUavHeap.Get() };
	m_cmd.commandList[ currentFrameIx ]->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );
	m_cmd.commandList[ currentFrameIx ]->SetGraphicsRootDescriptorTable( 0, m_textureResources[ currentFrameIx ][ m_mainDisplayImageIndex ].gpuHandle );
	m_cmd.commandList[ currentFrameIx ]->SetGraphicsRootDescriptorTable( 1, m_pipeline.cbvSrvGpuHandle );

	m_cmd.commandList[ currentFrameIx ]->RSSetViewports( 1, &view.viewport );
	m_cmd.commandList[ currentFrameIx ]->RSSetScissorRects( 1, &view.scissorRect );

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_swapChain.renderTargets[ currentFrameIx ].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET );
	m_cmd.commandList[ currentFrameIx ]->ResourceBarrier( 1, &barrier );

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( m_swapChain.rtvHeap->GetCPUDescriptorHandleForHeapStart(), currentFrameIx, m_swapChain.rtvDescriptorSize );
	m_cmd.commandList[ currentFrameIx ]->OMSetRenderTargets( 1, &rtvHandle, FALSE, nullptr );

	const float clearColor[] = { 1.0f, 0.1f, 0.1f, 1.0f };
	m_cmd.commandList[ currentFrameIx ]->ClearRenderTargetView( rtvHandle, clearColor, 0, nullptr );
	m_cmd.commandList[ currentFrameIx ]->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	m_cmd.commandList[ currentFrameIx ]->IASetVertexBuffers( 0, 1, &m_pipeline.vbView );
	m_cmd.commandList[ currentFrameIx ]->DrawInstanced( 6, 1, 0, 0 );

	barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_swapChain.renderTargets[ currentFrameIx ].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT );
	m_cmd.commandList[ currentFrameIx ]->ResourceBarrier( 1, &barrier );

	ThrowIfFailed( m_cmd.commandList[ currentFrameIx ]->Close() );
}


void wtRenderer::ExecuteDrawCommands()
{
	ID3D12CommandList* ppCommandLists[] = { m_cmd.commandList[ currentFrameIx ].Get()
	#ifdef IMGUI_ENABLE
		, m_cmd.imguiCommandList[ currentFrameIx ].Get()
	#endif
	};
	m_cmd.d3d12CommandQueue->Wait( sync.cpyFence.Get(), sync.fenceValues[ currentFrameIx ] );
	m_cmd.d3d12CommandQueue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );
}


void wtRenderer::UpdateD3D12()
{
	memcpy( m_pipeline.pCbvDataBegin, &m_pipeline.shaderData, sizeof( m_pipeline.shaderData ) );
}


void wtRenderer::SubmitFrame()
{
	UpdateD3D12();

	BuildDrawCommandList();
	BuildImguiCommandList();
	ExecuteDrawCommands();

	ThrowIfFailed( m_swapChain.dxgi->Present( 1, 0 ) );
	AdvanceNextFrame();
}


void wtRenderer::CreateFrameBuffers()
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	swapChainDesc.Width					= view.defaultWidth;
	swapChainDesc.Height				= view.defaultHeight;
	swapChainDesc.BufferCount			= FrameCount;
	swapChainDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo				= false;
	swapChainDesc.SampleDesc.Count		= 1;
	swapChainDesc.SampleDesc.Quality	= 0;
	swapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Scaling				= DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags					= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullDesc;
	swapChainFullDesc.Windowed			= true;
	swapChainFullDesc.RefreshRate		= { 60, 1 };
	swapChainFullDesc.ScanlineOrdering	= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainFullDesc.Scaling			= DXGI_MODE_SCALING_CENTERED;

	ComPtr<IDXGISwapChain1> tmpSwapChain;
	ThrowIfFailed( m_dxgiFactory->CreateSwapChainForHwnd( m_cmd.d3d12CommandQueue.Get(), hWnd, &swapChainDesc, &swapChainFullDesc, nullptr, &tmpSwapChain ) );

	ThrowIfFailed( tmpSwapChain.As( &m_swapChain.dxgi ) );
	currentFrameIx = m_swapChain.dxgi->GetCurrentBackBufferIndex();

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed( m_d3d12device->CreateDescriptorHeap( &rtvHeapDesc, IID_PPV_ARGS( &m_swapChain.rtvHeap ) ) );

	m_swapChain.rtvDescriptorSize = m_d3d12device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( m_swapChain.rtvHeap->GetCPUDescriptorHandleForHeapStart() );
	for ( uint32_t i = 0; i < FrameCount; ++i )
	{
		ThrowIfFailed( m_swapChain.dxgi->GetBuffer( i, IID_PPV_ARGS( &m_swapChain.renderTargets[ i ] ) ) );
		m_d3d12device->CreateRenderTargetView( m_swapChain.renderTargets[ i ].Get(), nullptr, rtvHandle );
		rtvHandle.Offset( 1, m_swapChain.rtvDescriptorSize );
	}
}


void wtRenderer::CreateTextureResources()
{
	// Texture Heap
	{
		m_textureHeap.Reset();

		const uint64_t SizeInBytes = 256 * 256 * 4; // Worst-case resolution
		const uint64_t heapSize = MaxTextures * SizeInBytes;
		CD3DX12_HEAP_DESC heapDesc( heapSize, D3D12_HEAP_TYPE_DEFAULT, 0, D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES );
		ThrowIfFailed( m_d3d12device->CreateHeap( &heapDesc, IID_PPV_ARGS( &m_textureHeap ) ) );
	}

	// Upload Heap
	{
		m_uploadHeap.Reset();

		const uint64_t SizeInBytes = 256 * 256 * 4; // Worst-case resolution
		const uint64_t heapSize = 2 * MaxTextures * SizeInBytes; // Doubled needed size because it's just a large scratch buffer

		CD3DX12_HEAP_DESC heapDesc( heapSize, D3D12_HEAP_TYPE_UPLOAD, 0, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES );
		ThrowIfFailed( m_d3d12device->CreateHeap( &heapDesc, IID_PPV_ARGS( &m_uploadHeap ) ) );
	}
}


void wtRenderer::InitImgui()
{
#ifdef IMGUI_ENABLE
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init( hWnd );
	ImGui_ImplDX12_Init( m_d3d12device.Get(), FrameCount,
		DXGI_FORMAT_R8G8B8A8_UNORM, m_pipeline.cbvSrvUavHeap.Get(),
		m_pipeline.cbvSrvUavHeap.Get()->GetCPUDescriptorHandleForHeapStart(),
		m_pipeline.cbvSrvUavHeap.Get()->GetGPUDescriptorHandleForHeapStart() );
#endif
}


void wtRenderer::DestroyD3D12()
{
	WaitForGpu();

#ifdef IMGUI_ENABLE
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

	CloseHandle( sync.fenceEvent );

	initD3D12 = false;
}


bool wtRenderer::NeedsResize( const uint32_t width, const uint32_t height )
{
	if ( !initD3D12 ) {
		return false;
	}

	return ( width != view.width || height != view.height );
}


void wtRenderer::RecreateSwapChain( const uint32_t width, const uint32_t height )
{
	if( !initD3D12 ) {
		return;
	}

	// Flush all current GPU commands.
	WaitForGpu();

	for ( uint32_t i = 0; i < FrameCount; i++ )
	{
		m_swapChain.renderTargets[ i ].Reset();
		sync.fenceValues[ i ] = sync.fenceValues[ currentFrameIx ];
	}

	// Resize the swap chain to the desired dimensions.
	DXGI_SWAP_CHAIN_DESC desc = {};
	m_swapChain.dxgi->GetDesc( &desc );
	ThrowIfFailed( m_swapChain.dxgi->ResizeBuffers( FrameCount, width, height, desc.BufferDesc.Format, desc.Flags ) );

	// Reset the frame index to the current back buffer index.
	currentFrameIx = m_swapChain.dxgi->GetCurrentBackBufferIndex();

	view.width = width;
	view.height = height;
	view.viewport = CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>( width ), static_cast<float>( height ) );
	view.scissorRect = CD3DX12_RECT( 0, view.overscanY0, width, height );

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( m_swapChain.rtvHeap->GetCPUDescriptorHandleForHeapStart() );
	for ( uint32_t i = 0; i < FrameCount; ++i )
	{
		ThrowIfFailed( m_swapChain.dxgi->GetBuffer( i, IID_PPV_ARGS( &m_swapChain.renderTargets[ i ] ) ) );
		m_d3d12device->CreateRenderTargetView( m_swapChain.renderTargets[ i ].Get(), nullptr, rtvHandle );
		rtvHandle.Offset( 1, m_swapChain.rtvDescriptorSize );
	}
}


void wtRenderer::BeginFrame()
{
	m_heapOffset = 0;
	m_uploadHeapOffset = 0;
}


void wtRenderer::EndFrame()
{
}


void wtRenderer::SetDisplayTexture( const uint32_t slotIndex )
{
	m_mainDisplayImageIndex = slotIndex;
}


void wtRenderer::BindTextureSlot( const uint32_t slotIndex, TomBoy::wtRawImageInterface* img )
{
	m_textureResources[ currentFrameIx ][ slotIndex ].desc.MipLevels = 1;
	m_textureResources[ currentFrameIx ][ slotIndex ].desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_textureResources[ currentFrameIx ][ slotIndex ].desc.Alignment = 0;
	m_textureResources[ currentFrameIx ][ slotIndex ].desc.Width = img->GetWidth();
	m_textureResources[ currentFrameIx ][ slotIndex ].desc.Height = img->GetHeight();
	m_textureResources[ currentFrameIx ][ slotIndex ].desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	m_textureResources[ currentFrameIx ][ slotIndex ].desc.DepthOrArraySize = 1;
	m_textureResources[ currentFrameIx ][ slotIndex ].desc.SampleDesc.Count = 1;
	m_textureResources[ currentFrameIx ][ slotIndex ].desc.SampleDesc.Quality = 0;
	m_textureResources[ currentFrameIx ][ slotIndex ].desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	m_textureResources[ currentFrameIx ][ slotIndex ].allocInfo = m_d3d12device->GetResourceAllocationInfo( 0, 1, &m_textureResources[ currentFrameIx ][ slotIndex ].desc );

	const UINT cbvSrvUavHeapIncrement = m_d3d12device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
	const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHeapStart( m_pipeline.cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart() );
	const CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHeapStart( m_pipeline.cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart() );

	// Shader Resource Texture
	{
		m_textureResources[ currentFrameIx ][ slotIndex ].heapOffset = m_heapOffset;

		ThrowIfFailed( m_d3d12device->CreatePlacedResource(
			m_textureHeap.Get(),
			m_textureResources[ currentFrameIx ][ slotIndex ].heapOffset,
			&m_textureResources[ currentFrameIx ][ slotIndex ].desc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS( &m_textureResources[ currentFrameIx ][ slotIndex ].srv ) ) );

		m_heapOffset += m_textureResources[ currentFrameIx ][ slotIndex ].allocInfo.SizeInBytes;
	}

	// Intermediate Upload Texture
	{
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize( m_textureResources[ currentFrameIx ][ slotIndex ].srv.Get(), 0, 1 );

		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer( uploadBufferSize );

		m_textureResources[ currentFrameIx ][ slotIndex ].uploadHeapOffset = m_uploadHeapOffset;

		ThrowIfFailed( m_d3d12device->CreatePlacedResource(
			m_uploadHeap.Get(),
			m_textureResources[ currentFrameIx ][ slotIndex ].uploadHeapOffset,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS( &m_textureResources[ currentFrameIx ][ slotIndex ].uploadBuffer ) ) );

		m_uploadHeapOffset += m_textureResources[ currentFrameIx ][ slotIndex ].allocInfo.SizeInBytes; // Offset by alignment not actually allocation size
	}

	wchar_t texName[ 128 ];
	swprintf_s( texName, 128, L"Texture #%i(%i)", slotIndex, currentFrameIx );
	m_textureResources[ currentFrameIx ][ slotIndex ].srv->SetName( texName );

	m_textureResources[ currentFrameIx ][ slotIndex ].cpuHandle.InitOffsetted( cpuHeapStart, slotIndex + 1, cbvSrvUavHeapIncrement );
	m_textureResources[ currentFrameIx ][ slotIndex ].gpuHandle.InitOffsetted( gpuHeapStart, slotIndex + 1, cbvSrvUavHeapIncrement );
	m_textureResources[ currentFrameIx ][ slotIndex ].width = img->GetWidth();
	m_textureResources[ currentFrameIx ][ slotIndex ].height = img->GetHeight();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_textureResources[ currentFrameIx ][ slotIndex ].desc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	m_d3d12device->CreateShaderResourceView( m_textureResources[ currentFrameIx ][ slotIndex ].srv.Get(), &srvDesc, m_textureResources[ currentFrameIx ][ slotIndex ].cpuHandle );

	m_sourceImage[ currentFrameIx ][ slotIndex ] = img;
}


void wtRenderer::IssueTextureCopyCommands( const uint32_t srcFrameIx, const uint32_t renderFrameIx )
{
	ThrowIfFailed( m_cmd.cpyCommandAllocator[ renderFrameIx ]->Reset() );
	ThrowIfFailed( m_cmd.cpyCommandList[ renderFrameIx ]->Reset( m_cmd.cpyCommandAllocator[ renderFrameIx ].Get(), m_pipeline.pso.Get() ) );

	TomBoy::wtFrameResult* fr = &app->frameResult[ srcFrameIx ];

	const uint32_t texturePixelSize = 4;

	for ( uint32_t i = 0; i < MaxTextures; ++i )
	{
		if( m_sourceImage[ currentFrameIx ][ i ] == nullptr ) {
			continue;
		}

		D3D12_SUBRESOURCE_DATA textureData;
		textureData.pData = m_sourceImage[ currentFrameIx ][ i ]->GetRawBuffer();
		textureData.RowPitch = m_textureResources[ renderFrameIx ][ i ].width * static_cast<uint64_t>( texturePixelSize );
		textureData.SlicePitch = textureData.RowPitch * m_textureResources[ renderFrameIx ][ i ].height;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT pLayouts = {};
		pLayouts.Offset = 0;
		pLayouts.Footprint.Depth = 1;
		pLayouts.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		pLayouts.Footprint.Width = m_textureResources[ renderFrameIx ][ i ].width;
		pLayouts.Footprint.Height = m_textureResources[ renderFrameIx ][ i ].height;
		pLayouts.Footprint.RowPitch = static_cast<uint32_t>( textureData.RowPitch );

		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_textureResources[ renderFrameIx ][ i ].srv.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST );
		m_cmd.cpyCommandList[ renderFrameIx ]->ResourceBarrier( 1, &barrier );

		// TODO: use only one intermediate buffer
		UpdateSubresources( m_cmd.cpyCommandList[ renderFrameIx ].Get(), m_textureResources[ renderFrameIx ][ i ].srv.Get(), m_textureResources[ renderFrameIx ][ i ].uploadBuffer.Get(), 0, 0, 1, &textureData );
	
		barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_textureResources[ renderFrameIx ][ i ].srv.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON );
		m_cmd.cpyCommandList[ renderFrameIx ]->ResourceBarrier( 1, &barrier );
	}

	ThrowIfFailed( m_cmd.cpyCommandList[ renderFrameIx ]->Close() );

	ID3D12CommandList* ppCommandLists[] = { m_cmd.cpyCommandList[ renderFrameIx ].Get() };
	m_cmd.d3d12CopyQueue->ExecuteCommandLists( 1, ppCommandLists );
	m_cmd.d3d12CopyQueue->Signal( sync.cpyFence.Get(), sync.fenceValues[ renderFrameIx ] );
}
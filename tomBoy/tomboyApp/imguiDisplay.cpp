#include "renderer_d3d12.h"
#include "audioEngine.h"

static const uint32_t FramePlotSampleCount = 500;
using frameSampleBuffer = wtQueue< float, FramePlotSampleCount >;
static frameSampleBuffer frameTimePlot;

static float ImGuiGetFrameSample( void* data, int32_t idx )
{
	frameSampleBuffer* queue = reinterpret_cast<frameSampleBuffer*>( data );
	return queue->Peek( idx );
}

static float ImGuiGetSoundSample( void* data, int32_t idx )
{
//	wtSampleQueue* queue = reinterpret_cast<wtSampleQueue*>( data );
//	return queue->Peek( idx );
	return 0.0f;
}

void wtRenderer::BuildImguiCommandList()
{
#ifdef IMGUI_ENABLE
	GameboySystem& nesSystem = *app->system->system;
	TomBoy::config_t& systemConfig = app->systemConfig;
	wtAppDebug_t& debugData = app->debugData;
	wtAudioEngine& audio = *app->audio;

	ThrowIfFailed( m_cmd.imguiCommandAllocator[ currentFrameIx ]->Reset() );
	ThrowIfFailed( m_cmd.imguiCommandList[ currentFrameIx ]->Reset( m_cmd.imguiCommandAllocator[ currentFrameIx ].Get(), m_pipeline.pso.Get() ) );

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_cmd.imguiCommandList[ currentFrameIx ]->SetGraphicsRootSignature( m_pipeline.rootSig.Get() );

	ID3D12DescriptorHeap* ppHeaps[] = { m_pipeline.cbvSrvUavHeap.Get() };
	m_cmd.imguiCommandList[ currentFrameIx ]->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );
	m_cmd.imguiCommandList[ currentFrameIx ]->SetGraphicsRootDescriptorTable( 0, m_pipeline.cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart() );

	m_cmd.imguiCommandList[ currentFrameIx ]->RSSetViewports( 1, &view.viewport );
	m_cmd.imguiCommandList[ currentFrameIx ]->RSSetScissorRects( 1, &view.scissorRect );

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_swapChain.renderTargets[ currentFrameIx ].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET );
	m_cmd.imguiCommandList[ currentFrameIx ]->ResourceBarrier( 1, &barrier );

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( m_swapChain.rtvHeap->GetCPUDescriptorHandleForHeapStart(), currentFrameIx, m_swapChain.rtvDescriptorSize );
	m_cmd.imguiCommandList[ currentFrameIx ]->OMSetRenderTargets( 1, &rtvHandle, FALSE, nullptr );

	if ( ImGui::BeginTabBar( "Debug Info" ) )
	{
		bool displayTabOpen = true;
		if ( ImGui::BeginTabItem( "Display Settings", &displayTabOpen ) )
		{
			ImGui::Checkbox( "Enable", &m_pipeline.shaderData.enable );
			ImGui::SliderFloat( "Hard Pix", &m_pipeline.shaderData.hardPix, -4.0f, -2.0f );
			ImGui::SliderFloat( "Hard Scan", &m_pipeline.shaderData.hardScan, -16.0f, -8.0f );
			ImGui::SliderFloat( "Warp X", &m_pipeline.shaderData.warp.x, 0.0f, 1.0f / 8.0f );
			ImGui::SliderFloat( "Warp Y", &m_pipeline.shaderData.warp.y, 0.0f, 1.0f / 8.0f );
			ImGui::SliderFloat( "Mask Light", &m_pipeline.shaderData.maskLight, 0.0f, 2.0f );
			ImGui::SliderFloat( "Mask Dark", &m_pipeline.shaderData.maskDark, 0.0f, 2.0f );
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData( ImGui::GetDrawData(), m_cmd.imguiCommandList[ currentFrameIx ].Get() );

	barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_swapChain.renderTargets[ currentFrameIx ].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT );
	m_cmd.imguiCommandList[ currentFrameIx ]->ResourceBarrier( 1, &barrier );

	ThrowIfFailed( m_cmd.imguiCommandList[ currentFrameIx ]->Close() );
#endif
}
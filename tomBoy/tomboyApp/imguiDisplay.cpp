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
	{
		ThrowIfFailed( cmd.imguiCommandAllocator[ currentFrameIx ]->Reset() );
		ThrowIfFailed( cmd.imguiCommandList[ currentFrameIx ]->Reset( cmd.imguiCommandAllocator[ currentFrameIx ].Get(), pipeline.pso.Get() ) );

		cmd.imguiCommandList[ currentFrameIx ]->SetGraphicsRootSignature( pipeline.rootSig.Get() );

		ID3D12DescriptorHeap* ppHeaps[] = { pipeline.cbvSrvUavHeap.Get() };
		cmd.imguiCommandList[ currentFrameIx ]->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );
		cmd.imguiCommandList[ currentFrameIx ]->SetGraphicsRootDescriptorTable( 0, pipeline.cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart() );

		cmd.imguiCommandList[ currentFrameIx ]->RSSetViewports( 1, &view.viewport );
		cmd.imguiCommandList[ currentFrameIx ]->RSSetScissorRects( 1, &view.scissorRect );

		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition( swapChain.renderTargets[ currentFrameIx ].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET );
		cmd.imguiCommandList[ currentFrameIx ]->ResourceBarrier( 1, &barrier );

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( swapChain.rtvHeap->GetCPUDescriptorHandleForHeapStart(), currentFrameIx, swapChain.rtvDescriptorSize );
		cmd.imguiCommandList[ currentFrameIx ]->OMSetRenderTargets( 1, &rtvHandle, FALSE, nullptr );

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if ( ImGui::BeginTabBar( "Debug Info" ) )
		{
			bool displayTabOpen = true;
			if ( ImGui::BeginTabItem( "Display Settings", &displayTabOpen ) )
			{
				ImGui::Checkbox( "Enable", &pipeline.shaderData.enable );
				ImGui::SliderFloat( "Hard Pix", &pipeline.shaderData.hardPix, -4.0f, -2.0f );
				ImGui::SliderFloat( "Hard Scan", &pipeline.shaderData.hardScan, -16.0f, -8.0f );
				ImGui::SliderFloat( "Warp X", &pipeline.shaderData.warp.x, 0.0f, 1.0f / 8.0f );
				ImGui::SliderFloat( "Warp Y", &pipeline.shaderData.warp.y, 0.0f, 1.0f / 8.0f );
				ImGui::SliderFloat( "Mask Light", &pipeline.shaderData.maskLight, 0.0f, 2.0f );
				ImGui::SliderFloat( "Mask Dark", &pipeline.shaderData.maskDark, 0.0f, 2.0f );
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData( ImGui::GetDrawData(), cmd.imguiCommandList[ currentFrameIx ].Get() );

		barrier = CD3DX12_RESOURCE_BARRIER::Transition( swapChain.renderTargets[ currentFrameIx ].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT );
		cmd.imguiCommandList[ currentFrameIx ]->ResourceBarrier( 1, &barrier );

		ThrowIfFailed( cmd.imguiCommandList[ currentFrameIx ]->Close() );
	}
/*
#ifdef IMGUI_ENABLE
	GameboySystem&		nesSystem		= *app->system;
	TomBoy::config_t&	systemConfig	= app->systemConfig;
	wtAppDebug_t&		debugData		= app->debugData;
	wtAudioEngine&		audio			= *app->audio;

	ThrowIfFailed( cmd.imguiCommandAllocator[ currentFrameIx ]->Reset() );
	ThrowIfFailed( cmd.imguiCommandList[ currentFrameIx ]->Reset( cmd.imguiCommandAllocator[ currentFrameIx ].Get(), pipeline.pso.Get() ) );

	cmd.imguiCommandList[ currentFrameIx ]->SetGraphicsRootSignature( pipeline.rootSig.Get() );

	ID3D12DescriptorHeap* ppHeaps[] = { pipeline.cbvSrvUavHeap.Get() };
	cmd.imguiCommandList[ currentFrameIx ]->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );
	cmd.imguiCommandList[ currentFrameIx ]->SetGraphicsRootDescriptorTable( 0, pipeline.cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart() );

	cmd.imguiCommandList[ currentFrameIx ]->RSSetViewports( 1, &view.viewport );
	cmd.imguiCommandList[ currentFrameIx ]->RSSetScissorRects( 1, &view.scissorRect );

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition( swapChain.renderTargets[ currentFrameIx ].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET );
	cmd.imguiCommandList[ currentFrameIx ]->ResourceBarrier( 1, &barrier );

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( swapChain.rtvHeap->GetCPUDescriptorHandleForHeapStart(), currentFrameIx, swapChain.rtvDescriptorSize );
	cmd.imguiCommandList[ currentFrameIx ]->OMSetRenderTargets( 1, &rtvHandle, FALSE, nullptr );

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	TomBoy::wtFrameResult* fr = &app->frameResult[ app->frameIx ];

	frameTimePlot.EnqueFIFO( fr->dbgInfo.frameTimeUs / 1000.0f );

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData( ImGui::GetDrawData(), cmd.imguiCommandList[ currentFrameIx ].Get() );

	barrier = CD3DX12_RESOURCE_BARRIER::Transition( swapChain.renderTargets[ currentFrameIx ].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT );
	cmd.imguiCommandList[ currentFrameIx ]->ResourceBarrier( 1, &barrier );

	ThrowIfFailed( cmd.imguiCommandList[ currentFrameIx ]->Close() );
#endif
*/
}
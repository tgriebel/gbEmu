/*
* MIT License
*
* Copyright( c ) 2025 Thomas Griebel
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this softwareand associated documentation files( the "Software" ), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions :
*
* The above copyright noticeand this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "interface.h"
#include "gbSystem.h"

namespace TomBoy
{
	config_t DefaultConfig()
	{
		config_t config;

		// System
		config.sys.flags = (emulationFlags_t)( (uint32_t)emulationFlags_t::CLAMP_FPS | (uint32_t)emulationFlags_t::LIMIT_STALL );

		// PPU
		config.ppu.chrPalette = 0;
		config.ppu.showBG = true;
		config.ppu.showSprite = true;

		// APU
		config.apu.frequencyScale = 1.0f;
		config.apu.volume = 1.0f;
		config.apu.waveShift = 0;
		config.apu.disableSweep = false;
		config.apu.disableEnvelope = false;
		config.apu.mutePulse1 = false;
		config.apu.muteTri = false;
		config.apu.muteNoise = false;
		config.apu.muteDMC = false;
		config.apu.dbgChannelBits = 0;

		return config;
	}

	uint32_t ScreenWidth()
	{
		return 160;
	}

	uint32_t ScreenHeight()
	{
		return 144;
	}

	uint32_t SpriteLimit()
	{
		return 16;
	}

	void Shutdown( Emulator* emu )
	{
		if ( emu == nullptr ) {
			return;
		}
		if ( system != nullptr ) {
			delete emu->system;
		}
	}

	bool Boot( Emulator* emu, const wchar_t* filePath )
	{
		if ( emu == nullptr ) {
			return false;
		}
		if ( emu->system != nullptr ) {
			delete emu->system;
		}
		emu->system = new GameboySystem();
		const int ret = emu->system->Init( filePath );
		if ( ret == 0 )
		{
			emu->system->AttachInputHandler( &emu->input );
			return true;
		}
		return false;
	}

	int RunEpoch( Emulator* emu, const std::chrono::nanoseconds& runCycles )
	{
		if ( emu == nullptr ) {
			return 0;
		}
		return emu->system->RunEpoch( runCycles );
	}

	void GetFrameResult( Emulator* emu, wtFrameResult& outFrameResult )
	{
		if ( emu == nullptr ) {
			return;
		}
		emu->system->GetFrameResult( outFrameResult );
	}

	void UpdateDebugImages( Emulator* emu )
	{
		if ( emu == nullptr ) {
			return;
		}
		emu->system->UpdateDebugImages();
	}

	void SetConfig( Emulator* emu, config_t& cfg )
	{
		if ( emu == nullptr ) {
			return;
		}
		emu->system->SetConfig( cfg );
	}

	ButtonFlags GetKeyBuffer( const Input* input )
	{
		return input->keyBuffer;
	}

	mouse_t GetMouse( const Input* input )
	{
		return input->mousePoint;
	}

	void BindKey( Input* input, const char key, const ButtonFlags button )
	{
		input->keyMap[ key ].buttonFlags = button;
	}

	void StoreKey( Input* input, const uint32_t key )
	{
		keyBinding_t keyBinding = input->keyMap[ key ];
		input->keyBuffer = input->keyBuffer | static_cast<ButtonFlags>( keyBinding.buttonFlags );
	}

	void ReleaseKey( Input* input, const uint32_t key )
	{
		keyBinding_t keyBinding = input->keyMap[ key ];
		input->keyBuffer = input->keyBuffer & static_cast<ButtonFlags>( ~static_cast<uint8_t>( keyBinding.buttonFlags ) );
	}

	void StoreMouseClick( Input* input, const int32_t x, const int32_t y )
	{
		input->mousePoint = mouse_t( { x, y } );
	}

	void ClearMouseClick( Input* input )
	{
		input->mousePoint = mouse_t( { -1, -1 } );
	}
}
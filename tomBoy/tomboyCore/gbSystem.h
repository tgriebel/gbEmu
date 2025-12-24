#pragma once

#include <stdint.h>
#include <assert.h>
#include "common.h"
#include "gbCart.h"
#include "z80.h"
#include "ppu.h"
#include "interface.h"
#include "image.h"

//using namespace TomBoy;

#define BG_TILE_DEBUG 1

class GameboySystem
{
public:
	// Memory Sizes
	static const uint32_t VirtualMemorySize		= 0x10000;
	static const uint32_t PhysicalMemorySize	= 0x2000;
	static const uint32_t RomBankSize			= 0x4000;
	static const uint32_t RamSize				= 0x2000;
	static const uint32_t WorkRamSize			= 0x1000;
	static const uint32_t VRamSize				= 0x2000;
	static const uint32_t ExtRamSize			= 0x2000;
	static const uint32_t AttributeSize			= 0x0100;
	static const uint32_t IoSize				= 0x004C;
	static const uint32_t HighRamSize			= 0x007F;

	// Banks and Memory-maps
	static const uint32_t RomBank0				= 0x0000;	// Fixed bank
	static const uint32_t RomBankN				= 0x4000;	// Switched by mapper
	static const uint32_t RomBankEnd			= 0x7FFF;
	static const uint32_t VRamBank				= 0x8000;	// Switchable 0-1
	static const uint32_t VRamBankEnd			= 0x9FFF;
	static const uint32_t ExtRamBank			= 0xA000;	// Switchable
	static const uint32_t ExtRamBankEnd			= 0xBFFF;
	static const uint32_t WorkRamBank0			= 0xC000;	// Fixed RAM
	static const uint32_t WorkRamBankEnd0		= 0xCFFF;
	static const uint32_t WorkRamBank1			= 0xD000;	// Switchable 1-7
	static const uint32_t WorkRamBankEnd1		= 0xDFFF;
	static const uint32_t EchoRegion			= 0xE000;	// Illegal
	static const uint32_t EchoRegionEnd			= 0xFDFF;
	static const uint32_t OamMemory				= 0xFE00;	// Sprites
	static const uint32_t OamMemoryEnd			= 0xFE9F;
	static const uint32_t IllegalRegion			= 0xFEA0;	// Illegal
	static const uint32_t IllegalRegionEnd		= 0xFEFF;
	static const uint32_t Mmio					= 0xFF00;	// Memory-mapped IO
	static const uint32_t MmioEnd				= 0xFF7F;
	static const uint32_t Hram					= 0xFF80;	// High RAM
	static const uint32_t HramEnd				= 0xFFFE;
	static const uint32_t InterruptFlagAddr		= 0xFF0F;	// Hardware interrupt flag register
	static const uint32_t InterruptEnableAddr	= 0xFFFF;	// Hardware interrupt enable register

	// MMIO
	static const uint32_t Joypad				= 0xFF00;
	static const uint32_t Serial0				= 0xFF01;
	static const uint32_t Serial1				= 0xFF02;
	static const uint32_t TimerandDivStart		= 0xFF04;
	static const uint32_t TimerandDivEnd		= 0xFF07; 
	static const uint32_t Interrupt				= 0xFF0F;
	static const uint32_t AudioStart			= 0xFF10;
	static const uint32_t AudioEnd				= 0xFF26;
	static const uint32_t WaveStart				= 0xFF30;
	static const uint32_t WaveEnd				= 0xFF3F;
	static const uint32_t LcdStart				= 0xFF40;
	static const uint32_t LcdEnd				= 0xFF4B;
	static const uint32_t LcdControl			= 0xFF40;
	static const uint32_t LcdStatus				= 0xFF41;
	static const uint32_t LcdScreenPos0			= 0xFF42;
	static const uint32_t LcdScreenPos1			= 0xFF43;
	static const uint32_t LcdLy					= 0xFF44;
	static const uint32_t LcdLyc				= 0xFF45;
	static const uint32_t LcdDma				= 0xFF46;
	static const uint32_t LcdBgPal				= 0xFF47;
	static const uint32_t LcdObjPal0			= 0xFF48;
	static const uint32_t LcdObjPal1			= 0xFF49;
	static const uint32_t LcdWindowPos0			= 0xFF4A;
	static const uint32_t LcdWindowPos1			= 0xFF4B;
	static const uint32_t VramBankSelect		= 0xFF4F;
	static const uint32_t BootDisable			= 0xFF50;
	static const uint32_t VramDmaStart			= 0xFF51;
	static const uint32_t VramDmaEnd			= 0xFF55;
	static const uint32_t IoStart				= 0xFF00;
	static const uint32_t IoEnd					= 0xFF7F;
	static const uint32_t HRamStart				= 0xFF80;
	static const uint32_t HRamEnd				= 0xFFFE;
	static const uint32_t PaletteStart			= 0xFF68;
	static const uint32_t PaletteEnd			= 0xFF6B;
	static const uint32_t WRamSelect			= 0xFF70;

	static const uint32_t OutputBuffersCount	= 3;

	std::wstring			fileName;
	std::wstring			baseFileName;

	TomBoy::wtRawImage<160, 144>	frameBuffer[ OutputBuffersCount ];
	TomBoy::wtRawImage<256, 256>	bgMap;
	TomBoy::wtRawImage<128, 64>		patternTable0;
	TomBoy::wtRawImage<128, 64>		patternTable1;

	CpuZ80					cpu;
	PPU						ppu;
	std::unique_ptr<gbCart>	cart;
	uint8_t					memory[ 1024 * 16 ];
	uint8_t					hram[ HighRamSize ];
	uint8_t					wram[ 8 ][ WorkRamSize ];
	uint8_t					vram[ VRamSize ];
	uint8_t					oam[ AttributeSize ];
	int64_t					overflowCycles;
	masterCycle_t			sysCycles;
	//const Input*			input;
	const TomBoy::config_t*	config;

	bool					toggledFrame;
	uint64_t				frameNumber;

	GameboySystem()
	{
		Reset();
	}


	GameboySystem( const std::wstring& filePath )
	{
		Init( filePath );
	}


	~GameboySystem()
	{
		Shutdown();
	}

	void Reset()
	{
		sysCycles = masterCycle_t( 0 );

		toggledFrame = false;
		frameNumber = 0;

		cpu.RegisterSystem( this );
		ppu.RegisterSystem( this );

		memset( memory, 0, 1024 * 16 );
		memset( hram, 0, HighRamSize );
		memset( wram, 0, 8 * WorkRamSize );
		memset( vram, 0, VRamSize );
		memset( oam, 0, AttributeSize );
	}

	int		Init( const std::wstring& filePath );
	void	Shutdown();
	bool	Run( const masterCycle_t& nextCycle );
	int		RunEpoch( const std::chrono::nanoseconds& runCycles );

	uint8_t	GetTilePalette( const uint8_t plane0, const uint8_t plane1, const uint8_t col );
	void	DrawTile( TomBoy::wtRawImage<128, 64>& tileMap, const uint32_t xOffset, const uint32_t yOffset, const int mode, const uint8_t tileId );
	void	UpdateDebugImages();

	void	GetFrameResult( TomBoy::wtFrameResult& outFrameResult );

	bool HasNewFrame() const
	{
		return toggledFrame;
	}

	void ToggleFrame()
	{
#if 0
		// Debug code. Should never see red flashes in final display
		frameBuffer[ currentFrameIx ].Clear( 0xFF0000FF );
#endif
		frameNumber++;
	}

	void RequestVBlankInterrupt()
	{
		cpu.IF.vblank = 1;
	}


	void ClearVBlankInterrupt()
	{
		cpu.IF.vblank = 0;
	}

	void LoadProgram()
	{
		cart->mapper = AssignMapper( cart->GetMapperId() );
		cart->mapper->system = this;
		cart->mapper->OnLoadCpu();
		cart->mapper->OnLoadPpu();
	}

	static inline bool IsRomBank( const uint16_t address )
	{
		return ( address <= RomBankEnd ) || InRange( address, ExtRamBank, ExtRamBankEnd );
	}

	static inline uint16_t MirrorAddress( const uint16_t address )
	{
		if ( InRange( address, EchoRegion, EchoRegionEnd ) )
		{
			return address - 0x2000;
		}
		return address;
	}

	void SetConfig( TomBoy::config_t& systemConfig )
	{
		config = &systemConfig;
	}

	uint8_t ReadMemory( const uint16_t address );

	void WriteMemory( const uint16_t address, const uint16_t offset, const uint8_t value );

	uint8_t& GetStack()
	{
		if( InRange( cpu.SP, WorkRamBank0, WorkRamBankEnd0 ) ) {
			return wram[ 0 ][ cpu.SP - WorkRamBank0 ];
		} else if ( InRange( cpu.SP, WorkRamBank1, WorkRamBankEnd1 ) ) {
			return wram[ 1 ][ cpu.SP - WorkRamBank1 ];
		}
		assert( 0 ); // Probably a bug, but possible error in game ROM
		return wram[ 0 ][ 0 ];
	}

	// mapper.h
	std::unique_ptr<gbMapper> AssignMapper( const uint32_t mapperId );
};
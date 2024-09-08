#pragma once

#include <stdint.h>
#include <assert.h>
#include "common.h"
#include "gbCart.h"
#include "z80.h"
#include "interface.h"

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
	static const uint32_t InterruptRegister		= 0xFFFF;	// Hardware interrupt register

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

	CpuZ80					cpu;
	unique_ptr<gbCart>		cart;
	uint8_t					memory[ 1024 * 16 ];
	uint8_t					hram[ HighRamSize ];
	uint8_t					wram[ 8 ][ WorkRamSize ];
	uint8_t					vram[ VRamSize ];
	uint8_t					oam[ AttributeSize ];

	GameboySystem()
	{
		memset( memory, 0, 1024 * 16 );
		memset( hram, 0, HighRamSize );
		memset( wram, 0, 8 * WorkRamSize );
		memset( vram, 0, VRamSize );
		memset( oam, 0, AttributeSize );
	}

	void LoadProgram()
	{

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

	uint8_t ReadMemory( const uint16_t address );

	void WriteMemory( const uint16_t address, const uint16_t offset, const uint8_t value );

	uint8_t& GetStack()
	{
		if( InRange( cpu.SP, WorkRamBank0, WorkRamBankEnd0 ) ) {
			wram[ 0 ][ cpu.SP ];
		} else if ( InRange( cpu.SP, WorkRamBank1, WorkRamBankEnd1 ) ) {
			return wram[ 1 ][ cpu.SP ];
		}
		assert( 0 ); // Probably a bug, but possible error in game ROM
		return wram[ 0 ][ 0 ];
	}

	// mapper.h
	unique_ptr<gbMapper> AssignMapper( const uint32_t mapperId );
};
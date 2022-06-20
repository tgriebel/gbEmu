#pragma once

#include <stdint.h>
#include <assert.h>
#include "common.h"
#include "gbCart.h"
#include "z80.h"

class GameboySystem
{
public:
	// Buffer and partition sizes
	static const uint32_t VirtualMemorySize		= 0x10000;
	static const uint32_t PhysicalMemorySize	= 0x2000;
	static const uint32_t RomBankSize			= 0x4000;
	static const uint32_t RamSize				= 0x2000;
	static const uint32_t AttributeSize			= 0x0100;
	static const uint32_t IoSize				= 0x004C;
	static const uint32_t HighRamSize			= 0x007F;

	CpuZ80					cpu;
	unique_ptr<gbCart>		cart;
	uint8_t					memory[ 1024 * 16 ];

	void LoadProgram()
	{
		//memset( memory, 0, PhysicalMemorySize );

		//cart->mapper = AssignMapper( cart->GetMapperId() );
		//cart->mapper->system = this;
		//cart->mapper->OnLoadCpu();
		//cart->mapper->OnLoadPpu();

		//if ( resetVectorManual == 0x10000 ) {
		//	cpu.resetVector = Combine( ReadMemory( ResetVectorAddr ), ReadMemory( ResetVectorAddr + 1 ) );
		//}
		//else {
		//	cpu.resetVector = static_cast<uint16_t>( resetVectorManual & 0xFFFF );
		//}

		//cpu.nmiVector = Combine( ReadMemory( NmiVectorAddr ), ReadMemory( NmiVectorAddr + 1 ) );
		//cpu.irqVector = Combine( ReadMemory( IrqVectorAddr ), ReadMemory( IrqVectorAddr + 1 ) );
		//cpu.PC = cpu.resetVector;

		//if ( cart->h.controlBits0.fourScreenMirror ) {
		//	mirrorMode = MIRROR_MODE_FOURSCREEN;
		//} else if ( cart->h.controlBits0.mirror ) {
		//	mirrorMode = MIRROR_MODE_VERTICAL;
		//} else {
		//	mirrorMode = MIRROR_MODE_HORIZONTAL;
		//}
	}

	bool IsRomBank( const uint16_t address ) {
		return ( address <= RomBankSize );
	}

	uint8_t ReadMemory( const uint16_t address ) {
		if ( IsRomBank( address ) ) {
		} else {
			return memory[ address ];
		}
	}

	void WriteMemory( const uint16_t address, const uint16_t offset, const uint8_t value ) {
		memory[ address + offset ] = value;
	}

	uint8_t& GetStack()
	{
		assert( cpu.SP < PhysicalMemorySize );
		return memory[ cpu.SP ];
	}
};
#pragma once

#include <stdint.h>
#include <assert.h>
#include "common.h"

class GameboySystem
{
public:
	static const uint32_t VirtualMemorySize		= 0x10000;
	static const uint32_t PhysicalMemorySize	= 0x2000;

	uint8_t memory[ 1024 * 16 ];

	uint8_t ReadMemory( const uint16_t address ) {
		return memory[ address ];
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
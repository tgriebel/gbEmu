#pragma once

#include <stdint.h>
#include "common.h"

class GameboySystem
{
public:
	uint8_t memory[ 1024 * 16 ];

	uint8_t					ReadMemory( const uint16_t address ) {
		return memory[ address ];
	}

	void					WriteMemory( const uint16_t address, const uint16_t offset, const uint8_t value ) {
		memory[ address + offset ] = value;
	}
};
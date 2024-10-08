#pragma once

#include "common.h"
#include "gbCart.h"

class NoMBC : public gbMapper
{
private:
	uint8_t* prgBank[ 2 ];
public:
	NoMBC( const uint32_t _mapperId )
	{
		prgBank[ 0 ] = nullptr;
		prgBank[ 1 ] = nullptr;
	}

	uint8_t OnLoadCpu() override
	{
		prgBank[ 0 ] = system->cart->GetPrgRomBank( 0 );
		prgBank[ 1 ] = system->cart->GetPrgRomBank( 1 );
		return 0;
	};

	uint8_t OnLoadPpu() override
	{
		return 0;
	};

	uint8_t	ReadRom( const uint16_t addr ) const override
	{
		const uint8_t bank = ( addr >> 14 );
		const uint16_t offset = ( addr & ( GameboySystem::RomBankSize - 1 ) );

		return prgBank[ bank ][ offset ];
	}
};
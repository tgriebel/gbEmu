#pragma once

#include "common.h"
#include "gbCart.h"

class NoMBC : public gbMapper
{
private:
	uint8_t* prgBank;
	uint8_t* chrBank;
public:
	NoMBC( const uint32_t _mapperId )
	{
		prgBank = nullptr;
		chrBank = nullptr;
	}

	uint8_t OnLoadCpu() override
	{
		prgBank = system->cart->GetPrgRomBank( 0 );
		return 0;
	};

	uint8_t OnLoadPpu() override
	{
		return 0;
	};

	uint8_t	ReadRom( const uint16_t addr ) const override
	{
		return prgBank[ addr ];
	}

	uint8_t	ReadChrRom( const uint16_t addr ) const override
	{
		return chrBank[ addr ];
	}
};
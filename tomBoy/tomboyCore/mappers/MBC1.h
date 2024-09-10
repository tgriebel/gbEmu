#pragma once

#include "common.h"
#include "gbCart.h"

class MBC1 : public gbMapper
{
private:
	uint32_t bankNum;
	uint32_t ramBankNum;
	uint8_t* prgBank[ 2 ];
public:
	MBC1( const uint32_t _mapperId )
	{
		bankNum = 0;
		ramBankNum = 0;

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

	void WriteRam( const uint16_t addr, const uint8_t value )
	{
		if( InRange( addr, 0x2000, 0x3FFF ) )
		{
			bankNum = ( value & 0x1F );
			bankNum = ( bankNum == 0 ) ? 1 : bankNum;
		}
		else if ( InRange( addr, 0x4000, 0x5FFF ) )
		{
			ramBankNum;
			assert( 0 );
		}
		else if ( InRange( addr, 0x6000, 0x7FFF ) )
		{
			assert( 0 );
		}
	};

	uint8_t	ReadRom( const uint16_t addr ) const override
	{
		const uint8_t bank = ( addr >> 14 );
		const uint16_t offset = ( addr & ( GameboySystem::RomBankSize - 1 ) );

		return prgBank[ bank ][ offset ];
	}
};
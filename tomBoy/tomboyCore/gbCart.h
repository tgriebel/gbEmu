#pragma once
#include "common.h"

struct gbRomHeader
{
	uint8_t	entry[ 4 ];
	uint8_t logo[ 48 ];
	uint8_t title[ 11 ];
	uint8_t manufacturer[ 4 ];
	uint8_t cgbFlag;
	uint8_t newLicenseeCode[ 2 ];
	uint8_t sgbFlag;
	uint8_t type;
	uint8_t romSize;
	uint8_t ramSize;
	uint8_t regionCode;
	uint8_t oldLicenseeCode;
	uint8_t maskRomVersion;
	uint8_t headerChecksum;
	uint8_t globalChecksum[ 2 ];
};

class gbCart
{
private:
	uint8_t*				rom;
	size_t					size;

public:
	gbRomHeader				h;

	gbCart()
	{
		memset( &h, 0, sizeof( gbRomHeader ) );
		rom = nullptr;
		size = 0;
	}

	gbCart( gbRomHeader& header, uint8_t* romData, uint32_t romSize )
	{
		rom = new uint8_t[ romSize ];
	}

	~gbCart()
	{
		memset( &h, 0, sizeof( gbRomHeader ) );
		if ( rom != nullptr )
		{
			delete[] rom;
			rom = nullptr;
		}
		size = 0;
	}
};
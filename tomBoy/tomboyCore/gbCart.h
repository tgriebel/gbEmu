#pragma once
#include "common.h"

class GameboySystem;

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

class gbMapper
{
protected:
	uint32_t mapperId;

public:
	GameboySystem* system;

	virtual uint8_t			OnLoadCpu() { return 0; };
	virtual uint8_t			OnLoadPpu() { return 0; };
	virtual uint8_t			ReadRom( const uint16_t addr ) const = 0;
	virtual void			WriteRam( const uint16_t addr, const uint8_t value ) {};
	virtual bool			InWriteWindow( const uint16_t addr, const uint16_t offset ) const { return false; };

	//virtual void			Serialize( Serializer& serializer ) {};
	//virtual void			Clock() {};
};



class gbCart
{
private:
	uint8_t*				rom;
	size_t					size;

public:
	gbRomHeader				h;
	shared_ptr<gbMapper>	mapper;

	gbCart()
	{
		memset( &h, 0, sizeof( gbRomHeader ) );
		rom = nullptr;
		size = 0;
	}

	gbCart( gbRomHeader& header, uint8_t* romData, uint32_t romSize )
	{
		h = header;
		rom = new uint8_t[ romSize ];
		memcpy( rom, romData, romSize );
		size = romSize;
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

	uint8_t* GetPrgRomBank( const uint32_t bankNum, const uint32_t bankSize = KB( 16 ) )
	{
		const size_t addr = ( bankNum * (size_t)bankSize );
		assert( addr < size );
		return &rom[ addr ];
	}

	uint32_t GetMapperId() const {
		return h.type;
	}
};
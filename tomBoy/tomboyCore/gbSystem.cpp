#include "gbSystem.h"
#include "mappers/NoMBC.h"

unique_ptr<gbMapper> GameboySystem::AssignMapper( const uint32_t mapperId )
{
	switch ( mapperId )
	{
	default:
	case 0: return make_unique<NoMBC>( mapperId );	break;
	}
}

/*static*/ void LoadGameboyFile( const std::wstring& fileName, unique_ptr<gbCart>& outCart )
{
	std::ifstream gbFile;
	gbFile.open( fileName, std::ios::binary );

	assert( gbFile.good() );

	gbFile.seekg( 0, std::ios::end );
	uint32_t size = static_cast<uint32_t>( gbFile.tellg() );

	const uint32_t headerOffset = 0x0100;	
	const uint32_t headerSize = 0x50;
	uint8_t headerBytes[ 100 ];

	uint8_t* romData = new uint8_t[ size ];

	gbFile.seekg( headerOffset, std::ios::beg );
	gbFile.read( reinterpret_cast<char*>( &headerBytes ), headerSize );
	gbFile.seekg( 0, std::ios::beg );
	gbFile.read( reinterpret_cast<char*>( romData ), size );
	gbFile.close();

	uint8_t chkSum = 0;
	for ( int i = 0x34; i < 0x4D; ++i ) {
		chkSum = chkSum - headerBytes[ i ] - 1;
	}

	gbRomHeader header;
	memcpy( &header, headerBytes, headerSize );
	assert( header.headerChecksum == chkSum ); // FIXME: assume all loaded carts must be valid

	outCart = make_unique<gbCart>( header, romData, size );

	delete[] romData;
}
#include "gbSystem.h"

/*static*/ void LoadGameboyFile( const std::wstring& fileName, unique_ptr<gbCart>& outCart )
{
	std::ifstream gbFile;
	gbFile.open( fileName, std::ios::binary );

	assert( gbFile.good() );

	gbFile.seekg( 0, std::ios::end );
	uint32_t len = static_cast<uint32_t>( gbFile.tellg() );

	const uint32_t headerOffset = 0x0100;	
	const uint32_t headerSize = 0x50;
	uint8_t headerBytes[ 100 ];

	uint32_t size = len - headerSize - headerOffset;
	uint8_t* romData = new uint8_t[ size ];

	gbFile.seekg( headerOffset, std::ios::beg );
	gbFile.read( reinterpret_cast<char*>( &headerBytes ), headerSize );
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
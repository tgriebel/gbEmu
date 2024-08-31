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

	const unsigned char logoCheck[] = {	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D
										, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99
										, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E };

	outCart = make_unique<gbCart>( header, romData, size );

	delete[] romData;
}
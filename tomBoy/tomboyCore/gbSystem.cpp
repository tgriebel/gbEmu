#include "gbSystem.h"
#include "mappers/NoMBC.h"
#include "mappers/MBC1.h"

unique_ptr<gbMapper> GameboySystem::AssignMapper( const uint32_t mapperId )
{
	switch ( mapperId )
	{
		default:
		case 0: return make_unique<NoMBC>( mapperId );	break;
		case 1: return make_unique<MBC1>( mapperId );	break;
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


uint8_t GameboySystem::ReadMemory( const uint16_t address )
{
	const uint16_t mAddr = MirrorAddress( address );

	if ( IsRomBank( mAddr ) )
	{
		return cart->mapper->ReadRom( address );
	}
	else if ( InRange( mAddr, VRamBank, VRamBankEnd ) )
	{
		return vram[ mAddr - VRamBank ];
	}
	else if ( InRange( mAddr, WorkRamBank0, WorkRamBankEnd0 ) )
	{
		return wram[ 0 ][ mAddr - WorkRamBank0 ];
	}
	else if ( InRange( mAddr, WorkRamBank1, WorkRamBankEnd1 ) )
	{
		return wram[ 1 ][ mAddr - WorkRamBank1 ];
	}
	else if ( InRange( mAddr, OamMemory, OamMemoryEnd ) )
	{
		return oam[ mAddr - OamMemory ];
	}
	else if ( InRange( mAddr, IllegalRegion, IllegalRegionEnd ) )
	{
	}
	else if ( InRange( mAddr, Hram, HramEnd ) )
	{
		return hram[ mAddr - HRamStart ];
	}
	else if ( InRange( mAddr, Interrupt ) ) {
	}
	else if ( InRange( mAddr, Mmio, MmioEnd ) )
	{
		if ( InRange( mAddr, Joypad ) ) {
		}
		else if ( InRange( mAddr, Serial0, Serial1 ) ) {
		}
		else if ( InRange( mAddr, TimerandDivStart, TimerandDivEnd ) ) {
		}
		else if ( InRange( mAddr, AudioStart, AudioEnd ) ) {
		}
		else if ( InRange( mAddr, WaveStart, WaveEnd ) ) {
		}
		else if ( InRange( mAddr, PaletteStart, PaletteEnd ) ) {
		}
		else if ( InRange( mAddr, VramDmaStart, VramDmaEnd ) ) {
		}
		else if ( InRange( mAddr, LcdStart, LcdEnd ) ) {
			static uint8_t ly = 0;
			ly++; // FIXME: stub
			return 0x90;
		}
	}
	else
	{
		return memory[ address ];
	}
	return 0;
}


void GameboySystem::WriteMemory( const uint16_t address, const uint16_t offset, const uint8_t value )
{
	const uint16_t mAddr = MirrorAddress( address + offset );

	if ( IsRomBank( mAddr ) )
	{
		cart->mapper->WriteRam( mAddr, value );
	}
	else if ( InRange( mAddr, VRamBank, VRamBankEnd ) )
	{ 
		vram[ mAddr - VRamBank ] = value;
	//	cout << hex << mAddr << ": " << hex << uint32_t( value ) << endl;
	}
	else if ( InRange( mAddr, WorkRamBank0, WorkRamBankEnd0 ) )
	{
		wram[ 0 ][ mAddr - WorkRamBank0 ] = value;
	}
	else if ( InRange( address, WorkRamBank1, WorkRamBankEnd1 ) )
	{
		wram[ 1 ][ mAddr - WorkRamBank1 ] = value;
	}
	else if ( InRange( mAddr, OamMemory, OamMemoryEnd ) )
	{
		oam[ mAddr - OamMemory ] = value;
	}
	else if ( InRange( mAddr, IllegalRegion, IllegalRegionEnd ) )
	{
	}
	else if ( InRange( mAddr, Hram, HramEnd ) )
	{
		hram[ mAddr - HRamStart ] = value;
	}
	else if ( InRange( mAddr, Interrupt ) ) {
	}
	else if ( InRange( mAddr, Mmio, MmioEnd ) )
	{
		if ( InRange( mAddr, Joypad ) ) {
		}
		else if ( InRange( mAddr, Serial0, Serial1 ) ) {
		}
		else if ( InRange( mAddr, TimerandDivStart, TimerandDivEnd ) ) {
		}
		else if ( InRange( mAddr, AudioStart, AudioEnd ) ) {
		}
		else if ( InRange( mAddr, WaveStart, WaveEnd ) ) {
		}
		else if ( InRange( mAddr, PaletteStart, PaletteEnd ) ) {
		}
		else if ( InRange( mAddr, VramDmaStart, VramDmaEnd ) ) {
		}
		else if ( InRange( mAddr, LcdStart, LcdEnd ) ) {
		}
	}
	else
	{
		memory[ mAddr ] = value;
	}
}
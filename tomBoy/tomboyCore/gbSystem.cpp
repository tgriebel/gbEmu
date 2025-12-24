#include "gbSystem.h"
#include "mappers/NoMBC.h"
#include "mappers/MBC1.h"

#include <iostream>
#include <fstream>
#include <string>
#include <memory>

using namespace TomBoy;
using namespace std;

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
	ifstream gbFile;
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
			if( InRange( mAddr, 0xFF44 ) ) {
				return ppu.LY();
			}
		}
	}
	else if ( InRange( mAddr, InterruptFlagAddr ) )
	{
		return cpu.IF.byte;
	}
	else if ( InRange( mAddr, InterruptEnableAddr ) )
	{
		return cpu.IE.byte;
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
	else if( InRange( mAddr, InterruptFlagAddr ) )
	{
		cpu.IF.byte = value;
	}
	else if ( InRange( mAddr, InterruptEnableAddr ) )
	{
		cpu.IE.byte = value;
	}
	else
	{
		memory[ mAddr ] = value;
	}
}


void GameboySystem::GetFrameResult( TomBoy::wtFrameResult& outFrameResult )
{
	outFrameResult.frameBuffer = &frameBuffer[ 0 ];
	outFrameResult.bgMap = &bgMap;
	outFrameResult.patternTable0 = &patternTable0;
	outFrameResult.patternTable1 = &patternTable1;

	outFrameResult.currentFrame = frameNumber;
}


int GameboySystem::Init( const wstring& filePath )
{
	Reset();

	LoadGameboyFile( filePath, cart );

	ppu.Reset();
	ppu.RegisterSystem( this );

	//apu.Reset();
	//apu.RegisterSystem( this );

	cpu.Reset();
	cpu.RegisterSystem( this );

	LoadProgram();
	fileName = filePath;

	const size_t offset = fileName.find( L".gb", 0 );
	baseFileName = fileName.substr( 0, offset );

	return 0;
}


void GameboySystem::Shutdown()
{
//	SaveSRam();
}


bool GameboySystem::Run( const masterCycle_t& nextCycle )
{
	bool isRunning = true;

	static const masterCycle_t ticks( CpuClockDivide );

	// TODO: CHECK WRAP AROUND LOGIC
	while ( ( sysCycles < nextCycle ) && isRunning )
	{
		sysCycles += ticks;

		const cpuCycle_t nextCpuCycle = MasterToCpuCycle( sysCycles );
		const ppuCycle_t nextPpuCycle = MasterToPpuCycle( sysCycles );

		isRunning = cpu.Step( nextCpuCycle );
		ppu.Step( nextPpuCycle );
	}

#if DEBUG_MODE == 1
	dbgInfo.masterCpu = chrono::duration_cast<masterCycle_t>( cpu.cycle );
	dbgInfo.masterPpu = chrono::duration_cast<masterCycle_t>( ppu.cycle );
	dbgInfo.masterApu = chrono::duration_cast<masterCycle_t>( apu.cpuCycle );
#endif // #if DEBUG_MODE == 1

#if DEBUG_ADDR == 1
	if ( cpu.IsTraceLogOpen() )
	{
		cpu.logFrameCount--;
		cpu.dbgLog.NewFrame();
	}
#endif // #if DEBUG_ADDR == 1

	return isRunning;
}


int GameboySystem::RunEpoch( const std::chrono::nanoseconds& runEpoch )
{
	const nano_t e = nano_t( runEpoch.count() );

	masterCycle_t cyclesPerFrame = masterCycle_t( overflowCycles );
	overflowCycles = 0;

	const bool clampFps = ( config->sys.flags & emulationFlags_t::CLAMP_FPS ) != 0;
	const bool stallLimit = ( config->sys.flags & emulationFlags_t::LIMIT_STALL ) != 0;

	if ( ( runEpoch > MaxFrameLatencyNs ) && stallLimit ) // Clamp simulation catch-up for hitches/debugging
	{
		cyclesPerFrame = NanoToCycle( MaxFrameLatencyNs.count() );
	}
	else if ( ( runEpoch > FrameLatencyNs ) && clampFps ) // Don't skip frames, instead even out bubbles over a few frames
	{
		cyclesPerFrame = NanoToCycle( FrameLatencyNs.count() );
		overflowCycles = ( runEpoch - FrameLatencyNs ).count();
	}
	else
	{
		cyclesPerFrame = masterCycle_t( NanoToCycle( e ) );
	}

	const masterCycle_t startCycle = sysCycles;
	const masterCycle_t nextCycle = sysCycles + cyclesPerFrame;

	toggledFrame = false;

	Timer emuTime;
	emuTime.Start();
	bool isRunning = Run( nextCycle );
	emuTime.Stop();

	// FIXME: TEMP until the PPU is finished
	if(( startCycle.count() / cyclesPerFrame.count() ) != ( nextCycle.count() / cyclesPerFrame.count() ))
	{
		ToggleFrame();
	}

	const masterCycle_t endCycle = sysCycles;
	overflowCycles += ( endCycle - nextCycle ).count();

	if ( ( config->sys.flags & emulationFlags_t::HEADLESS ) != 0 ) {
		return isRunning;
	}

	return isRunning;
}


uint8_t GameboySystem::GetTilePalette( const uint8_t plane0, const uint8_t plane1, const uint8_t col )
{
	const uint8_t planeBit0 = ( plane0 >> ( 7 - col ) ) & 0x01;
	const uint8_t planeBit1 = ( ( plane1 >> ( 7 - col ) ) & 0x01 ) << 1;

	return ( planeBit0 | planeBit1 );
}


void GameboySystem::DrawTile( TomBoy::wtRawImage<128, 64>& tileMap, const uint32_t xOffset, const uint32_t yOffset, const int mode, const uint8_t tileId )
{
	uint16_t baseAddr = 0;
	if ( mode == 0 ) {
		baseAddr = 16 * tileId;
	}
	else if ( mode == 1 ) {
		baseAddr = 0x1000 + 16 * u8i8( tileId ).i8;
	}
	else if ( mode == 2 ) {
		baseAddr = 16 * tileId;
	}
	else if ( mode == 3 ) {
		baseAddr = 16 * tileId + 0x0800;
	}
	else if ( mode == 4 ) {
		baseAddr = 16 * tileId + 0x1000;
	}

	uint32_t debugPalette[ 4 ] = { 0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000 };

	for ( uint32_t y = 0; y < 8; ++y )
	{
		const uint16_t rowOffset = 2 * y;
		const uint8_t loByte = vram[ baseAddr + rowOffset + 0 ];
		const uint8_t hiByte = vram[ baseAddr + rowOffset + 1 ];

		for ( int x = 0; x < 8; ++x )
		{
			const uint8_t palId = GetTilePalette( loByte, hiByte, x );

			Pixel px;
			px.rawABGR = debugPalette[ palId ];
			
			tileMap.SetPixel( xOffset + x, tileMap.GetHeight() - ( yOffset + y + 1 ), px );
		}
	}
}


void GameboySystem::UpdateDebugImages()
{
#define TILEMAP_DEBUG 1
#define BG_DEBUG 0

#if TILEMAP_DEBUG
	{
		for ( uint32_t bank = 0; bank < 3; ++bank )
		{
			for ( uint32_t tileId = 0; tileId < 128; ++tileId )
			{
				const uint32_t pixelOffsetX = ( tileId % 16 ) * 8;
				const uint32_t pixelOffsetY = ( tileId / 16 ) * 8;

				DrawTile( patternTable0, pixelOffsetX, pixelOffsetY, bank + 2, tileId );
			}
		}
	}
#endif

#if BG_DEBUG
	{
		for ( uint32_t tileY = 0; tileY < 32; ++tileY ) {
			for ( uint32_t tileX = 0; tileX < 32; ++tileX ) {
				const uint32_t pixelOffsetX = tileX * 8;
				const uint32_t pixelOffsetY = tileY * 8;

				const uint8_t tileId = vram[ 0x1800 + tileY * 32 + tileX ];
				DrawTile( patternTable0, pixelOffsetX, pixelOffsetY, 1, tileId );
			}
		}
	}
#endif
}
#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <Windows.h>
#include <string>
#include <wchar.h>
#include <sstream>

#include "z80.h"
#include "gbSystem.h"
#include "debug.h"

GameboySystem gbSystem;

void LoadGameboyFile( const std::wstring& fileName, unique_ptr<gbCart>& outCart );

uint8_t GetTilePalette( const uint8_t plane0, const uint8_t plane1, const uint8_t col )
{
	const uint8_t planeBit0 = ( plane0 >> ( 7 - col ) ) & 0x01;
	const uint8_t planeBit1 = ( ( plane1 >> ( 7 - col ) ) & 0x01 ) << 1;

	return ( planeBit0 | planeBit1 );
}

void DrawTile( Bitmap& tileMap, const uint32_t xOffset, const uint32_t yOffset, const uint32_t bankId, const uint32_t tileId )
{
	const uint16_t baseAddr = 16 * tileId + bankId * 0x0800;

	uint32_t debugPalette[ 4 ] = { 0xFFFFFFFF, 0xAAAAAAFF, 0x555555FF, 0x000000FF };

	for ( uint32_t y = 0; y < 8; ++y )
	{
		const uint16_t rowOffset = 2 * y;
		const uint8_t loByte = gbSystem.vram[ baseAddr + rowOffset + 0 ];
		const uint8_t hiByte = gbSystem.vram[ baseAddr + rowOffset + 1 ];

		for ( int x = 0; x < 8; ++x )
		{
			const uint8_t palId = GetTilePalette( loByte, hiByte, x );
			const uint32_t hexColor = debugPalette[ palId ];
			tileMap.SetPixel( xOffset + x, tileMap.GetHeight() - ( yOffset + y + 1 ), debugPalette[ palId ] );
		}
	}
}

void Debug( const uint32_t currentFrame )
{
#define TILEMAP_DEBUG 1
#define BG_DEBUG 0
#define BG_TILE_DEBUG 1

#if TILEMAP_DEBUG
	{
		Bitmap tileMap( 128, 64, 0x00000000 );

		for ( uint32_t bank = 0; bank < 3; ++bank )
		{
			for ( uint32_t tileId = 0; tileId < 128; ++tileId )
			{
				const uint32_t pixelOffsetX = ( tileId % 16 ) * 8;
				const uint32_t pixelOffsetY = ( tileId / 16 ) * 8;

				DrawTile( tileMap, pixelOffsetX, pixelOffsetY, bank, tileId );
			}

			stringstream ss;
			ss << "frame_" << currentFrame << "_tilemap_" << bank << ".bmp";
			tileMap.Write( "gfx_output\\" + ss.str() );
		}
	}
#endif

#if BG_TILE_DEBUG
	{
		Bitmap bgMap( 256, 256, 0x00000000 );

		for ( uint32_t tileY = 0; tileY < 32; ++tileY ) {
			for ( uint32_t tileX = 0; tileX < 32; ++tileX ) {
				const uint32_t pixelOffsetX = tileX * 8;
				const uint32_t pixelOffsetY = tileY * 8;

				const uint32_t tileId = gbSystem.vram[ 0x1800 + tileY * 32 + tileX ];
				DrawTile( bgMap, pixelOffsetX, pixelOffsetY, 0, tileId );
			}
		}

		stringstream ss;
		ss << "frame_" << currentFrame << "_bgmap" << ".bmp";
		bgMap.Write( "gfx_output\\" + ss.str() );
	}
#endif

#if BG_DEBUG
	{
		system( "cls" );

		cout << "\n\n";

		for ( int y = 0; y < 32; ++y ) {
			for ( int x = 0; x < 32; ++x ) {
				cout << setw( 2 ) << setfill( '0' ) << hex << uint32_t( gbSystem.vram[ 0x1800 + y * 32 + x ] ) << " ";
			}
			cout << "\n";
		}
		cout << endl;
	}
#endif
}

int main()
{
	int test = 0;
	switch( test )
	{	
		case 0:		LoadGameboyFile( L"Games/Alleyway.gb",				gbSystem.cart ); break;
		case 1:		LoadGameboyFile( L"Tests/01-special.gb",			gbSystem.cart ); break;	// Passed 9/27/24
		case 2:		LoadGameboyFile( L"Tests/02-interrupts.gb",			gbSystem.cart ); break; // Not implemented yet
		case 3:		LoadGameboyFile( L"Tests/03-op sp,hl.gb",			gbSystem.cart ); break; // Passed 9/27/24
		case 4:		LoadGameboyFile( L"Tests/04-op r,imm.gb",			gbSystem.cart ); break;	// Passed 9/27/24
		case 5:		LoadGameboyFile( L"Tests/05-op rp.gb",				gbSystem.cart ); break;	// Passed 9/27/24
		case 6:		LoadGameboyFile( L"Tests/06-ld r,r.gb",				gbSystem.cart ); break;	// Passed 9/27/24
		case 7:		LoadGameboyFile( L"Tests/07-jr,jp,call,ret,rst.gb",	gbSystem.cart ); break; // Passed 9/27/24
		case 8:		LoadGameboyFile( L"Tests/08-misc instrs.gb",		gbSystem.cart ); break; // Passed 9/27/24
		case 9:		LoadGameboyFile( L"Tests/09-op r,r.gb",				gbSystem.cart ); break; // Passed 9/27/24
		case 10:	LoadGameboyFile( L"Tests/10-bit ops.gb",			gbSystem.cart ); break; // Passed 9/27/24
		case 11:	LoadGameboyFile( L"Tests/11-op a,(hl).gb",			gbSystem.cart ); break; // Passed 9/27/24
	}

	gbSystem.cart->mapper = gbSystem.AssignMapper( gbSystem.cart->GetMapperId() );
	gbSystem.cart->mapper->system = &gbSystem;
	gbSystem.cart->mapper->OnLoadCpu();
	gbSystem.cart->mapper->OnLoadPpu();

	gbSystem.cpu.system = &gbSystem;

	gbSystem.cpu.dbgLog.Reset( 1000 );
	gbSystem.cpu.StartTraceLog( 100 );

	int currentFrame = 0;
	cpuCycle_t nextCycle = cpuCycle_t( 0 );

#define LOG_DEBUG 0

	while( currentFrame < 100 )
	{
		const cpuCycle_t cyclesPerFrame = MasterToCpuCycle( NanoToCycle( 1 * FrameLatencyNs.count() ) );
		for( cpuCycle_t i = 0; i < cyclesPerFrame; ++i )
		{
			nextCycle += 1;

			gbSystem.cpu.Step( nextCycle );
			gbSystem.ppu.Step( nextCycle );

			if( gbSystem.cpu.dbgLog.HasPendingLine() ) {
			//	cout << gbSystem.cpu.dbgLog.FlushLine() << endl;
			}
		}

		if ( gbSystem.cpu.IsTraceLogOpen() ) {
			gbSystem.cpu.dbgLog.NewFrame();
		}

		++currentFrame;
	}

	Debug( currentFrame );

#if LOG_DEBUG
	{
		std::ofstream logFile;
		logFile.open( "tomboy.log" );

		for ( uint32_t i = 0; i < gbSystem.cpu.dbgLog.GetRecordCount(); ++i ) {
			string logBuffer;
			gbSystem.cpu.dbgLog.ToString( logBuffer, i, i + 1 );
	
			logFile << logBuffer;	
		}
		logFile.close();
	}
#endif

	return 0;
}
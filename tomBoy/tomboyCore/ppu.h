#pragma once
#include "cart.h"

class GameboySystem;

class PPU
{
public:
	static const uint32_t VirtualMemorySize			= 0x10000;
	static const uint32_t PhysicalMemorySize		= 0x4000;
	static const uint32_t OamSize					= 0x0100;
	static const uint32_t OamSecondSize				= 0x0020;

	union
	{
		struct
		{
			uint8_t mode		: 2;
			uint8_t lyCp		: 1;
			uint8_t mode0Sel	: 1;
			uint8_t mode1Sel	: 1;
			uint8_t mode2Sel	: 1;
			uint8_t lcdSel		: 1;
			uint8_t unused		: 1;
		};
		uint16_t byte;
	} lcdStatus;

	union {
		struct {
			uint8_t bgPriority	: 1;
			uint8_t objEnable	: 1;
			uint8_t objSize		: 1;
			uint8_t bgTileMap	: 1;
			uint8_t bgTiles		: 1;
			uint8_t winEnable	: 1;
			uint8_t winTileMap	: 1;
			uint8_t lcdEnable	: 1;
		};
		uint16_t byte;
	} lcdControl;

private:
	GameboySystem*	system;
	ppuCycle_t		cycle;
	ppuCycle_t		dot;
	int32_t			ly;

	// Internal registers
	uint16_t		regX;
	uint16_t		regW;

	uint32_t		debugVramWriteCounter[VirtualMemorySize];
	uint8_t			primaryOAM[OamSize];
	uint8_t			secondaryOamSpriteCnt;

	uint8_t			ppuReadBuffer;
public:
	void			WriteReg( const uint16_t addr, const uint8_t value );
	uint8_t			ReadReg( uint16_t address );

	//void			DrawDebugPatternTables( wtPatternTableImage& imageBuffer, const RGBA dbgPalette[4], const uint32_t tableID, const bool isCartbank );
	//void			DrawDebugObject( wtRawImageInterface* imageBuffer, const RGBA dbgPalette[ 4 ], const spriteAttrib_t& attrib );
	//void			DrawDebugNametable( wtNameTableImage& nameTableSheet );
	//void			DrawDebugPalette( wtPaletteImage& imageBuffer );

	void			WriteVram();
	uint8_t			ReadVram( const uint16_t addr );
	//ppuCycle_t	GetCycle() const;
	uint32_t		LY() const;

	ppuCycle_t		Exec();
	bool			Step( const ppuCycle_t& nextCycle );	

	PPU()
	{
		Reset();
	}

	void Reset()
	{
		cycle	= ppuCycle_t( 0 );

		system	= nullptr;

		ly		= 0;

		regX	= 0;
		regW	= 0;
	}

	void			Begin();
	void			End();
	void			RegisterSystem( GameboySystem* sys );

	//void			Serialize( Serializer& serializer );

private:
	//static uint8_t	GetChrRomPalette( const uint8_t plane0, const uint8_t plane1, const uint8_t col );

	//uint8_t			GetChrRom8x8( const uint32_t tileId, const uint8_t plane, const uint8_t ptrnTableId, const uint8_t row );
	//uint8_t			GetChrRom8x16( const uint32_t tileId, const uint8_t plane, const uint8_t row, const uint8_t isUpper );
	//uint8_t			GetChrRomBank8x8( const uint32_t tileId, const uint8_t plane, const uint8_t bankId, const uint8_t row );

	//void			DrawBlankScanline( wtDisplayImage& imageBuffer, const wtRect& imageRect, const uint8_t scanY );
	//void			DrawTile( wtNameTableImage& imageBuffer, const wtRect& imageRect, const wtPoint& nametableTile, const uint32_t ntId, const uint32_t ptrnTableId );
	//void			DrawChrRomTile( wtRawImageInterface* imageBuffer, const wtRect& imageRect, const RGBA palette[4], const uint32_t tileId, const uint32_t tableId, const bool cartBank, const bool is8x16 = false, const bool isUpper = false );
	//bool			DrawSpritePixel( wtDisplayImage& fb, const spriteAttrib_t attribs, const ppuImageIx_t& index, const uint8_t bgPixel );
};
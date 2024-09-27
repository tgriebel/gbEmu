#include "ppu.h"
#include "gbSystem.h"

void PPU::RegisterSystem( GameboySystem* sys )
{
	system = sys;
}

uint32_t PPU::LY() const
{
	return ly;
}

bool PPU::Step( const ppuCycle_t& nextCycle )
{
	//while ( ( cycle < nextCycle ) ) {
	//	cycle += cpuCycle_t( Exec() );
	//}

	const uint32_t dot = nextCycle.count() % 70224;

	static uint32_t prevLy = ly;
	ly = static_cast<uint32_t>( dot / 456 );

	if( ly >= 144 && ly <= 153 )
	{
		// MODE 1 - VBLANK
		if( ( prevLy != ly ) && ( ly == 144 ) ) {
			system->RequestVBlankInterrupt();
		}
	}
	else
	{
		system->ClearVBlankInterrupt();
		// MODE 2 - 80 OAM
		// MODE 3 - Drawing Pixels
		// MODE 0 - HBLANK
	}
	return true;
}


ppuCycle_t PPU::Exec()
{

	return 0;
}
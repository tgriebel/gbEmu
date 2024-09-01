
#include "gbSystem.h"
/*
template<typename Cycle>
static inline void SerializeCycle( Serializer& serializer, Cycle& c )
{
	if ( serializer.GetMode() == serializeMode_t::LOAD )
	{
		uint64_t cycles;
		serializer.Next64b( *reinterpret_cast<uint64_t*>( &cycles ) );
		c = Cycle( cycles );
	}
	else if ( serializer.GetMode() == serializeMode_t::STORE )
	{
		uint64_t cycles = static_cast<uint64_t>( c.count() );
		serializer.Next64b( *reinterpret_cast<uint64_t*>( &cycles ) );
	}
}


template<typename Counter>
static inline void SerializeBitCounter( Serializer& serializer, Counter& c )
{
	if ( serializer.GetMode() == serializeMode_t::LOAD )
	{
		uint16_t value;
		serializer.Next16b( *reinterpret_cast<uint16_t*>( &value ) );
		c.Reload( value );
	}
	else if ( serializer.GetMode() == serializeMode_t::STORE )
	{
		uint16_t value = c.Value();
		serializer.Next16b( *reinterpret_cast<uint16_t*>( &value ) );
	}
}


static inline void SerializeEnvelope( Serializer& serializer, envelope_t& e )
{
	serializer.NextBool( e.startFlag );
	serializer.Next8b( e.decayLevel );
	serializer.Next8b( e.divCounter );
	serializer.Next8b( e.output );
}


static inline void SerializeSweep( Serializer& serializer, sweep_t& s )
{
	serializer.NextBool( s.mute );
	serializer.NextBool( s.reloadFlag );
	serializer.Next16b( s.period );
	SerializeBitCounter( serializer, s.divider );
}


void GameboySystem::Serialize( Serializer& serializer )
{
	SerializeCycle( serializer, sysCycles );

	serializer.Next64b( frameNumber );
	serializer.Next8b( *reinterpret_cast<uint8_t*>( &strobeOn ) );
	serializer.Next8b( btnShift[ 0 ] );
	serializer.Next8b( btnShift[ 1 ] );

	serializer.NewLabel( STATE_MEMORY_LABEL );
	serializer.NextArray( memory, PhysicalMemorySize );
	serializer.EndLabel( STATE_MEMORY_LABEL );

	cpu.Serialize( serializer );
	ppu.Serialize( serializer );
	apu.Serialize( serializer );
	cart->mapper->Serialize( serializer );
}


void CpuZ80::Serialize( Serializer& serializer )
{

}


void PPU::Serialize( Serializer& serializer )
{

}
*/

/*
void APU::Serialize( Serializer& serializer )
{

}


void PulseChannel::Serialize( Serializer& serializer )
{

}


void TriangleChannel::Serialize( Serializer& serializer )
{

}


void NoiseChannel::Serialize( Serializer& serializer )
{

}


void DmcChannel::Serialize( Serializer& serializer )
{

}
*/
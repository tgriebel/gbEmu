
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include "debug.h"
#include "z80.h"

static void PrintHex( std::stringstream& debugStream, const uint32_t value, const uint32_t charCount, const bool markup )
{
	std::ios_base::fmtflags flags = debugStream.flags();
	if( markup ) {
		debugStream << "$";
	}
	debugStream << setfill( '0' ) << setw( charCount ) << uppercase << hex << right << value << left;
	debugStream.setf( flags );
}


static std::string GetOpName( const OpDebugInfo& info )
{
	std::string name;
	if( info.isIllegal ) {
		name += "*";
	} else {
		name += " ";
	}

	if( ( info.opType == (uint8_t)opType_t::NOP ) || ( info.opType == (uint8_t)opType_t::ILL ) )
	{
		name += "NOP";
	}
	else
	{
		name += info.mnemonic[ 0 ];
		name += info.mnemonic[ 1 ];
		name += info.mnemonic[ 2 ];
	}
	return name;
}

static void PrintAddrName( std::stringstream& debugStream, const char* name, const addrMode_t mode, const uint16_t addr, const uint16_t value, const bool dereference ) {
	switch ( mode ) {
		case addrMode_t::DIRECT_N8:
		case addrMode_t::DIRECT_N16:
			{
				debugStream << "($" << hex << addr << ")";
			}
			break;
		case addrMode_t::IMMEDIATE_N8:
		case addrMode_t::IMMEDIATE_N16:
			{
				debugStream << hex << value;
			}
			break;
		case addrMode_t::DIRECT_C:
		case addrMode_t::DIRECT_BC:
		case addrMode_t::DIRECT_DE:
		case addrMode_t::DIRECT_HL:
			if ( dereference ) {
				debugStream << "($" << hex << value << ")";
			} else {
				string baseName = string( name );
				baseName = baseName.substr( 0, baseName.size() - 2 );
				debugStream << "($" << baseName << ")";
			}
			break;
		case addrMode_t::NONE: break;
		default:
			if ( dereference ) {
				debugStream << hex << value;
			} else {
				debugStream << name;
			}
			break;
	}
}

void OpDebugInfo::ToString( std::string& buffer, const bool registerDebug, const bool cycleDebug ) const
{
	std::stringstream debugStream;

	if( nmi )
	{
		debugStream << "[NMI - Cycle:" << cpuCycles << "]";
		buffer +=  debugStream.str();
		return;
	}

	if ( irq )
	{
		debugStream << "[IRQ - Cycle:" << cpuCycles << "]";
		buffer += debugStream.str();
		return;
	}

	int disassemblyBytes[6] = { byteCode, op0, op1,'\0' };
	stringstream hexString;

	if ( operands == 1 )
	{
		PrintHex( hexString, disassemblyBytes[ 0 ], 2, false );
		PrintHex( hexString, disassemblyBytes[ 1 ], 2, false );
	}
	else if ( operands == 2 )
	{
		PrintHex( hexString, disassemblyBytes[ 0 ], 2, false );
		PrintHex( hexString, disassemblyBytes[ 1 ], 2, false );
		PrintHex( hexString, disassemblyBytes[ 2 ], 2, false );
	}
	else
	{
		PrintHex( hexString, disassemblyBytes[ 0 ], 2, false );
	}

	PrintHex( debugStream, instrBegin, 4, false );
	debugStream << setfill( ' ' ) << "  " << setw( 9 ) << left << hexString.str();
	debugStream << GetOpName( *this ) << " ";
	
	if ( lhsName != "NONE" ) {
		const addrMode_t mode = static_cast<addrMode_t>( lhsAddrMode );
		PrintAddrName( debugStream, lhsName, mode, lhsAddress, lhsMemValue, false );
		debugStream << ", ";
	}
	if ( rhsName != "NONE" ) {
		const addrMode_t mode = static_cast<addrMode_t>( rhsAddrMode );
		PrintAddrName( debugStream, rhsName, mode, rhsAddress, rhsMemValue, false );
	}

	if ( registerDebug )
	{
		debugStream.seekg( 0, ios::end );
		const size_t alignment = 50;
		const size_t width = ( alignment - debugStream.tellg() );
		debugStream.seekg( 0, ios::beg );

		debugStream << setfill( ' ' ) << setw( width ) << right;
		debugStream << uppercase << "A:";
		PrintHex( debugStream, static_cast<int>( regInfo.A ), 2, false );
		debugStream << uppercase << " F:";
		PrintHex( debugStream, static_cast<int>( regInfo.F ), 2, false );
		debugStream << uppercase << " B:";
		PrintHex( debugStream, static_cast<int>( regInfo.B ), 2, false );
		debugStream << uppercase << " C:";
		PrintHex( debugStream, static_cast<int>( regInfo.C ), 2, false );
		debugStream << uppercase << " D:";
		PrintHex( debugStream, static_cast<int>( regInfo.D ), 2, false );
		debugStream << uppercase << " E:";
		PrintHex( debugStream, static_cast<int>( regInfo.E ), 2, false );
		debugStream << uppercase << " H:";
		PrintHex( debugStream, static_cast<int>( regInfo.H ), 2, false );
		debugStream << uppercase << " L:";
		PrintHex( debugStream, static_cast<int>( regInfo.L ), 2, false );
		debugStream << uppercase << " SP:";
		PrintHex( debugStream, static_cast<int>( regInfo.SP ), 2, false );
	}

	if( cycleDebug )
	{
		debugStream << uppercase << " PPU:" << setfill( ' ' ) << setw( 3 ) <<  dec << right << curScanline;
		debugStream << uppercase << "," << setfill( ' ' ) << setw( 3 ) << dec << ppuCycles;
		debugStream << uppercase << " CYC:" << dec << cpuCycles << "\0";
	}

	buffer += debugStream.str();
}


void wtLog::Reset( const uint32_t targetCount )
{
	log.clear();
	totalCount = ( 1 + targetCount );
	frameIx = 0;
	log.resize( totalCount );
	for ( size_t frameIndex = 0; frameIndex < totalCount; ++frameIndex ) {
		log[ frameIndex ].reserve( 10000 );
	}
	NewFrame();
}


void wtLog::NewFrame()
{
	if( IsFull() ) {
		return;
	}

	++frameIx;
	log[ frameIx ].reserve( 10000 );
}


OpDebugInfo& wtLog::NewLine()
{
	log[ frameIx ].resize( log[ frameIx ].size() + 1 );
	return GetLogLine();
}


const logFrame_t& wtLog::GetLogFrame( const uint32_t frameNum ) const
{
	const uint32_t frameIndex = ( frameNum >= totalCount ) ? 0 : frameNum;
	return log[ frameIndex ];
}


OpDebugInfo& wtLog::GetLogLine()
{
	return log[ frameIx ].back();
}


uint32_t wtLog::GetRecordCount() const
{
	return ( totalCount == 0 ) ? 0 : ( frameIx + 1 );
}


bool wtLog::IsFull() const
{
	assert( totalCount > 0 );
	return ( ( totalCount <= 1 ) || ( frameIx >= ( totalCount - 1 ) ) );
}


bool wtLog::IsFinished() const
{
	// This needs to be improved, but sufficient for now
	// Right now it's a weak condition
	return ( IsFull() && log[ frameIx ].size() > 0 );
}


void wtLog::ToString( std::string& buffer, const uint32_t frameBegin, const uint32_t frameEnd, const bool registerDebug ) const
{
	for( uint32_t i = ( frameBegin + 1 ); i <= frameEnd; ++i )
	{
		for ( const OpDebugInfo& dbgInfo : GetLogFrame( i ) )
		{
			dbgInfo.ToString( buffer, true, false );
			buffer += "\n";
		}
	}
}
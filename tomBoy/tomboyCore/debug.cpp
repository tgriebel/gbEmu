
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



	if( info.opType == (uint8_t)opType_t::ILL )
	{
		name += "<Illegal>";
	}
	else
	{
		std::string mnemonic = info.mnemonic;
		const size_t delim = mnemonic.find( '_' );
		if( delim != string::npos ) {
			mnemonic = mnemonic.substr( 0, delim );
		}
		name += mnemonic;
	}
	return name;
}

static std::string GetFlagName( const uint8_t flags, const bool fill )
{
	std::string bitNames;
	bitNames += ( flags & statusBit_t::STATUS_ZERO )		? "z" : fill ? "_" : "";
	bitNames += ( flags & statusBit_t::STATUS_NEGATIVE )	? "n" : fill ? "_" : "";
	bitNames += ( flags & statusBit_t::STATUS_HALF_CARRY )	? "h" : fill ? "_" : "";
	bitNames += ( flags & statusBit_t::STATUS_CARRY )		? "c" : fill ? "_" : "";
	return bitNames;
}

static void PrintAddrName( std::stringstream& debugStream, const char* name, const addrMode_t mode, const uint16_t addr, const uint16_t value, const bool dereference ) {
	switch ( mode ) {
		case addrMode_t::DIRECT_N8:
			{
				debugStream << "(";
				PrintHex( debugStream, addr, 2, true );
				debugStream << ")";
			}
			break;
		case addrMode_t::DIRECT_N16:
			{
				debugStream << "(";
				PrintHex( debugStream, addr, 4, true );
				debugStream << ")";
			}
			break;
		case addrMode_t::IMMEDIATE_N8:
			{
				PrintHex( debugStream, addr, 2, true );
			}
			break;
		case addrMode_t::IMMEDIATE_N16:
			{
				PrintHex( debugStream, addr, 4, true );
			}
			break;
		case addrMode_t::DIRECT_C:
		case addrMode_t::DIRECT_BC:
		case addrMode_t::DIRECT_DE:
		case addrMode_t::DIRECT_HL:
			if ( dereference ) {
				PrintHex( debugStream, value, 4, true );
			} else {
				debugStream << "(";
				PrintHex( debugStream, addr, 4, true );
				debugStream << ")";
			}
			break;
		case addrMode_t::NONE: break;
		default:
			if ( dereference ) {
				PrintHex( debugStream, value, 4, true );
			} else {
				debugStream << name;
			}
			break;
	}
}

std::string OpDebugInfo::ByteCodeToString( const uint32_t numBytes, const bool formatBin ) const
{
	int disassemblyBytes[ 6 ] = { bitOp ? 0xCB : byteCode, nextByte0, nextByte1, nextByte2, 0, 0 };
	stringstream hexString;

	for ( uint32_t i = 0; i < numBytes; ++i ) {
		PrintHex( hexString, disassemblyBytes[ i ], 2, formatBin );
		hexString << ( ( ( i + 1 ) >= numBytes ) ? "" : " " );
	}
	return hexString.str();
}

std::string OpDebugInfo::DisassemblyToString( const bool formatAddress, const bool dereferenceOperands ) const
{
	std::stringstream asmStream;
	if ( opType == (uint8_t)opType_t::JR )
	{
		const int8_t offset = u8i8( (uint8_t)rhsMemValue ).i8;
		asmStream << GetFlagName( bitCheck, false ) << ", ";
		PrintHex( asmStream, regInfo.PC + offset + operands, 4, formatAddress );
	}
	else
	{
		const bool incrementOp = ( opType == (uint8_t)opType_t::INC_R8 ) ||
			( opType == (uint8_t)opType_t::INC_R16 ) ||
			( opType == (uint8_t)opType_t::DEC_R8 ) ||
			( opType == (uint8_t)opType_t::DEC_R16 );

		if ( lhsName != "NONE" && !incrementOp )
		{
			const addrMode_t mode = static_cast<addrMode_t>( lhsAddrMode );
			PrintAddrName( asmStream, lhsName, mode, lhsAddress, lhsMemValue, dereferenceOperands );
			asmStream << ", ";
		}
		if ( rhsName != "NONE" )
		{
			const addrMode_t mode = static_cast<addrMode_t>( rhsAddrMode );
			PrintAddrName( asmStream, rhsName, mode, rhsAddress, rhsMemValue, dereferenceOperands );
		}
	}
	return GetOpName( *this ) + " " + asmStream.str();
}


std::string OpDebugInfo::RegistersToString() const
{
	std::stringstream regStream;
	regStream << uppercase << "A: ";
	PrintHex( regStream, static_cast<int>( regInfo.A ), 2, false );
	regStream << uppercase << " F: ";
	PrintHex( regStream, static_cast<int>( regInfo.F ), 2, false );
	regStream << uppercase << " B: ";
	PrintHex( regStream, static_cast<int>( regInfo.B ), 2, false );
	regStream << uppercase << " C: ";
	PrintHex( regStream, static_cast<int>( regInfo.C ), 2, false );
	regStream << uppercase << " D: ";
	PrintHex( regStream, static_cast<int>( regInfo.D ), 2, false );
	regStream << uppercase << " E: ";
	PrintHex( regStream, static_cast<int>( regInfo.E ), 2, false );
	regStream << uppercase << " H: ";
	PrintHex( regStream, static_cast<int>( regInfo.H ), 2, false );
	regStream << uppercase << " L: ";
	PrintHex( regStream, static_cast<int>( regInfo.L ), 2, false );
	regStream << uppercase << " SP: ";
	PrintHex( regStream, static_cast<int>( regInfo.SP ), 4, false );

	return regStream.str();
}

void OpDebugInfo::ToString( std::string& buffer, const bool registerDebug, const bool statusDebug, const bool cycleDebug ) const
{
	std::stringstream debugStream;

	const bool formatBin = false;
	const bool dereferenceOperands = false;
	const bool formatAddress = true;

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

	const bool refLogFormat = false;
 	if( refLogFormat )
	{
		buffer += RegistersToString();

		std::stringstream instString;
		PrintHex( instString, instrBegin, 4, formatBin );

		buffer += " PC: 00:" + instString.str();
		buffer += " (";
		buffer += ByteCodeToString( 4, formatBin );
		buffer += ")";
	}
	else
	{
		const int numPrintBytes = operands + 1;

		std::string byteString = ByteCodeToString( numPrintBytes, formatBin );
		std::string disasmString = DisassemblyToString( formatAddress, dereferenceOperands );
		
		PrintHex( debugStream, instrBegin, 4, formatBin );
		debugStream << setfill( ' ' ) << "  " << setw( 9 ) << left << byteString << " " << disasmString;
	
		if ( registerDebug )
		{
			debugStream.seekg( 0, ios::end );
			const size_t alignment = 150;
			const size_t width = ( alignment - debugStream.tellg() );
			assert( width > 0 );
			debugStream.seekg( 0, ios::beg );
			debugStream << setfill( ' ' ) << setw( width ) << right;

			debugStream << RegistersToString();
		}

		if( statusDebug )
		{
			debugStream << " PSW: " << GetFlagName( regInfo.psw, true );
		}

		if( cycleDebug )
		{
			//debugStream << uppercase << " PPU:" << setfill( ' ' ) << setw( 3 ) <<  dec << right << curScanline;
			//debugStream << uppercase << "," << setfill( ' ' ) << setw( 3 ) << dec << ppuCycles;
			debugStream << uppercase << " CYC:" << dec << cpuCycles << "\0";
		}
		buffer += debugStream.str();
	}
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
	latchLineFlush = false;
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
	latchLineFlush = false;
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


bool wtLog::HasPendingLine() const {
	return latchLineFlush == false;
}


string wtLog::FlushLine()
{
	string buffer;
	OpDebugInfo& line = GetLogLine();
	line.ToString( buffer, true, true, true );
	latchLineFlush = true;

	return buffer;
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
			dbgInfo.ToString( buffer, true, true, true );
			buffer += "\n";
		}
	}
}
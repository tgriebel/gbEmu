#pragma once

#include <string>
#include <sstream>

#include "common.h"

#if DEBUG_ADDR == 1
#define DEBUG_CPU_LOG 0;
#endif // #else // #if DEBUG_ADDR == 1


struct regDebugInfo_t
{
	uint8_t			A;
	uint8_t			F;
	uint8_t			B;
	uint8_t			C;
	uint8_t			D;
	uint8_t			E;
	uint8_t			H;
	uint8_t			L;
	uint8_t			psw;
	uint16_t		SP;
	uint16_t		PC;
};


class LogLine
{
	std::string ToString( std::string& buffer );
};

class OpDebugInfo
{
private:
	std::string DisassemblyToString( const bool formatAddress, const bool dereferenceOperands ) const;
	std::string ByteCodeToString( const uint32_t numBytes, const bool formatBin ) const;
	std::string RegistersToString() const;

public:

	regDebugInfo_t	regInfo;
	const char*		mnemonic;
	const char*		lhsName;
	const char*		rhsName;
	int32_t			curScanline;
	uint64_t		cpuCycles;
	uint64_t		ppuCycles;
	uint64_t		instrCycles;

	uint32_t		loadCnt;
	uint32_t		storeCnt;

	uint16_t		instrBegin;
	uint8_t			opType;
	uint8_t			byteCode;

	uint8_t			lhsAddrMode;
	uint8_t			lhsAddrType;
	uint8_t			rhsAddrMode;
	uint8_t			rhsAddrType;
	uint8_t			lhsMemValue;
	uint8_t			rhsMemValue;
	uint16_t		lhsAddress;
	uint16_t		rhsAddress;

	uint8_t			operands;
	uint8_t			nextByte0;
	uint8_t			nextByte1;
	uint8_t			nextByte2;

	uint8_t			bitCheck;
	uint8_t			check;

	bool			isIllegal;
	bool			irq;
	bool			nmi;
	bool			oam;
	bool			bitOp;
	
	OpDebugInfo()
	{
		loadCnt			= 0;
		storeCnt		= 0;

		opType			= 0;
		lhsAddrMode		= 0;
		lhsAddrType		= 0;
		rhsAddrMode		= 0;
		rhsAddrType		= 0;
		lhsMemValue		= 0;
		rhsMemValue		= 0;
		lhsAddress		= 0;
		rhsAddress		= 0;
		instrBegin		= 0;
		curScanline		= 0;
		cpuCycles		= 0;
		ppuCycles		= 0;
		instrCycles		= 0;

		nextByte0		= 0;
		nextByte1		= 0;
		nextByte2		= 0;

		isIllegal		= false;
		irq				= false;
		nmi				= false;
		oam				= false;

		mnemonic		= "";
		operands		= 0;
		byteCode		= 0;

		regInfo			= { 0, 0, 0, 0, 0, 0 };
	}

	void ToString( std::string& buffer, const bool registerDebug, const bool statusDebug, const bool cycleDebug ) const;
};

using logFrame_t = std::vector<OpDebugInfo>;
using logRecords_t = std::vector<logFrame_t>;

class wtLog
{
private:
	logRecords_t		log;
	uint32_t			frameIx;
	uint32_t			totalCount;
	bool				latchLineFlush;

public:
	wtLog() : frameIx( 0 ), totalCount( 0 )
	{
		Reset( 1 );
	}

	void				Reset( const uint32_t targetCount );
	void				NewFrame();
	OpDebugInfo&		NewLine();
	void				NewLine( const OpDebugInfo& debugInfo );
	const logFrame_t&	GetLogFrame( const uint32_t frameIx ) const;
	OpDebugInfo&		GetLogLine();
	uint32_t			GetRecordCount() const;
	bool				HasPendingLine() const;
	string				FlushLine();
	bool				IsFull() const;
	bool				IsFinished() const;
	void				ToString( std::string& buffer, const uint32_t frameBegin, const uint32_t frameEnd, const bool registerDebug = true ) const;
};
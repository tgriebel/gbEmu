#pragma once

#include <string>
#include <sstream>

#include "common.h"

#if DEBUG_ADDR == 1
#define DEBUG_ADDR_INDEXED_ZERO( _info )												\
	if( IsTraceLogOpen() )																\
	{																					\
		OpDebugInfo& dbgInfo	= dbgLog.GetLogLine();									\
		dbgInfo.address			= _info.addr;											\
		dbgInfo.targetAddress	= _info.targetAddr;										\
		dbgInfo.memValue		= _info.value;											\
	}

#define DEBUG_ADDR_INDEXED_ABS( _info )													\
	if( IsTraceLogOpen() )																\
	{																					\
		OpDebugInfo& dbgInfo	= dbgLog.GetLogLine();									\
		dbgInfo.address			= _info.addr;											\
		dbgInfo.targetAddress	= _info.targetAddr;										\
		dbgInfo.memValue		= _info.value;											\
	}

#define DEBUG_ADDR_ZERO( _info )														\
	if( cpu.IsTraceLogOpen() )															\
	{																					\
		OpDebugInfo& dbgInfo	= cpu.dbgLog.GetLogLine();								\
		dbgInfo.address			= _info.addr;											\
		dbgInfo.memValue		= _info.value;											\
	}

#define DEBUG_ADDR_ABS( _info )															\
	if( cpu.IsTraceLogOpen() )															\
	{																					\
		OpDebugInfo& dbgInfo	= cpu.dbgLog.GetLogLine();								\
		dbgInfo.address			= _info.addr;											\
		dbgInfo.memValue		= _info.value;											\
	}

#define DEBUG_ADDR_IMMEDIATE( _info )													\
	if( cpu.IsTraceLogOpen() )															\
	{																					\
		OpDebugInfo& dbgInfo	= cpu.dbgLog.GetLogLine();								\
		dbgInfo.address			= _info.addr;											\
		dbgInfo.memValue		= cpu.ReadOperand(0);									\
	}

#define DEBUG_ADDR_INDIRECT_INDEXED( _info )											\
	if( cpu.IsTraceLogOpen() )															\
	{																					\
		OpDebugInfo& dbgInfo	= cpu.dbgLog.GetLogLine();								\
		dbgInfo.address			= _info.addr;											\
		dbgInfo.targetAddress	= _info.targetAddr;										\
		dbgInfo.memValue		= _info.value;											\
		dbgInfo.offset			= _info.offset;											\
	}

#define DEBUG_ADDR_INDEXED_INDIRECT( _info )											\
	if( cpu.IsTraceLogOpen() )															\
	{																					\
		OpDebugInfo& dbgInfo	= cpu.dbgLog.GetLogLine();								\
		dbgInfo.address			= _info.addr;											\
		dbgInfo.memValue		= _info.value;											\
		dbgInfo.targetAddress	= _info.targetAddr;										\
	}

#define DEBUG_ADDR_ACCUMULATOR( _info )													\
	if( cpu.IsTraceLogOpen() )															\
	{																					\
	}

#define DEBUG_ADDR_JMP( _PC )															\
	if( IsTraceLogOpen() )																\
	{																					\
		OpDebugInfo& dbgInfo	= dbgLog.GetLogLine();									\
		dbgInfo.address			= _PC;													\
	}

#define DEBUG_ADDR_JMPI( _offset, _PC )													\
	if( IsTraceLogOpen() )																\
	{																					\
		OpDebugInfo& dbgInfo	= dbgLog.GetLogLine();									\
		dbgInfo.offset			= _offset;												\
		dbgInfo.address			= _PC;													\
	}

#define DEBUG_ADDR_JSR( _PC )															\
	if( IsTraceLogOpen() )																\
	{																					\
		OpDebugInfo& dbgInfo	= dbgLog.GetLogLine();									\
		dbgInfo.address			= _PC;													\
	}

#define DEBUG_ADDR_BRANCH( _PC )														\
	if( IsTraceLogOpen() )																\
	{																					\
		OpDebugInfo& dbgInfo	= dbgLog.GetLogLine();									\
		dbgInfo.address			= _PC;													\
	}

#define DEBUG_CPU_LOG 0;

#else //  #if DEBUG_ADDR == 1
#define DEBUG_ADDR_INDEXED_ZERO		{}
#define DEBUG_ADDR_INDEXED_ABS		{}
#define DEBUG_ADDR_ZERO				{}
#define DEBUG_ADDR_ABS				{}
#define DEBUG_ADDR_IMMEDIATE		{}
#define DEBUG_ADDR_INDIRECT_INDEXED	{}
#define DEBUG_ADDR_INDEXED_INDIRECT	{}
#define DEBUG_ADDR_ACCUMULATOR		{}
#define DEBUG_ADDR_JMP				{}
#define DEBUG_ADDR_JMPI				{}
#define DEBUG_ADDR_JSR				{}
#define DEBUG_ADDR_BRANCH			{}
#define DEBUG_CPU_LOG				{}
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

public:
	wtLog() : frameIx( 0 ), totalCount( 0 )
	{
		Reset( 1 );
	}

	void				Reset( const uint32_t targetCount );
	void				NewFrame();
	OpDebugInfo&		NewLine();
	const logFrame_t&	GetLogFrame( const uint32_t frameIx ) const;
	OpDebugInfo&		GetLogLine();
	uint32_t			GetRecordCount() const;
	bool				IsFull() const;
	bool				IsFinished() const;
	void				ToString( std::string& buffer, const uint32_t frameBegin, const uint32_t frameEnd, const bool registerDebug = true ) const;
};
#pragma once

#include <stdint.h>
#include "common.h"
#include "debug.h"

class CpuZ80;
class GameboySystem;
class wtLog;

enum class opType_t : uint8_t
{
	ILL, NOP, STOP, HALT,
	LD, LDI, LD_INC, LD_DEC,
	INC_R8, INC_R16, DEC_R8, DEC_R16, ADD, ADD_HL, ADD_SP, ADC, SUB, SBC, DAA, CPL,
	OR, XOR, AND, BIT, RES, SET, SWAP,
	RLC, RRC, RR, RL, SLA, SRA, SRL, RLA, RRA, RLCA, RRCA,
	JP, JR, CALL, RST, RET, RETI,
	PUSH, POP,
	DI, EI, SCF, CCF, CP,
};

enum class addrMode_t : uint8_t
{
	NONE,
	A,
	F,
	B,
	C,
	E,
	D,
	L,
	H,
	AF,
	BC,
	DE,
	HL,
	SP,
	PC,
	DIRECT_N8,
	DIRECT_N16,
	DIRECT_C,
	DIRECT_BC,
	DIRECT_DE,
	DIRECT_HL,
	IMMEDIATE_N8,
	IMMEDIATE_N16,
};

enum regIndex_t 
{
	A_IX = 0,
	F_IX = 1,
	C_IX = 2,
	B_IX = 3,
	E_IX = 4,
	D_IX = 5,
	L_IX = 6,
	H_IX = 7,
	AF_IX = 0,
	BC_IX = 1,
	DE_IX = 2,
	HL_IX = 3,
	SP_IX = 4,
	PC_IX = 5,
	UNKNOWN = 0xFF,
};

enum statusBit_t
{
	STATUS_NONE			= 0x00,
	STATUS_ALL			= 0x0F,
	STATUS_ZERO			= ( 1 << 0 ),
	STATUS_SUBRACTION	= ( 1 << 1 ),
	STATUS_HALF_CARRY	= ( 1 << 2 ),
	STATUS_CARRY		= ( 1 << 3 ),
};

enum addrType_t
{
	UNSPECIFIED	= 0,
	REGISTER_8	= 1,
	REGISTER_16	= 2,
	MEMORY		= 3,
};

struct addrInfo_t
{
	addrInfo_t() : addr( addrMode_t::NONE ), type( UNSPECIFIED ), value( 0 ) {}

	addrInfo_t( const addrMode_t _addr, const addrType_t _type ) :
		addr( _addr ), type( _type ) {}

	addrMode_t	addr;
	addrType_t	type;
	uint8_t		value;
	bool		indirect;
};

struct opState_t
{
	opState_t() : opCode( 0 ), op0( 0 ), op1( 0 ) {}

	cpuCycle_t		opCycles;
	uint8_t			opCode;
	addrInfo_t		lhsInfo;
	addrInfo_t		rhsInfo;
	union {
		struct {
			uint8_t op0;
			uint8_t op1;
		};
		uint16_t op;
	};
};

typedef void( CpuZ80::* OpCodeFn )( opState_t& o );
struct opInfo_t
{
	OpCodeFn	func;
	const char* mnemonic;
	const char* lhsName;
	const char* rhsName;
	opType_t	type		: 7;
	addrMode_t	lhsMode		: 5;
	addrMode_t	rhsMode		: 5;
	uint8_t		operands	: 2;
	uint8_t		baseCycles	: 4;
	uint8_t		pcInc		: 2;
};

union u8i8 {
	u8i8( uint8_t value ) : u8( value ) {}
	u8i8( int8_t value ) : i8( value ) {}
	uint8_t	u8;
	int8_t	i8;
};

union u16i16 {
	u16i16( uint16_t value ) : u16( value ) {}
	u16i16( int16_t value ) : i16( value ) {}
	uint16_t u16;
	int16_t	i16;
};

union u32i32 {
	u32i32( uint32_t value ) : u32( value ) {}
	u32i32( int32_t value ) : i32( value ) {}
	uint32_t u32;
	int32_t	i32;
};

#define OP_DEF( name )									template <class LHS, class RHS, size_t BIT, size_t CHK >				\
														void name( opState_t& o )

#define ADDR_MODE_DECL( name, addrType )				struct addrMode##name													\
														{																		\
															static const addrMode_t addrMode = addrMode_t::##name;				\
															static const addrType_t type = addrType;							\
															CpuZ80& cpu;														\
															addrMode##name( CpuZ80& _cpu ) : cpu( _cpu ) {};					\
															void operator()( opState_t& o, uint16_t& addr );					\
														};

#define ADDR_MODE_DEF( name )							void CpuZ80::addrMode##name::operator() ( opState_t& o, uint16_t& addr )

#define ADDR( name )									CpuZ80::addrMode##name

#define _OP_ADDR( num, name, lhs, rhs, ops, advance, cycles, bit, chk )															\
														{																		\
															opLUT[num].mnemonic		= #name;									\
															opLUT[num].lhsName		= #lhs;										\
															opLUT[num].rhsName		= #rhs;										\
															opLUT[num].type			= opType_t::##name;							\
															opLUT[num].operands		= ops;										\
															opLUT[num].baseCycles	= cycles;									\
															opLUT[num].pcInc		= advance;									\
															opLUT[num].lhsMode		= addrMode_t::##lhs;						\
															opLUT[num].rhsMode		= addrMode_t::##rhs;						\
															opLUT[num].func			= &CpuZ80::##name<ADDR(lhs),ADDR(rhs), bit, chk >; \
														}

#define OP_ADDR( num, name, lhs, rhs, ops, cycles )			_OP_ADDR( num, name, lhs, rhs, ops, ops, cycles, 0, 0 )
#define OP_JUMP( num, name, lhs, ops, cycles, bit, chk )	_OP_ADDR( num, name, lhs, NONE, ops, ops, cycles, bit, chk )
#define OP_BIT( num, name, rhs, bit, cycles )				_OP_ADDR( num + 0xFF, name, rhs, rhs, 0, 0, cycles, bit, 0 )

class CpuZ80
{
public:
	static const uint32_t NumInstructions = 512;

	union {
		struct {
			uint8_t unused : 4;
			uint8_t z	: 1; // Zero
			uint8_t n	: 1; // Substration
			uint8_t hc	: 1; // Half carry
			uint8_t cy	: 1; // Carry
		};
		struct {
			uint8_t FLow	: 4; // F lower
			uint8_t psw		: 4; // F higher
		};
		struct {
			uint8_t F;
			uint8_t A;
		};
		uint16_t AF;
	};
	union {
		struct {
			uint8_t C;
			uint8_t B;
		};
		uint16_t BC;
	};
	union {
		struct {
			uint8_t E;
			uint8_t D;
		};
		uint16_t DE;
	};
	union {
		struct {
			uint8_t L;
			uint8_t H;
		};
		uint16_t HL;
	};
	uint16_t SP;
	uint16_t PC;

	uint8_t* const		r8[ 8 ] = { &A, &F, &C, &B, &E, &D, &L, &H };
	uint16_t* const		r16[ 6 ] = { &AF, &BC, &DE, &HL, &SP, &PC };

	GameboySystem*		system;

	uint16_t			nmiVector;
	uint16_t			irqVector;
	uint16_t			resetVector;
	bool				ime; // interrupt-mask-enable

	cpuCycle_t			cycle;

	opInfo_t			opLUT[ NumInstructions ];
	wtLog				dbgLog;

#if DEBUG_ADDR == 1
	bool				logToFile = false;
	int32_t				logFrameCount = 0;
	int32_t				logFrameTotal = 0;
#endif

private:
	bool				halt;

public:
	CpuZ80() {
		BuildOpLUT();

		AF = 0;
		BC = 0;
		DE = 0;
		HL = 0;
		SP = 0;
		PC = 0x0150; // FIXME: set by rom header
	}

	template <class LHS>
	void Store( opState_t& o, const uint16 value ) {
		uint16_t addr;
		LHS( *this )( o, addr );

		if ( LHS::type == addrType_t::REGISTER_8 ) {
			*r8[ addr ] = static_cast<uint8_t>( value & 0xFF );
		} else if ( LHS::type == addrType_t::REGISTER_16 ) {
			*r16[ addr ] = value;
		} else if ( LHS::type == addrType_t::MEMORY ) {
			WriteMemoryBus( addr, 0, static_cast<uint8_t>( value & 0xFF ) );
		}
		
		OpDebugInfo& dbgInfo = dbgLog.GetLogLine();
		dbgInfo.lhsAddrMode = static_cast<uint8_t>( LHS::addrMode );
		dbgInfo.lhsAddrType = static_cast<uint8_t>( LHS::type );
		dbgInfo.lhsAddress = addr;
		dbgInfo.lhsMemValue = static_cast<uint8_t>( value );
	}

	template <class RHS>
	void Load( opState_t& o, uint16_t& value ) {
		uint16_t addr;
		RHS( *this )( o, addr );

		if ( RHS::type == addrType_t::REGISTER_8 ) {
			value = *r8[ addr ];
		} else if ( RHS::type == addrType_t::REGISTER_16 ) {
			value = *r16[ addr ];
		} else if ( RHS::type == addrType_t::MEMORY ) {
			value = ReadMemoryBus( addr );
		}

		OpDebugInfo& dbgInfo = dbgLog.GetLogLine();
		dbgInfo.rhsAddrMode = static_cast<uint8_t>( RHS::addrMode );
		dbgInfo.rhsAddrType = static_cast<uint8_t>( RHS::type );
		dbgInfo.rhsAddress = addr;
		dbgInfo.rhsMemValue = static_cast<uint8_t>( value );
	}

	void		Push( const uint8_t value );
	void		PushWord( const uint16_t value );
	uint8_t		Pop();
	uint16_t	PopWord();

	void		RegisterSystem( GameboySystem* sys );
	bool		IsTraceLogOpen() const;
	void		StartTraceLog( const uint32_t frameCount );
	void		StopTraceLog();

	void		SetAluFlags( const uint16_t value );
	bool		CheckSign( const uint16_t checkValue );
	bool		CheckCarry( const uint16_t checkValue );
	bool		CheckZero( const uint16_t checkValue );
	bool		CheckOverflow( const uint16_t src, const uint16_t temp, const uint8_t finalValue );

	// FIXME: These just wrap main system functions that need to be used in headers
	uint8_t		ReadMemoryBus( const uint16_t address );
	void		WriteMemoryBus( const uint16_t address, const uint16_t offset, const uint8_t value );
	uint16_t	GetRestartAddress( const uint16_t highAddr, const uint16_t lowAddr ) const;

	uint8_t		ReadOperand( const uint16_t offset ) const;
	void		AdvancePC( const uint16_t places );
	cpuCycle_t	OpExec( const uint16_t instrAddr, const uint8_t byteCode );
	cpuCycle_t	Exec();
	bool		Step( const cpuCycle_t& nextCycle );

	#include "z80_ops"

	ADDR_MODE_DECL( NONE, UNSPECIFIED )
	ADDR_MODE_DECL( A, REGISTER_8 )
	ADDR_MODE_DECL( F, REGISTER_8 )
	ADDR_MODE_DECL( B, REGISTER_8 )
	ADDR_MODE_DECL( C, REGISTER_8 )
	ADDR_MODE_DECL( E, REGISTER_8 )
	ADDR_MODE_DECL( D, REGISTER_8 )
	ADDR_MODE_DECL( L, REGISTER_8 )
	ADDR_MODE_DECL( H, REGISTER_8 )
	ADDR_MODE_DECL( AF, REGISTER_16 )
	ADDR_MODE_DECL( BC, REGISTER_16 )
	ADDR_MODE_DECL( DE, REGISTER_16 )
	ADDR_MODE_DECL( HL, REGISTER_16 )
	ADDR_MODE_DECL( SP, REGISTER_16 )
	ADDR_MODE_DECL( PC, REGISTER_16 )
	ADDR_MODE_DECL( DIRECT_BC, MEMORY )
	ADDR_MODE_DECL( DIRECT_DE, MEMORY )
	ADDR_MODE_DECL( DIRECT_HL, MEMORY )
	ADDR_MODE_DECL( DIRECT_C, MEMORY )
	ADDR_MODE_DECL( DIRECT_N8, MEMORY )
	ADDR_MODE_DECL( DIRECT_N16, MEMORY )
	ADDR_MODE_DECL( IMMEDIATE_N8, MEMORY )
	ADDR_MODE_DECL( IMMEDIATE_N16, MEMORY )
};
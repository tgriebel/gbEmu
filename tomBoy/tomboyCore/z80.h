#pragma once

#include <stdint.h>
#include "common.h"

class CpuZ80;
class GameboySystem;

enum class opType_t : uint8_t
{
	NOP, ILL,
	LD, LDI, LD_INC, LD_DEC,
	INC, DEC, ADD, ADC, SUB, SBC,
	OR, XOR, AND, BIT, RES, SET,
	RLC, RRC, RR, RL, SLA, SRA, DA, RLA, SRL, RLCA, RRCA,
	HALT, JP, JR, CALL,
	PUSH, POP, DI, EI, RST, RET, RETI, 
	STOP, SWAP, CP, CPL, CCF, DDA, SCF
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
	IM_8_A,
	IM_16_A,
	C_A,
	BC_A,
	DE_A,
	HL_A,
	IM_8,
	IM_16,
};

enum regIndex {
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
};

enum addrType_t {
	UNSPECIFIED = 0,
	REGISTER_8 = 1,
	REGISTER_16 = 2,
	MEMORY = 3,
};

struct opState_t
{
	opState_t() : opCode( 0 ), op0( 0 ), op1( 0 ) {}

	cpuCycle_t		opCycles;
	uint8_t			opCode;
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
	opType_t	type		: 7;
	addrMode_t	lhsMode		: 5;
	addrMode_t	rhsMode		: 5;
	uint8_t		operands	: 2;
	uint8_t		baseCycles	: 4;
	uint8_t		pcInc		: 2;
};

union u8i8 {
	uint8_t	u8;
	int8_t	i8;
};

union u16i16 {
	uint8_t	u16;
	int8_t	i16;
};

union u32i32 {
	uint8_t	u32;
	int8_t	i32;
};

#define OP_DEF( name )									template <class LHS, class RHS, size_t BIT >							\
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

#define _OP_ADDR( num, name, lhs, rhs, ops, advance, cycles, bit )																	\
														{																		\
															opLUT[num].mnemonic		= #name;									\
															opLUT[num].type			= opType_t::##name;							\
															opLUT[num].operands		= ops;										\
															opLUT[num].baseCycles	= cycles;									\
															opLUT[num].pcInc		= advance;									\
															opLUT[num].lhsMode		= addrMode_t::##lhs;						\
															opLUT[num].rhsMode		= addrMode_t::##rhs;						\
															opLUT[num].func			= &CpuZ80::##name<ADDR(lhs),ADDR(rhs), bit>;\
														}

#define OP_ADDR( num, name, lhs, rhs, ops, cycles )		_OP_ADDR( num, name, lhs, rhs, ops, ops, cycles, 0 )
#define OP_BIT( num, name, rhs, bit, cycles )			_OP_ADDR( num + 0xFF, name, rhs, rhs, 0, 0, cycles, bit )

class CpuZ80
{
public:
	static const uint32_t NumInstructions = 512;

	union {
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

	uint8_t* const r8[ 8 ] = { &A, &F, &C, &B, &E, &D, &L, &H };
	uint16_t* const r16[ 6 ] = { &AF, &BC, &DE, &HL, &SP, &PC };

	uint8_t memory[ 1024 * 16 ];
	GameboySystem* system;

	union {
		struct {
			uint8_t Z : 1;
			uint8_t N : 1;
			uint8_t H : 1;
			uint8_t C : 1;
		};
		uint16_t flags;
	};

	opInfo_t opLUT[ NumInstructions ];

	CpuZ80() {
		BuildOpLUT();
	}

	template <class LHS>
	void Store( opState_t& opState, const uint16 value ) {
		uint16_t addr;
		LHS( *this )( opState, addr );

		if ( LHS::type == addrType_t::REGISTER_8 ) {
			*r8[ addr ] = static_cast<uint8_t>( value & 0xFF );
		} else if ( LHS::type == addrType_t::REGISTER_16 ) {
			*r16[ addr ] = value;
		} else if ( LHS::type == addrType_t::MEMORY ) {
			memory[ addr ] = static_cast<uint8_t>( value & 0xFF );
		}
	}

	template <class RHS>
	void Load( opState_t& opState, uint16_t& value ) {
		uint16_t addr;
		RHS( *this )( opState, addr );

		if ( RHS::type == addrType_t::REGISTER_8 ) {
			value = *r8[ addr ];
		} else if ( RHS::type == addrType_t::REGISTER_16 ) {
			value = *r16[ addr ];
		} else if ( RHS::type == addrType_t::MEMORY ) {
			value = memory[ addr ];
		}
	}

	uint8_t		ReadOperand( const uint16_t offset ) const;
	void		AdvancePC( const uint16_t places );
	cpuCycle_t	OpExec( const uint16_t instrAddr, const uint8_t byteCode );

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
	ADDR_MODE_DECL( BC_A, MEMORY )
	ADDR_MODE_DECL( DE_A, MEMORY )
	ADDR_MODE_DECL( HL_A, MEMORY )
	ADDR_MODE_DECL( C_A, MEMORY )
	ADDR_MODE_DECL( IM_8_A, MEMORY )
	ADDR_MODE_DECL( IM_16_A, MEMORY )
	ADDR_MODE_DECL( IM_8, MEMORY )
	ADDR_MODE_DECL( IM_16, MEMORY )
};
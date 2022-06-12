#include "z80.h"
#include "gbSystem.h"

ADDR_MODE_DEF( NONE ) {
	addr = 0;
}


ADDR_MODE_DEF( A ) {
	addr = A_IX;
}

ADDR_MODE_DEF( F ) {
	addr = F_IX;
}

ADDR_MODE_DEF( B ) {
	addr = B_IX;
}

ADDR_MODE_DEF( C ) {
	addr = C_IX;
}

ADDR_MODE_DEF( E ) {
	addr = E_IX;
}

ADDR_MODE_DEF( D ) {
	addr = D_IX;
}

ADDR_MODE_DEF( L ) {
	addr = L_IX;
}

ADDR_MODE_DEF( H ) {
	addr = H_IX;
}

ADDR_MODE_DEF( AF ) {
	addr = AF_IX;
}

ADDR_MODE_DEF( BC ) {
	addr = BC_IX;
}

ADDR_MODE_DEF( DE ) {
	addr = DE_IX;
}

ADDR_MODE_DEF( HL ) {
	addr = HL_IX;
}

ADDR_MODE_DEF( SP ) {
	addr = SP_IX;
}

ADDR_MODE_DEF( PC ) {
	addr = PC_IX;
}

ADDR_MODE_DEF( BC_A ) {
	addr = cpu.BC;
}

ADDR_MODE_DEF( DE_A ) {
	addr = cpu.DE;
}

ADDR_MODE_DEF( HL_A ) {
	addr = cpu.HL;
}

ADDR_MODE_DEF( C_A ) {
	addr = cpu.C;
}

ADDR_MODE_DEF( IM_8_A ) {
	assert( 0 );
	addr = o.op0;
}

ADDR_MODE_DEF( IM_16_A ) {
	assert( 0 );
	addr = o.op0;
}

ADDR_MODE_DEF( IM_8 ) {
	assert( 0 );
	addr = o.op0;
}

ADDR_MODE_DEF( IM_16 ) {
	assert( 0 );
	addr = o.op;
}

void CpuZ80::SetAluFlags( const uint16_t value ) {
	fl = 0;
	z = CheckZero( value );
}

bool CpuZ80::CheckSign( const uint16_t checkValue ) {
	return ( checkValue & 0x0080 );
}

bool CpuZ80::CheckCarry( const uint16_t checkValue ) {
	return ( checkValue > 0x00ff );
}

bool CpuZ80::CheckZero( const uint16_t checkValue ) {
	return ( checkValue == 0 );
}

bool CpuZ80::CheckOverflow( const uint16_t src, const uint16_t temp, const uint8_t finalValue )
{
	const uint8_t signedBound = 0x80;
	return CheckSign( finalValue ^ src ) && CheckSign( temp ^ src ) && !CheckCarry( temp );
}

void CpuZ80::AdvancePC( const uint16_t places ) {
	PC += places;
}

uint8_t CpuZ80::ReadOperand( const uint16_t offset ) const {
	return system->ReadMemory( PC + offset );
}

cpuCycle_t CpuZ80::OpExec( const uint16_t instrAddr, const uint8_t byteCode ) {
	const opInfo_t& op = opLUT[ byteCode ];

	opState_t opState;
	opState.opCode = byteCode;
	opState.opCycles = cpuCycle_t( op.baseCycles );
	opState.op0 = 0;
	opState.op1 = 0;

	( this->*( op.func ) )( opState );

	return opState.opCycles;
}
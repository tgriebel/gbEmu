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

ADDR_MODE_DEF( DIRECT_BC ) {
	addr = cpu.BC;
}

ADDR_MODE_DEF( DIRECT_DE ) {
	addr = cpu.DE;
}

ADDR_MODE_DEF( DIRECT_HL ) {
	addr = cpu.HL;
}

ADDR_MODE_DEF( DIRECT_C ) {
	addr = cpu.C + 0xFF00;
}

ADDR_MODE_DEF( DIRECT_N8 ) {
	addr = o.op0 + 0xFF00;
}

ADDR_MODE_DEF( DIRECT_N16 ) {
	addr = o.op0 + 0xFF00;
}

ADDR_MODE_DEF( IMMEDIATE_N8 ) {
	addr = o.op0;
}

ADDR_MODE_DEF( IMMEDIATE_N16 ) {
	addr = o.op;
}

uint8_t CpuZ80::ReadMemoryBus( const uint16_t address ) {
	return system->ReadMemory( address );
}

void CpuZ80::WriteMemoryBus( const uint16_t address, const uint16_t offset, const uint8_t value ) {
	system->WriteMemory( address, offset, value );
}

void CpuZ80::Push( const uint8_t value )
{
	system->GetStack() = value;
	SP--;
}

void CpuZ80::PushWord( const uint16_t value )
{
	Push( ( value >> 8 ) & 0xFF );
	Push( value & 0xFF );
}

uint8_t CpuZ80::Pop()
{
	SP++;
	return system->GetStack();
}

uint16_t CpuZ80::PopWord()
{
	const uint8_t loByte = Pop();
	const uint8_t hiByte = Pop();

	return Combine( loByte, hiByte );
}

void CpuZ80::SetAluFlags( const uint16_t value ) {
	psw = CheckZero( value );
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

uint16_t CpuZ80::GetRestartAddress( const uint16_t highAddr, const uint16_t lowAddr ) const {
	const uint8_t high = system->ReadMemory( highAddr );
	const uint8_t low = system->ReadMemory( lowAddr );
	return ( high << 8 ) | low;
}

uint8_t CpuZ80::ReadOperand( const uint16_t offset ) const {
	return system->ReadMemory( PC + offset );
}

void CpuZ80::RegisterSystem( GameboySystem* sys )
{
	system = sys;
}


bool CpuZ80::IsTraceLogOpen() const
{
	return ( logFrameCount > 0 );
}


void CpuZ80::StartTraceLog( const uint32_t frameCount )
{
	logFrameCount = static_cast<int32_t>( frameCount );
	logFrameTotal = logFrameCount;

	dbgLog.Reset( logFrameTotal );
}


void CpuZ80::StopTraceLog()
{
	//	logFrameCount = logFrameTotal;
}

cpuCycle_t CpuZ80::OpExec( const uint16_t instrAddr, const uint8_t byteCode ) {
	const opInfo_t& op = opLUT[ byteCode ];

	opState_t opState;
	opState.opCode = byteCode;
	opState.opCycles = cpuCycle_t( op.baseCycles );
	opState.op0 = op.operands >= 1 ? system->ReadMemory( instrAddr + 1 ) : 0;
	opState.op1 = op.operands == 2 ? system->ReadMemory( instrAddr + 2 ) : 0;

#if DEBUG_ADDR == 1
	if ( IsTraceLogOpen() )
	{
		OpDebugInfo& instrDbg = dbgLog.NewLine();
		instrDbg.loadCnt = 0;
		instrDbg.storeCnt = 0;
		instrDbg.byteCode = opState.opCode;
		instrDbg.opType = static_cast<uint8_t>( op.type );
		instrDbg.regInfo = { A, F, B, C, D, E, H, L, psw, SP, PC };
		instrDbg.instrBegin = instrAddr;
		instrDbg.cpuCycles = cycle.count();
		instrDbg.instrCycles = opState.opCycles.count();
		instrDbg.op0 = opState.op0;
		instrDbg.op1 = opState.op1;
		instrDbg.mnemonic = op.mnemonic;
		instrDbg.lhsName = op.lhsName;
		instrDbg.rhsName = op.rhsName;
		instrDbg.operands = op.operands;
		instrDbg.bitCheck = op.bitCheck;
		instrDbg.check = op.check;
	}
#endif

	( this->*( op.func ) )( opState );

	AdvancePC( op.pcInc );

	return opState.opCycles;
}

cpuCycle_t CpuZ80::Exec()
{
	cpuCycle_t opCycle = cpuCycle_t( 0 );

	//if ( oamInProcess )
	//{
	//	// http://wiki.nesdev.com/w/index.php/PPU_registers#OAMDMA
	//	if ( ( cycle.count() % 2 ) == 0 ) {
	//		opCycle += cpuCycle_t( 514 );
	//	}
	//	else {
	//		opCycle += cpuCycle_t( 513 );
	//	}
	//	oamInProcess = false;

	//	return opCycle;
	//}
	//else if ( dmcTransfer )
	//{
	//	return cpuCycle_t( 1 );
	//}

	const uint16_t instrAddr = PC;

	const uint8_t curbyte = system->ReadMemory( instrAddr );
	AdvancePC( 1 );

	//if ( interruptRequestNMI )
	//{
	//	NMI( irqAddr );
	//	interruptRequestNMI = false;
	//	if ( IsTraceLogOpen() )
	//	{
	//		OpDebugInfo& instrDbg = dbgLog.NewLine();
	//		instrDbg.nmi = true;
	//		instrDbg.cpuCycles = cycle.count();
	//	}
	//	return opCycle;
	//}
	//else if ( interruptRequest )
	//{
	//	IRQ();
	//	interruptRequest = false;
	//	if ( IsTraceLogOpen() )
	//	{
	//		OpDebugInfo& instrDbg = dbgLog.NewLine();
	//		instrDbg.irq = true;
	//		instrDbg.cpuCycles = cycle.count();
	//	}
	//	return opCycle;
	//}

	return OpExec( instrAddr, curbyte );
}

bool CpuZ80::Step( const cpuCycle_t& nextCycle )
{
	while ( ( cycle < nextCycle ) && !halt ) {
		cycle += cpuCycle_t( Exec() );
	}
	return !halt;
}
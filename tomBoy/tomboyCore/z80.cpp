#include "z80.h"
#include "gbSystem.h"

#if DEBUG_ADDR == 1
#define TRACE_RHS( addr, value )														\
	if( cpu.IsTraceLogOpen() )															\
	{																					\
		OpDebugInfo& dbgInfo = cpu.dbgLog.GetLogLine();									\
		dbgInfo.rhsAddrMode = static_cast<uint8_t>( addrMode );							\
		dbgInfo.rhsAddrType = static_cast<uint8_t>( type );								\
		dbgInfo.rhsAddress = addr;														\
		dbgInfo.rhsMemValue = static_cast<uint8_t>( value );							\
	}

#define TRACE_LHS( addr, value )														\
	if( cpu.IsTraceLogOpen() )															\
	{																					\
		OpDebugInfo& dbgInfo = cpu.dbgLog.GetLogLine();									\
		dbgInfo.lhsAddrMode = static_cast<uint8_t>( addrMode );							\
		dbgInfo.lhsAddrType = static_cast<uint8_t>( type );								\
		dbgInfo.lhsAddress = addr;														\
		dbgInfo.lhsMemValue = static_cast<uint8_t>( value );							\
	}
#else
#define DEBUG_ADDR_RHS		{}
#define DEBUG_ADDR_LHS		{}
#endif

ADDR_MODE_RHS_DEF( NONE )
{
	value = 0;
}

ADDR_MODE_LHS_DEF( NONE )
{
	value = 0;
}

ADDR_MODE_RHS_DEF( A )
{
	value = cpu.A;

	TRACE_RHS( cpu.A, value );
}

ADDR_MODE_LHS_DEF( A )
{
	cpu.A = static_cast<uint8_t>( value );

	TRACE_LHS( cpu.A, value );
}

ADDR_MODE_RHS_DEF( F )
{
	value = cpu.F;

	TRACE_RHS( cpu.F, value );
}

ADDR_MODE_LHS_DEF( F )
{
	cpu.F = static_cast<uint8_t>( value );

	TRACE_LHS( cpu.F, value );
}

ADDR_MODE_RHS_DEF( B )
{
	value = cpu.B;

	TRACE_RHS( cpu.B, value );
}

ADDR_MODE_LHS_DEF( B )
{
	cpu.B = static_cast<uint8_t>( value );

	TRACE_LHS( cpu.B, value );
}

ADDR_MODE_RHS_DEF( C )
{
	value = cpu.C;

	TRACE_RHS( cpu.C, value );
}

ADDR_MODE_LHS_DEF( C )
{
	cpu.C = static_cast<uint8_t>( value );

	TRACE_LHS( cpu.C, value );
}

ADDR_MODE_RHS_DEF( E )
{
	value = cpu.E;

	TRACE_RHS( cpu.E, value );
}

ADDR_MODE_LHS_DEF( E )
{
	cpu.E = static_cast<uint8_t>( value );

	TRACE_LHS( cpu.E, value );
}

ADDR_MODE_RHS_DEF( D )
{
	value = cpu.D;

	TRACE_RHS( cpu.D, value );
}

ADDR_MODE_LHS_DEF( D )
{
	cpu.D = static_cast<uint8_t>( value );

	TRACE_LHS( cpu.D, value );
}

ADDR_MODE_RHS_DEF( L )
{
	value = cpu.L;

	TRACE_RHS( cpu.L, value );
}

ADDR_MODE_LHS_DEF( L )
{
	cpu.L = static_cast<uint8_t>( value );

	TRACE_LHS( cpu.L, value );
}

ADDR_MODE_RHS_DEF( H )
{
	value = cpu.H;

	TRACE_RHS( cpu.H, value );
}

ADDR_MODE_LHS_DEF( H )
{
	cpu.H = static_cast<uint8_t>( value );

	TRACE_LHS( cpu.H, value );
}

ADDR_MODE_RHS_DEF( AF )
{
	value = cpu.AF;

	TRACE_RHS( cpu.AF, value );
}

ADDR_MODE_LHS_DEF( AF )
{
	cpu.AF = value;

	TRACE_LHS( cpu.AF, value );
}

ADDR_MODE_RHS_DEF( BC )
{
	value = cpu.BC;

	TRACE_RHS( cpu.BC, value );
}

ADDR_MODE_LHS_DEF( BC )
{
	cpu.BC = value;

	TRACE_LHS( cpu.BC, value );
}

ADDR_MODE_RHS_DEF( DE )
{
	value = cpu.DE;

	TRACE_RHS( cpu.DE, value );
}

ADDR_MODE_LHS_DEF( DE )
{
	cpu.DE = value;

	TRACE_LHS( cpu.DE, value );
}

ADDR_MODE_RHS_DEF( HL )
{
	value = cpu.HL;

	TRACE_RHS( cpu.HL, value );
}

ADDR_MODE_LHS_DEF( HL )
{
	cpu.HL = value;

	TRACE_LHS( cpu.HL, value );
}

ADDR_MODE_RHS_DEF( SP )
{
	value = cpu.SP;

	TRACE_RHS( cpu.SP, value );
}

ADDR_MODE_LHS_DEF( SP )
{
	cpu.SP = value;

	TRACE_LHS( cpu.SP, value );
}

ADDR_MODE_RHS_DEF( DIRECT_BC )
{
	value = cpu.ReadMemoryBus( cpu.BC );

	TRACE_RHS( cpu.BC, value );
}

ADDR_MODE_LHS_DEF( DIRECT_BC )
{
	cpu.WriteMemoryBus( cpu.BC, 0, static_cast<uint8_t>( value & 0xFF ) );

	TRACE_LHS( cpu.BC, value & 0xFF );
}

ADDR_MODE_RHS_DEF( DIRECT_DE )
{
	value = cpu.ReadMemoryBus( cpu.DE );

	TRACE_RHS( cpu.DE, value );
}

ADDR_MODE_LHS_DEF( DIRECT_DE )
{
	cpu.WriteMemoryBus( cpu.DE, 0, static_cast<uint8_t>( value & 0xFF ) );

	TRACE_LHS( cpu.DE, value & 0xFF );
}

ADDR_MODE_RHS_DEF( DIRECT_HL )
{
	value = cpu.ReadMemoryBus( cpu.HL );

	TRACE_RHS( cpu.HL, value );
}

ADDR_MODE_LHS_DEF( DIRECT_HL )
{
	cpu.WriteMemoryBus( cpu.HL, 0, static_cast<uint8_t>( value & 0xFF ) );

	TRACE_LHS( cpu.HL, value & 0xFF );
}

ADDR_MODE_RHS_DEF( DIRECT_C )
{
	const uint16_t addr = cpu.C + 0xFF00;
	value = cpu.ReadMemoryBus( addr );

	TRACE_RHS( addr, value );
}

ADDR_MODE_LHS_DEF( DIRECT_C )
{
	const uint16_t addr = cpu.C + 0xFF00;
	cpu.WriteMemoryBus( addr, 0, static_cast<uint8_t>( value & 0xFF ) );

	TRACE_LHS( addr, value & 0xFF );
}

ADDR_MODE_RHS_DEF( DIRECT_N8 )
{
	const uint16_t addr = o.op0 + 0xFF00;
	value = cpu.ReadMemoryBus( addr );

	TRACE_RHS( addr, value );
}

ADDR_MODE_LHS_DEF( DIRECT_N8 )
{
	const uint16_t addr = o.op0 + 0xFF00;
	cpu.WriteMemoryBus( addr, 0, static_cast<uint8_t>( value & 0xFF ) );

	TRACE_LHS( addr, value );
}

ADDR_MODE_RHS_DEF( DIRECT_N16 )
{
	const uint16_t addr = Combine( o.op0, o.op1 );
	value = cpu.ReadMemoryBus( addr );

	TRACE_RHS( addr, value );
}

ADDR_MODE_LHS_DEF( DIRECT_N16 )
{
	const uint16_t addr = Combine( o.op0, o.op1 );
	cpu.WriteMemoryBus( addr, 0, static_cast<uint8_t>( value & 0xFF ) );

	TRACE_RHS( addr, value & 0xFF );
}

ADDR_MODE_RHS_DEF( IMMEDIATE_N8 )
{
	value = o.op0;

	TRACE_RHS( o.op0, value );
}

ADDR_MODE_LHS_DEF( IMMEDIATE_N8 )
{
	o.op0 = value & 0xFF;

	TRACE_LHS( o.op0, value & 0xFF );
}

ADDR_MODE_RHS_DEF( IMMEDIATE_N16 )
{
	value = Combine( o.op0, o.op1 );

//	TRACE_RHS( o.op0, value );
}

ADDR_MODE_LHS_DEF( IMMEDIATE_N16 )
{
	assert( 0 );
}

uint8_t CpuZ80::ReadMemoryBus( const uint16_t address ) {
	return system->ReadMemory( address );
}

void CpuZ80::WriteMemoryBus( const uint16_t address, const uint16_t offset, const uint8_t value ) {
	system->WriteMemory( address, offset, value );
}

void CpuZ80::Push( const uint8_t value )
{
	--SP;
	system->GetStack() = value;
}

void CpuZ80::PushWord( const uint16_t value )
{
	Push( ( value >> 8 ) & 0xFF );
	Push( value & 0xFF );
}

uint8_t CpuZ80::Pop()
{
	const uint8_t value = system->GetStack();
	++SP;
	return value;
}

uint16_t CpuZ80::PopWord()
{
	const uint8_t loByte = Pop();
	const uint8_t hiByte = Pop();

	return Combine( loByte, hiByte );
}

bool CpuZ80::CheckSign( const uint16_t checkValue ) {
	return ( checkValue & 0x0080 );
}

uint8_t CpuZ80::CheckZero( const uint16_t checkValue ) {
	return ( ( checkValue & 0xFF ) == 0 ) ? 0x01 : 0x00;
}

uint8_t CpuZ80::CheckHalfCarry( const uint16_t checkValue ) {
	return ( ( checkValue & 0xFF ) > 0x000F ) ? 0x01 : 0x00;
}

uint8_t CpuZ80::CheckCarry( const uint16_t checkValue ) {
	return ( checkValue > 0x00FF ) ? 0x01 : 0x00;
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

cpuCycle_t CpuZ80::OpExec( const uint16_t instrAddr, const uint8_t opCode, const bool bitOp ) {
	const uint16_t opcodeIndex = bitOp ? ( opCode + 0x100 ) : opCode;
	const opInfo_t& op = opLUT[ opcodeIndex ];
	assert( op.baseCycles > 0 );

	assert( ( bitOp && op.operands == 0 ) || !bitOp );

	opState_t opState;
	opState.opCode = opCode;
	opState.opCycles = cpuCycle_t( op.baseCycles );
	opState.op0 = op.operands >= 1 ? system->ReadMemory( instrAddr + 1 ) : 0;
	opState.op1 = op.operands == 2 ? system->ReadMemory( instrAddr + 2 ) : 0;

#if DEBUG_ADDR == 1
	if ( IsTraceLogOpen() )
	{
		OpDebugInfo instrDbg {};
		instrDbg.loadCnt = 0;
		instrDbg.storeCnt = 0;
		instrDbg.byteCode = opState.opCode;
		instrDbg.bitOp = bitOp;
		instrDbg.opType = static_cast<uint8_t>( op.type );
		instrDbg.regInfo = { A, F, B, C, D, E, H, L, psw, SP, PC };
		instrDbg.instrBegin = instrAddr;
		instrDbg.cpuCycles = cycle.count();
		instrDbg.instrCycles = opState.opCycles.count();
		instrDbg.nextByte0 = system->ReadMemory( instrAddr + 1 );
		instrDbg.nextByte1 = system->ReadMemory( instrAddr + 2 );
		instrDbg.nextByte2 = system->ReadMemory( instrAddr + 3 );
		instrDbg.mnemonic = op.mnemonic;
		instrDbg.lhsName = op.lhsName;
		instrDbg.rhsName = op.rhsName;
		instrDbg.operands = op.operands;
		instrDbg.bitCheck = op.bitCheck;
		instrDbg.check = op.check;

		dbgLog.NewLine( instrDbg );
	}
#endif

	( this->*( op.func ) )( opState );

	AdvancePC( op.pcInc );

	return opState.opCycles;
}

cpuCycle_t CpuZ80::Exec()
{
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

	return 0; 
}

bool CpuZ80::Step( const cpuCycle_t& nextCycle )
{
	if( ( IF.byte & IE.byte ) != 0 && ime == true )
	{
		PushWord( PC );

		if( IF.vblank & IE.vblank ) {
			PC = VBlankInterruptAddr;
		} else if ( IF.lcd & IE.lcd ) {
			PC = StatInterruptAddr;
		} else if ( IF.serial & IE.serial ) {
			PC = SerialInterruptAddr;
		} else if ( IF.joypad & IE.joypad ) {
			PC = JoypadInterruptAddr;
		}
		IF.byte = 0;
		ime = false;
		halt = false;
	}

	while ( ( cycle < nextCycle ) && !halt )
	{
		cpuCycle_t opCycle = cpuCycle_t( 0 );

		const uint16_t instrAddr = PC;
		uint8_t curbyte = system->ReadMemory( PC );

		static uint16_t bpAddr = 0x0226;
		if( instrAddr == bpAddr ) {
			cout << "Hit BP" << endl;
		}

		bool bitOp = false;
		if ( curbyte == 0xCB )
		{
			AdvancePC( 1 );
			bitOp = true;
			curbyte = system->ReadMemory( PC );
		}
		AdvancePC( 1 );

		// Execute
		opCycle = OpExec( instrAddr, curbyte, bitOp );
		
		cycle += cpuCycle_t( opCycle );
		
		if( instructionCount == pendingInterruptEnable ) {
			pendingInterruptEnable = 0;
			ime = true;
		}
		++instructionCount;
	}
	return !halt;
}
#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <Windows.h>
#include <string>
#include <wchar.h>
#include <sstream>

#include "z80.h"
#include "gbSystem.h"
#include "debug.h"

GameboySystem gbSystem;

void LoadGameboyFile( const std::wstring& fileName, unique_ptr<gbCart>& outCart );

int main()
{
	LoadGameboyFile( L"C:\\Users\\thoma\\source\\repos\\gbEmu\\tomBoy\\tomboyCore\\Games\\Alleyway.gb", gbSystem.cart );
	gbSystem.cpu.PC = 0x0100;
	gbSystem.cpu.system = &gbSystem;
	gbSystem.cpu.A = 0x8F;
	gbSystem.cpu.B = 0xF8;

	gbSystem.cpu.dbgLog.Reset( 10 );
	gbSystem.cpu.StartTraceLog( 1 );
	gbSystem.cpu.Step( 1 );

	string logBuffer;
	gbSystem.cpu.dbgLog.ToString( logBuffer, 0, 1 );
	std::cout << "Log: " << logBuffer << std::endl;
	return 0;
}
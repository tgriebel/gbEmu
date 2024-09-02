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
	//LoadGameboyFile( L"Games/Alleyway.gb", gbSystem.cart );
	LoadGameboyFile( L"Tests/01-special.gb", gbSystem.cart );
	gbSystem.cart->mapper = gbSystem.AssignMapper( gbSystem.cart->GetMapperId() );
	gbSystem.cart->mapper->system = &gbSystem;
	gbSystem.cart->mapper->OnLoadCpu();
	gbSystem.cart->mapper->OnLoadPpu();

	gbSystem.cpu.system = &gbSystem;

	gbSystem.cpu.dbgLog.Reset( 100 );
	gbSystem.cpu.StartTraceLog( 1 );

	for ( int i = 0; i < 1000; ++i ) {
		gbSystem.cpu.Step( i + 1 );
	}

	string logBuffer;
	gbSystem.cpu.dbgLog.ToString( logBuffer, 0, 1 );
	std::cout << "Log:\n" << logBuffer << std::endl;
	return 0;
}
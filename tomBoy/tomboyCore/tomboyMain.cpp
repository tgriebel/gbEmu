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

GameboySystem gbSystem;
CpuZ80 gbZ80;

int main()
{
	gbZ80.system = &gbSystem;
	gbZ80.A = 0x8F;
	gbZ80.B = 0xF8;
	gbZ80.OpExec( 0, 0xA8 );
	//gbSystem.Store< CpuZ80::addrModeA >( 0 );
	//std::cout << "Test: " << std::hex << gbSystem.AF << std::endl;
	return gbZ80.A;
}
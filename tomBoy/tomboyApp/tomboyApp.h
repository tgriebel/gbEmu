#pragma once

#define IMGUI_ENABLE
#define MAX_LOADSTRING ( 100 )

#include <combaseapi.h>
#include <comdef.h>
#include <wrl/client.h>
#include <shobjidl.h>
#include <queue>
#include <algorithm>
#include "resource.h"
#include "..\tomboyCore\common.h"
#include "..\tomboyCore\gbSystem.h"
#include "..\tomboyCore\input.h"
#include "..\tomboyCore\timer.h"
#ifdef IMGUI_ENABLE
#pragma warning(push, 0)        
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_memory_editor/imgui_memory_editor.h"
#pragma warning(pop)
#endif

static const uint32_t	FrameCount = 2;
static const uint32_t	FrameResultCount = ( FrameCount + 0 );

ATOM					RegisterWindow( HINSTANCE hInstance );
BOOL					InitAppInstance( HINSTANCE, int );
LRESULT CALLBACK		WndProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK		About( HWND, UINT, WPARAM, LPARAM );

class wtRenderer;
struct wtAudioEngine;

struct wtAppDebug_t
{
	static const uint32_t	MaxPrgRomBanks = 128;
	static const uint32_t	MaxChrRomBanks = 32;

	string					prgRomAsm[ MaxPrgRomBanks ];
	wtPatternTableImage		chrRom[ MaxChrRomBanks ];
};


struct timers_t
{
	Timer					elapsedCopyTimer;
	double					elapsedCopyTime;

	Timer					copyTimer;
	double					copyTime;

	Timer					audioSubmitTimer;
	double					audioSubmitTime;

	Timer					runTime;
};


class wtAppInterface
{
private:
	bool				pause;
	bool				reset;
	bool				emulatorRunning;

public:
	
	wtAppInterface()
	{
		emulatorRunning = true;
		pause = false;
		reset = true;
		refreshChrRomTables = false;

		r = nullptr;
		audio = nullptr;
		system = nullptr;

		currentSoundBuffer.Reset();

		for ( uint32_t i = 0; i < FrameResultCount; ++i )
		{
			frameSubmitWriteLock[ i ] = CreateSemaphore( NULL, 1, 1, NULL );
			frameSubmitReadLock[ i ] = CreateSemaphore( NULL, 1, 1, NULL );
			audioWriteLock[ i ] = CreateSemaphore( NULL, 1, 1, NULL );
			audioReadLock[ i ] = CreateSemaphore( NULL, 1, 1, NULL );
		}
		workerLock = CreateSemaphore( NULL, 1, 1, NULL );
	}

	~wtAppInterface()
	{
		for ( uint32_t i = 0; i < FrameResultCount; ++i )
		{
			CloseHandle( frameSubmitWriteLock[ i ] );
			CloseHandle( frameSubmitReadLock[ i ] );
			CloseHandle( audioWriteLock[ i ] );
			CloseHandle( audioReadLock[ i ] );
		}
		CloseHandle( workerLock );
	}

	std::string			traceLog;	
	bool				logUnpacked;

	uint32_t			clientWidth;
	uint32_t			clientHeight;

	wtFrameResult		frameResult[ FrameResultCount ];
	config_t			systemConfig;
	bool				refreshChrRomTables;
	wtSampleQueue		currentSoundBuffer;
	wtAppDebug_t		debugData;
	uint32_t			frameIx;

	HANDLE				frameSubmitWriteLock[ FrameResultCount ];
	HANDLE				frameSubmitReadLock[ FrameResultCount ];
	HANDLE				audioWriteLock[ FrameResultCount ];
	HANDLE				audioReadLock[ FrameResultCount ];
	HANDLE				workerLock;

	wtRenderer*			r;
	wtAudioEngine*		audio;
	GameboySystem*		system;
	timers_t			t;

	inline void ToggleEmulation() {
		pause = !pause;
	}

	inline void EnableEmulation() {
		pause = false;
	}

	inline void DisableEmulation() {
		pause = true;
	}

	inline bool IsEmulatorEnabled() {
		return !pause;
	}

	inline bool IsEmulatorRunning() {
		return emulatorRunning;
	}

	inline void TerminateEmulator() {
		emulatorRunning = false;
	}

	inline void RequestReset() {
		reset = true;
	}

	inline void AcknowledgeReset() {
		reset = false;
	}

	inline bool IsResetPending() {
		return reset;
	}
};


// http://www.cplusplus.com/forum/beginner/14349/
static inline void ToClipboard( const std::string& s )
{
	OpenClipboard( 0 );
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc( GMEM_MOVEABLE, s.size() );
	if ( !hg )
	{
		CloseClipboard();
		return;
	}
	memcpy( GlobalLock( hg ), s.c_str(), s.size() );
	GlobalUnlock( hg );
	SetClipboardData( CF_TEXT, hg );
	CloseClipboard();
	GlobalFree( hg );
}


static inline ATOM RegisterWindow( HINSTANCE hInstance, WCHAR windowClass[ MAX_LOADSTRING ] )
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof( WNDCLASSEX );

    wcex.style              = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc        = WndProc;
    wcex.cbClsExtra         = 0;
    wcex.cbWndExtra         = 0;
    wcex.hInstance          = hInstance;
    wcex.hIcon              = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_WINTENDOAPP ) );
    wcex.hCursor            = LoadCursor( nullptr, IDC_ARROW );
    wcex.hbrBackground      = (HBRUSH)( COLOR_WINDOW + 1 );
    wcex.lpszMenuName       = MAKEINTRESOURCEW( IDC_WINTENDOAPP );
    wcex.lpszClassName      = windowClass;
    wcex.hIconSm            = LoadIcon( wcex.hInstance, MAKEINTRESOURCE( IDI_SMALL ) );

    return RegisterClassExW( &wcex );
}


static inline float GetQueueSample( void* data, int idx )
{
	const wtSampleQueue* quque = reinterpret_cast<wtSampleQueue*>( data );
	return quque->Peek( idx );
}


static inline const float NormalizeCoordinate( const uint32_t x, const uint32_t length )
{
	return ( 2.0f * ( x / static_cast<float>( length ) ) - 1.0f );
}


static inline void PrintLog( const std::string& msg )
{
	OutputDebugStringA( msg.c_str() );
}


static inline void OpenNesGame( std::wstring& nesFilePath )
{
	IFileOpenDialog* pFileOpen;

	// Create the FileOpenDialog object.
	HRESULT hr = CoCreateInstance( CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
		IID_IFileOpenDialog, reinterpret_cast<void**>( &pFileOpen ) );

	if ( !SUCCEEDED( hr ) )
		return;

	// Show the Open dialog box.
	hr = pFileOpen->Show( NULL );

	// Get the file name from the dialog box.
	if ( !SUCCEEDED( hr ) )
		return;

	IShellItem* pItem;
	hr = pFileOpen->GetResult( &pItem );

	if ( !SUCCEEDED( hr ) )
		return;

	PWSTR filePath = nullptr;
	hr = pItem->GetDisplayName( SIGDN_FILESYSPATH, &filePath );

	// Display the file name to the user.
	if ( !SUCCEEDED( hr ) )
		return;

	if ( filePath == nullptr )
		return;

	nesFilePath = std::wstring( filePath );
	CoTaskMemFree( filePath );

	pItem->Release();

	pFileOpen->Release();
	CoUninitialize();
}
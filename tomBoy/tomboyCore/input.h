#pragma once
#include "base.h"

#include "stdafx.h"
#include <stdint.h>
#include <map>

#include "common.h"

namespace TomBoy
{
	enum class ButtonFlags : uint8_t
	{
		BUTTON_NONE = 0X00,

		BUTTON_RIGHT = 0X01,
		BUTTON_LEFT = 0X02,
		BUTTON_DOWN = 0X04,
		BUTTON_UP = 0X08,

		BUTTON_START = 0X10,
		BUTTON_SELECT = 0X20,
		BUTTON_B = 0X40,
		BUTTON_A = 0X80,
	};
	DEFINE_ENUM_OPERATORS( ButtonFlags, uint8_t );

	struct mouse_t
	{
		int32_t x;
		int32_t y;
	};

	struct EXPORT_CLASS_DLL keyBinding_t
	{
		ButtonFlags		buttonFlags = ButtonFlags::BUTTON_NONE;
	};

	struct EXPORT_CLASS_DLL Input
	{
		ButtonFlags		keyBuffer;
		mouse_t			mousePoint;
		keyBinding_t	keyMap[ 256 ];
	};

	EXPORT_DLL ButtonFlags	GetKeyBuffer( const Input* input );
	EXPORT_DLL mouse_t		GetMouse( const Input* input );
	EXPORT_DLL void			BindKey( Input* input, const char key, const ButtonFlags button );
	EXPORT_DLL void			StoreKey( Input* input, const uint32_t key );
	EXPORT_DLL void			ReleaseKey( Input* input, const uint32_t key );
	EXPORT_DLL void			StoreMouseClick( Input* input, const int32_t x, const int32_t y );
	EXPORT_DLL void			ClearMouseClick( Input* input );
};
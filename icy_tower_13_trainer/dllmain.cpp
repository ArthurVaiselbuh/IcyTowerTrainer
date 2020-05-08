// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "IcyTowerConsts.h"
#include <ostream>
#include <thread>
volatile static bool should_break = false;

DWORD WINAPI  mainloop(LPVOID param)
{
	char debug_message[1024];
	// first we need to calculate address of GameEntity
	pIcyTowerModule base = (pIcyTowerModule) GetModuleHandle(NULL);
	pGameEntity game = reinterpret_cast<pGameEntity>( *(DWORD*)((long)base->addr_multiplier * 4 + (long)&base->addr));
#ifdef _DEBUG 
	snprintf(debug_message, sizeof(debug_message), "icy_tower_trainer: Base addr is %p , GameEntity addr is %p, points to %d\n", base, game, *(DWORD*)game);
	OutputDebugStringA(debug_message);
	snprintf(debug_message, sizeof(debug_message), "icy_tower_trainer: Calculated address of last_floor: %p", &game->last_floor);
	OutputDebugStringA(debug_message);
#endif
	while (!should_break)
	{
		//freeze floor
		game->last_floor = 4242;
		Sleep(20);
	}
	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
#ifdef _DEBUG
		OutputDebugStringA("icy_tower_trainer: Received message DLL_PROCESS_ATTACH");
#endif
		CreateThread(0, 0, mainloop, NULL, 0, NULL);
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
#ifdef _DEBUG
		OutputDebugStringA("icy_tower_trainer: Received message DLL_PROCESS_DETACH");
#endif
		should_break = true;
        break;
    }
    return TRUE;
}


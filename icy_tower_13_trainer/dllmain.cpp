// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "IcyTowerConsts.h"
#include <ostream>
#include <thread>
#include <boost/interprocess/ipc/message_queue.hpp>

using namespace boost::interprocess;
volatile static bool dll_should_detach = false;

//only print if in debug mode
void my_output_debug(const char* format, ...) {
#ifdef _DEBUG
	static char buffer[1024];
	static char buffer2[1024];
	va_list arglist;
	va_start(arglist, format);
	vsprintf_s(buffer, sizeof(buffer), format, arglist);
	va_end(arglist);
	sprintf_s(buffer2, "icy_tower_trainer: %s", buffer);
	OutputDebugStringA(buffer2);
#endif
}

DWORD WINAPI  mainloop(LPVOID param)
{
	Settings settings;
	//Dummy settings for now:
	settings._should_freeze_floor = true;
	settings._frozen_floor = 411;
	IcyTowerMessage cur_message;
	size_t recv_size = 0;
	unsigned int priority = 0;
	// first we need to calculate address of GameEntity
	pIcyTowerModule base = (pIcyTowerModule) GetModuleHandle(NULL);
	pGameEntity game = reinterpret_cast<pGameEntity>( *(DWORD*)((long)base->addr_multiplier * 4 + (long)&base->addr));
	my_output_debug("Base addr is %p , GameEntity addr is %p, points to %d\n", base, game, *(DWORD*)game);
	my_output_debug("Calculated address of last_floor: %p\n", &game->last_floor);

	//Create a message_queue.
	message_queue::remove(MSGQUEUE_NAME);
	message_queue mq
	(	create_only					//only create
		, MSGQUEUE_NAME				//name
		, 100                       //max message number
		, sizeof(IcyTowerMessage)	//max message size
	);

	while (!dll_should_detach)
	{
		//check if received a new message
		if (mq.try_receive(&cur_message, sizeof(cur_message), recv_size, priority)) {
			if (recv_size != sizeof(cur_message)) {
				my_output_debug("Received incorrect size: %d instead of %d", recv_size, sizeof(cur_message));
			}
			else {
				bool should_enable;
				my_output_debug("Received message: %d %d %d", cur_message._msgtype, cur_message._ftr, cur_message._value);
				switch (cur_message._msgtype) {
				case INVALID_MSG_TYPE:
					my_output_debug("Received invalid message!");
					break;
				case ChangeEnabledFeature:
					should_enable = bool(cur_message._value);
					switch (cur_message._ftr) {
					case INVALID_FEATURE:
						my_output_debug("Received invalid feature!");
					case FreezeFloor:
						my_output_debug("Settings should_freeze_floor to %d", cur_message._value);
						settings._should_freeze_floor = should_enable;
						break;
					case FreezeCurrentCombo:
						my_output_debug("Settings should_freeze_combo to %d", cur_message._value);
						settings._should_freeze_combo = should_enable;
						break;
					case BestCombo:
					case AdditionalScore:
						my_output_debug("This message feature is invalid for this message type");
						break;
					}
				case ChangeValueFeature:
					switch (cur_message._ftr) {
					case INVALID_FEATURE:
						my_output_debug("Received invalid feature!");
					case FreezeFloor:
						my_output_debug("Setting frozen_floor to %d", cur_message._value);
						settings._frozen_floor = cur_message._value;
						break;
					case FreezeCurrentCombo:
						my_output_debug("Setting frozen_combo to %d", cur_message._value);
						settings._frozen_combo = cur_message._value;
						break;
					case BestCombo:
						my_output_debug("Setting best_combo to %d", cur_message._value);
						game->best_combo = cur_message._value;
						break;
					case AdditionalScore:
						my_output_debug("Setting additional_score to %d", cur_message._value);
						game->additional_Score = cur_message._value;
						break;
					}
					break;
				case ChangeEnableAll:
					settings._is_running = bool(cur_message._value);
					break;


				}
			}
		}
		if (settings._is_running) {
			//check freeze values
			if (settings._should_freeze_combo) {
				game->current_combo = settings._frozen_combo;
			}
			if (settings._should_freeze_floor) {
				game->last_floor = settings._frozen_floor;
			}
		}
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
		my_output_debug("Received message DLL_PROCESS_ATTACH\n");
		CreateThread(0, 0, mainloop, NULL, 0, NULL);
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		my_output_debug("Received message DLL_PROCESS_DETACH\n");
		dll_should_detach = true;
        break;
    }
    return TRUE;
}


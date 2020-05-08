#pragma once
#include <Windows.h>

#define MSGQUEUE_NAME "ICYTOWER13TRAINER"

// this should be at the base of the IcyTower exe address
typedef struct _IcyTower13Module{
	BYTE Unknown0[0xCB908];
	DWORD addr_multiplier;
	BYTE Unknown1[0x14];
	DWORD_PTR addr;
} IcyTower13Module, *pIcyTowerModule;


// located at [icytower13.exe.addr_multiplier]*4+&icytower13.exe.addr
typedef struct _Entity {
	BYTE Unknown0[0x28];
	DWORD last_floor;		//0x28 last floor user stepped on
	DWORD additional_Score; //0x2c accumulated score, excluding floor*10
	BYTE Unknown1[0x14];	
	DWORD current_combo;	//0x44
	BYTE Unknown2[0x20];
	DWORD best_combo;

} GameEntity, *pGameEntity;

//////////////////////////////////////////////////////////////////////////////////

enum IcyTowerMessageType {
	INVALID_MSG_TYPE,
	ChangeEnabledFeature,	//enable specific feature
	ChangeValueFeature,		//change value of a feature
	ChangeEnableAll			//enable\disable trainer
};

// Available trainer options
enum IcyTowerFeature {
	INVALID_FEATURE,
	FreezeFloor,			//freeze last floor stepped on
	FreezeCurrentCombo,		//freeze current combo
	BestCombo,				//set best combo
	AdditionalScore			//set additional score
};

class IcyTowerMessage {
public:
	IcyTowerMessage(enum IcyTowerMessageType msgtype_, enum IcyTowerFeature ftr_, DWORD value_):_msgtype(msgtype_), _ftr(ftr_), _value(value_) {};
	IcyTowerMessage() : _msgtype(INVALID_MSG_TYPE), _ftr(INVALID_FEATURE), _value(0) {};
	enum IcyTowerMessageType _msgtype;
	enum IcyTowerFeature _ftr;
	DWORD _value; //if we need to pass value. for booleans - 0 is false, everything else is true
};

//This defines last received settings from the pipe 
class Settings{
public:
	Settings() :_is_running(true), _should_freeze_combo(false), _should_freeze_floor(false), _frozen_floor(0), _frozen_combo(0){};

	bool _is_running;			//is trainer running
	bool _should_freeze_floor;	
	bool _should_freeze_combo;
	DWORD _frozen_floor;
	DWORD _frozen_combo;
};
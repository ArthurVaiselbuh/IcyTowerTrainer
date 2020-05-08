#pragma once
#include <Windows.h>


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
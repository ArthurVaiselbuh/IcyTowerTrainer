#pragma once
#include <windows.h>
#include <string>
#include <ScopeGuard.h>

using namespace std;

class Injector {
public:
	Injector(const wstring& name_);
	Injector(DWORD pid_);
	~Injector();
	//inject given dll into the process
	bool inject(const wstring& dll_path_);
	//  in: name_ - name of process, 
	// out: pid 
	static bool get_pid_for_process_name(const wstring& name_, DWORD& pid);
	//in:	pHandle
	//out:	name
	static bool get_process_name_from_handle(HANDLE pHandle_, wstring& name);

	DWORD _pid;
	wstring _process_name;
private:
	void _internal_init(const wstring& name_, DWORD pid_);

	HANDLE _pHandle;			//process handle

};
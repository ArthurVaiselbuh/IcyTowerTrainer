#include "pch.h"
#include "Injector.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <iostream>
using namespace std;

void throw_format_error(const char* format, ...) {
	char buffer[1024];
	va_list arglist;
	va_start(arglist, format);
	vsprintf_s(buffer, sizeof(buffer), format, arglist);
	va_end(arglist);
	throw std::runtime_error(buffer);
}

Injector::Injector(const wstring& name_){
	DWORD pid = 0;
	if (!Injector::get_pid_for_process_name(name_, pid)) {
		throw_format_error("Failed getting pid for process name %s", name_);
	}
	_internal_init(name_, pid);
}

Injector::Injector(DWORD pid_) {
	wstring tempname = L"<No process name>";
	_internal_init(tempname, pid_);
	//try getting process name from handle
	Injector::get_process_name_from_handle(_pHandle, _process_name);
}


void Injector::_internal_init(const wstring& name_, DWORD pid_) {
	_pid = pid_;
	_process_name = name_;
	_pHandle = OpenProcess(PROCESS_ALL_ACCESS, false, _pid);
	if (!_pHandle) {
		throw_format_error("Failed open_process for pid %d, lasterror: %d", _pid, GetLastError());
	}
}

Injector::~Injector() {
	if (_pHandle) {
		CloseHandle(_pHandle);
	}
}


bool Injector::get_pid_for_process_name(const wstring& name_, DWORD& pid){
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	ScopeGuard<HANDLE> processesSnapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL), CloseHandle);
	if (processesSnapshot.guarded() == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(processesSnapshot, &processInfo);
	do {
		if (!name_.compare(processInfo.szExeFile))
		{
			pid = processInfo.th32ProcessID;
			wcout << L"Found pid for process <" << name_ << ">: " << pid << endl;
			return true;
		}
	} while (Process32Next(processesSnapshot, &processInfo));
	wcout << L"Failed to find pid for process <" << name_ << ">" << endl;
	return false;
}

bool Injector::get_process_name_from_handle(HANDLE pHandle_, wstring& name) {
	WCHAR buffer[MAX_PATH];
	if (!GetModuleFileNameExW(pHandle_, NULL, buffer, sizeof(buffer))) {
		wcout << "Failed getting filename for process, lasterror: " << GetLastError() << endl;
		return false;
	}
	name = buffer;
	return true;
}


bool Injector::inject(const wstring& dll_path_) {
	//first need to get full dll path
	WCHAR fullpath[MAX_PATH];
	size_t path_len = GetFullPathNameW(dll_path_.c_str(), sizeof(fullpath)/sizeof(WCHAR), fullpath, NULL);
	if (!path_len) {
		wcout << "Failed getting full path for dll. Error: " << GetLastError() << endl;
		return false;
	}
	size_t size_to_alloc = (path_len+1) * sizeof(WCHAR);
	HMODULE tmp;
	//need to find offset of LoadLibraryW from kernel32.dll
	GetModuleHandleEx(0, L"kernel32.dll", &tmp);
	if (!tmp) {
		wcout << "Failed getting handle to kernel32! Error: " << GetLastError() << endl;
		return false;
	}
	ScopeGuard<HMODULE> kernel32(tmp, FreeLibrary);
	//get offset of LoadLibraryW from kernel32
	LPVOID load_library_addr = GetProcAddress(kernel32, "LoadLibraryW");
	if (!load_library_addr) {
		wcout << "Failed getting addr of LoadLibraryW: " << GetLastError() << endl;
		return false;
	}
	LPVOID alloc_addr = VirtualAllocEx(_pHandle, NULL, size_to_alloc, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	if (!alloc_addr) {
		wcout << "Failed allocating "<< size_to_alloc << "bytes in process. Last error: " << GetLastError() << endl;
		return false;
	}
	wcout << "Allocated " << size_to_alloc << "bytes in process at address: " << alloc_addr << endl;
	//ensure we end up freeing the memory, lamda function that'll free the memory in the process
	ScopeGuard<LPVOID> addrGuard(alloc_addr, [phandle = _pHandle](LPVOID addr_) {VirtualFreeEx(phandle, addr_, 0, MEM_RELEASE); });
	//copy dll path to process
	SIZE_T written_bytes;
	if (!WriteProcessMemory(_pHandle, alloc_addr, fullpath, size_to_alloc, &written_bytes) || written_bytes != size_to_alloc) {
		wcout << "Failed WriteProcessMemory: last error:" << GetLastError() << ", size to alloc:" << size_to_alloc << 
			", bytes written: " << written_bytes << endl;
		return false;
	}
	
	ScopeGuard<HANDLE> thread(CreateRemoteThreadEx(_pHandle, NULL, 0, static_cast<LPTHREAD_START_ROUTINE>(load_library_addr), alloc_addr, 0, 0, 0),
								CloseHandle);
	if (!thread.guarded()) {
		wcout << "Failed createRemoteThread: " << GetLastError() << endl;
		return false;
	}
	wcout << "waiting for thread to finish..." << endl;
	WaitForSingleObject(thread, INFINITE);
	wcout << "Success!" << endl;
	return  true;
}
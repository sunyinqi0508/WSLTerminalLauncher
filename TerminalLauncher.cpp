// ConsoleApplication9.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <fstream>
#include <cstring>
#include <Windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <filesystem>
using namespace std;

const char pp[2048] = { "ubuntu1804 -c \"echo \\\"#\"'!'\"\\\"/bin/bash$'\\n'DISPLAY=:0 "
						"gnome-terminal \\&$'\\n'sleep 10$'\\n'while [ '$''('ls -1 /dev/pts/"
						"\\|wc -l')' -ge 2 ]\\; do$'\\n'sleep 5$'\\n'done|sh\"" };

const char* _template1 = " -c \"echo \\\"#\"'!'\"\\\"/bin/bash$'\\n'DISPLAY=:0 ",
		  * _template2 = " \\&$'\\n'sleep 10$'\\n'while [ '$''('ls -1 /dev/pts/" 
						 "\\|wc -l')' -ge 2 ]\\; do$'\\n'sleep 5$'\\n'done|sh\"";
bool install = 0;
class Splits {
public:
	int size = 0;
	char** data = 0;
	
	Splits() = default;
	~Splits() {
		if (!size) {
			size = 0; free(data);
		}
	}
};

Splits* split(char* istr, char ch) {
	Splits* ret = 0;
	char* str = istr;
	int size = 1, len = 1;
	do {
		if (*str == ch)
		{
			++size; 
			while (*(++str) == ch);
			if(*str)
				*(str - 1) = -1;
		}
	} while (++len, *(str++));
	ret = new Splits{ size, new char*[size] };
	int n = 1;
	ret->data[0] = istr;
	for (int i = 1; i < len; ++i) {
		if (istr[i] == -1 && i < len - 1)
		{
			istr[i] = 0;
			ret->data[n++] = istr + i + 1;
		}
	}
	return ret;
}
char* find_last_ch(char* str, const char ch) {
	char* ret = str;
	int begin = strlen(ret) - 1;
	while (begin > 0 && *ret != ch) ret--,begin--;
	return ret + 1;
}
int* kmp_borders(char* needle, size_t nlen) {
	if (!needle) return NULL;
	int i, j, *borders = (int *)malloc((nlen + 1) * sizeof(*borders));
	if (!borders) return NULL;
	i = 0;
	j = -1;
	borders[i] = j;
	while ((size_t)i < nlen) {
		while (j >= 0 && needle[i] != needle[j]) {
			j = borders[j];
		}
		++i;
		++j;
		borders[i] = j;
	}
	return borders;
}

char* kmp_search(char* haystack, size_t haylen, char* needle, size_t nlen, int* borders) {
	size_t max_index = haylen - nlen, i = 0, j = 0;
	while (i <= max_index) {
		while (j < nlen && needle[j] == *haystack) {
			++j;
			++haystack;
		}
		if (j == nlen) {
			return haystack - nlen;
		}

		if (j == 0) {
			++haystack;
			++i;
		}
		else {
			do {
				i += j - (size_t)borders[j];
				j = borders[j];
			} while (j > 0 && needle[j] != *haystack);
		}
	}
	return NULL;
}

char* sstrnstr(char* haystack, char* needle, size_t haylen) {
	if (!haystack || !needle) {
		return NULL;
	}
	size_t nlen = strlen(needle);
	if (haylen < nlen) {
		return NULL;
	}
	int* borders = kmp_borders(needle, nlen);
	if (!borders) {
		return NULL;
	}
	char* match = kmp_search(haystack, haylen, needle, nlen, borders);
	free(borders);
	return match;
}
void Modify(string newcmd)
{
	DWORD pid = GetCurrentProcessId();
	HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE, pid);
	if(h)
	{
		char exe_path[2048] = {};

		if (GetModuleFileNameEx(h, 0, exe_path, sizeof(exe_path) - 1))
		{
			char* exe_name = find_last_ch(exe_path, '\\');
			error_code code;
			char temp_path[2048];
			GetTempPath(2048, temp_path);
			char* new_filename = strcat(temp_path, "/temp.exe");
			filesystem::copy_file(exe_path, new_filename,filesystem::copy_options::overwrite_existing,code);
			
			
			FILE* fp = 0;
			fp = fopen(new_filename, "r+");
			fseek(fp, 0, SEEK_END);
			size_t sz = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			char* buffer = new char[sz + 10];
			fread(buffer, 1, sz, fp);
			char* pos = sstrnstr(buffer, const_cast<char*>(pp), sz);
			if (pos) {
				
				fseek(fp, pos - buffer, SEEK_SET);
				//fprintf(fp, "%s", newcmd.c_str());
				fwrite(newcmd.c_str(), 1, newcmd.size()+1, fp);
			}
			fclose(fp);

			char temp_param[2048];
			
			char* system32 = new char[2048];
			GetSystemDirectory(system32, 2040);
			strcat(system32, "\\terminal.exe");
			sprintf(temp_param, "-cb %s %d", install?system32:exe_path, pid);
			ShellExecute(NULL, "runas", new_filename, temp_param, NULL, SW_SHOWNORMAL);
			delete[] system32;
		}
		CloseHandle(h);

	}
	exit(0);
}
int WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
){
	
	auto ret = split(lpCmdLine, ' ');
	char** argc = ret->data;
	int argv = ret->size;
	string newcmd;
	bool change = false;
	for (int i = 0; i < argv; ++i)
	{
		if ((!strcmp(argc[i], "-i") || !strcmp(argc[i], "install"))) {
			install = true;
		}
		else if (!strcmp(argc[i], "-c") && i + 2 < argv)
		{
			newcmd = string(argc[i + 1]) + _template1 + argc[i + 2] + _template2;
			change = true;
			i += 2;
		}
		else if (!strcmp(argc[i], "-n") && i + 2 < argv) {
			newcmd = argc[i + 1];
			change = true;
			++i;
		}
		else if (!strcmp(argc[i], "-cb") && i + 2 < argv) {
			//MessageBox(0, "", "", 0);
			HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE, GetCurrentProcessId());
			if (h)
			{

				HANDLE parent = (HANDLE)OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, (DWORD)atoi(argc[i + 2]));
				WaitForSingleObject(parent, INFINITE);
				//MessageBox(0, "parent exited", pp, 0);

				char exe_path[2048] = {};

				if (GetModuleFileNameEx(h, 0, exe_path, sizeof(exe_path) - 1))
					filesystem::copy_file(exe_path, argc[i + 1],
						filesystem::copy_options::overwrite_existing);

			}
			return 0;
		}
		else if (!strcmp(argc[i], "-s"))
			MessageBox(0, pp, "Current Commandline", 0);

	}
	PROCESS_INFORMATION pi = { 0 };	STARTUPINFO si = { 0 };
	char proc[2048] = { 0 };
	
	if(change)
		Modify(newcmd.c_str());
	if (install)
	{
		HANDLE this_handle;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &this_handle)) {
			TOKEN_ELEVATION Elevation;
			char exe_path[2048] = {};
			DWORD cbSize = sizeof(TOKEN_ELEVATION);
			GetModuleFileNameEx(GetCurrentProcess(), 0, exe_path, sizeof(exe_path) - 1);
					
			if (GetTokenInformation(this_handle, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
				if (!Elevation.TokenIsElevated)
					ShellExecute(NULL, "runas", exe_path, "-i", NULL, SW_SHOWNORMAL);
				else {
					char *system32 = new char[2048];
					GetSystemDirectory(system32, 2048);
					error_code code;
					filesystem::copy_file(exe_path, strcat(system32, "\\terminal.exe"), 
						filesystem::copy_options::overwrite_existing, code);
					MessageBox(0, code.message().c_str(), exe_path, 0);
					delete[] system32;
				}
			}
		}
		return 0;
	}
	{
		bool wsl_launched = false;
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if (Process32First(snapshot, &entry) == TRUE)
			while (Process32Next(snapshot, &entry) == TRUE)
				if (strcmp(entry.szExeFile, "init") == 0)
				{
					wsl_launched = true;
					break;
				}
		if (!wsl_launched)
		{
			char instance[2048];
			int i = 0;
			//for (; pp[i] && pp[i] == ' '; ++i);
			for (; pp[i] && pp[i] != ' '; ++i)
				instance[i] = pp[i];
			instance[i] = 0;
			//strcat(instance, " -c \"bash\"");
			//const_cast<char*>((string(split(const_cast<char*>(pp), ' ')->data[0]) + " -c \"bash &\"").c_str());
			//ShellExecute(NULL, NULL, instance, " -c \"bash\"", NULL, SW_HIDE);
			/*CreateProcess(NULL, (instance),
				NULL, NULL, true, NULL, 0, NULL, &si, &pi);*/
		}
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		ZeroMemory(&si, sizeof(STARTUPINFO));
		char* tt = const_cast<char*>(pp);
		/*FILE* fp;
		fp = fopen("cmd.sh", "w");
		fprintf(fp, "%s", pp);
		fclose(fp);*/
		CreateProcess(NULL, tt, NULL, NULL, false, CREATE_NO_WINDOW, 0, NULL, &si, &pi);
	}
	return 0;
}

#include "framework.h"
#include "CourseWork.h"
#include "Windows.h"
#include "TlHelp32.h"
#include "psapi.h"
#include "strsafe.h"

# define ID_LIST 1000
# define ID_LIST1 1001
# define ID_LIST2 1002
# define ID_LIST3 1003
# define ID_LIST4 1004
# define ID_LIST5 1006
# define ID_COMBO 1005
# define ID_BUTTON 1010
# define ID_BUTTON1 1011
# define ID_BUTTON2 1012
# define SECOND 1

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutProgram(HWND, UINT, WPARAM, LPARAM);
void SetProcessPriority(DWORD procID, int Priority,HWND hwnd);
int ProcessesInfo(HWND hwnd, DWORD dwProcessID);
void getProcesses();
void EndProc(DWORD procID, HWND hwnd);
void globalInfo(int Load);
__int64 FileTimeToInt64(FILETIME& ft);
static VOID WINAPI CPULOAD(LPVOID pParameter);
HINSTANCE hInst;
HWND hList, hList1,hList2,hCombo,hList3, hCPU, hProcNum;
HWND hBUpdate, hSet,hEnd;
int kursorID = -1;
int count = 0;
int cpu_val[400];
int mem_val[400];
HDC DCmemory;


int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{

	HMENU hmenu1,hsubmenu1;
	hmenu1 = CreateMenu();
	hsubmenu1 = CreateMenu();
	MSG msg{};
	HWND hwdn{};
	WNDCLASSEX wc{sizeof(WNDCLASSEX)};
	const wchar_t CLASS_NAME[] = L"TaskManager";
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hInstance = hInstance;
	hInst = hInstance;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = CLASS_NAME;
	wc.lpszMenuName = L"Abobus";
	wc.lpfnWndProc = WindowProc;

	if (!RegisterClassEx(&wc))
		return EXIT_FAILURE;

	HWND hwnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		L"TaskManager",    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, 900, 650,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);
	AppendMenu(hmenu1, MF_STRING | MF_POPUP, (UINT)hsubmenu1, L"&File");
	AppendMenu(hsubmenu1, MF_STRING, IDM_EXIT, L"Close");
	AppendMenu(hmenu1, MF_STRING, IDM_ABOUT, L"&About");
	SetMenu(hwnd, hmenu1);
	SetMenu(hwnd, hsubmenu1);
	if (hwnd == NULL)
	{
		return 0;
	}
	ShowWindow(hwnd, nCmdShow);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;

}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	int Index, wmId;
	DWORD Data;
	DWORD procID;
	RECT rect,rect_cpu_graph,rect_mem_graph, rect_window;
	HPEN hpen, hpen_cpu, hpen_mem;
	int b = 0;
	int c;

	switch (message)
	{
	case WM_CTLCOLORSTATIC:
		return GetSysColor(4);
	case WM_CREATE:
	{
		MoveWindow(hWnd, (GetSystemMetrics(SM_CXSCREEN) - 1200) / 2, (GetSystemMetrics(SM_CYSCREEN) - 800) / 2, 1200, 800, true);
		hList = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | LBS_STANDARD | LBS_WANTKEYBOARDINPUT, 0, 0, 250, 500, hWnd, (HMENU)ID_LIST, hInst, NULL);
		hList1 = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_BORDER | LBS_WANTKEYBOARDINPUT, 251, 0, 525, 125, hWnd, (HMENU)ID_LIST1, hInst, NULL);
		hList2 = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | LBS_STANDARD | WS_BORDER | LBS_WANTKEYBOARDINPUT,251, 140, 525, 125, hWnd, (HMENU)ID_LIST2, hInst, NULL);
		hList3 = CreateWindow(L"static", NULL, WS_CHILD | WS_VISIBLE,800, 0, 300, 150, hWnd, (HMENU)ID_LIST3, hInst, NULL);
		hBUpdate = CreateWindow(L"button", L"Обновить список", WS_CHILD | BS_DEFPUSHBUTTON | WS_VISIBLE, 2, 510, 250, 40, hWnd, (HMENU)ID_BUTTON, hInst, NULL);
		hCombo = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 260, 275, 100, 200, hWnd, (HMENU)ID_COMBO, hInst, NULL);
		hSet = hBUpdate = CreateWindow(L"button", L"Задать", WS_CHILD | BS_DEFPUSHBUTTON | WS_VISIBLE, 2, 560, 250, 40, hWnd, (HMENU)ID_BUTTON1, hInst, NULL);
		hEnd = hBUpdate = CreateWindow(L"button", L"Завершить", WS_CHILD | BS_DEFPUSHBUTTON | WS_VISIBLE, 2, 610, 250, 40, hWnd, (HMENU)ID_BUTTON2, hInst, NULL);
		hProcNum = CreateWindow(L"static", NULL, WS_CHILD | WS_VISIBLE, 800, 110, 190, 100, hWnd, (HMENU)ID_LIST5, hInst, NULL);
		int i;
		i = SendMessage(hCombo, CB_ADDSTRING, 0, (DWORD)(L"Real time"));
		SendMessage(hCombo, CB_SETITEMDATA, i, (DWORD)REALTIME_PRIORITY_CLASS);
		i = SendMessage(hCombo, CB_ADDSTRING, 0, (DWORD)(L"High"));
		SendMessage(hCombo, CB_SETITEMDATA, i, (DWORD)HIGH_PRIORITY_CLASS);
		i = SendMessage(hCombo, CB_ADDSTRING, 0, (DWORD)(L"Above-Normal"));
		SendMessage(hCombo, CB_SETITEMDATA, i, (DWORD)ABOVE_NORMAL_PRIORITY_CLASS);
		i = SendMessage(hCombo, CB_ADDSTRING, 0, (DWORD)(L"Normal"));
		SendMessage(hCombo, CB_SETITEMDATA, i, (DWORD)NORMAL_PRIORITY_CLASS);
		i = SendMessage(hCombo, CB_ADDSTRING, 0, (DWORD)(L"Below-Normal"));
		SendMessage(hCombo, CB_SETITEMDATA, i, (DWORD)BELOW_NORMAL_PRIORITY_CLASS);
		i = SendMessage(hCombo, CB_ADDSTRING, 0, (DWORD)(L"Low"));
		SendMessage(hCombo, CB_SETITEMDATA, i, (DWORD)IDLE_PRIORITY_CLASS);
		getProcesses();
		SendMessage(hCombo, CB_SETCURSEL, 0, 0);
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)CPULOAD, hWnd, 0, nullptr);
		//CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)globalInfo, hList3, 0, nullptr);
		for (int i = 399; i > 0; i--) {
			cpu_val[i] = -1;
			mem_val[i] = -1;
		}
		break;
	}
	case WM_PAINT:
		GetClientRect(hWnd, &rect_window);
		hdc = BeginPaint(hWnd, &ps);
		SetBkMode(hdc, 4);
		TextOut(hdc, 270, 258, L"Приоритет", 9);
		TextOut(hdc, 258, 120, L"Модули процесса", 15);
		SetRect(&rect_cpu_graph, 260, 310, 800, 470);
		SetRect(&rect_mem_graph, 260, 480, 800, 640);
		FillRect(hdc, &rect_cpu_graph, CreateSolidBrush(RGB(0, 0, 0)));
		FillRect(hdc, &rect_mem_graph, CreateSolidBrush(RGB(0, 0, 0)));
		hpen = CreatePen(PS_SOLID, 0, RGB(0, 128, 64));
		SelectObject(hdc, hpen);
		count += 3;

		for (int i = rect_cpu_graph.right - 3; i > rect_cpu_graph.left; i -= 20)
		{

			if (i - count % 20 >= rect_cpu_graph.left)
			{
				MoveToEx(hdc, i - count % 20, rect_mem_graph.top, 0);
				LineTo(hdc, i - count % 20, rect_mem_graph.bottom);
				MoveToEx(hdc, i - count % 20, rect_cpu_graph.top, 0);
				LineTo(hdc, i - count % 20, rect_cpu_graph.bottom);
			}

		}
		//c = b;
		
		for (int i = 0; i <= 140; i += 20)
		{
			MoveToEx(hdc, rect_cpu_graph.left, rect_cpu_graph.top + i, 0);
			LineTo(hdc, rect_cpu_graph.right, rect_cpu_graph.top + i);

			MoveToEx(hdc, rect_mem_graph.left, rect_mem_graph.top + i, 0);
			LineTo(hdc, rect_mem_graph.right, rect_mem_graph.top + i);
		}
		hpen_cpu = CreatePen(PS_SOLID, 3, RGB(0, 255, 0));
		SelectObject(hdc, hpen_cpu);

		for (int i = 1, j = 10; i < 400; i++, j += 3)
		{
			if ((cpu_val[i + 1] >= 0) && ((rect_cpu_graph.right - j) > rect_cpu_graph.left))
			{
				MoveToEx(hdc, rect_cpu_graph.right - j, rect_cpu_graph.bottom - 1 - (cpu_val[i] * 1.5), 0);
				LineTo(hdc, rect_cpu_graph.right - j - 3, rect_cpu_graph.bottom - 1 - (cpu_val[i + 1] * 1.5));

			}
		}
		DeleteObject(hpen);
		DeleteObject(hpen_cpu);

		//memory_graph
		hpen_mem = CreatePen(PS_SOLID, 3, RGB(128, 0, 255));
		SelectObject(hdc, hpen_mem);

		for (int i = 0, j = 10; i < 400 - 1; i++, j += 3)
		{
			if ((mem_val[i + 1] >= 0) && ((rect_mem_graph.right - j) > rect_mem_graph.left))
			{
				MoveToEx(hdc, rect_mem_graph.right - j, rect_mem_graph.bottom - 1 - (mem_val[i] * 1.5), 0);
				LineTo(hdc, rect_mem_graph.right - j - 3, rect_mem_graph.bottom - 1 - (mem_val[i + 1] * 1.5));
			}
		}

		DeleteObject(hpen_mem);
		EndPaint(hWnd, &ps);
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		switch (wmId)
		{
			case IDM_ABOUT:
				MessageBox(hWnd, L"Курсовая работа реализующая диспетчер задач. Неделько Антон 951003 2021", L"О программе", MB_OK);
				break;
			case ID_BUTTON:
				getProcesses();
				break;
			case ID_LIST:
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
					kursorID = SendMessage(hList, LB_GETCURSEL, 0, 0);
					SendMessage(hList, LB_SETCURSEL, kursorID, 0);
					procID = (DWORD)SendMessage(hList, LB_GETITEMDATA, (WPARAM)kursorID, 0);
					ProcessesInfo(hList1, procID);
				}
				break;
			case ID_COMBO:
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					int kursorID1 = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
					SendMessage(hCombo, CB_SETCURSEL, kursorID1, 0);
				}
				break;
			case ID_BUTTON1:
				if (kursorID == -1)
					MessageBox(hWnd, L"Выберите процесс", NULL, MB_OK);
				else
				{
					int kursorID1 = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
					int priority = (DWORD)SendMessage(hCombo, CB_GETITEMDATA, (WPARAM)kursorID1, 0);
					procID = (DWORD)SendMessage(hList, LB_GETITEMDATA, (WPARAM)kursorID, 0);
					SetProcessPriority(procID, priority, hWnd);
					ProcessesInfo(hList1, procID);
				}
				break;
			case ID_BUTTON2:
				if (kursorID == -1)
					MessageBox(hWnd, L"Выберите процесс", NULL, MB_OK);
				else
				{
					if (MessageBox(hWnd, L"Завершить процесс?", L"Подтверждение", MB_YESNO | MB_ICONWARNING) == IDYES)
					{
						procID = (DWORD)SendMessage(hList, LB_GETITEMDATA, (WPARAM)kursorID, 0);
						EndProc(procID, hWnd);
					}
				}
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

void getProcesses()
{
	HANDLE processHandle, currentProcess;
	SendMessage(hList, LB_RESETCONTENT, 0, 0);
	int number = 0;
	// structure which describe procces
	PROCESSENTRY32 processes;
	WCHAR buffer[255];
	TCHAR str[255];
	processHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	processes.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(processHandle, &processes))
	{
		do
		{
			number++;
			wsprintfW(buffer, L"%s", processes.szExeFile);

			int i = SendMessageW(hList, LB_ADDSTRING, 0, (WPARAM)buffer);
			SendMessage(hList, LB_SETITEMDATA, i, (DWORD)processes.th32ProcessID);
		} while (Process32Next(processHandle, &processes));
	}
	wsprintf(str, TEXT("Количество процессов: %d"), number);
	SetWindowText(hProcNum, str);
	CloseHandle(processHandle);
}

int ProcessesInfo(HWND hwnd, DWORD dwProcessID)
{

	PROCESS_MEMORY_COUNTERS pmc;
	PROCESSENTRY32 pe32 = { 0 };
	HANDLE myhandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	int i = 0;
	auto h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, 0, dwProcessID);
	DWORD minphmem, maxphmem;
	DWORD dw = GetPriorityClass(h);
	TCHAR s[20];
	TCHAR str[MAX_PATH] = L"";
	CHAR str1[MAX_PATH] = "";
	DWORD charsCarried = MAX_PATH;
	//Отображание списка приоритетов    
	SendMessage(hwnd, LB_RESETCONTENT, 0, 0);
	SendMessage(hList2, LB_RESETCONTENT, 0, 0);

	if (myhandle == INVALID_HANDLE_VALUE)
		return false;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	//Поиск выбранного процесса и получение информации о нем
	if (Process32First(myhandle, &pe32))
		do {
			if (pe32.th32ProcessID == dwProcessID) break;
		} while (i++, Process32Next(myhandle, &pe32));

		GetProcessMemoryInfo(h,&pmc,sizeof(pmc));

		TCHAR sz[255] = L"";
			if(GetModuleFileNameEx(h, NULL, str, MAX_PATH) != 0)
			{
				SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)str);
			}
			
			else if (QueryFullProcessImageName(h, 0, str, &charsCarried) != 0)
			{
				SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)str);
			}

		//Отображение информации о процессе в списке
		wsprintf(sz, TEXT("Имя: %s"), pe32.szExeFile);
		SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)sz);

		wsprintf(sz, TEXT("ID процесса: %lu"), pe32.th32ProcessID);
		SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)sz);

		wsprintf(sz, TEXT("Кол-во потоков: %d"), pe32.cntThreads);
		SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)sz);

		wsprintf(sz, TEXT("Базовый приоритет: %d"), pe32.pcPriClassBase);
		SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)sz);
		GetProcessWorkingSetSize(h, &minphmem, &maxphmem);

		wsprintf(sz, TEXT("Мин размер физической памяти(КБ): %lu"), minphmem / 1024);
		SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)sz);

		wsprintf(sz, TEXT("Используемый размер физической памяти(КБ): %lu"), pmc.WorkingSetSize / 1024);
		SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)sz);
		CloseHandle(h);
		myhandle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessID);
		if (myhandle == INVALID_HANDLE_VALUE)
			return false;
		MODULEENTRY32 me32 = { 0 };
		me32.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(myhandle, &me32))
			do 
			{
				SendMessage(hList2, LB_ADDSTRING, 0, (LPARAM)me32.szExePath);

			} while (Module32Next(myhandle, &me32));
			CloseHandle(myhandle);
			return true;

}

void SetProcessPriority(DWORD procID, int Priority, HWND hwnd)
{
	PROCESSENTRY32 pe32;
	HANDLE myhandle;
	if (myhandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0))
	{
		pe32.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(myhandle, &pe32))
		{
			do 
			{
				if (pe32.th32ProcessID == procID)
				{
					auto a = 0;
					break;
				}
			} 
			while (Process32Next(myhandle, &pe32));
			HANDLE h = OpenProcess(PROCESS_SET_INFORMATION, TRUE, pe32.th32ProcessID);
			if (h == 0)
			{
				auto a = GetLastError();
				MessageBox(hwnd, L"Не удалось сменить приоритет процесса, возможно недостаточно прав доступа", NULL, MB_OK);
			}
			else
			{
				MessageBox(hwnd, L"Успех", L"Сообщение", MB_OK);
				SetPriorityClass(h, Priority);
				CloseHandle(h);
				CloseHandle(myhandle);
			}
		}
	}
}

void EndProc(DWORD procID, HWND hwnd)
{
	auto h = OpenProcess(PROCESS_TERMINATE, 0, procID);
	if (h == 0)
	{
		MessageBox(hwnd, L"Не удалось завершить процесс, возможно недостаточно прав доступа", NULL, MB_OK);
	}
	else
	{
		WaitForSingleObject(h, 1000);
		TerminateProcess(h, 0);
		MessageBox(hwnd, L"Процесс успешно завершен", L"успех", MB_OK);
		getProcesses();
	}
}

void globalInfo(int Load)
{

		MEMORYSTATUSEX mem;
		mem.dwLength = sizeof(mem);
		GlobalMemoryStatusEx(&mem);
		TCHAR info[MAX_PATH] = L"";
		TCHAR infoTemp[MAX_PATH] = L"";
		DWORD size = MAX_PATH;
		GetUserName(infoTemp, &size);
		wsprintf(info, TEXT("Пользователь: %s\n"), infoTemp);
		wsprintf(infoTemp, TEXT("Память: %d%%\n"), mem.dwMemoryLoad);
		StringCchCat(info, sizeof(infoTemp), infoTemp);
		wsprintf(infoTemp, TEXT("Всего ОЗУ(МБ): %d\n"), mem.ullTotalPhys / 1048576);
		StringCchCat(info, sizeof(infoTemp), infoTemp);
		wsprintf(infoTemp, TEXT("Свободно ОЗУ(МБ): %d\n"), mem.ullAvailPhys / 1048576);
		StringCchCat(info, sizeof(infoTemp), infoTemp);
		wsprintf(infoTemp, TEXT("Свободно в файле подкачки(МБ): %lu\n"), mem.ullAvailPageFile / 1048576);
		StringCchCat(info, sizeof(infoTemp), infoTemp);
		wsprintf(infoTemp, TEXT("Всего в файле подкачки(МБ): %lu\n"), mem.ullTotalPageFile / 1048576);
		StringCchCat(info, sizeof(infoTemp), infoTemp);
		wsprintf(infoTemp, TEXT("ЦП: %d%%"), Load);
		StringCchCat(info, sizeof(infoTemp), infoTemp);
		SetWindowText(hList3, info);
		for (int i = 399 - 1; i > 0; i--) {
			mem_val[i] = mem_val[i - 1];
		}
		mem_val[0] = mem.dwMemoryLoad;
}

__int64 FileTimeToInt64(FILETIME& ft)
{
	ULARGE_INTEGER foo;

	foo.LowPart = ft.dwLowDateTime;
	foo.HighPart = ft.dwHighDateTime;

	return (foo.QuadPart);
}

static VOID WINAPI CPULOAD(LPVOID pParameter)
{
#define NUMBER_OF_PROCESSORS (8)
#define PROCESSOR_BUFFER_SIZE (NUMBER_OF_PROCESSORS * 8)
	static ULONG64 ProcessorIdleTimeBuffer[PROCESSOR_BUFFER_SIZE];
	FILETIME IdleTime, KernelTime, UserTime;
	static unsigned long long PrevTotal = 0;
	static unsigned long long PrevIdle = 0;
	static unsigned long long PrevUser = 0;
	unsigned long long ThisTotal;
	unsigned long long ThisIdle, ThisKernel, ThisUser;
	unsigned long long TotalSinceLast, IdleSinceLast;

	while (true)
	{
		GetSystemTimes(&IdleTime, &KernelTime, &UserTime);
		HWND hwnd = (HWND)pParameter;
		ThisIdle = FileTimeToInt64(IdleTime);
		ThisKernel = FileTimeToInt64(KernelTime);
		ThisUser = FileTimeToInt64(UserTime);
		ThisTotal = ThisKernel + ThisUser;
		TotalSinceLast = ThisTotal - PrevTotal;
		IdleSinceLast = ThisIdle - PrevIdle;
		double Headroom;
		Headroom = (double)IdleSinceLast / (double)TotalSinceLast;
		double Load;
		Load = 1.0 - Headroom;
		Load *= 100.0;  // percent
		PrevTotal = ThisTotal;
		PrevIdle = ThisIdle;
		PrevUser = ThisUser;
		TCHAR chCPU[255];
		//wsprintf(chCPU, L"ЦП: %d%%", (int)Load);
		//SetWindowText(hCPU, chCPU);
		for (int i = 399; i > 0; i--) {
			cpu_val[i] = cpu_val[i - 1];
		}
		cpu_val[0] = (int)Load;
		globalInfo((int) Load);
		InvalidateRect(hwnd, 0, FALSE);
		Sleep(1000);
	}
}


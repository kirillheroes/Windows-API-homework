#include <windows.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <iomanip>
#define elif(condition) else if (condition)
int const HK_CTRL_Q = 0;
int const HK_SHIFT_C = 1;

//лаба 5
int const WM_UPDATE = RegisterWindowMessage("window has been changed");
int const COLORCHANGE = 0;
int const SIZECHANGE = 1;

HANDLE hMemory;
HANDLE address_of_view;

//лаба 3
const char* config_file_name = "D:\\ОС\\Configuration.txt";

enum mode { MAPPING, DIRECT, STREAM, NATIVE };

struct configdata
{
	UINT width;
	UINT height;
	int x_coor;
	int y_coor;
	COLORREF background_color;
};
configdata configsavedata;

HANDLE myNewThread; //лаба 7

void KeyDown(HWND, WPARAM); //обрабатывает нажатия клавиш
void HotKey(HWND, WPARAM); //обрабатываешь нажатия сочетаний клавиш
void SectorColorChange(HWND, UINT, UINT); //меняет цвет при клике левой кнопкой мыши на заданный (4 сектора)
void ChangeBackgroundColor(HWND, COLORREF); //меняет цвет заднего фона
void CreateNodepad(); //открывает Блокнот
configdata InitializeStruct(HWND); //инициализация структуры
configdata ReadConfig(UINT); //чтение из файла
void WriteConfig(UINT, configdata); //запись в файл
mode Mode(int, char**); //выбор id режима
void Instructions(); //инструкции
configdata ReadBuffer(const char*, configdata, UINT); //запись из буфера в структуру
void RegisterChangeOfSize(HWND); //учёт смены размера окна

LRESULT CALLBACK myWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{/* лаба 5
	if (uMsg == WM_UPDATE)
	{
		configsavedata = *(configdata *) address_of_view;
		
		switch (wParam)
		{
		case COLORCHANGE:
			SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG)CreateSolidBrush(configsavedata.background_color));
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case SIZECHANGE:
			RECT size;
			GetWindowRect(hWnd, &size);
			MoveWindow(hWnd, size.left, size.top, configsavedata.width, configsavedata.height, TRUE);
			break;
		}
	}
	*/
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		KeyDown(hWnd, wParam); return 0;
	case WM_HOTKEY:
		HotKey(hWnd, wParam); return 0;
	case WM_LBUTTONDOWN:
		SectorColorChange(hWnd, LOWORD(lParam), HIWORD(lParam)); return 0;
	case WM_SIZE:
		RegisterChangeOfSize(hWnd); return 0; //лаба 5
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void RegisterChangeOfSize(HWND hWnd)
{
	RECT size;
	GetWindowRect(hWnd, &size);
	
	configsavedata.height = size.bottom - size.top;;
	configsavedata.width = size.right - size.left;
	/*
	if (address_of_view != nullptr)
	{
		memcpy(address_of_view, &configsavedata, sizeof(configsavedata));
	}
	else std::cout << "error mapping a view of file mapping object for shared memory into address space\nerror code: " << GetLastError() << std::endl;
	
	SendMessage(HWND_BROADCAST, WM_UPDATE, SIZECHANGE, 0);*/
}

void CreateNodepad()
{
	// Create empty process startup info.
	STARTUPINFO sInfo;
	ZeroMemory(&sInfo, sizeof(STARTUPINFO));

	// Reserve memoryt for process information.
	PROCESS_INFORMATION pInfo;

	CreateProcessA("C:\\Windows\\Notepad.exe",
		nullptr, nullptr, nullptr, FALSE, NULL, nullptr, nullptr, &sInfo, &pInfo);
}

void KeyDown(HWND hWnd, WPARAM wParam)
{
	switch (wParam)
	{
	case VK_ESCAPE:
		PostQuitMessage(0);
		break;
	case VK_RETURN:
		ChangeBackgroundColor(hWnd, RGB(rand() % 256, rand() % 256, rand() % 256));
		break;
	case VK_SPACE: //лаба 7. если не получается запустить поток снова, то он запущен => останавливаем его
		//иначе, если получается запустить снова, то он был остановлен, и дело тоже сделано
		if (!ResumeThread(myNewThread)) SuspendThread(myNewThread); break;
	case '1': //установка приоритетов 
		SetThreadPriority(myNewThread, THREAD_PRIORITY_IDLE);
		std::cout << "Thread priority has been changed to idle" << std::endl; break;
	case '2':
		SetThreadPriority(myNewThread, THREAD_PRIORITY_LOWEST);
		std::cout << "Thread priority has been changed to lowest" << std::endl; break;
	case '3':
		SetThreadPriority(myNewThread, THREAD_PRIORITY_BELOW_NORMAL);
		std::cout << "Thread priority has been changed to below normal" << std::endl; break;
	case '4':
		SetThreadPriority(myNewThread, THREAD_PRIORITY_NORMAL);
		std::cout << "Thread priority has been changed to normal" << std::endl; break;
	case '5':
		SetThreadPriority(myNewThread, THREAD_PRIORITY_ABOVE_NORMAL);
		std::cout << "Thread priority has been changed to above normal" << std::endl; break;
	case '6':
		SetThreadPriority(myNewThread, THREAD_PRIORITY_HIGHEST);
		std::cout << "Thread priority has been changed to highest" << std::endl; break;
	case '7':
		SetThreadPriority(myNewThread, THREAD_PRIORITY_TIME_CRITICAL);
		std::cout << "Thread priority has been changed to time critical" << std::endl; break;
	}
}

void HotKey(HWND hWnd, WPARAM id)
{
	switch (id)
	{
	case HK_CTRL_Q:
		PostQuitMessage(0);
		break;
	case HK_SHIFT_C:
		CreateNodepad();
		break;
	}
}

void ChangeBackgroundColor(HWND hWnd, COLORREF Color)
{
	HBRUSH brush = (HBRUSH)SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG)CreateSolidBrush(Color));
	InvalidateRect(hWnd, nullptr, TRUE);

	configsavedata.background_color = Color;
	
	/*if (address_of_view != nullptr)
	{
		memcpy(address_of_view, &configsavedata, sizeof(configsavedata));
	}
	else std::cout << "error mapping a view of file mapping object for shared memory into address space\nerror code: " << GetLastError() << std::endl;
	SendMessage(HWND_BROADCAST, WM_UPDATE, COLORCHANGE, 0);*/
	
	DeleteObject(brush);
}

void SectorColorChange(HWND hWnd, UINT x, UINT y)
{
	RECT size;
	GetWindowRect(hWnd, &size);
	UINT height = size.bottom - size.top;
	UINT width = size.right - size.left;
	COLORREF color;
	if (x < width / 2)
		if (y < height / 2) //левый
			color = RGB(255, 0, 0); //верхний
		else
			color = RGB(0, 255, 0); //нижний
	elif(y < height / 2) //правый
		color = RGB(255, 255, 0); //верхний
	else color = RGB(0, 0, 255); //нижний
	ChangeBackgroundColor(hWnd, color);
}

configdata GetWindowParams(HWND hWnd)
{
	configdata structure;
	RECT winSize;
	GetWindowRect(hWnd, &winSize);
	structure.height = winSize.bottom - winSize.top;
	structure.width = winSize.right - winSize.left;
	structure.x_coor = winSize.left;
	structure.y_coor = winSize.top;
	HDC hdc = GetDC(hWnd);
	structure.background_color = GetPixel(hdc, 0, 0);
	ReleaseDC(hWnd, hdc);
	return structure;
}

void Instructions()
{
	std::cout << "to use different read-write file modes input second parametr:" << std::endl;
	std::cout << std::setw(16) << std::left << "Mapping mode" << std::setw(25) << std::left << "(mapping view to RAM)" << "-m | --mapping\n";
	std::cout << std::setw(16) << std::left << "Direct mode" << std::setw(25) << std::left << "(C style file variables)" << "-d | --direct\n";
	std::cout << std::setw(16) << std::left << "Stream mode" << std::setw(25) << std::left << "(fstream class)" << "-s | --stream\n";
	std::cout << std::setw(16) << std::left << "Native API mode" << std::setw(25) << std::left << "(winAPI functions)" << "-n | --native\n";
}

mode Mode(int argc, char** argv)
{
	if (argc > 1)
	{
		if (argv[1] == std::string("--mapping")|| argv[1] == std::string("-m"))
		{
			std::cout << "mapping mode" << std::endl;
			return MAPPING;
		}
		elif(argv[1] == std::string("--direct") || argv[1] == std::string("-d"))
		{
			std::cout << "direct mode" << std::endl;
			return DIRECT;
		}
		elif(argv[1] == std::string("--stream") || argv[1] == std::string("-s"))
		{
			std::cout << "stream mode" << std::endl;
			return STREAM;
		}
		elif(argv[1] == std::string("--native") || argv[1] == std::string("-n"))
		{
			std::cout << "native API mode" << std::endl;
			return NATIVE;
		}
		else
		{
			std::cout << "wrong file read-write mode\nusing Native API mode by default\n";
			Instructions();
			return NATIVE;
		}
	}
	//по-умолчанию пусть будет "родной" способ
	std::cout << "native API mode" << std::endl;
	Instructions();
	return NATIVE;
}

configdata ReadBuffer(const char* buffer, configdata structure, UINT size)
{
	//считываем поля структуры в 5 строк
	std::string elements[5];
	for (int i = 0, j = 0; j < 5 && i < size; i++)
	{
		if (buffer[i] == '\n') j++;
		else elements[j] += buffer[i];
	}
	structure.width = stoi(elements[0]); structure.height = stoi(elements[1]);  structure.x_coor = stoi(elements[2]);
	structure.y_coor = stoi(elements[3]); structure.background_color = stoi(elements[4]);
	return structure;
}

configdata ReadConfig(UINT mode)
{
	configdata structure{ 320, 240, 100, 100, RGB(0,0,255) }; //параметры по-умолчанию
	switch (mode)
	{
		case MAPPING:	//отображение на память
		{
			HANDLE file = CreateFile(config_file_name, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (file != INVALID_HANDLE_VALUE)
			{
				UINT file_size = GetFileSize(file, nullptr);
				HANDLE file_mapping = CreateFileMapping(file, nullptr, PAGE_READONLY, 0, file_size, "WINAPI_TEST_configuration");
				if (file_mapping != nullptr)
				{
					char* address = (char*)MapViewOfFile(file_mapping, FILE_MAP_READ, 0, 0, 0);
					if (address != nullptr)
					{
						structure = ReadBuffer(address, structure, file_size);
						UnmapViewOfFile(address);
					}
					else std::cout << "error mapping a view of file mapping object for configuraton.txt into address space\nerror code: " << GetLastError() << std::endl;
					CloseHandle(file_mapping);
				}
				else std::cout << "error creating file mapping object for configuraton.txt\nerror code: " << GetLastError() << std::endl;
				CloseHandle(file);
			}
			else std::cout << "error reading configuration.txt file\nerror code: " << GetLastError() << std::endl;
			break;
		}
		case DIRECT:	//файловые переменные
		{
			FILE *file;
			if (fopen_s(&file, config_file_name, "r") == NULL)
			{
				fseek(file, 0, SEEK_END);
				UINT elements_count = ftell(file) / sizeof(char) - 4; //из-за преобразования \r\n и \n, строк 5
				fseek(file, 0, SEEK_SET);
				char* buff = new char[elements_count];
				if (fread(buff, sizeof(char), elements_count, file) != elements_count)
					std::cout << "error reading configuration.txt file\nerror code: " << GetLastError() << std::endl;
				else structure = ReadBuffer(buff, structure, elements_count);
				fclose(file);
				delete[] buff;
			}
			else std::cout << "error loading configuration.txt file\nerror code: " << GetLastError() << std::endl;
			break;
		}
		case STREAM:	//потоки
		{
			std::ifstream file(config_file_name);
			if (file) file >> structure.width >> structure.height >> structure.x_coor >> structure.y_coor >> structure.background_color;
			else std::cout << "error loading configuration.txt file\nerror code: " << GetLastError() << std::endl;
			file.close(); break;
		}
		case NATIVE:	//встроенные функции winAPI
		{
			HANDLE file = CreateFile(config_file_name, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (file != INVALID_HANDLE_VALUE)
			{
				DWORD bytes_read;
				UINT file_size = GetFileSize(file, nullptr); char* buff = new char[file_size];
				ReadFile(file, buff, file_size, &bytes_read, nullptr);
				if (file_size != bytes_read) std::cout << "error reading configuration.txt file\nerror code: " << GetLastError() << std::endl;
				else structure = ReadBuffer(buff, structure, file_size);
				delete[] buff;
				CloseHandle(file);
			}
			else std::cout << "error loading configuration.txt file\nerror code: " << GetLastError() << std::endl;
			
		}
	}
	return structure;
}

void WriteConfig(UINT mode, configdata data)
{
	switch (mode)
	{
		case MAPPING:	//отображение на память
		{
			HANDLE file = CreateFile(config_file_name, GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (file != INVALID_HANDLE_VALUE)
			{	//записываем данные из структуры в единую строку
				std::string buff_str = std::to_string(data.width) + "\r\n" + std::to_string(data.height) + "\r\n" + std::to_string(data.x_coor) +
					"\r\n" + std::to_string(data.y_coor) + "\r\n" + std::to_string(data.background_color);
				size_t buff_size = buff_str.size();
				HANDLE file_mapping = CreateFileMapping(file, nullptr, PAGE_READWRITE, 0, buff_size, "WINAPI_TEST_configuration");
				if (file_mapping != nullptr)
				{
					char* address = (char*)MapViewOfFile(file_mapping, FILE_MAP_WRITE, 0, 0, 0);
					if (address != nullptr)
					{
						for (int i = 0; i < buff_size; i++)
							address[i] = buff_str[i];
						UnmapViewOfFile(address);
					}
					else std::cout << "error mapping a view of file mapping object for configuraton.txt into address space\nerror code: " << GetLastError();
					CloseHandle(file_mapping);
				}
				else std::cout << "error creating file mapping object for configuraton.txt\nerror code: " << GetLastError();
				CloseHandle(file);
			}
			else std::cout << "error creating configuration.txt file\nerror code: " << GetLastError();
			break;
		}
		case DIRECT:	//файловые переменные
		{
			FILE *file;
			if (fopen_s(&file, config_file_name, "w") == NULL)
			{	//записываем данные из структуры в единую строку
				std::string buff_str = std::to_string(data.width) + "\r\n" + std::to_string(data.height) + "\r\n" + std::to_string(data.x_coor) +
					"\r\n" + std::to_string(data.y_coor) + "\r\n" + std::to_string(data.background_color);
				//приводим эту строку в массив char-ов 
				const char* buffer = buff_str.c_str();
				size_t buff_size = buff_str.size();
				if (fwrite(buffer, sizeof(char), buff_size, file) != buff_size) std::cout << "error writing to configuration.txt file\nerror code: " << GetLastError();
				fclose(file);
			}
			else std::cout << "error creating configuration.txt file\nerror code: " << GetLastError();
			break;
		}
		case STREAM: //потоки
		{
			std::ofstream file(config_file_name);
			if (file) file << data.width << "\r\n" << data.height << "\r\n" << data.x_coor << "\r\n" << data.y_coor << "\r\n" << data.background_color;
			else std::cout << "error creating configuration.txt file\nerror code: " << GetLastError() << std::endl;
			file.close();
			break;
		}
		case NATIVE:	//встроенные функции win API
		{
			HANDLE file = CreateFile(config_file_name, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (file != INVALID_HANDLE_VALUE)
			{	//записываем данные из структуры в единую строку
				std::string buff_str = std::to_string(data.width) + "\r\n" + std::to_string(data.height) + "\r\n" + std::to_string(data.x_coor) +
					"\r\n" + std::to_string(data.y_coor) + "\r\n" + std::to_string(data.background_color);
				//приводим эту строку в массив char-ов
				const char* buffer = buff_str.c_str(); DWORD bytes_written;
				WriteFile(file, buffer, buff_str.size(), &bytes_written, nullptr);
				if (buff_str.size() != bytes_written) std::cout << "error writing to configuration.txt file\nerror code: " << GetLastError();
				CloseHandle(file);
			}
			else std::cout << "error creating configuration.txt file\nerror code: " << GetLastError();
			break;
		}
	}
}

void NewThreadFunction(LPVOID lpParameter) //лаба 7
//функция должна делать что-то другое! в этом недостаток
{
	while( *(bool*)lpParameter)
	{
		Beep(247, 500);
		Beep(417, 500);
		Beep(417, 500);
		Beep(370, 500);
		Beep(417, 500);
		Beep(329, 500);
		Beep(247, 500);
		Beep(247, 500);
		Beep(247, 500);
		Beep(417, 500);
		Beep(417, 500);
		Beep(370, 500);
		Beep(417, 500);
		Beep(497, 500);
		Sleep(500);
		Beep(497, 500);
		Beep(277, 500);
		Beep(277, 500);
		Beep(440, 500);
		Beep(440, 500);
		Beep(417, 500);
		Beep(370, 500);
		Beep(329, 500);
		Beep(247, 500);
		Beep(417, 500);
		Beep(417, 500);
		Beep(370, 500);
		Beep(417, 500);
		Beep(329, 500);
		Sleep(2000);
	}
	std::cout << "New Thread has stopped working" << std::endl;
	ExitThread(0);
}

int main(int argc, char** argv)
{
	UINT mode = Mode(argc, argv);
	configsavedata = ReadConfig(mode);
	
	hMemory = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "my shared memory");
	if (hMemory != nullptr) //считываем данные из разделяемой памяти, если она есть
	{
		address_of_view = MapViewOfFile(hMemory, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (address_of_view != nullptr)
		{
			memmove(&configsavedata, address_of_view, sizeof(configsavedata)); 
		}
		else std::cout << "error mapping a view of file mapping object for shared memory into address space\nerror code: " << GetLastError() << std::endl;
	}
	else //записываем данные в разделяемую память, создавая её
	{
		hMemory = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(configsavedata), "my shared memory");
		if (hMemory != nullptr)
		{
			address_of_view = MapViewOfFile(hMemory, FILE_MAP_ALL_ACCESS, 0, 0, 128);
			if (address_of_view != nullptr)
			{
				memmove(address_of_view, &configsavedata, sizeof(configsavedata));
			}
			else std::cout << "error mapping a view of file mapping object for shared memory into address space\nerror code: " << GetLastError() << std::endl;
		}
		else std::cout << "error creating file mapping object for shared memory\nerror code: " << GetLastError();
	}

	// Get the handler of module that will be associated with a window.
	// In this case it will be handler of the executable file of current process.
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	HBRUSH hBrush = CreateSolidBrush(configsavedata.background_color);

	// Create window class.
	WNDCLASS winClass;
	winClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	winClass.lpfnWndProc = (WNDPROC)myWndProc;
	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;
	winClass.hInstance = hInstance;
	winClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	winClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	winClass.hbrBackground = hBrush;
	winClass.lpszMenuName = nullptr;
	winClass.lpszClassName = "MyOwnTestWindowClass";

	// Register class in the system.
	RegisterClass(&winClass);

	// Create window.
	HWND hWnd = CreateWindow("MyOwnTestWindowClass", "Kirill Oshev FIT-2017-7-8",
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		configsavedata.x_coor, configsavedata.y_coor, configsavedata.width, configsavedata.height,
		nullptr, nullptr, hInstance, nullptr);

	ShowWindow(hWnd, SW_SHOW);

	srand(time(nullptr));
	RegisterHotKey(hWnd, HK_CTRL_Q, MOD_CONTROL, 'Q'); //(message.wParam == 'Q' && GetKeyState(VK_CONTROL) < 0)
	RegisterHotKey(hWnd, HK_SHIFT_C, MOD_SHIFT, 'C'); //(message.wParam == 'C' && GetKeyState(VK_SHIFT) < 0)
	MSG message;

	//лаба 7
	bool newThreadIsWorking = true;
	myNewThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)NewThreadFunction, &newThreadIsWorking, 0, nullptr);

	while (true)
	{
		if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
				break;
			else
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
	}
	configsavedata = GetWindowParams(hWnd);
	WriteConfig(mode, configsavedata);

	DestroyWindow(hWnd);
	// Remove window class.
	UnregisterClass("MyOwnTestWindowClass", hInstance);
	DeleteObject(hBrush);
	
	if (hMemory != nullptr) CloseHandle(hMemory);
	if (address_of_view != nullptr) UnmapViewOfFile(address_of_view);

	newThreadIsWorking = false; //лаба 7
	CloseHandle(myNewThread); //лаба 7
	system("pause > nul");
	return 0;
}
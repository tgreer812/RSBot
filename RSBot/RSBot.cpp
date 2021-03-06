#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <chrono>
#include <thread>
#include <time.h>
#include <processthreadsapi.h>


#define QUIT 5

using namespace std;

string scriptDirectory = "";

int selectAndRunScript()
{

	return 0;
}

int createNewScript()
{
	string scriptName = "";
	cout << "Enter script name: ";
	cin >> scriptName;
	cout << "Press [ to begin profile creation: ";
	return 0;
}

BOOL recording = false;



LRESULT CALLBACK keyboardResponse(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION )
	{
		PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
		switch (wParam)
		{
			case WM_KEYDOWN:
			
				break;
			case WM_SYSKEYDOWN:
				break;
			case WM_KEYUP:
				if (p->vkCode == 0xDB)
				{     //redirect a to b
					
					recording = true;
					PostMessage(NULL, 2, NULL, NULL);
					//keybd_event('B', 0, 0, 0);
					//keybd_event('B', 0, KEYEVENTF_KEYUP, 0);
					//break;
				}
				else if (p->vkCode == 0xDD)
				{
					if (recording)
					{
						PostMessage(NULL, 2, NULL, NULL);
					}
				}
				break;
			case WM_SYSKEYUP:

			
				break;
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
	//return 1;
}

#define arraysize 100000
unsigned __int64 previous = 1;
unsigned __int64 clickDelays[arraysize];
unsigned int clicks = 0;

LRESULT CALLBACK mouseClickRecord(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		switch (wParam)
		{
			case WM_LBUTTONDOWN:
				break;
			case WM_LBUTTONUP:
				//printf("mouse clicked!\n");
				
				unsigned __int64 newTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				if (clicks == arraysize)
					PostMessage(NULL, 2, NULL, NULL);
				if(clicks != 0)
				{ 
					
					clickDelays[clicks] = newTime - previous;
				}
				previous = newTime;
				clicks++;
				/*
				printf("clickDelays[%d]: %d\n", clicks - 1, clickDelays[clicks - 1]);
				*/
				break;
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}



int createClickProfile()
{
	HOOKPROC myHookProc = keyboardResponse;
	HOOKPROC myMouseHook = mouseClickRecord;
	HHOOK llKybdHook = SetWindowsHookEx(WH_KEYBOARD_LL, myHookProc, 0,0);
	
	MSG msg;
	
	while (!GetMessage(&msg, NULL, NULL, NULL)) {    //this while loop keeps the hook
		printf("test\n");
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		cout << "looping\n";
	}
	printf("Recording started!\n");
	previous = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	HHOOK llMCHook = SetWindowsHookEx(WH_MOUSE_LL, myMouseHook, 0, 0);
	
	while (!GetMessage(&msg, NULL, NULL, NULL)) {    //this while loop keeps the hook
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	recording = false;
	printf("Recording ended!\n");

	/*
	for (int i = 0; i < clicks; i++)
	{
		printf("clickDelays[%d]: %d\n", i, clickDelays[i]);
	}
	*/
	UnhookWindowsHookEx(llMCHook);
	UnhookWindowsHookEx(llKybdHook);

	//write the array to a file
	string fileName = "";
	cout << "Enter a name for the click profile: ";
	cin >> fileName;

	ofstream myfile;
	myfile.open(fileName, ios::app);
	myfile << (clicks - 1)<< "\n";
	for (int i = 1; i < clicks; i++)
	{
		myfile << clickDelays[i] << ",";
	}
	myfile << "\n";
	for (int i = 1; i < clicks; i++)
	{
		myfile << i << ",";
	}
	myfile.close();


	return 0;
}

bool playing = false;

LRESULT CALLBACK keyboardResponseStop(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
		switch (wParam)
		{
		case WM_KEYDOWN:

			break;
		case WM_SYSKEYDOWN:
			break;
		case WM_KEYUP:
			if (p->vkCode == 0xDB)
			{     //redirect a to b
				if (!playing)
				{
					playing = true;
					PostMessage(NULL, 2, NULL, NULL);
				}
				//keybd_event('B', 0, 0, 0);
				//keybd_event('B', 0, KEYEVENTF_KEYUP, 0);
				//break;
			}
			else if (p->vkCode == 0xDD)
			{
				if (playing)
				{
					playing = false;
					PostMessage(NULL, 2, NULL, NULL);
				}
			}
			break;
		case WM_SYSKEYUP:


			break;
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
	//return 1;
}

typedef struct ClickData {
	unsigned int clicks[arraysize];
	int click_count;
} CLICKDATA, *PCLICKDATA;


#define CLICK_DELAY_VARIANCE 15
#define CLICK_LIFT_VARIANCE 5

DWORD WINAPI clickThreadStart(LPVOID lpParam)
{

	PCLICKDATA clickData = (PCLICKDATA)lpParam;

	int random_index, random_click_delay, random_lift_delay;
	srand(time(NULL));

	while (true)
	{
		random_index = rand() % clickData->click_count;
		random_click_delay = rand() % CLICK_DELAY_VARIANCE;
		random_lift_delay = rand() % CLICK_LIFT_VARIANCE;
		
		std::this_thread::sleep_for(std::chrono::milliseconds(clickData->clicks[random_index] + random_click_delay));
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, NULL);

		std::this_thread::sleep_for(std::chrono::milliseconds(random_lift_delay));
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, NULL);

	}
}

typedef struct TimerData {
	unsigned int minutes;
	HANDLE tid;
} TIMERDATA, *PTIMERDATA;

DWORD WINAPI timerThreadStart(LPVOID lpParam)
{
	PTIMERDATA timePtr = (PTIMERDATA)lpParam;
	
	std::this_thread::sleep_for(std::chrono::milliseconds(timePtr->minutes*60*1000));
	cout << "Timer ended - killing clicker" << endl;
	TerminateThread(timePtr->tid, NULL);

	return 0;
}

void selectAndRunClickProfile()
{
	PCLICKDATA clickData = (PCLICKDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CLICKDATA));
	PTIMERDATA timerData = (PTIMERDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TIMERDATA));

	string fname, line;
	cout << "Enter the name of the click profile: ";
	cin >> fname;

	ifstream myfile;
	myfile.open(fname);

	cout << "file opened" << endl;

	//read to click count for the profile
	getline(myfile, line);
	clickData->click_count = std::stoi(line, nullptr);

	//read the delays for this profile
	getline(myfile, line);

	int prev = 0;
	int firstCommaIndex, secondCommaIndex;
	clickData->clicks[0] = std::stoi(line.substr(0, line.find(",", prev, 1)), nullptr);
	for (int i = 1; i < clickData->click_count; i++)
	{
		firstCommaIndex = line.find(",", prev + 1, 1);

		secondCommaIndex = line.find(",", firstCommaIndex + 1, 1);


		if (firstCommaIndex != string::npos && secondCommaIndex != string::npos)
		{
			clickData->clicks[i] = std::stoi(line.substr(firstCommaIndex + 1, secondCommaIndex - firstCommaIndex - 1), nullptr);
		}
		prev = firstCommaIndex;
	}
	cout << "Profile loaded successfully!" << endl;

	int answer = 0;
	cout << "Timer? (1 for yes, 0 for no)\n" << endl;
	cin >> answer;

	if (answer)
	{
		cout << "Enter time in minutes to run!" << endl;
		cin >> timerData->minutes;
	}


	/*
	for (int i = 0; i < click_count; i++)
	{
		printf("clickDelays[%d]: %d\n", i, clicks[i]);
	}*/

	cout << "Press [ to begin clicking and ] to stop" << endl;

	HOOKPROC myHookProc = keyboardResponseStop;
	HHOOK llKybdHook = SetWindowsHookEx(WH_KEYBOARD_LL, myHookProc, 0, 0);

	MSG msg;

	while (!GetMessage(&msg, NULL, NULL, NULL)) {    //this while loop keeps the hook
		printf("test\n");
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		cout << "looping\n";
	}
	cout << "Clicking!" << endl;

	//spawn child thread to handle clicks

	HANDLE childTID = CreateThread(NULL, 0, clickThreadStart, clickData, 0, NULL);

	if (answer)
	{
		timerData->tid = childTID; //the timer has a reference to the click thread so it can kill it if the time runs out
		HANDLE timerTID = CreateThread(NULL, 0, timerThreadStart, timerData, 0, NULL);
	}	

	while (!GetMessage(&msg, NULL, NULL, NULL))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	cout << "Clicking terminated!" << endl << endl;
	LPDWORD lpExitCode;
	
	//who the fuck said I was elegant
	TerminateThread(childTID, NULL);
	UnhookWindowsHookEx(llKybdHook);

}

int main()
{
	int option = -1;

	while (option != QUIT)
	{
		cout << "Please Select an Option:\n" << endl;
		cout << "1. Start a premade script" << endl;
		cout << "2. Create a new script" << endl;
		cout << "3. Create a new click profile" << endl;
		cout << "4. Start a premade click profile" << endl;
		cout << "5. Quit" << endl;

		cin >> option;

		if (option == 1)
		{
			selectAndRunScript();

		}
		else if (option == 2)
		{
			createNewScript();
		}
		else if (option == 3)
		{
			createClickProfile();
		}
		else if (option == 4)
		{
			selectAndRunClickProfile();
		}
	}

}
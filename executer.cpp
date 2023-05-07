// executer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <Windows.h>
#include <vector>
#include <string>
#include <array>
#include <sstream>
#include <fstream>
#include <locale>
#include <codecvt>
using namespace std;

const string VERSION = "1.3.0";
string filePath = "";
string fileName = "";

#include <Shlobj.h>

std::wstring stringToWString(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(str);
}

std::string wstringToString(const std::wstring& wstr)
{
	int length = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	std::string str(length, '\0');
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], length, NULL, NULL);
	return str;
}

std::string createCeriumFolderAndBatFiles()
{
	TCHAR documentsPath[MAX_PATH];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, documentsPath);

	if (result != S_OK)
	{
		std::cerr << "Error getting the Documents folder path." << std::endl;
		return "";
	}

	std::wstring ceriumFolderPath = std::wstring(documentsPath) + L"\\Cerium";
	if (CreateDirectory(ceriumFolderPath.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
	{
		
		std::wstring loadCeriumBatPath = ceriumFolderPath + L"\\loadCerium.bat";


		std::wofstream loadCeriumBatFile(loadCeriumBatPath);
		loadCeriumBatFile << L"timeout /t 5 /nobreak\n";
		loadCeriumBatFile << L"echo Running loadCerium batch file.\n";
		loadCeriumBatFile << L"move ";
		loadCeriumBatFile << stringToWString(filePath);
		loadCeriumBatFile << L" ";
		loadCeriumBatFile << ceriumFolderPath;
		loadCeriumBatFile << L"\nCD ";
		loadCeriumBatFile << ceriumFolderPath;
		loadCeriumBatFile << L"\nren ";
		loadCeriumBatFile << stringToWString(fileName);
		
		loadCeriumBatFile << L" c.exe\n";
		loadCeriumBatFile << L"setx PATH \"%CD%;%PATH%\"\n";
		loadCeriumBatFile << L"timeout /t 5 /nobreak\n";
		loadCeriumBatFile << L"echo Cerium Loaded\n";
		
		loadCeriumBatFile.close();

		return wstringToString(ceriumFolderPath);
	}
	else
	{
		std::cerr << "Error creating the Cerium folder." << std::endl;
		return "";
	}
}

HWND stringToHWND(const std::string& str)
{
	unsigned long long hwndValue;
	std::stringstream ss;

	ss << std::hex << str;
	ss >> hwndValue;

	return reinterpret_cast<HWND>(hwndValue);
}

class cWindow
{
public:
	string name;
	HWND hwnd;
	vector<int> coords;
	vector<int> size;
	bool visible;
	bool active;

	cWindow(string n, HWND h, vector<int> coord,vector<int> siz, bool vis, bool act)
	{
		name = n;
		hwnd = h;
		coords = coord;
		size = siz;
		visible = vis;
		active = act;
	}
};


int INDEX = 0;
string flags;

std::string removeMinusAndSpaces(std::string input) {
	std::string result;

	for (char ch : input) {
		if (ch != '/' && ch != ' ') {
			result.push_back(ch);
		}
	}

	return result;
}

void printWindowInfo(const cWindow& win)
{
	
	std::cout << "Window Information:{" << std::endl;
	std::cout << "\tName    : " << win.name << std::endl;
	std::cout << "\tHWND    : " << win.hwnd << std::endl;
	std::cout << "\tCoords  : (" << win.coords[0] << ", " << win.coords[1] << ")" << std::endl;
	std::cout << "\tSize    : (" << win.size[0] << ", " << win.size[1] << ")" << std::endl;
	std::cout << "\tVisible : " << (win.visible ? "Yes" : "No") << std::endl;
	std::cout << "\tActive  : " << (win.active ? "Yes" : "No") << std::endl << "}\n";
}

std::vector<std::string> processVector(std::vector<std::string> input) {
	bool found = false;
	input.push_back("/");
	int index;
	for (int i = 0; i < input.size(); i++)
	{
		if (input[i].find('/')!=string::npos && !found)
		{
			index = i;
			found = true;
			input[i] = removeMinusAndSpaces(input[i]);
			input[i] = "/"+input[i];
			
		}else if (input[i].find('/') != string::npos && found)
		{
			input[i] = removeMinusAndSpaces(input[i]);
		}
	}
	vector<string> change;
	for (int i = 0; i < index+1; i++)
	{
		change.push_back(input[i]);
	}
	for (int i = index+1; i < input.size(); i++)
	{
		change[change.size()-1]+=input[i];
	}
	flags = change[change.size()-1].substr(1);
	return change;
}
std::string exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}

string getWindowName(HWND win)
{
	
	int cTxtLen = 256;

	// Allocate memory for the string and copy
	// the string into the memory.

	LPWSTR pszMem = (LPWSTR)VirtualAlloc((LPVOID)NULL,
		(DWORD)(cTxtLen + 1), MEM_COMMIT,
		PAGE_READWRITE);
	GetWindowText(win, pszMem, cTxtLen + 1);

	int ad = GetWindowText(win, pszMem, cTxtLen);
	wstring ws(pszMem);
	string myVarS = string(ws.begin(), ws.end());
	if (win == GetDesktopWindow())
	{
		return "Desktop";
	}
	return myVarS;
}
vector<int> getMouse()
{
	POINT p;;
	GetCursorPos(&p);
	return { p.x,p.y };
}



// EnumWindows callback function
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	vector<cWindow>* windows = reinterpret_cast<vector<cWindow>*>(lParam);

	if (IsWindowVisible(hwnd))
	{
		string windowName = getWindowName(hwnd);
		bool visible = IsWindowVisible(hwnd);
		bool active = GetForegroundWindow() == hwnd;

		RECT rect;
		GetWindowRect(hwnd, &rect);
		vector<int> coords = { rect.left, rect.top };
		vector<int> size = { rect.right - rect.left, rect.bottom - rect.top };

		cWindow win(windowName, hwnd, coords, size, visible, active);
		windows->push_back(win);
	}
	return TRUE;
}

// Function to get all open windows and return a vector of cWindow objects
vector<cWindow> getWindows()
{
	vector<cWindow> windows;
	EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));

	return windows;
}



string getWifiPass()
{
	vector<string> pass;
	vector<string> wifi;
	string info = exec("netsh wlan show profiles");
	while (info.find(" All User Profile     :") != string::npos)
	{
		info = info.substr(info.find(" All User Profile     :") + 24);
		wifi.push_back(info.substr(0, info.find('\n')));
	}
	for (int i = 0; i < wifi.size(); i++)
	{
		string command = "netsh wlan show profile name = \"" + wifi[i] + "\" key = clear";
		string ps = exec(command.c_str());
		int pos = ps.find(" Key Content            :");
		if (pos == string::npos)
		{
			pass.push_back("#ABSENT#");
		}
		else
		{
			ps = ps.substr(pos + 26);
			ps = ps.substr(0, ps.find('\n'));
			pass.push_back(ps);
		}
		////cout << ps << endl;
	}
	//make nice and organized
	info = "";
	for (int i = 0; i < pass.size(); i++)
	{
		info += wifi[i] + ":" + pass[i] + "\n";
	}
	return info;
}
string getPCInfo()
{
	string info = "------SystemInfo------\n";
	info += exec("systeminfo");
	info += "\n------GlobalVariables------\n" + exec("set") + "\n";
	//wifi passwords
	info += "------WifiPasswords------\n" + getWifiPass() + "\n";
	info += "------IpConfig------\n" + exec("ipconfig") + "\n";
	return info;
}
int sendStr(string msg)
{
	//std::string msg = "ABCD - abcd - 1234";

	std::vector<INPUT> vec;
	for (auto ch : msg)
	{
		INPUT input = { 0 };
		input.type = INPUT_KEYBOARD;
		input.ki.dwFlags = KEYEVENTF_UNICODE;
		input.ki.wScan = ch;
		vec.push_back(input);

		input.ki.dwFlags |= KEYEVENTF_KEYUP;
		vec.push_back(input);
	}



	return SendInput(vec.size(), vec.data(), sizeof(INPUT));
}


#define TYPE_STRING 1
#define TYPE_INT 2
#define TYPE_CHAR 4
#define TYPE_FLOAT 3

#define INVALID_TYPE -1

//check for type
int checkType(string str, int type)
{
	//remove extra '-'
	int find = str.find('-');
	while(find!=string::npos)
	{
		str.replace(find,1,"");
		find = str.find('-');
	}


	switch(type)
	{
		case TYPE_STRING:
			return true;
			
		break;
		case TYPE_INT:
			return (str == to_string(stoi(str)));
		break;
		case TYPE_CHAR:
			return (str.length() == 1);
		break;
		case TYPE_FLOAT:
			return (str == to_string(stod(str)));
		break;

		default:
		return INVALID_TYPE;
	}
}
//class to hold dynamically typed value
class Param
{
	public:

	int type;
		string valString;
		int valInt;
		char valChar;
		double valDouble;

	
	Param(string val, int typ)
	{
		type = typ;
		switch(type)
		{
			case 1:
				valString = val;
				break;
			case 2:
				valInt = stoi(val);
				break;
			case 3:
				valChar = val[0];
				break;
			case 4:
				valDouble = stod(val);
				break;
			
		}
	
	}
};

class Command
{
	public:

	string name;
	string desc;
	vector<string> alias;
	vector<int> paramType;
	Command(string n, vector<string> alia = {}, vector<int> paramTyp = {})
	{
		name = n;
		alias = alia;
		paramType = paramTyp;
	
	}
	
	vector<Param> getParams(vector<string> args, int index)
	{
		vector<Param> params;
		for (int i = 0; i < paramType.size(); i++)
		{
			params.push_back(Param(args[i+1+index],paramType[i]));
		}
		return params;
	}

	bool goodName(string a)
	{
		//check for right command
		//cout << "NAME: \"" << args[index] << "\"\t\""<<name<<"\"\n";
		bool cName = false;
		if (a == name)
		{
			//cout << "test";
			return true;
		}
		else
		{
			for (int i = 0; i < alias.size(); i++)
			{
				//cout << "\t\""<<alias[i] << "\"\n";
				if (a == alias[i])
				{

					//name = a;
					return true;
					break;
				}
			}
		}
		return false;
	}
	
	int input(vector<string> args, int index)
	{
		bool cName = true;
		bool cParamNum= false;
		bool cParamType = false;

		

		//make sure right amount of params
		const int ar = args.size() - 1 - index;
		if (ar >= paramType.size())
		{
			
			cParamNum = true;
		}else
		{
			cerr << "Incorrect amount of parameters (expected: " << paramType.size() << ")\n";
			return 2;
		}

		//make sure right type
		int k = 0;
		for (int i = index+1; i < index+paramType.size(); i++)
		{
			int check = checkType(args[i], paramType[k]);
			if (check == INVALID_TYPE)
			{
				cerr << "Type not recognized (" << check << "): " << args[i] << endl;
				return INVALID_TYPE;
			}else if (check == false)
			{
				cerr << "Type invalid ("<<check<<"): " << args[i] << endl;
				return false;
			}	
			k++;
		}
		return 0;
	}
};

//displays help
void displayHelp(vector<Command> all)
{

	cout << "Cerium // "<<VERSION<<"\nCerium is a multi-purpose program made to make remote ssh navigation easier\n\n";
	for (int i = 0; i < all.size(); i++)
	{
		cout << all[i].name << "\n\t" << all[i].desc << "\n\n";
	}
}
void pressKey(int key)
{
	INPUT ip;
	// ...
		// Set up a generic keyboard event.
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// Press the "A" key
	ip.ki.wVk = key; // virtual-key code for the "a" key
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// ...
}

void perform(Command cmd, vector<Command> all, vector<string> s, int index)
{
	string name = cmd.name;
	
	if (flags.find('?')!=string::npos)
	{
		cout << name<<":" << endl << "\t" << cmd.desc << endl;
		return;
	}
	
	if (name == "help")
	{
		displayHelp(all);
	}
	if (name == "moveMouse")
	{
		vector<Param> coord = cmd.getParams(s,index);
		int x = coord[0].valInt;
		int y = coord[1].valInt;
		if (flags.find('r')!=string::npos)
		{
			//move relative
			vector<int> mpos = getMouse();
			SetCursorPos(mpos[0]+x, mpos[1]+y);
			cout << "Moved the mouse. (" << mpos[0] + x << "," << mpos[1] + y << ")\n";
		}else
		{
			//move mouse
			SetCursorPos(x,y);
			cout << "Moved the mouse. ("<<x<<","<<y<< ")\n";
		}
	}
	if (name == "getPcInfo")
	{
		getPCInfo();
	}
	if (name == "getWindows")
	{	cout << "Getting info...\n";
		vector<cWindow> wins = getWindows();
		for (int i = 0; i < wins.size(); i++)
		{
			printWindowInfo(wins[i]);
		}
	}
	if (name == "getMouse")
	{
		vector<int> pos = getMouse();
		cout << "(" << pos[0] << "," << pos[1] << ")\n";
	}
	if (name == "exec")
	{
		string a = "";
		for (int i = index+1; i < s.size()-1; i++)
		{
			a+=s[i]+=" ";
		}
		if (flags.find('s') != string::npos) {
		cout << "Executing (alternate) \"" + a + "\"...\n";
		system(a.c_str());
		}else
		{
			cout << "Executing \"" + a + "\"...\n";
			cout << exec(a.c_str()) << endl;
		}
	}
	if (name == "sendKey")
	{
		cout << "Simulating key...\n";
		vector<Param> key = cmd.getParams(s, index);
		pressKey(key[0].valInt);
	}
	if (name == "sendString")
	{
		cout << "Sending keys...\n";
		vector<Param> key = cmd.getParams(s, index);
		sendStr(key[0].valString);
	}
	if (name == "setVisibility")
	{
		cout << "Setting visibility...\n";
		vector<Param> para = cmd.getParams(s, index);
		ShowWindow(stringToHWND(para[0].valString), para[1].valInt);
	}
	if (name == "setActive")
	{
		cout << "Setting window focus...\n";
		vector<Param> para = cmd.getParams(s, index);

		SetForegroundWindow(stringToHWND(para[0].valString));
	}
	if (name == "close")
	{
		cout << "Closing program...\n";
		vector<Param> para = cmd.getParams(s, index);
		SendMessageW(stringToHWND(para[0].valString), WM_SYSCOMMAND, SC_CLOSE, NULL);
	}
	if (name == "loadCerium")
	{
		cout << "Loading cerium...\n";
		string p = createCeriumFolderAndBatFiles();
		string cmd = "start " + p + "\\loadCerium.bat";
		system(cmd.c_str());
		cout << cmd << endl;
	}
	if (name == "update")
	{
	//update
		cout << "Checking for update...\n";
		
	}
}

vector<string> codes = {
"SW_HIDE           ",
"SW_SHOWNORMAL       ",
"SW_NORMAL       ",
"SW_SHOWMINIMIZED",
"SW_SHOWMAXIMIZED",
"SW_MAXIMIZE       ",
"SW_SHOWNOACTIVATE",
"SW_SHOW           ",
"SW_MINIMIZE       ",
"SW_SHOWMINNOACTIVE",
"SW_SHOWNA       ",
"SW_RESTORE       ",
"SW_SHOWDEFAULT       ",
"SW_FORCEMINIMIZE",
"SW_MAX           "};
vector<int> codesInt = { 0 
,1
,1
,2
,3
,3
,4
,5
,6
,7
,8
,9
,10
,11
,11};

string getCodes()
{
	string a = "\t\tCodes:\n";
	for (int i = 0; i < codes.size();i++)
	{
		a+="\t\t"+codes[i] + "\t" + to_string(codesInt[i]) + "\n";
	}
	return a;
}

int main(int argc, char* argv[])
{
	filePath = argv[0];
	//cout << filePath << endl;
	fileName = filePath.substr(filePath.find_last_of('\\')+1);
	//cout << fileName << endl;
	if (argc == 1)
	{
		cerr << "No arguments were given, use help for more info.\n";
		return 0;
	}





	
   // cout << "Found " << argc-1 << " arguments\n";
	vector<Command> args;


	args.push_back(Command("help"));
	args[args.size()-1].desc = "Shows a help screen explaining commands";
	args.push_back(Command("moveMouse",{"mouseMove","moveCursor","cursorMove"},{TYPE_INT,TYPE_INT}));
	args[args.size() - 1].desc = "Moves the mouse to specified coordinates [-r]";
	args.push_back(Command("getPcInfo",{"pcInfo"}));
	args[args.size() - 1].desc = "Gives lots of info about the system";
	args.push_back(Command("loadCerium", {}));
	args[args.size() - 1].desc = "Fully integrates cerium into the system by adding it to PATH variable (should only run one time! see update)";
	args.push_back(Command("getWindows", {"getWin"}));
	args[args.size() - 1].desc = "Gets all currently open windows as cWindow objects";
	args.push_back(Command("getMouse", { "getMousePos","mousePos"}));
	args[args.size() - 1].desc = "Gets the current mouse position";
	args.push_back(Command("exec", { "execute","system","sys"}));
	args[args.size() - 1].desc = "Executes a system command through cerium [-s]";
	args.push_back(Command("sendKey", {"key","press"}, {TYPE_INT}));
	args[args.size() - 1].desc = "Sends a single c++ keystroke";
	args.push_back(Command("sendString", { "type","typeString","string"}, {TYPE_STRING}));
	args[args.size() - 1].desc = "Sends keystrokes as they are in the string parameter";
	args.push_back(Command("showWindow", {"show","showWin"}, {TYPE_INT}));
	args[args.size() - 1].desc = "Sets a windows visibility to true";
	args.push_back(Command("setVisibility", { "winShow","setShow","winVis","setWindowVis","setWinVis"}, {TYPE_STRING,TYPE_INT}));
	args[args.size() - 1].desc = "Sets a windows visibility to parameter 2:\n"+getCodes();
	args.push_back(Command("setActive", { "winActive","active" }, { TYPE_STRING }));
	args[args.size() - 1].desc = "Sets a certain window to the active window";
	args.push_back(Command("close", { "kill","end" }, { TYPE_STRING }));
	args[args.size() - 1].desc = "Closes a specific window based on HWND";
	args.push_back(Command("update"));
	args[args.size() - 1].desc = "Updates the program if an update is available";

	vector<string> s;
    for (int i = 1; i < argc; i++)
    {
      // cout << i << ": " << argv[i] << endl;
		
		s.push_back(argv[i]);
		
    }	
	s=processVector(s);
	argc = s.size();
	
	
	bool sName = false;
	for (int i = 0; i < args.size(); i++){
		if (args[i].goodName(s[0]))
		{
			//cout << "Get input\n";
			int a = args[i].input(s,INDEX);
			sName = true;
			if (a==0)
			{
				//succeed
				//cout << "GOOD! :D\n";
				perform(args[i],args,s,INDEX);
				
			}
			break;
		}
	}
	if (!sName)
	{
		cerr << "Unknown command: " << s[0] << endl;
	}
	
	
	//system("pause");
	return 0;
}

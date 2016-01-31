#include "global.h"
#include "accelerators.h"
#include<unordered_map>
using namespace std;

extern HWND win;
extern unordered_map<int, UserFunction<void(void)>> userCommands, timers;

extern tstring msg (const char* name);

static unordered_map<string,int> KEYNAMES = {
#define K(n) {"F" #n, VK_F##n}
K(1), K(2), K(3), K(4), K(5), K(6), K(7), K(8), K(9), K(10), K(11), K(12), K(13), K(14), K(15), K(16), K(17), K(18), K(19), K(20), K(21), K(22), K(23), K(24),
#undef K
{"Insert", VK_INSERT},
{"Delete", VK_DELETE},
{"Home", VK_HOME},
{"End", VK_END},
{"PageUp", VK_PRIOR},
{"PageDown", VK_NEXT},
{"Pause", VK_PAUSE},
{"Left", VK_LEFT},
{"Right", VK_RIGHT},
{"Down", VK_DOWN},
{"Up", VK_UP},
{"Tab", VK_TAB},
{"Enter", VK_RETURN},
{"Space", VK_SPACE},
{"Backspace", VK_BACK},
{"Escape", VK_ESCAPE},
#define K(n) {"Numpad" #n, VK_NUMPAD##n}
K(0), K(1), K(2), K(3), K(4), K(5), K(6), K(7), K(8), K(9),
#undef K
{"Numpad*", VK_MULTIPLY},
{"Numpad+", VK_ADD},
{"Numpad-", VK_SUBTRACT},
{"Numpad/", VK_DIVIDE},
{"Numpad.", VK_DECIMAL},
{"NumpadEnter", VK_SEPARATOR},
{"Numlock", VK_NUMLOCK},
{"ScrollLock", VK_SCROLL},
{",", 188}, {";", 188},
{".", 190}, {":", 190},
{"-", 189}, {"_", 189},
{"{", 220}, {"}", 223}, {"$", 222},
{"[", 186}, {"]", 192}, {"!", 192},
{"^", 221}, {"'", 219}, {"<", 226}, {">", 226}, {"§", 191}, {"°", 191},
{"BrowserBack", 0xA6}, {"BrowserForward", 0xA7}, 
};

tstring KeyCodeToName (int flags, int vk, bool i18n) {
tstring kn = TEXT("");
if (flags&FCONTROL) kn += (i18n?msg("Ctrl"):TEXT("Ctrl")) +TEXT("+");
if (flags&FALT) kn += (i18n?msg("Alt"):TEXT("Alt")) +TEXT("+");
if (flags&FSHIFT) kn += (i18n?msg("Shift"):TEXT("Shift")) +TEXT("+");
if (flags&FVIRTKEY) {
TCHAR ws[256]={0};
UINT scan = MapVirtualKey(vk, 0);
GetKeyNameText(scan<<16, ws, 255);
kn += ws;
}
else kn += (char)(toupper(vk));
return kn;
}

bool KeyNameToCode (const tstring& kn, int& flags, int& key) {
flags = FVIRTKEY;
key=0;
vector<tstring> parts = split(kn, TEXT("+ "));
for (int i=0, n=parts.size(); i<n-1; i++) {
tstring s = parts[i];
to_lower(s);
if (s==TEXT("ctrl")) flags |= FCONTROL;
else if (s==TEXT("shift")) flags  |= FSHIFT;
else if (s==TEXT("alt")) flags |= FALT;
}
tstring s = parts[parts.size() -1];
if (s.size()==1) {
TCHAR zz = tolower(s[0]);
short x = VkKeyScan(zz);
if ((x&0x7F00)!=0x7F00) {
key = x&0xFF;
if (x&0x100) flags |= FSHIFT;
if (x&0x200) flags |= FCONTROL;
if (x&0x400) flags |= FALT;
}}
else {
key = KEYNAMES[toString(s)];
}
return !!key;
}

int AddUserCommand (const UserFunction<void(void)>& f, int cmd) {
if (cmd<=0) {
cmd = IDM_USER_COMMAND;
while(userCommands[cmd]) cmd++;
}
userCommands[cmd] = f;
return cmd;
}

bool RemoveUserCommand (int cmd) {
auto it = userCommands.find(cmd);
if (it!=userCommands.end()) {
userCommands.erase(it);
return true;
}
return false;
}

const UserFunction<void(void)>& findUserCommand (int cmd) {
auto it = userCommands.find(cmd);
if (it!=userCommands.end()) return it->second;
else return UserFunction<void(void)>();
}

static void LoadAccelTable (HACCEL hAccel, vector<ACCEL>& accel) {
if (!hAccel) return;
int n = CopyAcceleratorTable(hAccel, NULL, 0);
accel.resize(n);
CopyAcceleratorTable(hAccel, &accel[0], n);
}

bool AddAccelerator (HACCEL& hAccel, int flags, int key, int cmd) {
vector<ACCEL> accel;
LoadAccelTable(hAccel, accel);
ACCEL acc;
memset(&acc, 0, sizeof(acc));
acc.fVirt = flags;
acc.key = key;
acc.cmd = cmd;
accel.push_back(acc);
HACCEL hNew = CreateAcceleratorTable(&accel[0], accel.size());
if (hNew) {
DestroyAcceleratorTable(hAccel);
hAccel = hNew;
}}

BOOL RemoveAccelerator (HACCEL& hAccel, int cmd) {
vector<ACCEL> accel;
LoadAccelTable(hAccel, accel);
bool found = false;
for(int i=0, n=accel.size(); i<n; i++) {
if (accel[i].cmd==cmd) {
accel[i] = accel[n -1];
accel.pop_back();
found = true;
break;
}}
if (!found) return false;
HACCEL hNew = CreateAcceleratorTable(&accel[0], accel.size());
if (hNew) {
DestroyAcceleratorTable(hAccel);
hAccel = hNew;
}
return true;
}

bool FindAccelerator (HACCEL& hAccel, int& cmd, int& flags, int& key) {
vector<ACCEL> accel;
LoadAccelTable(hAccel, accel);
for (int i=0, n=accel.size(); i<n; i++) {
if (cmd>0 && accel[i].cmd==cmd) {
flags = accel[i].fVirt;
key = accel[i].key;
return true;
}
else if (cmd<=0 && accel[i].key==key && accel[i].fVirt==flags) {
cmd = accel[i].cmd;
return true;
}}
return false;
}

int SetTimeout (const UserFunction<void(void)>& f, int time, bool repeat) {
int id = 0;
if (!repeat) id |= 0x8000;
while(timers[++id]);
timers[id] = f;
SetTimer(win, id, time, NULL);
}

void ClearTimeout (int id) {
KillTimer(win, id);
auto it = timers.find(id);
if (it!=timers.end()) timers.erase(it);
}


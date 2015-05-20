#ifndef _____ACCELERATORS_H____2
#define _____ACCELERATORS_H____2
#include "global.h"
#include<functional>

int AddUserCommand (std::function<void(void)> f, int cmd) ;
bool RemoveUserCommand (int cmd);
bool AddAccelerator (int flags, int key, int cmd);
BOOL RemoveAccelerator (int cmd);
bool FindAccelerator (int cmd, int& flags, int& key);
tstring KeyCodeToName (int flags, int vk, bool i18n);
bool KeyNameToCode (const tstring& kn, int& flags, int& key);
int SetTimeout (const std::function<void(void)>& f, int time, bool repeat);
void ClearTimeout (int id);

#endif

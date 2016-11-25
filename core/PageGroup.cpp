#include "global.h"
#include "page.h"
#include "sixpad.h"
using namespace std;

unordered_map<tstring, std::weak_ptr<PageGroup>> PageGroup::groups;

static void RecursiveDestroyMenuAndUserCommands (HMENU hMenu) {
for (int i = GetMenuItemCount(hMenu) -1; i>=0; i--) {
UINT id = GetMenuItemID(hMenu,i);
HMENU hSub = GetSubMenu(hMenu,i);
if (hSub) RecursiveDestroyMenuAndUserCommands(hSub);
else sp->RemoveUserCommand(id);
}
DestroyMenu(hMenu);
}

PageGroup::~PageGroup () {
if (accel) DestroyAcceleratorTable(accel);
for (auto itr = menus.rbegin(); itr!=menus.rend(); ++itr) {
auto& item = *itr;
if (item.flags&MF_POPUP) RecursiveDestroyMenuAndUserCommands((HMENU)item.id);
else sp->RemoveUserCommand(item.id);
}}


PageGroup::PageGroup (const tstring& n): name(n), accel(0), menus() {}

void PageGroup::AddMenu (const tstring& name, HMENU menu, UINT id, UINT pos, UINT flags) {
menus.push_back(PageGroupMenu(name, menu, id, pos, flags));
}

void PageGroup::RemoveMenu (HMENU menu, UINT id) {
auto it = std::find_if(menus.begin(), menus.end(), [&](const PageGroupMenu& m){ return m.menu==menu && m.id==id; });
if (it!=menus.end()) menus.erase(it);
}

bool PageGroup::ContainsMenu (HMENU menu, UINT id) {
auto it = std::find_if(menus.begin(), menus.end(), [&](const PageGroupMenu& m){ return m.menu==menu && m.id==id; });
return it!=menus.end();
}

PageGroupMenu* PageGroup::ContainsMenu (const tstring& name) {
printf("Finding %ls ammong %d items: \n", name.c_str(), menus.size());
for (auto& menu: menus) printf("'%ls', ", menu.name.c_str());
printf("\n");
auto it = std::find_if(menus.begin(), menus.end(), [&](const PageGroupMenu& m){ return m.name==name; });
if (it!=menus.end()) return &(*it);
else return 0;
}

shared_ptr<PageGroup> PageGroup::getGroup (const tstring& name) {
weak_ptr<PageGroup> wg = groups[name];
shared_ptr<PageGroup> g = wg.lock();
if (!g) {
g = shared_ptr<PageGroup>( new PageGroup(name) );
groups[name] = g;
}
return g;
}

shared_ptr<PageGroup> PageGroup::FindGroupContainingMenu (HMENU menu, UINT id) {
for (auto& it: groups) {
auto group = it.second.lock();
if (group && group->ContainsMenu(menu,id)) return group;
}
return 0;
}


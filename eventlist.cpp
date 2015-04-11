#include "eventlist.h"

void eventlist::add (const string& type, const EventCallback& cb) {
m.insert(pair<string,EventCallback>(type,cb));
}

bool eventlist::remove (const string& type, const EventCallback& cb) {
for (auto it = m.find(type); it!=m.end(); ++it) {
if (it->second == cb) { m.erase(it); return true; }
}
return false;
}

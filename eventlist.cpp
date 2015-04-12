#include "eventlist.h"

eventlist::eventlist () { }

void eventlist::add (const string& type, const EventCallback& cb) {
pair<string,EventCallback> x(type,cb);
m.insert(x);
}

bool eventlist::remove (const string& type, const EventCallback& cb) {
for (auto it = m.find(type); it!=m.end(); ++it) {
if (it->second == cb) { m.erase(it); return true; }
}
return false;
}

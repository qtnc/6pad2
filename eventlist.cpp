#include "eventlist.h"

void eventlist::add (const string& type, const EventCallback& cb) {
m.insert(pair<string,EventCallback>(type,cb));
}

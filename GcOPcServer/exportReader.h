#pragma once
#include "open62541.h"
#include <string>
#include <vector>
using namespace std;

extern void addRealtimeVariable(UA_Server* server);

typedef struct {
	string name;
	UA_Double value;
} NAME_VALUE;
extern UA_DateTime injectionDate;
extern UA_Int32 selector;
extern vector<NAME_VALUE> mtValues;
extern void updateRealtimeData();
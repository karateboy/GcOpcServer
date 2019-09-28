#include <thread>
#include <chrono>  
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include "open62541.h"
#include "exportReader.h"

volatile extern UA_Boolean running;

UA_DateTime injectionDate;
UA_Int32 selector = 0;
vector<NAME_VALUE> mtValues;

void exportReader() {
	

	do {
		std::ifstream infile("realtime.txt");
		std::string line;
		vector<NAME_VALUE> newMtValues;
		while (std::getline(infile, line, '\r'))
		{
			string token, value;
			int pos = line.find(',');
			token = line.substr(0, pos);
			value = line.substr(pos + 1);

			if (token == "Selector") {
				selector = atoi(value.c_str());
			}
			else if (token == "InjectionDate") {
				injectionDate = UA_DateTime_fromUnixTime(atoll(value.c_str())/1000l);
			}
			else {
				NAME_VALUE nv;
				nv.name = token;
				nv.value = stod(value);
				newMtValues.push_back(nv);
			}
		}
		infile.close();
		mtValues = newMtValues;
		this_thread::sleep_for(chrono::seconds(2));
	} while (running);
}

/*
void updateCurrentTime(UA_Server* server) {
	UA_DateTime now = UA_DateTime_now();
	UA_Variant value;
	UA_Variant_setScalar(&value, &now, &UA_TYPES[UA_TYPES_DATETIME]);
	UA_NodeId currentNodeId = UA_NODEID_STRING(1, const_cast <char*>("current-time"));
	UA_Server_writeValue(server, currentNodeId, value);
}
*/

void addRealtimeVariable(UA_Server* server) {
	UA_DateTime now = UA_DateTime_now();
	UA_VariableAttributes attr = UA_VariableAttributes_default;
	attr.displayName = UA_LOCALIZEDTEXT(const_cast <char*>("en-US"), const_cast <char*>("Injection Date"));
	attr.description = UA_LOCALIZEDTEXT(const_cast <char*>("en-US"), const_cast <char*>("Injection Date"));
	attr.accessLevel = UA_ACCESSLEVELMASK_READ;
	UA_Variant_setScalar(&attr.value, &injectionDate, &UA_TYPES[UA_TYPES_DATETIME]);
	UA_NodeId currentNodeId = UA_NODEID_STRING(1, const_cast <char*>("Injection_Date"));
	UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, const_cast <char*>("Injection_Date"));
	UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
	UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
	UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

	UA_Server_addVariableNode(server, currentNodeId, parentNodeId,
		parentReferenceNodeId, currentName,
		variableTypeNodeId, attr, NULL, NULL);
	//updateCurrentTime(server);
}
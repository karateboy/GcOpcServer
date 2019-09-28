#include <thread>
#include <chrono>  
#include <iostream>
#include <sstream>
#include <map>
#include <fstream>
#include "open62541.h"
#include "exportReader.h"

volatile extern UA_Boolean running;

UA_DateTime injectionDate;
UA_Int32 selector = 0;
map<string, double> mtValueMap;
map<string, UA_NodeId> noteIdMap;
UA_Server* server;

void addRealtimeVariable(UA_Server* serv) {
	server = serv;
	std::ifstream infile("realtime.txt");
	std::string line;

	const UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
	const UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
	const UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
	UA_VariableAttributes attr = UA_VariableAttributes_default;
	UA_NodeId nodeId = UA_NODEID_STRING(1, const_cast <char*>("Selector_channel"));
	UA_QualifiedName name = UA_QUALIFIEDNAME(1, const_cast <char*>("Selector_channel"));

	while (std::getline(infile, line, '\r'))
	{
		string token, value;
		int pos = line.find(',');
		token = line.substr(0, pos);
		value = line.substr(pos + 1);

		if (token == "Selector") {
			selector = atoi(value.c_str());
			attr.displayName = UA_LOCALIZEDTEXT(const_cast <char*>("en-US"), const_cast <char*>("Selector channel"));
			attr.description = UA_LOCALIZEDTEXT(const_cast <char*>("en-US"), const_cast <char*>("Selector channel"));
			attr.accessLevel = UA_ACCESSLEVELMASK_READ;
			UA_Variant_setScalar(&attr.value, &selector, &UA_TYPES[UA_TYPES_INT32]);
			nodeId = UA_NODEID_STRING(1, const_cast <char*>("Selector_channel"));
			name = UA_QUALIFIEDNAME(1, const_cast <char*>("Selector_channel"));
			noteIdMap[token] = nodeId;
		}
		else if (token == "InjectionDate") {
			injectionDate = UA_DateTime_fromUnixTime(atoll(value.c_str()) / 1000l);
			attr.displayName = UA_LOCALIZEDTEXT(const_cast <char*>("en-US"), const_cast <char*>("Injection Date"));
			attr.description = UA_LOCALIZEDTEXT(const_cast <char*>("en-US"), const_cast <char*>("Injection Date"));
			attr.accessLevel = UA_ACCESSLEVELMASK_READ;
			UA_Variant_setScalar(&attr.value, &injectionDate, &UA_TYPES[UA_TYPES_DATETIME]);
			nodeId = UA_NODEID_STRING(1, const_cast <char*>("Injection_Date"));
			name = UA_QUALIFIEDNAME(1, const_cast <char*>("Injection_Date"));
			noteIdMap[token] = nodeId;
		}
		else {
			double v = stod(value);
			attr.displayName = UA_LOCALIZEDTEXT(const_cast <char*>("en-US"), const_cast <char*>(token.c_str()));
			attr.description = UA_LOCALIZEDTEXT(const_cast <char*>("en-US"), const_cast <char*>(token.c_str()));
			attr.accessLevel = UA_ACCESSLEVELMASK_READ;
			UA_Variant_setScalar(&attr.value, &v, &UA_TYPES[UA_TYPES_DOUBLE]);
			nodeId = UA_NODEID_STRING(1, const_cast <char*>(token.c_str()));
			name = UA_QUALIFIEDNAME(1, const_cast <char*>(token.c_str()));
			noteIdMap[token] = nodeId;
			mtValueMap[token] = v;
		}
		UA_Server_addVariableNode(server, nodeId, parentNodeId,
			parentReferenceNodeId, name,
			variableTypeNodeId, attr, NULL, NULL);
	}
	infile.close();
}

void updateRealtimeData() {
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
				UA_Int32 v = atoi(value.c_str());
				if (v != selector) {
					selector = v;
					UA_Variant variant;
					UA_Variant_init(&variant);
					UA_Variant_setScalar(&variant, &selector, &UA_TYPES[UA_TYPES_INT32]);
					UA_NodeId nodeId = noteIdMap[token];
					UA_Server_writeValue(server, nodeId, variant);
				}
			}
			else if (token == "InjectionDate") {
				UA_DateTime v = UA_DateTime_fromUnixTime(atoll(value.c_str()) / 1000l);
				if (v != injectionDate) {
					injectionDate = v;
					UA_Variant variant;
					UA_Variant_init(&variant);
					UA_Variant_setScalar(&variant, &injectionDate, &UA_TYPES[UA_TYPES_DATETIME]);
					UA_NodeId nodeId = noteIdMap[token];
					UA_Server_writeValue(server, nodeId, variant);
				}
			}
			else {
				double v = stod(value);
				if (v != mtValueMap[token]) {
					mtValueMap[token] = v;
					UA_Variant variant;
					UA_Variant_init(&variant);
					UA_Variant_setScalar(&variant, &v, &UA_TYPES[UA_TYPES_DOUBLE]);
					UA_NodeId nodeId = noteIdMap[token];
					UA_Server_writeValue(server, nodeId, variant);
				}
			}
		}
		infile.close();
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
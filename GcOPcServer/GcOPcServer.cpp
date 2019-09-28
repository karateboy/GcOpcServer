// GcOPcServer.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//
#include "open62541.h"
#include <signal.h>
#include <stdlib.h>
#include "exportReader.h"

volatile UA_Boolean running = true;
static void stopHandler(int sig) {
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
	running = false;
}

#include <iostream>
#include <thread>
using namespace std;


static void beforeReadTime(UA_Server* server,
	const UA_NodeId* sessionId, void* sessionContext,
	const UA_NodeId* nodeid, void* nodeContext,
	const UA_NumericRange* range, const UA_DataValue* data) {
	UA_Variant value;
	UA_Variant_setScalar(&value, &injectionDate, &UA_TYPES[UA_TYPES_DATETIME]);
	UA_NodeId currentNodeId = UA_NODEID_STRING(1, const_cast <char*>("Injection_Date"));
	UA_Server_writeValue(server, currentNodeId, value);
}

static void afterWriteTime(UA_Server* server,
	const UA_NodeId* sessionId, void* sessionContext,
	const UA_NodeId* nodeId, void* nodeContext,
	const UA_NumericRange* range, const UA_DataValue* data) {
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
		"The variable was updated");
}
static void addValueCallbackToCurrentTimeVariable(UA_Server* server) {
	UA_NodeId currentNodeId = UA_NODEID_STRING(1, const_cast <char*>("current-time"));
	UA_ValueCallback callback;
	callback.onRead = beforeReadTime;
	callback.onWrite = afterWriteTime;
	UA_Server_setVariableNode_valueCallback(server, currentNodeId, callback);
}

int main()
{
	signal(SIGINT, stopHandler);
	signal(SIGTERM, stopHandler);

	std::cout << "Gc OPC Server Start!\n";
	UA_ServerConfig* config = UA_ServerConfig_new_default();
	UA_Server* server = UA_Server_new(config);
	addRealtimeVariable(server);
	thread readerThread(updateRealtimeData);

	//addValueCallbackToCurrentTimeVariable(server);

	UA_StatusCode retval = UA_Server_run(server, &running);
	UA_Server_delete(server);
	UA_ServerConfig_delete(config);
	std::cout << "Gc OPC Server stopped!\n";

	readerThread.join();

	return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}

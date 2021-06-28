#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <stdint.h>

#include "SimConnect.h"

BOOL quit_game = FALSE;

HANDLE hSimConnect = NULL;

static enum GROUP_ID {
	GROUP0
};

static enum EVENT_ID {
	EVENT0,
	EVENT1,
};

static enum DATA_DEFINE_ID {
	DEFINITION_COM_ALL,
	DEFINITION_COM_ACTIVE,
	DEFINITION_COM_STANDBY
};

static enum DATA_REQUEST_ID {
	REQUEST_COM,
	REQUEST_OTHER
};

typedef struct COM_t {
	DWORD com_active_1;
	DWORD com_active_2;
	DWORD com_standby_1;
	DWORD com_standby_2;
};

// parameter: bcd 16-bit binary-coded decimal
void printCOMFrequency(DWORD bcd) {
	// the leading 1 in com frequency is assumed
	std::cout << "1";
	DWORD freq[4];
	for (int i = 3; i >= 0; --i) {
		freq[i] = bcd % 16;
		bcd = bcd >> 4;
	}

	for (int i = 0; i < 4; ++i) {
		std::cout << freq[i];
		if (i == 1) std::cout << ".";
	}
	std::cout << std::endl;
}

// params: pointer to data buffer, size of data buffer, pointer passed
void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext) {
	switch (pData->dwID) {

	case SIMCONNECT_RECV_ID_NULL: {
		break;
	}

	case SIMCONNECT_RECV_ID_OPEN: {
		// struct returned after a successful open call
		SIMCONNECT_RECV_OPEN* pOpen = (SIMCONNECT_RECV_OPEN*)pData;

		std::cout << "SimConnect connection successful!" << std::endl;
		std::cout << "Application name: " << pOpen->szApplicationName << std::endl;
		std::cout << "Application version: " << pOpen->dwApplicationVersionMajor << "."
			<< pOpen->dwApplicationVersionMinor << std::endl;
		std::cout << "Appliation build: " << pOpen->dwApplicationBuildMajor << "."
			<< pOpen->dwApplicationBuildMinor << std::endl;
		std::cout << "SimConnect version: " << pOpen->dwSimConnectVersionMajor << "."
			<< pOpen->dwSimConnectVersionMinor << std::endl;
		std::cout << "SimConnect build: " << pOpen->dwSimConnectBuildMajor << "."
			<< pOpen->dwSimConnectBuildMinor << std::endl;
		std::cout << std::endl;
		break;
	}

	case SIMCONNECT_RECV_ID_QUIT: {
		// when user exits from msfs
		std::cout << "Application closed!" << std::endl;
		quit_game = TRUE;
		break;
	}

	case SIMCONNECT_RECV_ID_EXCEPTION: {
		std::cout << "An Exception has occurred!" << std::endl;
		SIMCONNECT_RECV_EXCEPTION* except = (SIMCONNECT_RECV_EXCEPTION*)pData;
		std::cout << "Exception ID: " << except->dwException << " Packet ID: " << except->dwSendID
			<< " Index ID: " << except->dwIndex << std::endl;
		break;
	}

	case SIMCONNECT_RECV_ID_EVENT: {
		break;
	}

	case SIMCONNECT_RECV_ID_EVENT_OBJECT_ADDREMOVE: {
		break;
	}

	case SIMCONNECT_RECV_ID_EVENT_FILENAME: {
		break;
	}

	case SIMCONNECT_RECV_ID_EVENT_FRAME: {
		break;
	}

	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
		// receives after a successful call to either 
		// SimConnect_RequestDataOnSimObject or
		// SimConnect_RequestDataOnSimObjectType
		SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData;

		switch (pObjData->dwRequestID) {
		case REQUEST_COM: {
			COM_t* pComData = (COM_t*)&pObjData->dwData;
			// prints each com frequency
			std::cout << "Number of 8-byte elements received: " << pObjData->dwDefineCount << std::endl;
			std::cout << "COM Active 1: ";
			printCOMFrequency(pComData->com_active_1);
			std::cout << "COM Active 2: ";
			printCOMFrequency(pComData->com_active_2);
			std::cout << "COM Standby 1: ";
			printCOMFrequency(pComData->com_standby_1);
			std::cout << "COM Standby 2: ";
			printCOMFrequency(pComData->com_standby_2);
			std::cout << std::endl;
			break;
		}

		case REQUEST_OTHER: {
			break;
		}
		}


		break;
	}

	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE: {
		break;
	}

	case SIMCONNECT_RECV_ID_CLOUD_STATE: {
		break;
	}

	case SIMCONNECT_RECV_ID_WEATHER_OBSERVATION: {
		break;
	}

	case SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID: {
		break;
	}

	case SIMCONNECT_RECV_ID_RESERVED_KEY: {
		break;
	}

	case SIMCONNECT_RECV_ID_CUSTOM_ACTION: {
		break;
	}

	case SIMCONNECT_RECV_ID_SYSTEM_STATE: {
		break;
	}

	case SIMCONNECT_RECV_ID_CLIENT_DATA: {
		break;
	}

	case SIMCONNECT_RECV_ID_EVENT_WEATHER_MODE: {
		break;
	}

	case SIMCONNECT_RECV_ID_AIRPORT_LIST: {
		break;
	}

	case SIMCONNECT_RECV_ID_VOR_LIST: {
		break;
	}

	case SIMCONNECT_RECV_ID_NDB_LIST: {
		break;
	}

	case SIMCONNECT_RECV_ID_WAYPOINT_LIST: {
		break;
	}

	case SIMCONNECT_RECV_ID_EVENT_MULTIPLAYER_SERVER_STARTED: {
		break;
	}

	case SIMCONNECT_RECV_ID_EVENT_MULTIPLAYER_CLIENT_STARTED: {
		break;
	}

	case SIMCONNECT_RECV_ID_EVENT_MULTIPLAYER_SESSION_ENDED: {
		break;
	}

	case SIMCONNECT_RECV_ID_EVENT_RACE_END: {
		break;
	}

	case SIMCONNECT_RECV_ID_EVENT_RACE_LAP: {
		break;
	}

	default: {
		break;
	}
	}
}


int __cdecl main(int argc, char* argv[]) {
	HANDLE hEventHandle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hEventHandle == NULL) {
		std::cout << "Error: Event creation failed! Exiting" << std::endl;
		return GetLastError();
	}

	if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Com Viewer", NULL, 0, hEventHandle, 0))) {
		std::cout << "Connected...!" << std::endl;

		HRESULT hr;

		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT0);
		hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP0, EVENT0, TRUE);
		hr = SimConnect_SetNotificationGroupPriority(hSimConnect, GROUP0, SIMCONNECT_GROUP_PRIORITY_HIGHEST);

		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_COM_ALL, "COM ACTIVE FREQUENCY:1", 
											"Frequency BCD16", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_COM_ALL, "COM ACTIVE FREQUENCY:2", 
											"Frequency BCD16", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_COM_ALL, "COM STANDBY FREQUENCY:1", 
											"Frequency BCD16", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_COM_ALL, "COM STANDBY FREQUENCY:2", 
											"Frequency BCD16", SIMCONNECT_DATATYPE_INT32);

		hr = SimConnect_RequestDataOnSimObject(hSimConnect, REQUEST_COM, DEFINITION_COM_ALL, 
			SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SECOND);

		while (!quit_game && ::WaitForSingleObject(hEventHandle, INFINITE) == WAIT_OBJECT_0) {
			hr = SimConnect_CallDispatch(hSimConnect, MyDispatchProc, NULL);
			if (FAILED(hr)) {
				break;
			}
		}

		hr = SimConnect_Close(hSimConnect);
		CloseHandle(hEventHandle);
	}


	return 0;
}
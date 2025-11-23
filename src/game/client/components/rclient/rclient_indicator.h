#ifndef RCLIENT_RCLIENT_INDICATOR_H
#define RCLIENT_RCLIENT_INDICATOR_H

#include "engine/client.h"

#include <engine/shared/console.h>

#include <game/client/component.h>

#include <sio_client.h>

#include <map>
#include <mutex>
#include <string>
#include <vector>

class CRClientIndicator : public CComponent
{
	static constexpr const char *RCLIENT_SERVER_URL = "http://localhost:5050";

	// Socket.IO client
	sio::client m_Socket;

	// Authentication
	char m_aAuthToken[128] = {0};
	bool m_TokenReceived = false;

	// Current connection state
	bool m_IsConnected = false;
	bool m_Registered = false;
	char m_aCurrentServerAddress[256];
	int m_CurrentPlayerId = -1;
	int m_CurrentDummyId = -1;

	// Track previous client state
	int m_PrevClientState = IClient::STATE_OFFLINE;

	// RClient users data: server_address -> player_id -> has_player
	std::map<std::string, std::map<int, bool>> m_RClientUsers;
	std::mutex m_RClientUsersMutex;

	// Socket.IO event handlers
	void OnSocketConnected();
	void OnSocketDisconnected(sio::client::close_reason const &Reason);
	void OnSocketFailed();
	void OnTokenReceived(sio::event &Event);
	void OnRegistrationSuccess(sio::event &Event);
	void OnUnregisterSuccess(sio::event &Event);
	void OnPlayersUpdate(sio::event &Event);
	void OnError(sio::event &Event);

	// Connection management
	void ConnectToServer();
	void DisconnectFromServer();
	void RegisterPlayer();
	void UpdateServerInfo();
	void SetupSocketListeners();

public:
	CRClientIndicator();
	~CRClientIndicator();

	int Sizeof() const override { return sizeof(*this); }
	void OnInit() override;
	void OnShutdown() override;
	void OnRender() override;

	bool IsPlayerRClient(int ClientId);
	bool IsConnected() const { return m_IsConnected; }

	sio::client* GetSocket() { return &m_Socket; }
};

#endif // RCLIENT_RCLIENT_INDICATOR_H
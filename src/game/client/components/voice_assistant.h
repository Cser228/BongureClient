#ifndef GAME_CLIENT_COMPONENTS_VOICE_ASSISTANT_H
#define GAME_CLIENT_COMPONENTS_VOICE_ASSISTANT_H

#include <game/client/component.h>
#include <engine/shared/config.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

struct VoskModel;
struct VoskRecognizer;

class CVoiceAssistant : public CComponent
{
public:
	CVoiceAssistant();
	virtual ~CVoiceAssistant();

	virtual int Sizeof() const override { return sizeof(*this); }
	virtual void OnConsoleInit() override;
	virtual void OnInit() override;
	virtual void OnRender() override;
	virtual void OnShutdown() override;

	void Toggle();
	bool IsActive() const { return m_Active.load(); }

private:
	VoskModel *m_pModel;
	VoskRecognizer *m_pRecognizer;
	unsigned int m_AudioDevice;

	std::thread m_Thread;
	std::atomic<bool> m_Active;
	std::atomic<bool> m_Running;

	std::mutex m_Mutex;
	std::queue<std::string> m_vPendingChat;
	std::queue<std::string> m_vPendingLogs;

	enum EState
	{
		STATE_IDLE,
		STATE_WAITING_COMMAND
	};
	EState m_State;
	std::chrono::steady_clock::time_point m_TriggerTime;
	bool m_Initialized;

	void Run();
	void ProcessText(const std::string &Text);
	size_t FindTrigger(const std::string &Text) const;
	std::string ExtractCommand(const std::string &Text) const;
	std::string ParseJsonText(const char *pJson) const;
	void QueueChat(const std::string &Msg);
	void QueueLog(const std::string &Msg);

	static void ConToggle(IConsole::IResult *pResult, void *pUserData);
};

#endif
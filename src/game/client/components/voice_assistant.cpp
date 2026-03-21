#include "voice_assistant.h"

#include <vosk_api.h>

#include <SDL.h>

#include <base/system.h>
#include <engine/console.h>
#include <engine/shared/config.h>
#include <game/client/gameclient.h>

CVoiceAssistant::CVoiceAssistant()
{
	m_pModel = nullptr;
	m_pRecognizer = nullptr;
	m_AudioDevice = 0;
	m_Active = false;
	m_Running = false;
	m_State = STATE_IDLE;
	m_Initialized = false;
}

CVoiceAssistant::~CVoiceAssistant()
{
	OnShutdown();
}

// ── Console ──

void CVoiceAssistant::ConToggle(IConsole::IResult *pResult, void *pUserData)
{
	((CVoiceAssistant *)pUserData)->Toggle();
}

void CVoiceAssistant::OnConsoleInit()
{
	Console()->Register(
		"toggle_voice_assistant", "", CFGFLAG_CLIENT,
		ConToggle, this,
		"Toggle Bongure voice assistant on/off");
}

// ── Init ──

void CVoiceAssistant::OnInit()
{
	vosk_set_log_level(-1);

	// Ищем модель
	const char *apPaths[] = {
		"vosk-model-ru",
		"data/vosk-model-ru",
		"../vosk-model-ru",
		"./vosk-model-ru",
	};

	for(const char *pPath : apPaths)
	{
		m_pModel = vosk_model_new(pPath);
		if(m_pModel)
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "[Bongure] Vosk model loaded from: %s", pPath);
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bongure", aBuf);
			break;
		}
	}

	if(!m_pModel)
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bongure",
			"[Bongure] ERROR: vosk-model-ru not found!");
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bongure",
			"[Bongure] Download vosk-model-small-ru-0.22, rename folder to vosk-model-ru");
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bongure",
			"[Bongure] Place it next to the .exe file");
		return;
	}

	m_pRecognizer = vosk_recognizer_new(m_pModel, 16000.0f);
	if(!m_pRecognizer)
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bongure",
			"[Bongure] Failed to create recognizer");
		vosk_model_free(m_pModel);
		m_pModel = nullptr;
		return;
	}

	// Открываем микрофон
	if(SDL_WasInit(SDL_INIT_AUDIO) == 0)
		SDL_InitSubSystem(SDL_INIT_AUDIO);

	SDL_AudioSpec Desired;
	SDL_zero(Desired);
	Desired.freq = 16000;
	Desired.format = AUDIO_S16SYS;
	Desired.channels = 1;
	Desired.samples = 4096;
	Desired.callback = nullptr;

	m_AudioDevice = SDL_OpenAudioDevice(nullptr, 1, &Desired, nullptr, 0);
	if(m_AudioDevice == 0)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "[Bongure] Microphone error: %s", SDL_GetError());
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bongure", aBuf);
		vosk_recognizer_free(m_pRecognizer);
		vosk_model_free(m_pModel);
		m_pRecognizer = nullptr;
		m_pModel = nullptr;
		return;
	}

	m_Initialized = true;
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bongure",
		"[Bongure] Voice assistant ready! Use: toggle_voice_assistant");
}

// ── Toggle ──

void CVoiceAssistant::Toggle()
{
	if(!m_Initialized)
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bongure",
			"[Bongure] Not initialized (model missing?)");
		return;
	}

	if(m_Active.load())
	{
		m_Running = false;
		if(m_Thread.joinable())
			m_Thread.join();
		SDL_PauseAudioDevice(m_AudioDevice, 1);
		m_Active = false;
		m_State = STATE_IDLE;
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bongure",
			"[Bongure] Voice assistant OFF");
	}
	else
	{
		m_Active = true;
		m_Running = true;
		m_State = STATE_IDLE;

		vosk_recognizer_free(m_pRecognizer);
		m_pRecognizer = vosk_recognizer_new(m_pModel, 16000.0f);

		SDL_ClearQueuedAudio(m_AudioDevice);
		SDL_PauseAudioDevice(m_AudioDevice, 0);

		m_Thread = std::thread(&CVoiceAssistant::Run, this);

		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bongure",
			"[Bongure] Voice assistant ON - say 'Bonga, skazhi ...'");
	}
}

// ── OnRender (main thread — отправка в чат) ──

void CVoiceAssistant::OnRender()
{
	if(!m_Active.load())
		return;

	std::lock_guard<std::mutex> Lock(m_Mutex);

	while(!m_vPendingLogs.empty())
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bongure",
			m_vPendingLogs.front().c_str());
		m_vPendingLogs.pop();
	}

	while(!m_vPendingChat.empty())
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "say %s", m_vPendingChat.front().c_str());
		Console()->ExecuteLine(aBuf, -1);
		m_vPendingChat.pop();
	}
}

// ── Shutdown ──

void CVoiceAssistant::OnShutdown()
{
	m_Running = false;
	m_Active = false;

	if(m_Thread.joinable())
		m_Thread.join();

	if(m_AudioDevice)
	{
		SDL_CloseAudioDevice(m_AudioDevice);
		m_AudioDevice = 0;
	}
	if(m_pRecognizer)
	{
		vosk_recognizer_free(m_pRecognizer);
		m_pRecognizer = nullptr;
	}
	if(m_pModel)
	{
		vosk_model_free(m_pModel);
		m_pModel = nullptr;
	}
	m_Initialized = false;
}

// ── Background thread ──

void CVoiceAssistant::Run()
{
	const int BufSize = 8000;
	char aBuffer[BufSize];

	while(m_Running.load())
	{
		unsigned int Queued = SDL_GetQueuedAudioSize(m_AudioDevice);
		if(Queued >= (unsigned int)BufSize)
		{
			unsigned int Read = SDL_DequeueAudio(m_AudioDevice, aBuffer, BufSize);
			if(Read > 0)
			{
				int Final = vosk_recognizer_accept_waveform(m_pRecognizer, aBuffer, (int)Read);
				if(Final)
				{
					const char *pJson = vosk_recognizer_result(m_pRecognizer);
					std::string Text = ParseJsonText(pJson);
					if(!Text.empty())
					{
						QueueLog(std::string("[Heard] ") + Text);
						ProcessText(Text);
					}
				}
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
		}
	}

	const char *pFinal = vosk_recognizer_final_result(m_pRecognizer);
	std::string Last = ParseJsonText(pFinal);
	if(!Last.empty())
	{
		QueueLog(std::string("[Heard-final] ") + Last);
		ProcessText(Last);
	}
}

// ── Parse Vosk JSON: {"text" : "слова"} ──

std::string CVoiceAssistant::ParseJsonText(const char *pJson) const
{
	if(!pJson)
		return "";

	const char *pKey = "\"text\"";
	const char *pPos = strstr(pJson, pKey);
	if(!pPos)
		return "";

	pPos += strlen(pKey);

	// skip to opening quote of value
	while(*pPos && *pPos != '"')
		pPos++;
	if(!*pPos)
		return "";
	pPos++; // skip opening "

	const char *pEnd = pPos;
	while(*pEnd && *pEnd != '"')
		pEnd++;

	if(pEnd <= pPos)
		return "";

	return std::string(pPos, (size_t)(pEnd - pPos));
}

// ── Find trigger word ──

size_t CVoiceAssistant::FindTrigger(const std::string &Text) const
{
	// Длинные варианты первыми
	const char *apTriggers[] = {
		"\xd1\x8d\xd0\xb9 \xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd0\xb0",         // "эй бонга"
		"\xd1\x8d\xd0\xb9 \xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd1\x83\xd1\x80\xd0\xb0", // "эй бонгура"
		"\xd1\x85\xd0\xb5\xd0\xb9 \xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd0\xb0",  // "хей бонга"
		"\xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd1\x83\xd1\x80\xd0\xb0",           // "бонгура"
		"\xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd0\xb0",                           // "бонга"
		"\xd1\x8d\xd0\xb9 \xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd0\xbe",           // "эй бонго"
		"\xd1\x85\xd0\xb5\xd0\xb9 \xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd0\xbe",   // "хей бонго"
		"\xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd0\xbe",                           // "бонго"
		"\xd1\x85\xd0\xb5\xd0\xb9 \xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd1\x83\xd1\x80\xd0\xb0", // "хей бонгура"
	};

	for(const char *pTrigger : apTriggers)
	{
		std::string Trigger(pTrigger);
		size_t Pos = Text.find(Trigger);
		if(Pos != std::string::npos)
			return Pos + Trigger.length();
	}

	return std::string::npos;
}

// ── Extract command after trigger ──

std::string CVoiceAssistant::ExtractCommand(const std::string &Text) const
{
	std::string Trimmed = Text;
	size_t Start = Trimmed.find_first_not_of(' ');
	if(Start == std::string::npos)
		return "";
	Trimmed = Trimmed.substr(Start);

	// Команды (длинные первыми)
	const char *apCmds[] = {
		"\xd0\xbd\xd0\xb0\xd0\xbf\xd0\xb8\xd1\x88\xd0\xb8 \xd0\xb2 \xd1\x87\xd0\xb0\xd1\x82 ",    // "напиши в чат "
		"\xd0\xbd\xd0\xb0\xd0\xbf\xd0\xb8\xd1\x88\xd0\xb8 \xd0\xb2 \xd1\x87\xd0\xb0\xd1\x82\xd0\xb5 ", // "напиши в чате "
		"\xd1\x81\xd0\xba\xd0\xb0\xd0\xb6\xd0\xb8 \xd0\xb2 \xd1\x87\xd0\xb0\xd1\x82 ",              // "скажи в чат "
		"\xd0\xbe\xd1\x82\xd0\xbf\xd1\x80\xd0\xb0\xd0\xb2\xd1\x8c \xd0\xb2 \xd1\x87\xd0\xb0\xd1\x82 ", // "отправь в чат "
		"\xd0\xbd\xd0\xb0\xd0\xbf\xd0\xb8\xd1\x88\xd0\xb8 ",  // "напиши "
		"\xd1\x81\xd0\xba\xd0\xb0\xd0\xb6\xd0\xb8 ",          // "скажи "
		"\xd0\xbe\xd1\x82\xd0\xbf\xd1\x80\xd0\xb0\xd0\xb2\xd1\x8c ", // "отправь "
		"\xd0\xbf\xd0\xb8\xd1\x88\xd0\xb8 ",                  // "пиши "
	};

	for(const char *pCmd : apCmds)
	{
		std::string Cmd(pCmd);
		if(Trimmed.find(Cmd) == 0)
		{
			std::string Msg = Trimmed.substr(Cmd.length());
			size_t End = Msg.find_last_not_of(' ');
			if(End != std::string::npos)
				Msg = Msg.substr(0, End + 1);
			if(!Msg.empty())
				return Msg;
		}
	}

	return "";
}

// ── Process recognized text (state machine) ──

void CVoiceAssistant::ProcessText(const std::string &Text)
{
	auto Now = std::chrono::steady_clock::now();

	if(m_State == STATE_WAITING_COMMAND)
	{
		auto Elapsed = std::chrono::duration_cast<std::chrono::seconds>(
			Now - m_TriggerTime).count();

		if(Elapsed > 8)
		{
			m_State = STATE_IDLE;
			QueueLog("[Bongure] Timeout, trigger cancelled");
		}
		else
		{
			std::string Message = ExtractCommand(Text);
			if(!Message.empty())
			{
				QueueChat(Message);
				QueueLog(std::string("[Bongure] Sending to chat: ") + Message);
				m_State = STATE_IDLE;
				return;
			}
			return;
		}
	}

	// STATE_IDLE — ищем триггер
	size_t TriggerEnd = FindTrigger(Text);
	if(TriggerEnd == std::string::npos)
		return;

	// Триггер найден — есть ли команда в той же фразе?
	if(TriggerEnd < Text.length())
	{
		std::string After = Text.substr(TriggerEnd);
		std::string Message = ExtractCommand(After);
		if(!Message.empty())
		{
			QueueChat(Message);
			QueueLog(std::string("[Bongure] Sending to chat: ") + Message);
			return;
		}
	}

	// Триггер есть, команды нет — ждём
	m_State = STATE_WAITING_COMMAND;
	m_TriggerTime = Now;
	QueueLog("[Bongure] Heard trigger! Waiting for command...");
}

void CVoiceAssistant::QueueChat(const std::string &Msg)
{
	std::lock_guard<std::mutex> Lock(m_Mutex);
	m_vPendingChat.push(Msg);
}

void CVoiceAssistant::QueueLog(const std::string &Msg)
{
	std::lock_guard<std::mutex> Lock(m_Mutex);
	m_vPendingLogs.push(Msg);
}
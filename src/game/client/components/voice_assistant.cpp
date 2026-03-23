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

void CVoiceAssistant::ConBongaVoice(IConsole::IResult *pResult, void *pUserData)
{
	CVoiceAssistant *pSelf = (CVoiceAssistant *)pUserData;

	if(pResult->NumArguments() == 0)
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "bonga_voice %d", pSelf->IsActive() ? 1 : 0);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bongure", aBuf);
		return;
	}

	int Value = pResult->GetInteger(0);
	if(Value == 1 && !pSelf->IsActive())
		pSelf->Toggle();
	else if(Value == 0 && pSelf->IsActive())
		pSelf->Toggle();
}

void CVoiceAssistant::OnConsoleInit()
{
	Console()->Register(
		"bonga_voice", "?i[enabled]", CFGFLAG_CLIENT,
		ConBongaVoice, this,
		"Enable/disable Bongure voice assistant (1/0)");
}

void CVoiceAssistant::OnInit()
{
	vosk_set_log_level(-1);

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
			break;
	}

	if(!m_pModel)
		return;

	m_pRecognizer = vosk_recognizer_new(m_pModel, 16000.0f);
	if(!m_pRecognizer)
	{
		vosk_model_free(m_pModel);
		m_pModel = nullptr;
		return;
	}

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
		vosk_recognizer_free(m_pRecognizer);
		vosk_model_free(m_pModel);
		m_pRecognizer = nullptr;
		m_pModel = nullptr;
		return;
	}

	m_Initialized = true;
}

void CVoiceAssistant::Toggle()
{
	if(!m_Initialized)
		return;

	if(m_Active.load())
	{
		m_Running = false;
		if(m_Thread.joinable())
			m_Thread.join();
		SDL_PauseAudioDevice(m_AudioDevice, 1);
		m_Active = false;
		m_State = STATE_IDLE;
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
	}
}

void CVoiceAssistant::OnRender()
{
	if(!m_Active.load())
		return;

	std::lock_guard<std::mutex> Lock(m_Mutex);

	while(!m_vPendingChat.empty())
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "say %s", m_vPendingChat.front().c_str());
		Console()->ExecuteLine(aBuf, -1);
		m_vPendingChat.pop();
	}
}

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
						ProcessText(Text);
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
		ProcessText(Last);
}

std::string CVoiceAssistant::ParseJsonText(const char *pJson) const
{
	if(!pJson)
		return "";

	const char *pPos = strstr(pJson, "\"text\"");
	if(!pPos)
		return "";

	pPos += strlen("\"text\"");
	while(*pPos && *pPos != '"')
		pPos++;
	if(!*pPos)
		return "";
	pPos++;

	const char *pEnd = pPos;
	while(*pEnd && *pEnd != '"')
		pEnd++;

	if(pEnd <= pPos)
		return "";

	return std::string(pPos, (size_t)(pEnd - pPos));
}

size_t CVoiceAssistant::FindTrigger(const std::string &Text) const
{
	const char *apTriggers[] = {
		// эй + варианты
		"\xd1\x8d\xd0\xb9 \xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd0\xb0",               // эй бонга
		"\xd1\x8d\xd0\xb9 \xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd1\x83\xd1\x80\xd0\xb0", // эй бонгура
		"\xd1\x8d\xd0\xb9 \xd0\xbc\xd0\xb0\xd0\xbd\xd0\xb3\xd0\xbe",                 // эй манго
		"\xd1\x8d\xd0\xb9 \xd0\xb2\xd0\xb0\xd0\xbd\xd0\xb3\xd0\xb0",                 // эй ванга
		// хей + варианты
		"\xd1\x85\xd0\xb5\xd0\xb9 \xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd0\xb0",         // хей бонга
		"\xd1\x85\xd0\xb5\xd0\xb9 \xd0\xbc\xd0\xb0\xd0\xbd\xd0\xb3\xd0\xbe",         // хей манго
		"\xd1\x85\xd0\xb5\xd0\xb9 \xd0\xb2\xd0\xb0\xd0\xbd\xd0\xb3\xd0\xb0",         // хей ванга
		// одиночные
		"\xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd1\x83\xd1\x80\xd0\xb0",                   // бонгура
		"\xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd0\xb0",                                   // бонга
		"\xd0\xbc\xd0\xb0\xd0\xbd\xd0\xb3\xd0\xbe",                                   // манго
		"\xd0\xb2\xd0\xb0\xd0\xbd\xd0\xb3\xd0\xb0",                                   // ванга
		"\xd0\xb1\xd0\xbe\xd0\xbd\xd0\xb3\xd0\xbe",                                   // бонго
		"\xd0\xb1\xd0\xb0\xd0\xbd\xd0\xb3\xd0\xb0",                                   // банга
		"\xd0\xb1\xd0\xb0\xd0\xbd\xd0\xba\xd0\xb0",                                   // банка
		"\xd0\xb1\xd0\xb0\xd0\xbd\xd0\xba",                                           // банк
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

std::string CVoiceAssistant::ExtractCommand(const std::string &Text, bool *pCaps) const
{
	std::string Trimmed = Text;
	size_t Start = Trimmed.find_first_not_of(' ');
	if(Start == std::string::npos)
		return "";
	Trimmed = Trimmed.substr(Start);

	if(pCaps)
		*pCaps = false;

	// Капс-команды (крикни, орни) — проверяем ПЕРВЫМИ
	const char *apCapsCmds[] = {
		"\xd0\xba\xd1\x80\xd0\xb8\xd0\xba\xd0\xbd\xd0\xb8 \xd0\xb2 \xd1\x87\xd0\xb0\xd1\x82 ",   // крикни в чат
		"\xd0\xbe\xd1\x80\xd0\xbd\xd0\xb8 \xd0\xb2 \xd1\x87\xd0\xb0\xd1\x82 ",                     // орни в чат
		"\xd0\xbf\xd1\x80\xd0\xbe\xd0\xbe\xd1\x80\xd0\xb8 \xd0\xb2 \xd1\x87\xd0\xb0\xd1\x82 ",     // проори в чат
		"\xd0\xba\xd1\x80\xd0\xb8\xd0\xba\xd0\xbd\xd0\xb8 ",     // крикни
		"\xd0\xbe\xd1\x80\xd0\xbd\xd0\xb8 ",                       // орни
		"\xd0\xbf\xd1\x80\xd0\xbe\xd0\xbe\xd1\x80\xd0\xb8 ",       // проори
	};

	for(const char *pCmd : apCapsCmds)
	{
		std::string Cmd(pCmd);
		if(Trimmed.find(Cmd) == 0)
		{
			std::string Msg = Trimmed.substr(Cmd.length());
			size_t End = Msg.find_last_not_of(' ');
			if(End != std::string::npos)
				Msg = Msg.substr(0, End + 1);
			if(!Msg.empty())
			{
				if(pCaps)
					*pCaps = true;
				return Msg;
			}
		}
	}

	// Обычные команды
	const char *apCmds[] = {
		"\xd0\xbd\xd0\xb0\xd0\xbf\xd0\xb8\xd1\x88\xd0\xb8 \xd0\xb2 \xd1\x87\xd0\xb0\xd1\x82 ",
		"\xd0\xbd\xd0\xb0\xd0\xbf\xd0\xb8\xd1\x88\xd0\xb8 \xd0\xb2 \xd1\x87\xd0\xb0\xd1\x82\xd0\xb5 ",
		"\xd1\x81\xd0\xba\xd0\xb0\xd0\xb6\xd0\xb8 \xd0\xb2 \xd1\x87\xd0\xb0\xd1\x82 ",
		"\xd0\xbe\xd1\x82\xd0\xbf\xd1\x80\xd0\xb0\xd0\xb2\xd1\x8c \xd0\xb2 \xd1\x87\xd0\xb0\xd1\x82 ",
		"\xd0\xbd\xd0\xb0\xd0\xbf\xd0\xb8\xd1\x88\xd0\xb8 ",
		"\xd1\x81\xd0\xba\xd0\xb0\xd0\xb6\xd0\xb8 ",
		"\xd0\xbe\xd1\x82\xd0\xbf\xd1\x80\xd0\xb0\xd0\xb2\xd1\x8c ",
		"\xd0\xbf\xd0\xb8\xd1\x88\xd0\xb8 ",
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

std::string CVoiceAssistant::ToUpperCase(const std::string &Text) const
{
	std::string Result;
	Result.reserve(Text.size());

	for(size_t i = 0; i < Text.size(); i++)
	{
		unsigned char c = (unsigned char)Text[i];

		// ASCII a-z
		if(c >= 'a' && c <= 'z')
		{
			Result += (char)(c - 32);
		}
		// Русские а-п: 0xD0 0xB0..0xBF → А-П: 0xD0 0x90..0x9F
		else if(c == 0xD0 && i + 1 < Text.size())
		{
			unsigned char c2 = (unsigned char)Text[i + 1];
			if(c2 >= 0xB0 && c2 <= 0xBF)
			{
				Result += (char)0xD0;
				Result += (char)(c2 - 0x20);
				i++;
			}
			else
			{
				Result += (char)c;
			}
		}
		// Русские р-я: 0xD1 0x80..0x8F → Р-Я: 0xD0 0xA0..0xAF
		// ё: 0xD1 0x91 → Ё: 0xD0 0x81
		else if(c == 0xD1 && i + 1 < Text.size())
		{
			unsigned char c2 = (unsigned char)Text[i + 1];
			if(c2 >= 0x80 && c2 <= 0x8F)
			{
				Result += (char)0xD0;
				Result += (char)(c2 + 0x20);
				i++;
			}
			else if(c2 == 0x91) // ё → Ё
			{
				Result += (char)0xD0;
				Result += (char)0x81;
				i++;
			}
			else
			{
				Result += (char)c;
			}
		}
		else
		{
			Result += (char)c;
		}
	}

	return Result;
}

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
		}
		else
		{
			bool Caps = false;
			std::string Message = ExtractCommand(Text, &Caps);
			if(!Message.empty())
			{
				if(Caps)
					Message = ToUpperCase(Message);
				QueueChat(Message);
				m_State = STATE_IDLE;
				return;
			}
			return;
		}
	}

	size_t TriggerEnd = FindTrigger(Text);
	if(TriggerEnd == std::string::npos)
		return;

	if(TriggerEnd < Text.length())
	{
		std::string After = Text.substr(TriggerEnd);
		bool Caps = false;
		std::string Message = ExtractCommand(After, &Caps);
		if(!Message.empty())
		{
			if(Caps)
				Message = ToUpperCase(Message);
			QueueChat(Message);
			return;
		}
	}

	m_State = STATE_WAITING_COMMAND;
	m_TriggerTime = Now;
}

void CVoiceAssistant::QueueChat(const std::string &Msg)
{
	std::lock_guard<std::mutex> Lock(m_Mutex);
	m_vPendingChat.push(Msg);
}
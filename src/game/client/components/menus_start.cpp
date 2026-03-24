/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "menus_start.h"

#include <base/system.h>

#include <engine/client/updater.h>
#include <engine/font_icons.h>
#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <generated/client_data.h>

#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/localization.h>
#include <game/version.h>

#if defined(CONF_PLATFORM_ANDROID)
#include <android/android_main.h>
#endif

void CMenusStart::RenderStartMenu(CUIRect MainView)
{
	GameClient()->m_MenuBackground.ChangePosition(CMenuBackground::POS_START);
	
	CUIRect FullScreen = *Ui()->Screen();

	Input()->StartTextInput();

	// Полностью чёрный фон
	Graphics()->TextureClear();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.0f, 0.0f, 0.0f, 1.0f);
	IGraphics::CQuadItem Quad(FullScreen.x, FullScreen.y, FullScreen.w, FullScreen.h);
	Graphics()->QuadsDrawTL(&Quad, 1);
	Graphics()->QuadsEnd();

	// Мигание курсора
	const bool ShowCursor = ((time_get() / (time_freq() / 2)) % 2) == 0;

	char aBuf[512];
	if(ShowCursor)
		str_format(aBuf, sizeof(aBuf), "%s_", GameClient()->m_Menus.GetTerminalInput());
	else
		str_format(aBuf, sizeof(aBuf), "%s", GameClient()->m_Menus.GetTerminalInput());

	// Рисуем введённый текст в самом верху
	TextRender()->TextColor(0.4f, 1.0f, 0.4f, 1.0f);
	TextRender()->Text(8.0f, 4.0f, 24.0f, aBuf, -1.0f);
}

bool CMenusStart::CheckHotKey(int Key) const
{
	return !Input()->ShiftIsPressed() && !Input()->ModifierIsPressed() && !Input()->AltIsPressed() && // no modifier
	       Input()->KeyPress(Key) &&
	       !GameClient()->m_GameConsole.IsActive();
}

#include "edgeinfo.h"

#include "engine/font_icons.h"
#include "game/localization.h"

#include <engine/graphics.h>
#include <engine/shared/config.h>

#include <generated/protocol.h>

#include <game/client/components/camera.h>
#include <game/client/components/chat.h>
#include <game/client/components/console.h>
#include <game/client/components/controls.h>
#include <game/client/components/emoticon.h>
#include <game/client/components/menus.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/ui.h>
struct SEdgeInfoProperties {
	static constexpr float ms_Padding = 3.0f;
	static constexpr float ms_Rounding = 3.0f;

	static constexpr float ms_ItemSpacing = 2.0f;

	static constexpr float ms_CubeSize = 24.0f;
	static constexpr float ms_ArrowsSize = 18.0f;
	static constexpr float ms_WallWidth = 3.0f;
	static constexpr float ms_CircleRadius = 8.0f;
	static constexpr float ms_CircleThickness = 2.0f;

	static constexpr float ms_HeadlineFontSize = 8.0f;

	static ColorRGBA WindowColor() { return ColorRGBA(0.451f, 0.451f, 0.451f, 0.9f); };
	static ColorRGBA WindowColorDark() { return ColorRGBA(0.2f, 0.2f, 0.2f, 0.9f); };
	static ColorRGBA WindowColorMedium() { return ColorRGBA(0.35f, 0.35f, 0.35f, 0.9f); };

	static ColorRGBA ActionActiveButtonColor() { return ColorRGBA(0.53f, 0.78f, 0.53f, 0.8f); };
	static ColorRGBA ActionAltActiveButtonColor() { return ColorRGBA(1.0f, 0.42f, 0.42f, 0.8f); };
	static ColorRGBA BlueSteelButtonColor() { return ColorRGBA(0.2f, 0.4f, 0.65f, 0.8f); };
	static ColorRGBA ActionWhiteButtonColor() { return ColorRGBA(1.0f, 1.0f, 1.0f, 0.8f); };
};

CEdgeInfo::CEdgeInfo() {
	OnReset();
}

void CEdgeInfo::OnConsoleInit() {
	Console()->Register("bc_edge_info_toggle", "", CFGFLAG_CLIENT, ConToggleEdgeInfo, this, "Toggle edge info");
}

void CEdgeInfo::ConToggleEdgeInfo(IConsole::IResult *pResult, void *pUserData) {
	g_Config.m_BcEdgeInfo ^= 1;
}

void CEdgeInfo::OnReset() {
	BCReset();
	g_Config.m_BcEdgeInfo = 0;
}

void CEdgeInfo::OnRelease() {
	BCReset();
	g_Config.m_BcEdgeInfo = 0;
}

void CEdgeInfo::OnRender() {
	if(g_Config.m_BcEdgeInfo == 0) return;

	RenderEdgeInfo();
}

void CEdgeInfo::RenderEdgeInfo() {
	CUIRect Base, EdgeInfo, JumpInfo;

	Base.h = 100.0f * 3.0f / (g_Config.m_BcEdgeInfoJump && g_Config.m_BcEdgeInfoCords ? 6 : 12);
	Base.w = 100.0f * 3.0f * Graphics()->ScreenAspect() / 5;
	Base.x = (100.0f * 3.0f * Graphics()->ScreenAspect() / 2 - Base.w / 2) *
		(g_Config.m_BcEdgeInfoPosX / 50.0f);
	Base.y = ((100.0f * 3.0f) / 2) * (g_Config.m_BcEdgeInfoPosY / 50.0f);

	vec2 ScreenTL, ScreenBR;
	Graphics()->GetScreen(&ScreenTL.x, &ScreenTL.y, &ScreenBR.x, &ScreenBR.y);

	if(Base.y + Base.h > ScreenBR.y) Base.y -= Base.y + Base.h - ScreenBR.y;
	if(Base.x + Base.w > ScreenBR.x) Base.x -= Base.x + Base.w - ScreenBR.x;

	m_Rect = Base;

	Base.Draw(SEdgeInfoProperties::WindowColorDark(), IGraphics::CORNER_ALL,
		SEdgeInfoProperties::ms_Rounding);
	Base.Margin(SEdgeInfoProperties::ms_Padding, &Base);
	const int ClientId = GameClient()->m_Snap.m_SpecInfo.m_Active ?
		GameClient()->m_Snap.m_SpecInfo.m_SpectatorId : GameClient()->m_Snap.m_LocalClientId;
	m_Pos_x = GetPositionEdgeInfo(ClientId, g_Config.m_ClDummy);

	if(g_Config.m_BcEdgeInfoCords && g_Config.m_BcEdgeInfoJump) Base.HSplitMid(&EdgeInfo, &JumpInfo);
	if(g_Config.m_BcEdgeInfoCords)
		RenderEdgeInfoEdgeInfo(g_Config.m_BcEdgeInfoJump ? &EdgeInfo : &Base);
	if(g_Config.m_BcEdgeInfoJump)
		RenderEdgeInfoJumpInfo(g_Config.m_BcEdgeInfoCords ? &JumpInfo : &Base);
}

float CEdgeInfo::GetPositionEdgeInfo(int ClientId, int Conn) {
	float ValuePos;
	if(ClientId == SPEC_FREEVIEW) ValuePos = GameClient()->m_Camera.m_Center.x / 32.0f;
	else if(GameClient()->m_aClients[ClientId].m_SpecCharPresent)
		ValuePos = GameClient()->m_aClients[ClientId].m_SpecChar.x / 32.0f;
	else {
		const CNetObj_Character *pPrevChar = &GameClient()->m_Snap.m_aCharacters[ClientId].m_Prev;
		const CNetObj_Character *pCurChar = &GameClient()->m_Snap.m_aCharacters[ClientId].m_Cur;
		const float IntraTick = Client()->IntraGameTick(Conn);
		ValuePos = mix(vec2(pPrevChar->m_X, pPrevChar->m_Y), vec2(pCurChar->m_X, pCurChar->m_Y),
			IntraTick).x / 32.0f;
	}

	ValuePos = std::round(ValuePos * 100.0f) / 100.0f;
	float temp = std::round(ValuePos * 100.0f);
	int result = static_cast<int>(temp) % 100;
	return result;
}

void CEdgeInfo::RenderEdgeInfoEdgeInfo(CUIRect *pBase) {
	CUIRect LeftZone, RightZone, CenterZone;
	pBase->HSplitBottom(SEdgeInfoProperties::ms_ItemSpacing, pBase, nullptr);
	float ActionSpacing = (pBase->w - (2 * (SEdgeInfoProperties::ms_WallWidth +
		SEdgeInfoProperties::ms_CircleRadius + SEdgeInfoProperties::ms_CubeSize))) / 4;
	pBase->VSplitLeft(SEdgeInfoProperties::ms_CubeSize + ActionSpacing, &LeftZone, &CenterZone);
	CenterZone.VSplitRight(SEdgeInfoProperties::ms_CubeSize + ActionSpacing, &CenterZone, &RightZone);
	LeftZone.VSplitRight(ActionSpacing + 2, &LeftZone, nullptr);
	RightZone.VSplitLeft(ActionSpacing + 2, nullptr, &RightZone);
	LeftZone.Margin(SEdgeInfoProperties::ms_ItemSpacing, &LeftZone);
	RightZone.Margin(SEdgeInfoProperties::ms_ItemSpacing, &RightZone);
	LeftZone.Draw(m_Pos_x >= 44 ? color_cast<ColorRGBA>(ColorHSLA(g_Config.m_BcEdgeInfoColorFreeze)) :
		m_Pos_x >= 28 ? color_cast<ColorRGBA>(ColorHSLA(g_Config.m_BcEdgeInfoColorSafe)) :
		color_cast<ColorRGBA>(ColorHSLA(g_Config.m_BcEdgeInfoColorKill)), IGraphics::CORNER_ALL,
		SEdgeInfoProperties::ms_Rounding);
	RightZone.Draw(m_Pos_x <= 53 ?
		color_cast<ColorRGBA>(ColorHSLA(g_Config.m_BcEdgeInfoColorFreeze)) : m_Pos_x <= 69 ?
		color_cast<ColorRGBA>(ColorHSLA(g_Config.m_BcEdgeInfoColorSafe)) :
		color_cast<ColorRGBA>(ColorHSLA(g_Config.m_BcEdgeInfoColorKill)), IGraphics::CORNER_ALL,
		SEdgeInfoProperties::ms_Rounding);
	CenterZone.VSplitLeft(SEdgeInfoProperties::ms_WallWidth + ActionSpacing, &LeftZone, &CenterZone);
	CenterZone.VSplitRight(SEdgeInfoProperties::ms_WallWidth + ActionSpacing, &CenterZone,
		&RightZone);
	LeftZone.VSplitRight(ActionSpacing - 3, &LeftZone, nullptr);
	LeftZone.VSplitLeft(3, nullptr, &LeftZone);
	RightZone.VSplitLeft(ActionSpacing - 3, nullptr, &RightZone);
	RightZone.VSplitRight(3, &RightZone, nullptr);
	LeftZone.Draw(m_Pos_x >= 44 && m_Pos_x < 53 ? SEdgeInfoProperties::ActionWhiteButtonColor() :
		SEdgeInfoProperties::WindowColorMedium(), IGraphics::CORNER_NONE, 0);
	RightZone.Draw(m_Pos_x <= 53 && m_Pos_x > 44 ? SEdgeInfoProperties::ActionWhiteButtonColor() :
		SEdgeInfoProperties::WindowColorMedium(), IGraphics::CORNER_NONE, 0);
	CenterZone.Margin(SEdgeInfoProperties::ms_ItemSpacing, &CenterZone);
	Graphics()->TextureClear();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(m_Pos_x > 44 && m_Pos_x < 53 ?
		SEdgeInfoProperties::ActionWhiteButtonColor() : SEdgeInfoProperties::WindowColorMedium());
	Graphics()->DrawCircle(CenterZone.x + CenterZone.w / 2, CenterZone.y + CenterZone.h / 2,
		SEdgeInfoProperties::ms_CircleRadius, 16);
	Graphics()->QuadsEnd();
	//
	if(m_Pos_x == 44 || m_Pos_x == 53) {
		Graphics()->TextureClear();
		Graphics()->QuadsBegin();
		Graphics()->SetColor(SEdgeInfoProperties::ActionWhiteButtonColor());

		const int Segments = 16;
		const float CenterX = CenterZone.x + CenterZone.w / 2.0f;
		const float CenterY = CenterZone.y + CenterZone.h / 2.0f;
		const float Radius = SEdgeInfoProperties::ms_CircleRadius;
		const float Thickness = SEdgeInfoProperties::ms_CircleThickness;

		for(int i = 0; i < Segments; ++i) {
			float a1 = 2.0f * pi * i / Segments;
			float a2 = 2.0f * pi * (i + 1) / Segments;
			IGraphics::CFreeformItem Quad(
				CenterX + std::cos(a1) * (Radius - Thickness / 2.0f),
				CenterY + std::sin(a1) * (Radius - Thickness / 2.0f),
				CenterX + std::cos(a2) * (Radius - Thickness / 2.0f),
				CenterY + std::sin(a2) * (Radius - Thickness / 2.0f),
				CenterX + std::cos(a1) * (Radius + Thickness / 2.0f),
				CenterY + std::sin(a1) * (Radius + Thickness / 2.0f),
				CenterX + std::cos(a2) * (Radius + Thickness / 2.0f),
				CenterY + std::sin(a2) * (Radius + Thickness / 2.0f));
			Graphics()->QuadsDrawFreeform(&Quad, 1);
		}
		Graphics()->QuadsEnd();
	}
}

void CEdgeInfo::RenderEdgeInfoJumpInfo(CUIRect *pBase) {
	CUIRect LeftZone, RightZone, CenterZone;
	pBase->HSplitTop(SEdgeInfoProperties::ms_ItemSpacing, nullptr, pBase);
	float ActionSpacing = (pBase->w - (2 * SEdgeInfoProperties::ms_ArrowsSize + 3 *
		SEdgeInfoProperties::ms_ArrowsSize)) / 4;
	pBase->VSplitLeft(SEdgeInfoProperties::ms_ArrowsSize + ActionSpacing, &LeftZone, &CenterZone);
	CenterZone.VSplitRight(SEdgeInfoProperties::ms_ArrowsSize + ActionSpacing, &CenterZone,
		&RightZone);
	LeftZone.VSplitRight(ActionSpacing, &LeftZone, nullptr);
	RightZone.VSplitLeft(ActionSpacing, nullptr, &RightZone);
	LeftZone.Margin(SEdgeInfoProperties::ms_ItemSpacing, &LeftZone);
	RightZone.Margin(SEdgeInfoProperties::ms_ItemSpacing, &RightZone);
	DoIconButton(&RightZone, FontIcon::BC_ANGLES_UP, SEdgeInfoProperties::ms_ArrowsSize,
		(m_Pos_x == 56 || m_Pos_x == 69 || m_Pos_x == 72 || m_Pos_x == 84) ?
		SEdgeInfoProperties::ActionWhiteButtonColor() : SEdgeInfoProperties::WindowColorMedium());
	if(m_Pos_x == 62 || m_Pos_x == 63 || m_Pos_x == 66 || m_Pos_x == 81) {
		RightZone.HSplitTop(5, nullptr, &RightZone);
		DoIconButton(&RightZone, FontIcon::BC_ANGLE_UP, SEdgeInfoProperties::ms_ArrowsSize,
		SEdgeInfoProperties::ActionWhiteButtonColor());
	}
	DoIconButton(&LeftZone, FontIcon::BC_ANGLES_UP, SEdgeInfoProperties::ms_ArrowsSize,
		(m_Pos_x == 13 || m_Pos_x == 25 || m_Pos_x == 28 || m_Pos_x == 41) ?
		SEdgeInfoProperties::ActionWhiteButtonColor() : SEdgeInfoProperties::WindowColorMedium());
	if(m_Pos_x == 16 || m_Pos_x == 31) {
		LeftZone.HSplitTop(5, nullptr, &LeftZone);
		DoIconButton(&LeftZone, FontIcon::BC_ANGLE_UP, SEdgeInfoProperties::ms_ArrowsSize,
			SEdgeInfoProperties::ActionWhiteButtonColor());
	}
	CenterZone.VSplitLeft(SEdgeInfoProperties::ms_ArrowsSize + ActionSpacing, &LeftZone, &CenterZone);
	CenterZone.VSplitRight(SEdgeInfoProperties::ms_ArrowsSize + ActionSpacing, &CenterZone,
		&RightZone);
	LeftZone.VSplitRight(ActionSpacing - 3, &LeftZone, nullptr);
	LeftZone.VSplitLeft(3, nullptr, &LeftZone);
	RightZone.VSplitLeft(ActionSpacing - 3, nullptr, &RightZone);
	RightZone.VSplitRight(3, &RightZone, nullptr);
	std::sort(values.begin(), values.end());

	int lower = std::numeric_limits<int>::min();
	int upper = std::numeric_limits<int>::max();

	for (int v : values) {
		if (v <= m_Pos_x) lower = v;
		if (v >= m_Pos_x) {
			upper = v;
			break;
		}
	}

	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "%02i", m_Pos_x);
	if(m_Pos_x == lower)
		TextRender()->TextColor(SEdgeInfoProperties::ActionActiveButtonColor());
	Ui()->DoLabel(&CenterZone, aBuf, 12, TEXTALIGN_MC);
	TextRender()->TextColor(TextRender()->DefaultTextColor());

	str_format(aBuf, sizeof(aBuf), "%s |", (lower == std::numeric_limits<int>::min()) ? "-" :
		std::to_string(lower).c_str());
	if(m_Pos_x == lower || m_Pos_x == upper)
		TextRender()->TextColor(SEdgeInfoProperties::ActionActiveButtonColor());
	Ui()->DoLabel(&LeftZone, aBuf, 12, TEXTALIGN_MC);
	TextRender()->TextColor(TextRender()->DefaultTextColor());

	str_format(aBuf, sizeof(aBuf), "| %s", (upper == std::numeric_limits<int>::max()) ? "-" :
		std::to_string(upper).c_str());
	if(m_Pos_x == upper)
		TextRender()->TextColor(SEdgeInfoProperties::ActionActiveButtonColor());
	Ui()->DoLabel(&RightZone, aBuf, 12, TEXTALIGN_MC);
	TextRender()->TextColor(TextRender()->DefaultTextColor());
}

void CEdgeInfo::DoIconButton(CUIRect *pRect, const char *pIcon, float TextSize,
	ColorRGBA IconColor) const {

	TextRender()->SetFontPreset(EFontPreset::ICON_FONT);
	TextRender()->SetRenderFlags(ETextRenderFlags::TEXT_RENDER_FLAG_ONLY_ADVANCE_WIDTH |
		ETextRenderFlags::TEXT_RENDER_FLAG_NO_X_BEARING |
		ETextRenderFlags::TEXT_RENDER_FLAG_NO_Y_BEARING |
		ETextRenderFlags::TEXT_RENDER_FLAG_NO_OVERSIZE);
	TextRender()->TextColor(IconColor);
	Ui()->DoLabel(pRect, pIcon, TextSize, TEXTALIGN_MC);
	TextRender()->TextColor(TextRender()->DefaultTextColor());
	TextRender()->SetRenderFlags(0);
	TextRender()->SetFontPreset(EFontPreset::DEFAULT_FONT);
}

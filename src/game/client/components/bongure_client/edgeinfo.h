#ifndef GAME_CLIENT_COMPONENTS_BONGURE_CLIENT_EDGEINFO_H
#define GAME_CLIENT_COMPONENTS_BONGURE_CLIENT_EDGEINFO_H

#include "game/client/ui_rect.h"

#include <engine/console.h>

#include <game/client/component.h>
class CEdgeInfo : public CComponent {
	int m_Pos_x;
	void DoIconButton(CUIRect *pRect, const char *pIcon, float TextSize, ColorRGBA IconColor) const;
	static void ConToggleEdgeInfo(IConsole::IResult *pResult, void *pUserData);

	std::vector<int> values = {41, 31, 28, 25, 16, 13, 56, 62, 63, 66, 69, 72, 81, 84};

	CUIRect m_Rect;

	void BCReset() {}

	float GetPositionEdgeInfo(int ClientId, int Conn);

	void RenderEdgeInfo();
	void RenderEdgeInfoEdgeInfo(CUIRect *pBase);
	void RenderEdgeInfoJumpInfo(CUIRect *pBase);

public:
	CEdgeInfo();
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnRender() override;
	void OnConsoleInit() override;
	void OnRelease() override;
};

#endif

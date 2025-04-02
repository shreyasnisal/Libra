#pragma once

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/Camera.hpp"

#include "Game/Game.hpp"


class App
{
public:
						App							();
						~App						();
	void				Startup						();
	void				Shutdown					();
	void				Run							();
	void				RunFrame					();

	bool				IsQuitting					() const		{ return m_isQuitting; }
	bool				HandleQuitRequested			();

	static bool			HandleQuitEvent				(EventArgs& args);

public:
	Game*				m_game;

private:
	void				LoadGameConfigXml			();
	void				BeginFrame					();
	void				Update						(float deltaSeconds);
	void				Render						() const;
	void				RenderDebug					() const;
	void				EndFrame					();

private:
	bool				m_isQuitting				= false;
	double				m_previousFrameTime			= 0;
	int					m_framePerSecond			= 0;
};

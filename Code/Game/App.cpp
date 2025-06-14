#include "Game/App.hpp"

#include "Game/GameCommon.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Window.hpp"


App* g_app = nullptr;
AudioSystem* g_audio = nullptr;
RandomNumberGenerator* g_RNG = nullptr;
Renderer* g_renderer = nullptr;
Window* g_window = nullptr;
Game* g_game = nullptr;


App::App()
{
	
}

App::~App()
{
	
}

void App::Startup()
{
	LoadGameConfigXml();

	InputConfig inputConfig;
	g_input = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_input;
	windowConfig.m_windowTitle = g_gameConfigBlackboard.GetValue("gameTitle", "Unnamed Application");
	windowConfig.m_clientAspect = 2.f;
	g_window = new Window(windowConfig);

	RenderConfig renderConfig;
	renderConfig.m_window = g_window;
	g_renderer = new Renderer(renderConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_renderer = g_renderer;
	devConsoleConfig.m_consoleFontFilePathWithNoExtension = "Data/Images/SquirrelFixedFont";
	g_console = new DevConsole(devConsoleConfig);

	EventSystemConfig eventSystemConfig;
	g_eventSystem = new EventSystem(eventSystemConfig);

	AudioConfig audioConfig;
	g_audio = new AudioSystem(audioConfig);

	g_input->Startup();
	g_window->Startup();
	g_renderer->Startup();
	g_console->Startup();
	g_audio->Startup();

	g_game = new Game();

	SubscribeEventCallbackFunction("Quit", App::HandleQuitEvent);
}

void App::LoadGameConfigXml()
{
	g_gameConfigBlackboard = NamedProperties();
	XmlDocument gameConfigXmlFile("Data/GameConfig.xml");
	XmlResult loadResult = gameConfigXmlFile.LoadFile("Data/GameConfig.xml");
	if (loadResult == XmlResult::XML_SUCCESS)
	{
		DebuggerPrintf("GameConfig.xml loaded successfully\n");
		XmlElement* gameConfigRootElement = gameConfigXmlFile.RootElement();
		g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*gameConfigRootElement);

		g_screenSizeX = g_gameConfigBlackboard.GetValue("screenSizeX", g_screenSizeX);
		g_screenSizeY = g_gameConfigBlackboard.GetValue("screenSizeY", g_screenSizeY);
	}
	else
	{
		ERROR_RECOVERABLE("Could not load Data/GameConfig.xml");
	}
}

void App::Run()
{
	while (!IsQuitting())
	{
		RunFrame();
	}
}

void App::RunFrame()
{
	static int frameCounter = 0;
	double timeNow = GetCurrentTimeSeconds();
	float deltaSeconds = GetClamped(static_cast<float>(timeNow - m_previousFrameTime), 0, 0.1f);
	m_previousFrameTime = timeNow;
	if (frameCounter % 60 == 0)
	{
		m_framePerSecond = RoundDownToInt(1.f / deltaSeconds);
	}
	frameCounter++;

	BeginFrame();
	Update(deltaSeconds);
	Render();
	EndFrame();

}

bool App::HandleQuitRequested()
{
	m_isQuitting = true;

	return true;
}

void App::BeginFrame()
{
	g_input->BeginFrame();
	g_window->BeginFrame();
	g_renderer->BeginFrame();
	g_console->BeginFrame();
	g_eventSystem->BeginFrame();
	g_audio->BeginFrame();
}

void App::Update(float deltaSeconds)
{
	g_game->Update(deltaSeconds);

	if (g_input->WasKeyJustPressed(KEYCODE_F8))
	{
		g_audio->StopSound(g_game->m_attractMusicPlayback);
		g_audio->StopSound(g_game->m_gameplayMusicPlayback);

		delete g_game;
		g_game = new Game();
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F11))
	{
		LoadGameConfigXml();
	}
	if (g_input->WasKeyJustPressed(KEYCODE_TILDE))
	{
		DebuggerPrintf("Console key pressed\n");
		g_console->ToggleMode(DevConsoleMode::OPENFULL);
	}
}

void App::Render() const
{
	g_renderer->ClearScreen(Rgba8(0, 0, 0, 255));

	// Globally set renderer state to prevent back culling and depth
	g_renderer->SetModelConstants();
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->SetBlendMode(BlendMode::ALPHA);

	g_game->Render();

	if (g_game->m_drawDebug)
	{
		RenderDebug();
	}

	AABB2 screenBounds(Vec2::ZERO, Vec2(g_screenSizeX, g_screenSizeY));
	g_console->Render(screenBounds);
}

void App::RenderDebug() const
{
	std::vector<Vertex_PCU> debugDrawVerts;
	AddVertsForTextTriangles2D(debugDrawVerts, Stringf("FPS: %d", m_framePerSecond), g_game->m_screenCamera.GetOrthoTopRight() - Vec2(150.f, 50.f), 25.f, Rgba8::RED);

	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(debugDrawVerts);
}

void App::EndFrame()
{
	g_audio->EndFrame();
	g_eventSystem->EndFrame();
	g_console->EndFrame();
	g_renderer->EndFrame();
	g_window->EndFrame();
	g_input->EndFrame();
}

void App::Shutdown()
{
	g_audio->Shutdown();
	g_eventSystem->Shutdown();
	g_console->Shutdown();
	g_renderer->Shutdown();
	g_window->Shutdown();
	g_input->Shutdown();

	delete g_renderer;
	g_renderer = nullptr;

	delete g_input;
	g_input = nullptr;

	delete g_audio;
	g_audio = nullptr;

	delete g_RNG;
	g_RNG = nullptr;

	delete g_game;
	g_game = nullptr;
}


bool App::HandleQuitEvent(EventArgs&)
{
	g_app->HandleQuitRequested();

	return true;
}

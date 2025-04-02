#pragma once

#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"

#include "Game/GameCommon.hpp"


class		App;
class		Entity;
class		Map;
class		Texture;

enum GameState
{
	GAME_STATE_INVALID = -1,

	GAME_STATE_ATTRACT,
	GAME_STATE_PLAYING,
	GAME_STATE_PAUSED,
	GAME_STATE_GAMEOVER,
	GAME_STATE_VICTORY,
	GAME_STATE_MAP_SELECT,

	GAME_STATE_NUM
};

class Game
{
public:
	~Game				() = default;
	Game				();

	void				Update												(float deltaSeconds);
	void				Render												() const;

	bool				IsEntityAlive										(Entity* entity) const;
	void				StartGame											();
	void				QuitToAttractScreen									();

	void				SwitchToPreviousMap									();
	void				AdvanceToNextMap									();

	void				PlaySound											(int soundIndex, Vec2 const& sourcePosition = Vec2(-1.f, -1.f));

public:	
	GameState			m_gameState											= GAME_STATE_ATTRACT;
	bool				m_pauseAfterUpdate									= false;
	bool				m_drawDebug											= false;
	unsigned char		m_attractScreenRingOpacity							= 255;
	bool				m_isAttractScreenRingOpacityIncreasing				= false;
	bool				m_isNoClipActive									= false;
	bool				m_isPlayerInvincible								= false;

	Camera				m_worldCamera;
	Camera				m_screenCamera;

	int					m_numTilesInViewVertically							= 8;
	Map*				m_currentMap										= nullptr;

	SoundPlaybackID		m_gameplayMusicPlayback								= 0;
	SoundPlaybackID		m_attractMusicPlayback								= 0;

private:

	void				UpdateAttractScreen									(float deltaSeconds);
	void				UpdatePlaying										(float deltaSeconds);
	void				UpdatePaused										(float deltaSeconds);
	void				UpdateGameover										(float deltaSeconds);
	void				UpdateVictory										(float deltaSeconds);

	void				UpdateMapSelect										(float deltaSeconds);

	void				HandleDeveloperCheats								(float& deltaSeconds);

	void				UpdateCameras										(float deltaSeconds);

	void				RenderAttractScreen									() const;
	void				RenderPlaying										() const;
	void				RenderPaused										() const;
	void				RenderGameover										() const;
	void				RenderVictory										() const;

	void				RenderMapSelect										() const;

	void				RenderHUD											() const;
	void				RenderPausedOverlay									() const;
	void				RenderGameoverOverlay								() const;
	void				RenderDebug											() const;
	void				RenderTransitionOverlay								() const;

	void				LoadAssets											();
	void				LoadSounds											();
	void				LoadTextures										();
	void				LoadSpriteSheets									();
	void				LoadAnimations										();

	void				SetZoomedOutNumTilesInViewVertically				();

	void				PlayIntroTransition									(float deltaSecods);
	void				PlayOutroTransition									(float deltaSeconds);

	void				SwitchMapOrGameState								();
	void				SwitchMap											();
	void				SwitchGameState										();

	void				GoToMapSelect										();
	void				AddMapToGame										();
	void				RemoveMapFromGame									();
	void				SaveSelectedMaps									();

private:
	static constexpr float	BACKGROUND_MUSIC_VOLUME							= 0.2f;

	std::vector<Map*>	m_allMaps;
	int					m_currentMapIndex									= 0;

	float				m_transitionSecondsRemaining						= 1.f;
	Map*				m_nextMap											= nullptr;
	int					m_nextMapIndex										= 0;
	GameState			m_nextGameState										= GAME_STATE_INVALID;

	Rgba8				m_transitionOverlayColor							= Rgba8::TRANSPARENT_BLACK;
	bool				m_shouldPlayOutroTransition							= false;
	bool				m_shouldPlayIntroTransition							= false;

	Strings				m_mapOptions;
	Strings				m_selectedMaps;
	int					m_selectedMapIndexFromOptions						= 0;
	int					m_selectedAddedMapIndex								= -1;

	float				m_timeSinceStart									= 0.f;

	float				m_secondsSinceSoundWasLastPlayed[SOUNDS_NUM]		= {0.f};
};
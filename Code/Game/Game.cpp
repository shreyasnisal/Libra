#include "Game/Game.hpp"

#include "Game/App.hpp"
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Bullet.hpp"

#include "Engine/Renderer/SpriteAnimDefinition.hpp"

Game::Game()
{
	LoadAssets();
	TileDefinition::InitializeTileDefitions();
	MapDefinition::InitializeMapDefinitions();
	m_attractMusicPlayback = g_audio->StartSound(g_sounds[ATTRACT_MUSIC], true, BACKGROUND_MUSIC_VOLUME);
}

void Game::LoadAssets()
{	
	g_bitmapFonts[SQUIRREL_BITMAP_FONT] = g_renderer->CreateOrGetBitmapFont("Data/Images/SquirrelFixedFont");

	LoadSounds();
	LoadTextures();
	LoadSpriteSheets();
	LoadAnimations();
}

void Game::LoadSounds()
{
	g_sounds[CLICK_SOUND] = g_audio->CreateOrGetSound("Data/Audio/Click.mp3");
	g_sounds[VICTORY_SOUND] = g_audio->CreateOrGetSound("Data/Audio/Victory.mp3");
	g_sounds[PAUSE_SOUND] = g_audio->CreateOrGetSound("Data/Audio/Pause.mp3");
	g_sounds[UNPAUSE_SOUND] = g_audio->CreateOrGetSound("Data/Audio/Unpause.mp3");
	g_sounds[WELCOME_SOUND] = g_audio->CreateOrGetSound("Data/Audio/Welcome.mp3");
	g_sounds[MAP_EXIT_SOUND] = g_audio->CreateOrGetSound("Data/Audio/ExitMap.wav");
	g_sounds[ATTRACT_MUSIC] = g_audio->CreateOrGetSound("Data/Audio/AttractMusic.mp3");
	g_sounds[GAMEPLAY_MUSIC] = g_audio->CreateOrGetSound("Data/Audio/GameplayMusic.mp3");

	g_sounds[PLAYER_SHOOT_SOUND] = g_audio->CreateOrGetSound("Data/Audio/PlayerShootNormal.ogg");
	g_sounds[PLAYER_HIT_SOUND] = g_audio->CreateOrGetSound("Data/Audio/PlayerHit.wav");
	g_sounds[GAMEOVER_SOUND] = g_audio->CreateOrGetSound("Data/Audio/GameOver.mp3");

	g_sounds[ENEMY_HIT_SOUND] = g_audio->CreateOrGetSound("Data/Audio/EnemyHit.wav");
	g_sounds[ENTITY_DIE_SOUND] = g_audio->CreateOrGetSound("Data/Audio/EnemyDied.wav");

	g_sounds[BULLET_BOUNCE_SOUND] = g_audio->CreateOrGetSound("Data/Audio/BulletBounce.wav");
	g_sounds[BULLET_RICOTHET_SOUND] = g_audio->CreateOrGetSound("Data/Audio/BulletRicochet2.wav");

	g_sounds[ENEMY_SHOOT_SOUND] = g_audio->CreateOrGetSound("Data/Audio/EnemyShoot.wav");
	g_sounds[DISCOVERY_SOUND] = g_audio->CreateOrGetSound("Data/Audio/ISeeYou.ogg");

	for (int soundIndex = 0; soundIndex < SOUNDS_NUM; soundIndex++)
	{
		if (g_sounds[soundIndex] == 9999)
		{
			ERROR_RECOVERABLE(Stringf("Could not load sound #%d", soundIndex));
		}
	}
}

void Game::LoadTextures()
{
	g_textures[ATTRACT_BACKGROUND_TEXTURE] = g_renderer->CreateOrGetTextureFromFile("Data/Images/AttractScreen.png");
	g_textures[GAMEOVER_TEXTURE] = g_renderer->CreateOrGetTextureFromFile("Data/Images/YouDiedScreen.png");
	g_textures[VICTORY_TEXTURE] = g_renderer->CreateOrGetTextureFromFile("Data/Images/VictoryScreen.jpg");
	g_textures[PLAYERTANK_BASE_TEXTURE] = g_renderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankBase.png");
	g_textures[PLAYERTANK_TURRET_TEXTURE] = g_renderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankTop.png");
	g_textures[ARIES_BASE_TEXTURE] = g_renderer->CreateOrGetTextureFromFile("Data/Images/EnemyAries.png");
	g_textures[CAPRICORN_BASE_TEXTURE] = g_renderer->CreateOrGetTextureFromFile("Data/Images/EnemyTank3.png");
	g_textures[LEO_BASE_TEXTURE] = g_renderer->CreateOrGetTextureFromFile("Data/Images/EnemyTank4.png");
	g_textures[SCORPIO_BASE_TEXTURE] = g_renderer->CreateOrGetTextureFromFile("Data/Images/EnemyTurretBase.png");
	g_textures[SCORPIO_TURRET_TEXTURE] = g_renderer->CreateOrGetTextureFromFile("Data/Images/EnemyCannon.png");
	g_textures[GOOD_BOLT_TEXTURE] = g_renderer->CreateOrGetTextureFromFile("Data/Images/FriendlyBolt.png");
	g_textures[EVIL_BOLT_TEXTURE] = g_renderer->CreateOrGetTextureFromFile("Data/Images/EnemyBolt.png");
	g_textures[GOOD_BULLET_TEXTURE] = g_renderer->CreateOrGetTextureFromFile("Data/Images/FriendlyBullet.png");
	g_textures[EVIL_BULLET_TEXTURE] = g_renderer->CreateOrGetTextureFromFile("Data/Images/EnemyBullet.png");

	for (int textureIndex = 0; textureIndex < TEXTURES_NUM; textureIndex++)
	{
		if (g_textures[textureIndex] == nullptr)
		{
			ERROR_RECOVERABLE(Stringf("Could not load texture #%d", textureIndex));
		}
	}
}

void Game::LoadSpriteSheets()
{
	Texture* terrainTilesTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	g_terrainSpriteSheet = new SpriteSheet(terrainTilesTexture, IntVec2(8, 8));

	Texture* explosionTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Explosion_5x5.png");
	g_spriteSheets[EXPLOSION_SPRITESHEET] = new SpriteSheet(explosionTexture, IntVec2(5, 5));
}

void Game::LoadAnimations()
{
	Bullet::CreateAnimations();
}

void Game::Update(float deltaSeconds)
{
	if (m_shouldPlayOutroTransition)
	{
		PlayOutroTransition(deltaSeconds);
	}
	else if (m_shouldPlayIntroTransition)
	{
		PlayIntroTransition(deltaSeconds);
	}

	switch (m_gameState)
	{
		case GAME_STATE_ATTRACT:			UpdateAttractScreen(deltaSeconds);			break;
		case GAME_STATE_PLAYING:			UpdatePlaying(deltaSeconds);				break;
		case GAME_STATE_PAUSED:				UpdatePaused(deltaSeconds);					break;
		case GAME_STATE_VICTORY:			UpdateVictory(deltaSeconds);				break;
		case GAME_STATE_GAMEOVER:			UpdateGameover(deltaSeconds);				break;
		case GAME_STATE_MAP_SELECT:			UpdateMapSelect(deltaSeconds);				break;
	}
}

void Game::UpdatePlaying(float deltaSeconds)
{
	HandleDeveloperCheats(deltaSeconds);

	if (m_pauseAfterUpdate)
	{
		m_gameState = GAME_STATE_PLAYING;
	}

	for (int soundIndex = 0; soundIndex < SOUNDS_NUM; soundIndex++)
	{
		m_secondsSinceSoundWasLastPlayed[soundIndex] += deltaSeconds;
	}

	if (m_currentMap)
	{
		m_currentMap->Update(deltaSeconds);
	}

	UpdateCameras(deltaSeconds);

	if (m_pauseAfterUpdate)
	{
		m_pauseAfterUpdate = false;
		m_gameState = GAME_STATE_PAUSED;
	}
}

void Game::UpdatePaused(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	XboxController controller = g_input->GetController(0);

	if (g_input->WasKeyJustPressed(KEYCODE_SPACE) || controller.WasButtonJustPressed(XBOX_BUTTON_START))
	{
		m_attractMusicPlayback = g_audio->StartSound(g_sounds[ATTRACT_MUSIC]);
		m_gameState = GAME_STATE_PLAYING;
	}
	if (g_input->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
	{
		QuitToAttractScreen();
	}
	if (g_input->WasKeyJustPressed('P') || controller.WasButtonJustPressed(XBOX_BUTTON_START))
	{
		PlaySound(UNPAUSE_SOUND);
		m_gameState = GAME_STATE_PLAYING;
		float musicSpeed = 1.f;
		if (g_input->IsKeyDown('Y'))
		{
			musicSpeed *= 2.f;
		}
		if (g_input->IsKeyDown('T'))
		{
			musicSpeed *= 0.5f;
		}
		g_audio->SetSoundPlaybackSpeed(m_gameplayMusicPlayback, musicSpeed);
	}

	if (g_input->WasKeyJustPressed(KEYCODE_F1))
	{
		m_drawDebug = !m_drawDebug;
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F2))
	{
		m_isPlayerInvincible = !m_isPlayerInvincible;
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F3))
	{
		m_isNoClipActive = !m_isNoClipActive;
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F4))
	{
		if (m_numTilesInViewVertically == 8)
		{
			SetZoomedOutNumTilesInViewVertically();
		}
		else
		{
			m_numTilesInViewVertically = 8;
		}
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F5))
	{
		SwitchToPreviousMap();
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F6))
	{
		m_currentMap->GoToNextHeatMap();
	}
	if (g_input->WasKeyJustPressed(KEYCODE_RIGHTARROW))
	{
		m_currentMap->GoToNextEntityForHeatMapDebugDraw();
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F9))
	{
		AdvanceToNextMap();
	}
}

void Game::UpdateGameover(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	XboxController controller = g_input->GetController(0);

	if (g_input->WasKeyJustPressed('N'))
	{
		for (int entityIndex = 0; entityIndex < static_cast<int>(m_currentMap->m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER].size()); entityIndex++)
		{
			Entity*& player = m_currentMap->m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][entityIndex];
			player->m_isDead = false;
			player->m_health = g_gameConfigBlackboard.GetValue("playerMaxHealth", player->m_maxHealth);

			m_gameState = GAME_STATE_PLAYING;
		}
	}
	if (g_input->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
	{
		QuitToAttractScreen();
	}
}

void Game::UpdateVictory(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	XboxController controller = g_input->GetController(0);

	if (g_input->WasKeyJustPressed(KEYCODE_SPACE) || g_input->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XBOX_BUTTON_START) || controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
	{
		QuitToAttractScreen();
	}
}

void Game::HandleDeveloperCheats(float& deltaSeconds)
{
	XboxController xboxController = g_input->GetController(0);
	m_pauseAfterUpdate = g_input->IsKeyDown('O');

	if (g_input->IsKeyDown('T'))
	{
		g_audio->SetSoundPlaybackSpeed(m_gameplayMusicPlayback, 0.5f);
		deltaSeconds *= 0.1f;
	}

	if (g_input->WasKeyJustPressed('Y'))
	{
		g_audio->SetSoundPlaybackSpeed(m_gameplayMusicPlayback, 2.f);
	}
	if (g_input->IsKeyDown('Y'))
	{
		deltaSeconds *= 4.f;
	}
	if (g_input->WasKeyJustReleased('Y'))
	{
		g_audio->SetSoundPlaybackSpeed(m_gameplayMusicPlayback, 1.f);
	}

	if (g_input->WasKeyJustPressed('P') || xboxController.WasButtonJustPressed(XBOX_BUTTON_START))
	{
		PlaySound(PAUSE_SOUND);
		m_gameState = GAME_STATE_PAUSED;
		g_audio->SetSoundPlaybackSpeed(m_gameplayMusicPlayback, 0.f);
	}

	if (g_input->WasKeyJustPressed(KEYCODE_ESC) || xboxController.WasButtonJustPressed(XBOX_BUTTON_BACK))
	{
		PlaySound(PAUSE_SOUND);
		m_gameState = GAME_STATE_PAUSED;
		g_audio->SetSoundPlaybackSpeed(m_gameplayMusicPlayback, 0.f);
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F1))
	{
		m_drawDebug = !m_drawDebug;
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F2))
	{
		m_isPlayerInvincible = !m_isPlayerInvincible;
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F3))
	{
		m_isNoClipActive = !m_isNoClipActive;
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F4))
	{
		if (m_numTilesInViewVertically == 8)
		{
			SetZoomedOutNumTilesInViewVertically();
		}
		else
		{
			m_numTilesInViewVertically = 8;
		}
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F5))
	{
		SwitchToPreviousMap();
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F6))
	{
		m_currentMap->GoToNextHeatMap();
	}
	if (g_input->WasKeyJustPressed(KEYCODE_RIGHTARROW))
	{
		m_currentMap->GoToNextEntityForHeatMapDebugDraw();
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F9))
	{
		AdvanceToNextMap();
	}
}

void Game::PlayIntroTransition(float deltaSeconds)
{
	m_transitionOverlayColor = Interpolate(Rgba8::TRANSPARENT_BLACK, Rgba8::BLACK, m_transitionSecondsRemaining);

	EntityList const& players = m_currentMap->m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER];
	Player* player = nullptr;
	for (int playerIndex = 0; playerIndex < static_cast<int>(players.size()); playerIndex++)
	{
		player = dynamic_cast<Player*>(players[playerIndex]);
		player->m_orientationDegrees -= 7200.f * m_transitionSecondsRemaining * m_transitionSecondsRemaining * deltaSeconds;
		player->m_turretOrientationDegrees -= 7200.f * m_transitionSecondsRemaining * m_transitionSecondsRemaining * deltaSeconds;
		player->m_scale = 1.f - m_transitionSecondsRemaining;
	}

	m_transitionSecondsRemaining -= deltaSeconds;

	if (m_transitionSecondsRemaining <= 0)
	{
		if (player)
		{
			player->m_orientationDegrees = 45.f;
			player->m_turretOrientationDegrees = 45.f;
		}
		m_transitionSecondsRemaining = 1.f;
		m_shouldPlayIntroTransition = false;
	}
}

void Game::PlayOutroTransition(float deltaSeconds)
{
	m_transitionOverlayColor = Interpolate(Rgba8::BLACK, Rgba8::TRANSPARENT_BLACK, m_transitionSecondsRemaining);

	EntityList const& players = m_currentMap->m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER];
	for (int playerIndex = 0; playerIndex < static_cast<int>(players.size()); playerIndex++)
	{
		Player* player = dynamic_cast<Player*>(players[playerIndex]);
		player->m_orientationDegrees += 7200.f * (1.f - m_transitionSecondsRemaining) * (1.f - m_transitionSecondsRemaining) * deltaSeconds;
		player->m_turretOrientationDegrees += 7200.f * (1.f - m_transitionSecondsRemaining) * (1.f - m_transitionSecondsRemaining) * deltaSeconds;
		player->m_scale = m_transitionSecondsRemaining;
		IntVec2 playerTile = IntVec2(RoundDownToInt(player->m_position.x), RoundDownToInt(player->m_position.y));
		Vec2 playerTileCenter = Vec2(static_cast<float>(playerTile.x) + 0.5f, static_cast<float>(playerTile.y) + 0.5f);
		player->m_position.x = Interpolate(player->m_position.x, playerTileCenter.x, 1.f - m_transitionSecondsRemaining);
		player->m_position.y = Interpolate(player->m_position.y, playerTileCenter.y, 1.f - m_transitionSecondsRemaining);
	}

	m_transitionSecondsRemaining -= deltaSeconds;

	if (m_transitionSecondsRemaining <= 0)
	{
		SwitchMapOrGameState();
		m_shouldPlayOutroTransition = false;
		m_transitionSecondsRemaining = 1.f;
		m_shouldPlayIntroTransition = true;
	}
}

void Game::SwitchMapOrGameState()
{
	if (m_nextMap)
	{
		SwitchMap();
	}
	else if (m_nextGameState != GAME_STATE_INVALID)
	{
		SwitchGameState();
	}
}

void Game::SwitchMap()
{
	EntityList const& players = m_currentMap->m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER];
	for (int playerIndex = 0; playerIndex < static_cast<int>(players.size()); playerIndex++)
	{
		Player* player = dynamic_cast<Player*>(players[playerIndex]);
		m_currentMap->RemoveEntityFromMap(player);
		m_nextMap->AddEntityToMap(player);

		player->m_position = Vec2(1.5f, 1.5f);
		player->m_orientationDegrees = 45.f;
		player->m_turretOrientationDegrees = 45.f;
	}

	m_currentMap = m_nextMap;
	m_nextMap = nullptr;
	m_currentMapIndex = m_nextMapIndex;
	m_nextMapIndex = -1;

	if (m_numTilesInViewVertically != 8)
	{
		SetZoomedOutNumTilesInViewVertically();
	}
}

void Game::SwitchGameState()
{
	m_gameState = m_nextGameState;

	switch (m_gameState)
	{
		case GAME_STATE_VICTORY:
		{
			PlaySound(VICTORY_SOUND);
		}
	}
}

void Game::SwitchToPreviousMap()
{
	if (m_currentMapIndex > 0)
	{
		Map* newMap = m_allMaps[m_currentMapIndex - 1];
		m_nextMap = newMap;
		m_nextMapIndex = m_currentMapIndex - 1;
		m_shouldPlayOutroTransition = true;
	}
}

void Game::AdvanceToNextMap()
{
	if (m_currentMapIndex == static_cast<int>(m_allMaps.size() - 1))
	{
		m_nextGameState = GAME_STATE_VICTORY;
		m_shouldPlayOutroTransition = true;
	}

	if (m_currentMapIndex != static_cast<int>(m_allMaps.size() - 1))
	{
		Map* newMap = m_allMaps[m_currentMapIndex + 1];
		m_nextMap = newMap;
		m_nextMapIndex = m_currentMapIndex + 1;
		m_shouldPlayOutroTransition = true;
	}
}

void Game::SetZoomedOutNumTilesInViewVertically()
{
	if (m_currentMap->GetDimensions().x > 2 * m_currentMap->GetDimensions().y)
	{
		m_numTilesInViewVertically =  m_currentMap->GetDimensions().x / RoundDownToInt(g_window->GetAspect());
	}
	else
	{
		m_numTilesInViewVertically = m_currentMap->GetDimensions().y;
	}
}

void Game::UpdateCameras(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	m_worldCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(g_window->GetAspect() * static_cast<float>(m_numTilesInViewVertically), static_cast<float>(m_numTilesInViewVertically)));
	m_screenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(g_screenSizeX, g_screenSizeY));

	if (!m_currentMap)
	{
		return;
	}

	EntityList& players = m_currentMap->m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER];
	if (players.size() == 0)
	{
		return;
	}

	Entity*& player = players[0];

	if (player)
	{
		float cameraWidth = m_worldCamera.GetOrthoTopRight().x - m_worldCamera.GetOrthoBottomLeft().x;
		float cameraHeight = m_worldCamera.GetOrthoTopRight().y - m_worldCamera.GetOrthoBottomLeft().y;
		float cameraTranslationX = player->m_position.x - cameraWidth * 0.5f;
		float cameraTranslationY = player->m_position.y - cameraHeight * 0.5f;
		cameraTranslationX = GetClamped(cameraTranslationX, 0.f, m_currentMap->GetDimensions().x - cameraWidth);
		cameraTranslationY = GetClamped(cameraTranslationY, 0.f, m_currentMap->GetDimensions().y - cameraHeight);
		m_worldCamera.Translate2D(Vec2(cameraTranslationX, cameraTranslationY));
	}
}

void Game::Render() const
{
	switch (m_gameState)
	{
		case GAME_STATE_ATTRACT:			RenderAttractScreen();			break;
		case GAME_STATE_PLAYING:			RenderPlaying();				break;
		case GAME_STATE_PAUSED:				RenderPaused();					break;
		case GAME_STATE_VICTORY:			RenderVictory();				break;
		case GAME_STATE_GAMEOVER:			RenderGameover();				break;
		case GAME_STATE_MAP_SELECT:			RenderMapSelect();				break;
	}
}

void Game::RenderAttractScreen() const
{
	g_renderer->BeginCamera(m_screenCamera);
	std::vector<Vertex_PCU> attractScreenVertexes;
	AddVertsForAABB2(attractScreenVertexes, AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), Rgba8(255, 255, 255, 255));
	g_renderer->BindTexture(g_textures[ATTRACT_BACKGROUND_TEXTURE]);
	g_renderer->DrawVertexArray(attractScreenVertexes);

	//// Testing TextInBox
	//std::vector<Vertex_PCU> textInBoxTestVerts;
	//AABB2 textInBoxTestBox(Vec2(100.f, 200.f), Vec2(400.f, 400.f));
	//AddVertsForAABB2(textInBoxTestVerts, textInBoxTestBox, Rgba8(255, 255, 255, 127));
	//g_theRenderer->BindTexture(nullptr);
	//g_theRenderer->DrawVertexArray(textInBoxTestVerts);

	//std::vector<Vertex_PCU> testTextVerts;
	//g_bitmapFonts[SQUIRREL_BITMAP_FONT]->AddVertsForTextInBox2D(testTextVerts, textInBoxTestBox, 20.f, "Hello, world!\nThis is a TextInBox test.", Rgba8::WHITE, 1.f, Vec2(0.5f, 1.f), TextBoxMode::SHRINK_TO_FIT);
	//g_theRenderer->BindTexture(&(g_bitmapFonts[SQUIRREL_BITMAP_FONT]->GetTexture()));
	//g_theRenderer->DrawVertexArray(testTextVerts);
	
	// Testing animations
	//Texture* testAnimTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	//SpriteSheet testAnimSpriteSheet = SpriteSheet(*testAnimTexture, IntVec2(8, 8));
	//SpriteAnimDefinition testAnim = SpriteAnimDefinition(testAnimSpriteSheet, 56, 63, 8.f, SpriteAnimPlaybackType::PINGPONG);

	//std::vector<Vertex_PCU> testAnimVerts;
	//SpriteDefinition currentSpriteDef = testAnim.GetSpriteDefAtTime(m_timeSinceStart);
	//AddVertsForAABB2(testAnimVerts, AABB2(Vec2(100.f, 200.f), Vec2(200.f, 300.f)), Rgba8::WHITE, currentSpriteDef.GetUVs().m_mins, currentSpriteDef.GetUVs().m_maxs);
	//g_theRenderer->BindTexture(testAnimTexture);
	//g_theRenderer->DrawVertexArray(testAnimVerts);


	DebugDrawRing(m_screenCamera.GetOrthoTopRight() * 0.5f, g_screenSizeY * 0.25f, 40.f, Rgba8(255, 0, 255, m_attractScreenRingOpacity));
	
	RenderTransitionOverlay();
	g_renderer->EndCamera(m_screenCamera);
}

void Game::RenderPlaying() const
{
	g_renderer->BeginCamera(m_worldCamera);
	m_currentMap->Render();
	g_renderer->EndCamera(m_worldCamera);

	g_renderer->BeginCamera(m_screenCamera);
	RenderHUD();
	RenderTransitionOverlay();
	g_renderer->EndCamera(m_screenCamera);
}

void Game::RenderPaused() const
{
	g_renderer->BeginCamera(m_worldCamera);
	m_currentMap->Render();
	g_renderer->EndCamera(m_worldCamera);

	g_renderer->BeginCamera(m_screenCamera);
	RenderHUD();
	RenderPausedOverlay();
	g_renderer->EndCamera(m_screenCamera);
}

void Game::RenderGameover() const
{
	g_renderer->BeginCamera(m_worldCamera);
	m_currentMap->Render();
	g_renderer->EndCamera(m_worldCamera);

	g_renderer->BeginCamera(m_screenCamera);
	RenderHUD();
	RenderGameoverOverlay();
	g_renderer->EndCamera(m_screenCamera);
}

void Game::RenderVictory() const
{
	g_renderer->BeginCamera(m_screenCamera);
	std::vector<Vertex_PCU> attractScreenVertexes;
	AddVertsForAABB2(attractScreenVertexes, AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), Rgba8(255, 255, 255, 255));
	g_renderer->BindTexture(g_textures[VICTORY_TEXTURE]);
	g_renderer->DrawVertexArray(attractScreenVertexes);
	g_renderer->EndCamera(m_screenCamera);
}

bool Game::IsEntityAlive(Entity* entity) const
{
	if (entity == nullptr)
	{
		return false;
	}

	return !(entity->m_isDead);
}

void Game::StartGame()
{
	std::string mapNamesString = g_gameConfigBlackboard.GetValue("maps", "Approach");
	Strings mapNames;
	int numMaps = SplitStringOnDelimiter(mapNames, mapNamesString, ',');
	for (int mapIndex = 0; mapIndex < numMaps; mapIndex++)
	{
		auto mapDefIterator = MapDefinition::s_mapDefs.find(mapNames[mapIndex]);
		MapDefinition mapDef = mapDefIterator->second;
		m_allMaps.push_back(new Map(mapDef));
	}

	m_currentMap = m_allMaps[0];
	m_currentMap->SpawnNewEntityOfType(ENTITY_TYPE_GOOD_PLAYER, Vec2(1.5f, 1.5f), 45.f);

	PlaySound(CLICK_SOUND);
	//m_gameState = GAME_STATE_PLAYING;
	m_nextGameState = GAME_STATE_PLAYING;
	m_shouldPlayOutroTransition = true;

	g_audio->StopSound(m_attractMusicPlayback);
	m_gameplayMusicPlayback = g_audio->StartSound(g_sounds[GAMEPLAY_MUSIC], true, BACKGROUND_MUSIC_VOLUME);
}

void Game::QuitToAttractScreen()
{
	m_drawDebug = false;
	g_audio->StopSound(m_gameplayMusicPlayback);
	m_attractMusicPlayback = g_audio->StartSound(g_sounds[ATTRACT_MUSIC]);

	for (int mapIndex = 0; mapIndex < static_cast<int>(m_allMaps.size()); mapIndex++)
	{
		delete m_allMaps[mapIndex];
		m_allMaps[mapIndex] = nullptr;
	}
	m_allMaps.resize(0);
	m_currentMap = nullptr;
	m_currentMapIndex = 0;

	PlaySound(MAP_EXIT_SOUND);
	m_gameState = GAME_STATE_ATTRACT;
}

void Game::UpdateAttractScreen(float deltaSeconds)
{
	m_timeSinceStart += deltaSeconds;

	XboxController xboxController = g_input->GetController(0);

	if (g_input->WasKeyJustPressed('M'))
	{
		SoundID testSound = g_audio->CreateOrGetSound("Data/Audio/TestSound.mp3");
		g_audio->StartSound(testSound);
	}

	bool wasAnyStartKeyPressed =	g_input->WasKeyJustPressed('P') ||
									g_input->WasKeyJustPressed(KEYCODE_SPACE) ||
									xboxController.WasButtonJustPressed(XBOX_BUTTON_START);

	if (wasAnyStartKeyPressed)
	{
		StartGame();
	}
	if (g_input->WasKeyJustPressed(KEYCODE_ESC) || xboxController.WasButtonJustPressed(XBOX_BUTTON_BACK))
	{
		g_app->HandleQuitRequested();
	}
	if (g_input->WasKeyJustPressed(KEYCODE_ENTER))
	{
		GoToMapSelect();
	}

	if (m_attractScreenRingOpacity <= 127)
	{
		m_isAttractScreenRingOpacityIncreasing = true;
	}
	else if (m_attractScreenRingOpacity >= 255)
	{
		m_isAttractScreenRingOpacityIncreasing = false;
	}

	if (m_isAttractScreenRingOpacityIncreasing)
	{
		m_attractScreenRingOpacity = (unsigned char)GetClamped((float)m_attractScreenRingOpacity + 255 * deltaSeconds, 127, 255);
	}
	else
	{
		m_attractScreenRingOpacity = (unsigned char)GetClamped((float)m_attractScreenRingOpacity - 255 * deltaSeconds, 127, 255);
	}

	UpdateCameras(deltaSeconds);
}

void Game::RenderTransitionOverlay() const
{
	std::vector<Vertex_PCU> transitionOverlayVerts;
	AddVertsForAABB2(transitionOverlayVerts, AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), m_transitionOverlayColor);

	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(transitionOverlayVerts);
}

void Game::RenderHUD() const
{
	std::vector<Vertex_PCU> hudVerts;

	if (m_drawDebug)
	{
		RenderDebug();
	}

	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(hudVerts);
}

void Game::RenderPausedOverlay() const
{
	std::vector<Vertex_PCU> overlayVerts;
	AABB2 screenPanel(Vec2(0.f, 0.f), Vec2(g_screenSizeX, g_screenSizeY));
	AddVertsForAABB2(overlayVerts, screenPanel, Rgba8(0, 0, 0, 127));

	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(overlayVerts);
}

void Game::RenderGameoverOverlay() const
{
	std::vector<Vertex_PCU> overlayVerts;
	AABB2 screenPanel(Vec2(0.f, 0.f), Vec2(g_screenSizeX, g_screenSizeY));
	AddVertsForAABB2(overlayVerts, screenPanel, Rgba8::WHITE);

	g_renderer->BindTexture(g_textures[GAMEOVER_TEXTURE]);
	g_renderer->DrawVertexArray(overlayVerts);
}

void Game::RenderDebug() const
{
	std::vector<Vertex_PCU> debugDrawVerts;

	if (m_isNoClipActive)
	{
		AddVertsForTextTriangles2D(debugDrawVerts, "NoClip", m_screenCamera.GetOrthoTopRight() - Vec2(150.f, 100.f), 25.f, Rgba8::RED);
	}

	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(debugDrawVerts);
}

void Game::GoToMapSelect()
{
	std::string selectedMapsString = g_gameConfigBlackboard.GetValue("maps", "");
	SplitStringOnDelimiter(m_selectedMaps, selectedMapsString, ',');

	for (auto mapDefsIter = MapDefinition::s_mapDefs.begin(); mapDefsIter != MapDefinition::s_mapDefs.end(); mapDefsIter++)
	{
		m_mapOptions.push_back(mapDefsIter->first);
	}

	m_gameState = GAME_STATE_MAP_SELECT;
}

void Game::UpdateMapSelect(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (g_input->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_selectedMaps.clear();
		m_mapOptions.clear();
		m_selectedAddedMapIndex = -1;
		m_selectedMapIndexFromOptions = 0;
		m_gameState = GAME_STATE_ATTRACT;
	}
	if (g_input->WasKeyJustPressed(KEYCODE_DOWNARROW))
	{
		if (m_selectedMapIndexFromOptions != -1 && m_selectedMapIndexFromOptions != static_cast<int>(m_mapOptions.size() - 1))
		{
			m_selectedMapIndexFromOptions++;
		}
		else if (m_selectedAddedMapIndex != -1 && m_selectedAddedMapIndex != static_cast<int>(m_selectedMaps.size() - 1))
		{
			m_selectedAddedMapIndex++;
		}
	}
	if (g_input->WasKeyJustPressed(KEYCODE_UPARROW))
	{
		if (m_selectedMapIndexFromOptions != -1 && m_selectedMapIndexFromOptions != 0)
		{
			m_selectedMapIndexFromOptions--;
		}
		else if (m_selectedAddedMapIndex != -1 && m_selectedAddedMapIndex != 0)
		{
			m_selectedAddedMapIndex--;
		}
	}
	if (g_input->WasKeyJustPressed(KEYCODE_RIGHTARROW))
	{
		if (m_selectedAddedMapIndex == -1)
		{
			m_selectedAddedMapIndex = 0;
			m_selectedMapIndexFromOptions = -1;
		}
	}
	if (g_input->WasKeyJustPressed(KEYCODE_LEFTARROW))
	{
		if (m_selectedMapIndexFromOptions == -1)
		{
			m_selectedMapIndexFromOptions = 0;
			m_selectedAddedMapIndex = -1;
		}
	}
	if (g_input->WasKeyJustPressed('A') && m_selectedMapIndexFromOptions != -1)
	{
		AddMapToGame();
	}
	if (g_input->WasKeyJustPressed('Q') && m_selectedAddedMapIndex != -1)
	{
		RemoveMapFromGame();
	}

	if (g_input->WasKeyJustPressed(KEYCODE_ENTER))
	{
		SaveSelectedMaps();
	}
}

void Game::SaveSelectedMaps()
{
	std::string selectedMapsString = "";
	for (int selectedMapIndex = 0; selectedMapIndex < static_cast<int>(m_selectedMaps.size()); selectedMapIndex++)
	{
		selectedMapsString += m_selectedMaps[selectedMapIndex];

		if (selectedMapIndex != static_cast<int>(m_selectedMaps.size() - 1))
		{
			selectedMapsString += ",";
		}
	}

	g_gameConfigBlackboard.SetValue("maps", selectedMapsString);
}

void Game::AddMapToGame()
{
	m_selectedMaps.push_back(m_mapOptions[m_selectedMapIndexFromOptions]);
}

void Game::RemoveMapFromGame()
{
	m_selectedMaps.erase(m_selectedMaps.begin() + m_selectedAddedMapIndex);
}

void Game::RenderMapSelect() const
{
	std::vector<Vertex_PCU> mapSelectVerts;
	std::vector<Vertex_PCU> mapSelectTextVerts;

	g_bitmapFonts[SQUIRREL_BITMAP_FONT]->AddVertsForText2D(mapSelectTextVerts, Vec2(20.f, g_screenSizeY - 100.f), 40.f, "Game Levels");
	g_bitmapFonts[SQUIRREL_BITMAP_FONT]->AddVertsForText2D(mapSelectTextVerts, Vec2(210.f, g_screenSizeY - 180.f), 25.f, "Map Options");
	g_bitmapFonts[SQUIRREL_BITMAP_FONT]->AddVertsForText2D(mapSelectTextVerts, Vec2(1010.f, g_screenSizeY - 180.f), 25.f, "Added Maps");

	AddVertsForAABB2(mapSelectVerts, AABB2(Vec2::ZERO, Vec2(g_screenSizeX, g_screenSizeY)), Rgba8::GREEN);
	AddVertsForAABB2(mapSelectVerts, AABB2(Vec2(200.f, 100.f), Vec2(600.f, 600.f)), Rgba8(150, 150, 150, 150));
	AddVertsForAABB2(mapSelectVerts, AABB2(Vec2(1000.f, 100.f), Vec2(1400.f, 600.f)), Rgba8(150, 150, 150, 150));

	for (int mapOptionIndex = 0; mapOptionIndex < static_cast<int>(m_mapOptions.size()); mapOptionIndex++)
	{
		Rgba8 textColor = Rgba8::WHITE;
		if (mapOptionIndex == m_selectedMapIndexFromOptions)
		{
			textColor = Rgba8::NAVY;
		}
		g_bitmapFonts[SQUIRREL_BITMAP_FONT]->AddVertsForText2D(mapSelectTextVerts, Vec2(210.f, 550.f - mapOptionIndex * 25.f), 20.f, m_mapOptions[mapOptionIndex], textColor);
	}

	for (int selectedMapIndex = 0; selectedMapIndex < static_cast<int>(m_selectedMaps.size()); selectedMapIndex++)
	{
		Rgba8 textColor = Rgba8::WHITE;
		if (selectedMapIndex == m_selectedAddedMapIndex)
		{
			textColor = Rgba8::NAVY;
		}
		g_bitmapFonts[SQUIRREL_BITMAP_FONT]->AddVertsForText2D(mapSelectTextVerts, Vec2(1010.f, 550.f - selectedMapIndex * 25.f), 20.f, m_selectedMaps[selectedMapIndex], textColor);
	}

	AddVertsForAABB2(mapSelectVerts, AABB2(Vec2(1400.f, 20.f), Vec2(1500.f, 50.f)), Rgba8(150, 150, 150, 150));
	float textWidth = g_bitmapFonts[SQUIRREL_BITMAP_FONT]->GetTextWidth(20.f, "Save");
	g_bitmapFonts[SQUIRREL_BITMAP_FONT]->AddVertsForText2D(mapSelectTextVerts, Vec2(1450.f - textWidth * 0.5f, 25.f), 20.f, "Save");

	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(mapSelectVerts);

	g_renderer->BindTexture(g_bitmapFonts[SQUIRREL_BITMAP_FONT]->GetTexture());
	g_renderer->DrawVertexArray(mapSelectTextVerts);
}

void Game::PlaySound(int soundIndex, Vec2 const& sourcePosition)
{
	UNUSED(sourcePosition);

	if (m_secondsSinceSoundWasLastPlayed[soundIndex] >= 0.1f)
	{
		m_secondsSinceSoundWasLastPlayed[soundIndex] = 0.f;
		g_audio->StartSound(g_sounds[soundIndex]);
	}
}

#pragma once

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/SimpleTriangleFont.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/HeatMaps/TileHeatMap.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/Spritesheet.hpp"
#include "Engine/Renderer/Texture.hpp"

class App;
class Game;

extern App*							g_app;
extern InputSystem*					g_input;
extern RandomNumberGenerator*		g_RNG;
extern Renderer*					g_renderer;
extern AudioSystem*					g_audio;
extern Window*						g_window;
extern Game*						g_game;

extern SpriteSheet*					g_terrainSpriteSheet;

extern float g_screenSizeX;
extern float g_screenSizeY;

enum Sounds
{
	INVALID = -1,

	ATTRACT_MUSIC,
	GAMEPLAY_MUSIC,
	WELCOME_SOUND,
	BULLET_RICOTHET_SOUND,
	BULLET_BOUNCE_SOUND,
	ENEMY_SHOOT_SOUND,
	ENEMY_HIT_SOUND,
	ENTITY_DIE_SOUND,
	PAUSE_SOUND,
	UNPAUSE_SOUND,
	VICTORY_SOUND,
	CLICK_SOUND,
	GAMEOVER_SOUND,
	PLAYER_SHOOT_SOUND,
	PLAYER_HIT_SOUND,
	MAP_EXIT_SOUND,
	DISCOVERY_SOUND,

	SOUNDS_NUM
};

enum Textures
{
	TEXTURE_INVALID = -1,

	ATTRACT_BACKGROUND_TEXTURE,
	PLAYERTANK_BASE_TEXTURE,
	PLAYERTANK_TURRET_TEXTURE,
	SCORPIO_BASE_TEXTURE,
	SCORPIO_TURRET_TEXTURE,
	LEO_BASE_TEXTURE,
	ARIES_BASE_TEXTURE,
	CAPRICORN_BASE_TEXTURE,
	GOOD_BOLT_TEXTURE,
	EVIL_BOLT_TEXTURE,
	GOOD_BULLET_TEXTURE,
	EVIL_BULLET_TEXTURE,
	VICTORY_TEXTURE,
	GAMEOVER_TEXTURE,

	TEXTURES_NUM
};

enum BitmapFonts
{
	BITMAP_FONT_INVALID = -1,

	SQUIRREL_BITMAP_FONT,

	BITMAP_FONTS_NUM
};

enum SpriteSheets
{
	SPRITESHEET_INVALID = -1,

	TERRAIN_SPRITESHEET,
	EXPLOSION_SPRITESHEET,

	SPRITESHEETS_NUM
};

enum Animations
{
	ANIMATION_INVALID = -1,
	
	ANIMATIONS_NUM
};

extern SoundID								g_sounds[SOUNDS_NUM];
extern Texture*								g_textures[TEXTURES_NUM];
extern BitmapFont*							g_bitmapFonts[TEXTURES_NUM];
extern SpriteSheet*							g_spriteSheets[SPRITESHEETS_NUM];

constexpr float						SPECIAL_VALUE_FOR_HEATMAPS = 9999.f;

void DebugDrawRing					(Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void DebugDrawLine					(Vec2 const& pointA, Vec2 const& pointB, float thickness, Rgba8 const& color);
void DebugDrawDisc					(Vec2 const& center, float radius, Rgba8 const& color);
void DebugDrawGlow					(Vec2 const& center, float radius, Rgba8 const& centerColor, Rgba8 const& edgeColor);
void DrawBoxOutline					(AABB2 const& boundedBox, float thickness, Rgba8 const& color);

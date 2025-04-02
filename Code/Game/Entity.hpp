#pragma once

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/HeatMaps/TileHeatMap.hpp"

#include <vector>

class Entity;

typedef std::vector<Entity*> EntityList;

class App;
class Game;
class Map;
class Bullet;
struct Vertex_PCU;

enum EntityType
{
	ENTITY_TYPE_INVALID = -1,

	ENTITY_TYPE_GOOD_BOLT,
	ENTITY_TYPE_GOOD_BULLET,
	ENTITY_TYPE_EVIL_BOLT,
	ENTITY_TYPE_EVIL_BULLET,
	ENTITY_TYPE_GOOD_PLAYER,
	ENTITY_TYPE_EVIL_SCORPIO,
	ENTITY_TYPE_EVIL_LEO,
	ENTITY_TYPE_EVIL_ARIES,
	ENTITY_TYPE_EVIL_CAPRICORN,
	ENTITY_TYPE_GOOD_FLAMETHROWER_BULLET,

	ENTITY_TYPE_NEUTRAL_EXPLOSION,

	ENTITY_TYPE_NUM
};

enum EntityFaction
{
	FACTION_INVALID = -1,

	FACTION_GOOD,
	FACTION_NEUTRAL,
	FACTION_EVIL,

	FACTION_NUM
};

class Entity
{

public:
	virtual						~Entity										() = default;
								Entity										(EntityType entityType, EntityFaction faction, Vec2 const& startPosition, float orientationDegrees);
	
	virtual void				Update										(float deltaSeconds) = 0;
	virtual void				Render										() const = 0;
	virtual void				RenderHealthBar								() const;
	virtual void				TakeDamage									(int damage);
	virtual void				ReactToBulletHit							(Bullet& bullet);
	
	virtual void				Die											();
	bool						IsOffScreen									() const;

public:
	Vec2					m_position;
	Vec2					m_velocity;
	float					m_orientationDegrees				= 0.f;
	float					m_angularVelocity					= 0.f;
	float					m_physicsRadius						= 0.f;
	float					m_cosmeticRadius					= 0.f;
	int						m_maxHealth							= 1;
	int						m_health							= 1;
	bool					m_isDead							= false;
	bool					m_isGarbage							= false;
	Map*					m_map;
	Rgba8					m_color;
	float					m_scale								= 1.f;
	EntityType				m_entityType						= ENTITY_TYPE_INVALID;
	bool					m_isPushedByWalls					= false;
	bool					m_isPushedByEntities				= false;
	bool					m_pushesEntities					= false;
	bool					m_isHitByBullets					= false;
	bool					m_canSwim							= false;
	EntityFaction			m_faction							= FACTION_INVALID;

	TileHeatMap*			m_distanceFieldFromGoalPosition		= nullptr;


protected:
	Vec2							GetForwardNormal						() const;
	bool							IsAlive									() { return !m_isDead; }
	virtual void					RenderDebug								() const;

protected:
};
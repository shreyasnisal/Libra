#include "Game/Scorpio.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/Entity.hpp"

#include "Engine/Math/RaycastUtils.hpp"

Scorpio::Scorpio(EntityType type, EntityFaction faction, Vec2 const& startPosition, float orientationDegrees)
	: Entity(type, faction, startPosition, orientationDegrees)
{
	m_isPushedByWalls = true;
	m_isPushedByEntities = false;
	m_pushesEntities = true;
	m_isHitByBullets = true;

	m_physicsRadius = 0.3f;
	m_cosmeticRadius = 0.4f;

	m_maxHealth = g_gameConfigBlackboard.GetValue("scorpioMaxHealth", 5);
	m_health = m_maxHealth;

	InitializeVertexes();
}

void Scorpio::InitializeVertexes()
{
	AABB2 baseVertexBox = AABB2(Vec2(-m_cosmeticRadius, -m_cosmeticRadius), Vec2(m_cosmeticRadius, m_cosmeticRadius));
	AABB2 turretVertexBox = AABB2(Vec2(-m_cosmeticRadius, -m_cosmeticRadius), Vec2(m_cosmeticRadius, m_cosmeticRadius));
	AddVertsForAABB2(m_baseVertexes, baseVertexBox, Rgba8(255, 255, 255, 255));
	AddVertsForAABB2(m_turretVertexes, turretVertexBox, Rgba8(255, 255, 255, 255));
}

void Scorpio::Update(float deltaSeconds)
{
	if (m_isDead)
	{
		return;
	}

	EntityList& players = g_game->m_currentMap->m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER];

	for (int playerIndex = 0; playerIndex < static_cast<int>(players.size()); playerIndex++)
	{
		Entity*& player = players[playerIndex];
		float range = g_gameConfigBlackboard.GetValue("enemyVisibleRange", 10.f);
		if (g_game->IsEntityAlive(player) && GetDistance2D(m_position, player->m_position) <= range && g_game->m_currentMap->HasLineOfSight(m_position, player->m_position))
		{
			if (!m_isInPursuit)
			{
				g_game->PlaySound(DISCOVERY_SOUND, m_position);
				m_isInPursuit = true;
			}

			Vec2 directionToPlayer = (player->m_position - m_position).GetNormalized();
			float orientationToPlayer = directionToPlayer.GetOrientationDegrees();
			float turretAngularVelocity = g_gameConfigBlackboard.GetValue("scorpioTurnRate", 30.f);
			m_turretOrientationDegrees = GetTurnedTowardDegrees(m_turretOrientationDegrees, orientationToPlayer, turretAngularVelocity * deltaSeconds);

			if (fabsf(GetShortestAngularDispDegrees(m_turretOrientationDegrees, orientationToPlayer)) <= 0.5f * g_gameConfigBlackboard.GetValue("scorpioTurnAperture", 10.f))
			{
				m_timeRemainingUntilNextBullet -= deltaSeconds;
				if (m_timeRemainingUntilNextBullet <= 0.f)
				{
					FireBullet();
					m_timeRemainingUntilNextBullet = g_gameConfigBlackboard.GetValue("scorpioShootCooldownSeconds", 0.1f);
				}
			}

			return;
		}
	}

	m_isInPursuit = false;
	float turretAngularVelocity = g_gameConfigBlackboard.GetValue("scorpioTurnRate", 30.f);
	m_turretOrientationDegrees += turretAngularVelocity * deltaSeconds;
}

void Scorpio::Render() const
{
	if (m_isDead)
	{
		return;
	}

	std::vector<Vertex_PCU> worldBaseVertexes = m_baseVertexes;
	std::vector<Vertex_PCU> worldTurretVertexes = m_turretVertexes;

	TransformVertexArrayXY3D(worldBaseVertexes, 1.f, m_orientationDegrees, m_position);
	TransformVertexArrayXY3D(worldTurretVertexes, 1.f, m_turretOrientationDegrees, m_position);

	float range = g_gameConfigBlackboard.GetValue("enemyVisibleRange", 10.f);

	std::vector<Vertex_PCU> weaponLineOfSightVerts;

	//RaycastResult2D forwardRaycastResult = g_game->m_currentMap->StepAndSampleRaycastVsTiles(m_position, Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees), range);
	RaycastResult2D forwardRaycastResult = g_game->m_currentMap->BetterMoreConfusingRaycastVsTiles(m_position, Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees), range);
	if (forwardRaycastResult.m_didImpact)
	{
		unsigned char endOpacity = static_cast<unsigned char>(RoundDownToInt(RangeMap(forwardRaycastResult.m_impactDistance, 0.f, range, 0, 127)));
		AddVertsForGradientLineSegment2D(weaponLineOfSightVerts, m_position, forwardRaycastResult.m_impactPosition, 0.05f, Rgba8(255, 0, 0, 127), Rgba8(255, 0, 0, endOpacity));
	}
	else
	{
		AddVertsForGradientLineSegment2D(weaponLineOfSightVerts, m_position, m_position + Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees, range), 0.05f, Rgba8(255, 0, 0, 127), Rgba8(255, 0, 0, 0));
	}

	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(weaponLineOfSightVerts);

	g_renderer->BindTexture(g_textures[SCORPIO_BASE_TEXTURE]);
	g_renderer->DrawVertexArray(worldBaseVertexes);

	g_renderer->BindTexture(g_textures[SCORPIO_TURRET_TEXTURE]);
	g_renderer->DrawVertexArray(worldTurretVertexes);

	if (m_health < m_maxHealth)
	{
		RenderHealthBar();
	}

	if (g_game->m_drawDebug)
	{
		RenderDebug();
	}
}

void Scorpio::FireBullet()
{
	g_game->PlaySound(ENEMY_SHOOT_SOUND, m_position);
	g_game->m_currentMap->SpawnNewEntityOfType(ENTITY_TYPE_EVIL_BOLT, m_position + m_cosmeticRadius * Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees), m_turretOrientationDegrees);
}

void Scorpio::RenderDebug() const
{
	Entity::RenderDebug();

	std::vector<Vertex_PCU> renderDebugVerts;

	for (int playerIndex = 0; playerIndex < static_cast<int>(g_game->m_currentMap->m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER].size()); playerIndex++)
	{
		Entity*& player = g_game->m_currentMap->m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][playerIndex];
		if (!g_game->IsEntityAlive(player))
		{
			continue;
		}
		float range = g_gameConfigBlackboard.GetValue("enemyVisibleRange", 10.f);
		float debugElementsThickness = g_gameConfigBlackboard.GetValue("debugDrawLineThickness", 0.03f);
		if (g_game->m_currentMap->HasLineOfSight(m_position, player->m_position) && GetDistance2D(m_position, player->m_position) <= range)
		{
			AddVertsForLineSegment2D(renderDebugVerts, m_position, player->m_position, debugElementsThickness, Rgba8::BLACK);
		}
	}

	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(renderDebugVerts);
}

void Scorpio::Die()
{
	m_map->SpawnNewEntityOfType(ENTITY_TYPE_NEUTRAL_EXPLOSION, m_position, m_orientationDegrees, 0.8f, 0.4f);
	Entity::Die();
}

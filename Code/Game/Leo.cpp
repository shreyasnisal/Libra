#include "Game/Leo.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"

#include "Engine/Math/MathUtils.hpp"

Leo::Leo(EntityType type, EntityFaction faction, Vec2 const& startPosition, float orientationDegrees)
	: Entity(type, faction, startPosition, orientationDegrees)
	, m_goalPosition(startPosition)
	, m_nextWaypointPosition(startPosition)
{
	m_isPushedByWalls = true;
	m_isPushedByEntities = true;
	m_pushesEntities = true;
	m_isHitByBullets = true;

	m_physicsRadius = 0.3f;
	m_cosmeticRadius = 0.4f;

	m_maxHealth = g_gameConfigBlackboard.GetValue("leoMaxHealth", 5);
	m_health = m_maxHealth;

	InitializeVertexes();
}

void Leo::InitializeVertexes()
{
	AABB2 baseVertexBox = AABB2(Vec2(-m_cosmeticRadius, -m_cosmeticRadius), Vec2(m_cosmeticRadius, m_cosmeticRadius));
	AddVertsForAABB2(m_baseVertexes, baseVertexBox, Rgba8(255, 255, 255, 255));
}

void Leo::SetGoalPosition()
{
	Entity* player = m_map->m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][0];

	if (!m_map || !g_game->IsEntityAlive(player))
	{
		return;
	}

	float range = g_gameConfigBlackboard.GetValue("enemyVisibleRange", 10.f);
	if (GetDistance2D(player->m_position, m_position) < range && m_map->HasLineOfSight(m_position, player->m_position))
	{
		int goalTileX = RoundDownToInt(player->m_position.x);
		int goalTileY = RoundDownToInt(player->m_position.y);
		m_goalPosition = Vec2(static_cast<float>(goalTileX) + 0.5f, static_cast<float>(goalTileY) + 0.5f);

		return;
	}

	PickRandomGoalPosition();
}

void Leo::PickRandomGoalPosition()
{
	if (!m_map)
	{
		return;
	}

	m_isInPursuit = false;

	GenerateHeatMap();

	int goalTileX = -1;
	int goalTileY = -1;
	do
	{
		goalTileX = g_RNG->RollRandomIntInRange(1, m_map->GetDimensions().x - 2);
		goalTileY = g_RNG->RollRandomIntInRange(1, m_map->GetDimensions().y - 2);
		m_goalPosition = Vec2(static_cast<float>(goalTileX) + 0.5f, static_cast<float>(goalTileY) + 0.5f);
	} while (m_distanceFieldFromGoalPosition->GetValueAtTile(IntVec2(goalTileX, goalTileY)) == SPECIAL_VALUE_FOR_HEATMAPS);
}

void Leo::GenerateHeatMap()
{
	if (!m_map)
	{
		return;
	}
	if (m_map && !m_distanceFieldFromGoalPosition)
	{
		m_distanceFieldFromGoalPosition = new TileHeatMap(m_map->GetDimensions());
	}
	m_map->PopulateDistanceField(*m_distanceFieldFromGoalPosition, IntVec2(RoundDownToInt(m_goalPosition.x), RoundDownToInt(m_goalPosition.y)), SPECIAL_VALUE_FOR_HEATMAPS, !m_canSwim, true, true);
}

void Leo::SetNextWaypointPosition()
{
	m_nextWaypointPosition = m_pathPoints.back();
	if (CanTakeShortcut())
	{
		m_pathPoints.pop_back();
		if (m_pathPoints.size() > 0)
		{
			m_nextWaypointPosition = m_pathPoints[m_pathPoints.size() - 1];
		}
	}
}

bool Leo::IsPointReachable()
{
	return m_distanceFieldFromGoalPosition->GetValueAtTile(IntVec2(RoundDownToInt(m_goalPosition.x), RoundDownToInt(m_goalPosition.y))) != SPECIAL_VALUE_FOR_HEATMAPS;
}

void Leo::TurnAndMoveTowardsNextWaypointPosition(float deltaSeconds)
{
	Vec2 directionToNextWaypointPosition = (m_nextWaypointPosition - m_position).GetNormalized();
	float orientationDegreesToNextWaypointPosiiton = directionToNextWaypointPosition.GetOrientationDegrees();
	float angularVelocity = g_gameConfigBlackboard.GetValue("leoTurnRate", 90.f);
	m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, orientationDegreesToNextWaypointPosiiton, angularVelocity * deltaSeconds);

	if (GetAngleDegreesBetweenVectors2D(GetForwardNormal(), directionToNextWaypointPosition) > 0.5f * g_gameConfigBlackboard.GetValue("leoDriveAperture", 90.f))
	{
		return;
	}
	m_velocity = GetForwardNormal() * 0.5f;
	m_position += m_velocity * deltaSeconds;
}

void Leo::SetPlayerAsGoalPositionIfPlayerIsVisible()
{
	Entity* player = m_map->m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][0];

	if (!m_map || !g_game->IsEntityAlive(player))
	{
		return;
	}
	float range = g_gameConfigBlackboard.GetValue("enemyVisibleRange", 10.f);
	if (GetDistance2D(player->m_position, m_position) < range && m_map->HasLineOfSight(m_position, player->m_position))
	{
		if (!m_isInPursuit)
		{
			g_game->PlaySound(DISCOVERY_SOUND, m_position);
			m_isInPursuit = true;
		}

		int goalTileX = RoundDownToInt(player->m_position.x);
		int goalTileY = RoundDownToInt(player->m_position.y);
		m_goalPosition = Vec2(static_cast<float>(goalTileX) + 0.5f, static_cast<float>(goalTileY) + 0.5f);
		GenerateHeatMap();
		m_pathPoints = m_distanceFieldFromGoalPosition->GeneratePath(m_position, m_goalPosition);
		SetNextWaypointPosition();
	}
}

void Leo::CheckPlayerPositionAndFire(float deltaSeconds)
{
	Entity* player = m_map->m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][0];
	if (!g_game->IsEntityAlive(player))
	{
		return;
	}
	float shootAperture = g_gameConfigBlackboard.GetValue("leoShootAperture", 10.f);
	bool hasLineOfSightToPlayer = m_map->HasLineOfSight(m_position, player->m_position);
	float shootRange = g_gameConfigBlackboard.GetValue("enemyVisibleRange", 10.f);
	if (g_game->IsEntityAlive(player) && IsPointInsideOrientedSector2D(player->m_position, m_position, m_orientationDegrees, shootAperture, shootRange) && hasLineOfSightToPlayer)
	{
		m_timeRemainingUntilNextBullet -= deltaSeconds;

		if (m_timeRemainingUntilNextBullet <= 0.f)
		{
			FireBullet();
			m_timeRemainingUntilNextBullet = g_gameConfigBlackboard.GetValue("leoShootCooldownSeconds", 1.f);
		}
	}
}

bool Leo::CanTakeShortcut()
{
	if (m_pathPoints.size() < 2)
	{
		return false;
	}

	Vec2 rayEndPosition = m_pathPoints[m_pathPoints.size() - 2];
	float rayMaxLength = GetDistance2D(m_position, rayEndPosition);
	Vec2 rayDirection = (rayEndPosition - m_position).GetNormalized();
	TileHeatMap solidMap = m_map->m_solidMapForLand;
	if (m_canSwim)
	{
		solidMap = m_map->m_solidMapForAmphibian;
	}
	RaycastResult2D raycastResult = solidMap.Raycast(m_position, rayDirection, rayMaxLength);

	return !raycastResult.m_didImpact;
}

void Leo::TurnAndMoveTowardsGoalPosition(float deltaSeconds)
{
	Vec2 directionToGoalPosition = (m_goalPosition - m_position).GetNormalized();
	float orientationDegreesToGoalPosition = directionToGoalPosition.GetOrientationDegrees();
	float angularVelocity = g_gameConfigBlackboard.GetValue("leoTurnRate", 90.f);
	m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, orientationDegreesToGoalPosition, angularVelocity * deltaSeconds);

	if (GetAngleDegreesBetweenVectors2D(GetForwardNormal(), directionToGoalPosition) > 0.5f * g_gameConfigBlackboard.GetValue("leoDriveAperture", 90.f))
	{
		return;
	}
	m_velocity = GetForwardNormal() * 0.5f;
	m_position += m_velocity * deltaSeconds;
}

void Leo::Update(float deltaSeconds)
{
	if (m_isDead)
	{
		return;
	}

	SetPlayerAsGoalPositionIfPlayerIsVisible();

	if (m_pathPoints.size() == 0)
	{
		do
		{
			SetGoalPosition();
			GenerateHeatMap();
			m_pathPoints = m_distanceFieldFromGoalPosition->GeneratePath(m_position, m_goalPosition);
		} while (!IsPointReachable());
	}

	SetNextWaypointPosition();

	TurnAndMoveTowardsNextWaypointPosition(deltaSeconds);

	CheckPlayerPositionAndFire(deltaSeconds);
}

void Leo::Render() const
{
	if (m_isDead)
	{
		return;
	}

	std::vector<Vertex_PCU> worldBaseVertexes = m_baseVertexes;

	TransformVertexArrayXY3D(worldBaseVertexes, 1.f, m_orientationDegrees, m_position);

	g_renderer->BindTexture(g_textures[LEO_BASE_TEXTURE]);
	g_renderer->DrawVertexArray(worldBaseVertexes);

	if (m_health < m_maxHealth)
	{
		RenderHealthBar();
	}

	if (g_game->m_drawDebug)
	{
		RenderDebug();
	}
}

void Leo::RenderDebug() const
{
	Entity::RenderDebug();

	std::vector<Vertex_PCU> debugRenderVerts;

	float debugElementsThickness = g_gameConfigBlackboard.GetValue("debugDrawLineThickness", 0.03f);
		
	AddVertsForLineSegment2D(debugRenderVerts, m_position, m_goalPosition, debugElementsThickness, Rgba8::BLACK);
	AddVertsForDisc2D(debugRenderVerts, m_goalPosition, debugElementsThickness * 2.5f, Rgba8::BLACK);

	AddVertsForLineSegment2D(debugRenderVerts, m_position, m_nextWaypointPosition, debugElementsThickness, Rgba8::GRAY);
	AddVertsForDisc2D(debugRenderVerts, m_nextWaypointPosition, debugElementsThickness * 2.5f, Rgba8::GRAY);
		
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(debugRenderVerts);
}

void Leo::FireBullet()
{
	g_game->PlaySound(ENEMY_SHOOT_SOUND, m_position);
	m_map->SpawnNewEntityOfType(ENTITY_TYPE_EVIL_BOLT, m_position + m_cosmeticRadius * GetForwardNormal(), m_orientationDegrees);
}

void Leo::Die()
{
	m_map->SpawnNewEntityOfType(ENTITY_TYPE_NEUTRAL_EXPLOSION, m_position, m_orientationDegrees, 0.8f, 0.4f);
	Entity::Die();
}

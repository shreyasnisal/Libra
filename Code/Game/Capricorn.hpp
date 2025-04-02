#pragma once

#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/HeatMaps/TileHeatMap.hpp"

#include <vector>

class Texture;
class Map;

class Capricorn : public Entity
{
public:
	~Capricorn() = default;
	Capricorn(EntityType type, EntityFaction faction, Vec2 const& startPos, float orientationDegrees);

	void							Update(float deltaSeconds) override;
	void							Render() const override;
	void							Die();

public:

private:
	void							InitializeVertexes();

	void							SetPlayerAsGoalPositionIfPlayerIsVisible();
	void							SetGoalPosition();
	void							PickRandomGoalPosition();
	void							GenerateHeatMap();
	void							SetNextWaypointPosition();
	bool							IsPointReachable();
	bool							CanTakeShortcut();
	void							TurnAndMoveTowardsNextWaypointPosition(float deltaSeconds);
	void							TurnAndMoveTowardsGoalPosition(float deltaSeconds);
	void							CheckPlayerPositionAndFire(float deltaSeconds);

	void							FireBullet();
	void							RenderDebug() const override;

private:
	std::vector<Vertex_PCU>			m_baseVertexes;

	Vec2							m_goalPosition;
	std::vector<Vec2>				m_pathPoints;
	Vec2							m_nextWaypointPosition;

	float							m_timeRemainingUntilNextBullet = 0.f;
	bool							m_isInPursuit = false;
};

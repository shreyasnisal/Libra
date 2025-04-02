#pragma once

#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <vector>

class Texture;
class Map;

class Aries : public Entity
{
public:
	~Aries() = default;
	Aries(EntityType type, EntityFaction faction, Vec2 const& startPos, float orientationDegrees);

	void							Update(float deltaSeconds) override;
	void							Render() const override;
	void							ReactToBulletHit(Bullet& bullet) override;
	void							Die() override;

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

	void							RenderDebug() const override;

private:
	std::vector<Vertex_PCU>			m_baseVertexes;

	Vec2							m_goalPosition;
	std::vector<Vec2>				m_pathPoints;
	Vec2							m_nextWaypointPosition;
	bool							m_isInPursuit = false;
};

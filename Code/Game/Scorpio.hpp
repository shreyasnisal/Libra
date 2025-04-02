#pragma once

#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#include <vector>

class Texture;
class Map;

class Scorpio : public Entity
{
public:
	~Scorpio() = default;
	Scorpio(EntityType type, EntityFaction faction, Vec2 const& startPos, float orientationDegrees);
	
	void							Update(float deltaSeconds) override;
	void							Render() const override;
	void							RenderDebug() const override;
	void							Die() override;

public:

private:
	void							InitializeVertexes();
	void							FireBullet();

private:
	std::vector<Vertex_PCU>			m_baseVertexes;
	std::vector<Vertex_PCU>			m_turretVertexes;
	float							m_turretOrientationDegrees = 0.f;
	float							m_timeRemainingUntilNextBullet = 0.f;
	bool							m_isInPursuit = false;
};

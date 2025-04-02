#pragma once

#include "Game/Entity.hpp"

#include <vector>

class Texture;
class Map;
class SpriteSheet;
class SpriteAnimDefinition;

class Bullet : public Entity
{
public:
	~Bullet() = default;
	Bullet(EntityType type, EntityFaction faction, Vec2 const& startPos, float orientationDegrees);

	static void							CreateAnimations();

	void								LoadAssets();
	void								InitializeVertexes();
	void								Update(float deltaSeconds) override;
	void								Render() const override;
	void								Die() override;

	void								BounceOff(Vec2 const& impactNormal);

public:
	static SpriteAnimDefinition*		s_flamethrowerBulletAnimation;

	int									m_damage = 1;

private:
	void								UpdateGuided(float deltaSeconds);
	void								UpdateFlamethrowerBullet(float deltaSeconds);

private:
	Texture* m_baseTexture = nullptr;
	std::vector<Vertex_PCU>				m_baseVertexes;

	int									m_numWallsHit = 0;

	float								m_flamethrowerBulletAge = 0.f;
};


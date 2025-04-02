#pragma once

#include "Game/Entity.hpp"

#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/Spritesheet.hpp"


class Explosion : public Entity
{
public:
	Explosion(EntityType type, EntityFaction faction, Vec2 const& startPosition, float orientationDegrees, float scale, float lifetime);

	void							Update(float deltaSeconds) override;
	void							Render() const override;

public:

private:

private:
	float							m_lifetimeSeconds = 0.f;
	float							m_ageSeconds = 0.f;

	SpriteAnimDefinition*			m_spriteAnimDef;
};
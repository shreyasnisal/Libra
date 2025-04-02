#include "Game/Explosion.hpp"

#include "Game/GameCommon.hpp"

Explosion::Explosion(EntityType type, EntityFaction faction, Vec2 const& startPosition, float orientationDegrees, float scale, float lifetimeSeconds)
	: Entity(type, faction, startPosition, orientationDegrees)
	, m_lifetimeSeconds(lifetimeSeconds)
{
	m_scale = scale;
	m_cosmeticRadius = 1.f;

	m_spriteAnimDef = new SpriteAnimDefinition(g_spriteSheets[EXPLOSION_SPRITESHEET], 0, 24, m_lifetimeSeconds, SpriteAnimPlaybackType::ONCE);
}

void Explosion::Update(float deltaSeconds)
{
	m_ageSeconds += deltaSeconds;

	if (m_ageSeconds >= m_lifetimeSeconds)
	{
		Die();
	}
}

void Explosion::Render() const
{
	std::vector<Vertex_PCU> verts;
	SpriteDefinition currentSpriteDef = m_spriteAnimDef->GetSpriteDefAtTime(m_ageSeconds);
	AddVertsForAABB2(verts, AABB2(Vec2(-m_scale, -m_scale), Vec2(m_scale, m_scale)), Rgba8::WHITE, currentSpriteDef.GetUVs().m_mins, currentSpriteDef.GetUVs().m_maxs);
	TransformVertexArrayXY3D(verts, m_scale, m_orientationDegrees, m_position);
	g_renderer->SetBlendMode(BlendMode::ADDITIVE);
	g_renderer->BindTexture(g_spriteSheets[EXPLOSION_SPRITESHEET]->GetTexture());
	g_renderer->DrawVertexArray(verts);
	g_renderer->SetBlendMode(BlendMode::ALPHA);
}

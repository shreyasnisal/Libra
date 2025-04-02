#include "Game/Entity.hpp"

#include "Engine/Math/MathUtils.hpp"

#include "Game/App.hpp"
#include "Game/Bullet.hpp"
#include "Game/Map.hpp"


Entity::Entity(EntityType entityType, EntityFaction faction, Vec2 const& startPosition, float orientationDegrees)
	: m_entityType(entityType)
	, m_faction(faction)
	, m_position(startPosition)
	, m_orientationDegrees(orientationDegrees)
{
}

bool Entity::IsOffScreen() const
{
	return 
	(
			(m_position.x + m_cosmeticRadius <= g_game->m_worldCamera.GetOrthoBottomLeft().x) ||
			(m_position.x - m_cosmeticRadius >= g_game->m_worldCamera.GetOrthoTopRight().x) ||
			(m_position.y + m_cosmeticRadius <= g_game->m_worldCamera.GetOrthoBottomLeft().y) ||
			( m_position.y - m_cosmeticRadius >= g_game->m_worldCamera.GetOrthoTopRight().y)
	);
}

Vec2 Entity::GetForwardNormal() const
{
	return Vec2(CosDegrees(m_orientationDegrees), SinDegrees(m_orientationDegrees));
}

void Entity::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}

void Entity::RenderDebug() const
{
	float debugElementsThickness = g_gameConfigBlackboard.GetValue("debugDrawLineThickness", 0.03f);

	DebugDrawRing		(m_position,	m_physicsRadius,			debugElementsThickness, Rgba8(0, 255, 255, 255));
	DebugDrawRing		(m_position,	m_cosmeticRadius,			debugElementsThickness, Rgba8(255, 0, 255, 255));
	DebugDrawLine		(m_position,	m_position + m_velocity,	debugElementsThickness, Rgba8(255, 255, 0, 255));
	
	Vec2 const forwardNormal = GetForwardNormal();
	Vec2 const relativeLeftNormal = forwardNormal.GetRotated90Degrees();
	
	DebugDrawLine		(m_position,	m_position + forwardNormal * m_cosmeticRadius * m_scale,		debugElementsThickness, Rgba8(255, 0, 0, 255));
	DebugDrawLine		(m_position,	m_position + relativeLeftNormal * m_cosmeticRadius * m_scale,	debugElementsThickness, Rgba8(0, 255, 0, 255));
}

void Entity::RenderHealthBar() const
{
	float healthBarYOffset = g_gameConfigBlackboard.GetValue("healthBarYOffset", 0.5f);
	float healthBarLength = g_gameConfigBlackboard.GetValue("healthBarLength", 0.8f);
	float healthBarHeight = g_gameConfigBlackboard.GetValue("healthBarHeight", 0.08f);

	std::vector<Vertex_PCU> healthBarVerts;
	AddVertsForAABB2(healthBarVerts, AABB2(Vec2(-healthBarLength * 0.5f, healthBarYOffset - healthBarHeight * 0.5f), Vec2(healthBarLength * 0.5f, healthBarYOffset + healthBarHeight * 0.5f)), Rgba8::RED);
	AddVertsForAABB2(healthBarVerts, AABB2(Vec2(-healthBarLength * 0.5f, healthBarYOffset - healthBarHeight * 0.5f), Vec2(-healthBarLength * 0.5f + healthBarLength * static_cast<float>(m_health) / static_cast<float>(m_maxHealth), healthBarYOffset + healthBarHeight * 0.5f)), Rgba8::GREEN);
	TransformVertexArrayXY3D(healthBarVerts, m_scale, 0.f, m_position);
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(healthBarVerts);
}

void Entity::TakeDamage(int damage)
{
	m_health -= damage;
}

void Entity::ReactToBulletHit(Bullet& bullet)
{
	TakeDamage(bullet.m_damage);
	bullet.Die();

	if (m_health <= 0)
	{
		Die();
		if (m_entityType != ENTITY_TYPE_GOOD_PLAYER)
		{
			g_game->PlaySound(ENTITY_DIE_SOUND, m_position);
		}
	}
	else
	{
 		g_game->PlaySound(ENEMY_HIT_SOUND, m_position);
	}
}

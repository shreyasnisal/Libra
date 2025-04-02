#include "Game/Bullet.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"

#include "Engine/Renderer/Spritesheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

SpriteAnimDefinition* Bullet::s_flamethrowerBulletAnimation = nullptr;

Bullet::Bullet(EntityType type, EntityFaction faction, Vec2 const& startPos, float orientationDegrees)
	: Entity(type, faction, startPos, orientationDegrees)
{
	m_isPushedByEntities = false;
	m_isPushedByWalls = false;
	m_pushesEntities = false;
	m_isHitByBullets = false;
	
	float speed = 0.f;
	if (type == ENTITY_TYPE_GOOD_BULLET || type == ENTITY_TYPE_EVIL_BULLET)
	{
		speed = g_gameConfigBlackboard.GetValue("defaultBulletSpeed", 5.f);
	}
	else if (type == ENTITY_TYPE_GOOD_BOLT || type == ENTITY_TYPE_EVIL_BOLT)
	{
		speed = g_gameConfigBlackboard.GetValue("defaultBoltSpeed", 6.f);
	}
	else if (type == ENTITY_TYPE_GOOD_FLAMETHROWER_BULLET)
	{
		speed = g_gameConfigBlackboard.GetValue("defaultFlamethrowerBulletSpeed", 3.f);
	}

	m_angularVelocity = g_RNG->RollRandomFloatInRange(-5.f, 5.f);
	m_velocity = Vec2::MakeFromPolarDegrees(orientationDegrees, speed);

	m_physicsRadius = 0.1f;
	m_cosmeticRadius = 0.1f;

	LoadAssets();
	InitializeVertexes();
}

void Bullet::CreateAnimations()
{
	s_flamethrowerBulletAnimation = new SpriteAnimDefinition(g_spriteSheets[EXPLOSION_SPRITESHEET], 0, 24, 1.f, SpriteAnimPlaybackType::LOOP);
}

void Bullet::LoadAssets()
{
	switch (m_entityType)
	{
		case ENTITY_TYPE_GOOD_BULLET:
		{
			m_baseTexture = g_textures[GOOD_BULLET_TEXTURE];
			break;
		}
		case ENTITY_TYPE_EVIL_BULLET:
		{
			m_baseTexture = g_textures[EVIL_BULLET_TEXTURE];
			break;
		}
		case ENTITY_TYPE_GOOD_BOLT:
		{
			m_baseTexture = g_textures[GOOD_BOLT_TEXTURE];
			break;
		}
		case ENTITY_TYPE_EVIL_BOLT:
		{
			m_baseTexture = g_textures[EVIL_BOLT_TEXTURE];
			break;
		}
		case ENTITY_TYPE_GOOD_FLAMETHROWER_BULLET:
		{
			m_baseTexture = g_spriteSheets[EXPLOSION_SPRITESHEET]->GetTexture();
			break;
		}
	}
}

void Bullet::InitializeVertexes()
{
	AABB2 bulletBaseVertexBounds(Vec2(-m_cosmeticRadius, -m_cosmeticRadius / 2.8f), Vec2(m_cosmeticRadius, m_cosmeticRadius / 2.8f));
	AddVertsForAABB2(m_baseVertexes, bulletBaseVertexBounds, Rgba8::WHITE);
}

void Bullet::Update(float deltaSeconds)
{
	if (m_isDead)
	{
		return;
	}
	if (m_position.x < 0 || m_position.x > m_map->GetDimensions().x || m_position.y < 0 || m_position.y > m_map->GetDimensions().y)
	{
		Die();
	}

	if (m_entityType == ENTITY_TYPE_EVIL_BULLET)
	{
		UpdateGuided(deltaSeconds);
		return;
	}
	else if (m_entityType == ENTITY_TYPE_GOOD_FLAMETHROWER_BULLET)
	{
		UpdateFlamethrowerBullet(deltaSeconds);
		return;
	}

	Vec2 prevPosition = m_position;
	m_position += m_velocity * deltaSeconds;

	if (m_map->IsPointInSolid(m_position))
	{
		if (m_map->IsPointInDestructibleTile(m_position))
		{
			int tileIndex = m_map->GetTileIndexFromTileCoords(RoundDownToInt(m_position.x), RoundDownToInt(m_position.y));
			m_map->m_tiles[tileIndex].TakeDamage(*this);
		}

		if (m_faction == FACTION_GOOD)
		{
			IntVec2 reflectionNormalIntVec2 = IntVec2(RoundDownToInt(m_position.x), RoundDownToInt(m_position.y)) - IntVec2(RoundDownToInt(prevPosition.x), RoundDownToInt(prevPosition.y));
			Vec2 reflectionNormal = Vec2(static_cast<float>(reflectionNormalIntVec2.x), static_cast<float>(reflectionNormalIntVec2.y)).GetNormalized(); 
			BounceOff(reflectionNormal);
		}
		else
		{
			Die();
		}
	}
}

void Bullet::UpdateGuided(float deltaSeconds)
{
	if (!m_map)
	{
		return;
	}

	if (m_map->IsPointInSolid(m_position))
	{
		Die();
	}

	Entity* player = m_map->m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][0];

	if (!g_game->IsEntityAlive(player))
	{
		Vec2 prevPosition = m_position;
		m_position += m_velocity * deltaSeconds;
		return;
	}

	float angularVelocity = g_gameConfigBlackboard.GetValue("guidedBulletTurnRate", 360.f);
	Vec2 directionToPlayer = (player->m_position - m_position).GetNormalized();
	float orientationToPlayer = directionToPlayer.GetOrientationDegrees();
	m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, orientationToPlayer, angularVelocity);
	m_position += GetForwardNormal() * m_velocity.GetLength() * deltaSeconds;
}

void Bullet::UpdateFlamethrowerBullet(float deltaSeconds)
{
	Vec2 prevPosition = m_position;
	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees += m_angularVelocity;

	if (m_map->IsPointInSolid(m_position))
	{
		Die();
	}

	m_flamethrowerBulletAge += deltaSeconds;
	if (m_flamethrowerBulletAge >= 1.f)
	{
		Die();
	}
}

void Bullet::Render() const
{
	std::vector<Vertex_PCU> worldBaseVertexes;
	if (m_entityType == ENTITY_TYPE_GOOD_FLAMETHROWER_BULLET)
	{
		SpriteDefinition currentSpriteDef = s_flamethrowerBulletAnimation->GetSpriteDefAtTime(m_flamethrowerBulletAge);
		AddVertsForAABB2(worldBaseVertexes, AABB2(Vec2(-m_cosmeticRadius * 3.f, -m_cosmeticRadius * 3.f), Vec2(m_cosmeticRadius * 3.f, m_cosmeticRadius * 3.f)), Rgba8::WHITE, currentSpriteDef.GetUVs().m_mins, currentSpriteDef.GetUVs().m_maxs);
		g_renderer->SetBlendMode(BlendMode::ADDITIVE);
	}
	else
	{
		worldBaseVertexes = m_baseVertexes;
	}

	TransformVertexArrayXY3D(worldBaseVertexes, 1.f, m_orientationDegrees, m_position);
	g_renderer->BindTexture(m_baseTexture);
	g_renderer->DrawVertexArray(worldBaseVertexes);
	g_renderer->SetBlendMode(BlendMode::ALPHA);

	if (g_game->m_drawDebug)
	{
		RenderDebug();
	}
}

void Bullet::BounceOff(Vec2 const& impactNormal)
{
	g_game->PlaySound(BULLET_BOUNCE_SOUND, m_position);

	m_velocity.Reflect(impactNormal);
	m_orientationDegrees = m_velocity.GetOrientationDegrees();
	float bounceVarianceDegrees = g_gameConfigBlackboard.GetValue("bulletBounceVarianceDegrees", 5.f);
	m_orientationDegrees += g_RNG->RollRandomFloatInRange(-bounceVarianceDegrees, bounceVarianceDegrees);
	m_velocity.SetOrientationDegrees(m_orientationDegrees);

	m_numWallsHit++;
	if (m_numWallsHit >= 3)
	{
		Die();
	}
}

void Bullet::Die()
{
	if (m_entityType != ENTITY_TYPE_GOOD_FLAMETHROWER_BULLET)
	{
		m_map->SpawnNewEntityOfType(ENTITY_TYPE_NEUTRAL_EXPLOSION, m_position, m_orientationDegrees, 0.4f, 0.4f);
	}
	Entity::Die();
}

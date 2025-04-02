#include "Game/Tile.hpp"

#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/TileDefinition.hpp"

Tile::Tile(std::string typeName, IntVec2 tileCoords)
{
	m_tileDefinition = &(TileDefinition::s_tileDefs.find(typeName)->second);
	m_tileCoords = tileCoords;
	m_health = m_tileDefinition->m_maxHealth;
}

Tile::Tile(std::string typeName, int x, int y)
{
	m_tileDefinition = &(TileDefinition::s_tileDefs.find(typeName)->second);
	m_tileCoords = IntVec2(x, y);
	m_health = m_tileDefinition->m_maxHealth;
}

AABB2 Tile::GetBounds() const
{
	AABB2 bounds;
	bounds.m_mins.x = static_cast<float>(m_tileCoords.x);
	bounds.m_mins.y = static_cast<float>(m_tileCoords.y);
	bounds.m_maxs.x = static_cast<float>(m_tileCoords.x + 1);
	bounds.m_maxs.y = static_cast<float>(m_tileCoords.y + 1);
	return bounds;
}

Rgba8 Tile::GetColor() const
{
	return m_tileDefinition->m_tint;
}

bool Tile::IsSolid() const
{
	return m_tileDefinition->m_isSolid;
}

bool Tile::IsWater() const
{
	return m_tileDefinition->m_isWater;
}

std::string Tile::GetType() const
{
	return m_tileDefinition->m_typeName;
}

AABB2 Tile::GetUVs() const
{
	return m_tileDefinition->m_uvs;
}

bool Tile::IsDestructible() const
{
	return m_tileDefinition->m_isDestructible;
}

void Tile::TakeDamage(Bullet& bullet)
{
	if (IsDestructible())
	{
		m_health -= bullet.m_damage;

		if (m_health <= 0)
		{
			m_tileDefinition = &(TileDefinition::s_tileDefs.find(m_tileDefinition->m_alternateTileType)->second);
			m_health = m_tileDefinition->m_maxHealth;
		}
	}
}

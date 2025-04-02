#pragma once

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <string>
#include <vector>

class Bullet;
struct TileDefinition;

class Tile
{
public:
	~Tile() = default;
	Tile() = default;
	Tile(std::string typeName, IntVec2 tileCoords);
	Tile(std::string typeName, int x, int y);

	AABB2									GetBounds() const;
	std::string								GetType() const;
	AABB2									GetUVs() const;
	Rgba8									GetColor() const;
	bool									IsSolid() const;
	bool									IsWater() const;
	bool									IsDestructible() const;
	void									TakeDamage(Bullet& bullet);

public:
	TileDefinition*							m_tileDefinition;
	IntVec2									m_tileCoords;
	int										m_health = 1;

private:

};
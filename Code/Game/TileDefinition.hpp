#pragma once

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <map>
#include <string>

struct TileDefinition
{
public:
	std::string											m_typeName;
	bool												m_isSolid = false;
	bool												m_isWater = false;
	bool												m_isDestructible = false;
	std::string											m_alternateTileType = "Grass";
	int													m_maxHealth = 5;
	AABB2												m_uvs = AABB2(Vec2::ZERO, Vec2::ONE);
	Rgba8												m_tint = Rgba8::WHITE;
	Rgba8												m_mapImageColor = Rgba8::TRANSPARENT_BLACK;

	// #TODO std::map may not be worth it, consider switching to a vector instead
	static std::map<std::string, TileDefinition>		s_tileDefs;

public:
	~TileDefinition() = default;
	TileDefinition() = default;
	explicit TileDefinition(XmlElement const* element);

	static void									InitializeTileDefitions();

};

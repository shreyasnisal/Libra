#pragma once

#include "Game/Entity.hpp"

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/XmlUtils.hpp"

#include <map>
#include <string>

struct MapDefinition
{
public:
	std::string m_name								= "";
	std::string m_mapImageName						= "";
	IntVec2 m_mapImageOffset						= IntVec2::ZERO;
	IntVec2 m_dimensions							= IntVec2::ZERO;
	std::string m_fillTileType						= "Grass";
	std::string m_edgeTileType						= "StoneWall";
	std::string m_worm1TileType						= "Grass";
	int m_worm1Count								= 0;
	int m_worm1MaxLength							= 0;
	std::string m_worm2TileType						= "Grass";
	int m_worm2Count								= 0;
	int m_worm2MaxLength							= 0;
	std::string m_startFloorTileType				= "StonePavers";
	std::string m_startBunkerTileType				= "IronWall";
	std::string m_endFloorTileType					= "StonePavers";
	std::string m_endBunkerTileType					= "IronWall";
	IntVec2 m_mapEntryTileCoords					= IntVec2(1, 1);
	std::string m_mapEntryTileType					= "MapEntry";
	IntVec2 m_mapExitTileCoords						= IntVec2(1, 1);
	std::string m_mapExitTileType					= "MapExit";
	int m_numEntitiesOfType[ENTITY_TYPE_NUM]		= {};

public:
	~MapDefinition() = default;
	MapDefinition() = default;
	MapDefinition(XmlElement const* element);

	static void										InitializeMapDefinitions();
	// #TODO std::map may not be worth it, consider switching to a vector instead
	static std::map<std::string, MapDefinition>		s_mapDefs;
};
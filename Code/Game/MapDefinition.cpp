#include "Game/MapDefinition.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/GameCommon.hpp"

std::map<std::string, MapDefinition> MapDefinition::s_mapDefs;

void MapDefinition::InitializeMapDefinitions()
{
	XmlDocument mapDefinitionsXmlFile("Data/Definitions/MapDefinitions.xml");
	XmlResult xmlFileLoadResult = mapDefinitionsXmlFile.LoadFile("Data/Definitions/MapDefinitions.xml");
	if (xmlFileLoadResult != XmlResult::XML_SUCCESS)
	{
		ERROR_AND_DIE("Could not find or open file Data/Definitions/MapDefinitions.xml");
	}

	XmlElement* mapDefinitionsXmlElement = mapDefinitionsXmlFile.RootElement();
	XmlElement* mapDefinitionXmlElement = mapDefinitionsXmlElement->FirstChildElement();

	while (mapDefinitionXmlElement)
	{
		MapDefinition mapDef = MapDefinition(mapDefinitionXmlElement);
		s_mapDefs[mapDef.m_name] = mapDef;
		mapDefinitionXmlElement = mapDefinitionXmlElement->NextSiblingElement();
	}
}

MapDefinition::MapDefinition(XmlElement const* element)
{
	m_name = ParseXmlAttribute(*element, "name", m_name);
	m_mapImageName = ParseXmlAttribute(*element, "mapImageName", m_mapImageName);
	m_mapImageOffset = ParseXmlAttribute(*element, "mapImageOffset", m_mapImageOffset);
	m_dimensions = ParseXmlAttribute(*element, "dimensions", m_dimensions);
	m_fillTileType = ParseXmlAttribute(*element, "fillTileType", m_fillTileType);
	m_edgeTileType = ParseXmlAttribute(*element, "edgeTileType", m_edgeTileType);
	m_worm1TileType = ParseXmlAttribute(*element, "worm1TileType", m_worm1TileType);
	m_worm1Count = ParseXmlAttribute(*element, "worm1Count", m_worm1Count);
	m_worm1MaxLength = ParseXmlAttribute(*element, "worm1MaxLength", m_worm1MaxLength);
	m_worm2TileType = ParseXmlAttribute(*element, "worm2TileType", m_worm2TileType);
	m_worm2Count = ParseXmlAttribute(*element, "worm2Count", m_worm2Count);
	m_worm2MaxLength = ParseXmlAttribute(*element, "worm2MaxLength", m_worm2MaxLength);
	m_startFloorTileType = ParseXmlAttribute(*element, "startFloorTileType", m_startFloorTileType);
	m_startBunkerTileType = ParseXmlAttribute(*element, "startBunkerTileType", m_startBunkerTileType);
	m_endFloorTileType = ParseXmlAttribute(*element, "endFloorTileType", m_endFloorTileType);
	m_endBunkerTileType = ParseXmlAttribute(*element, "endBunkerTileType", m_endBunkerTileType);
	m_mapEntryTileCoords = ParseXmlAttribute(*element, "mapEntryTileCoords", m_mapEntryTileCoords);
	m_mapEntryTileType = ParseXmlAttribute(*element, "mapEntryTileType", m_mapEntryTileType);
	m_mapExitTileCoords = ParseXmlAttribute(*element, "mapExitTileCoords", m_mapExitTileCoords);
	m_mapExitTileType = ParseXmlAttribute(*element, "mapExitTileType", m_mapExitTileType);
	m_numEntitiesOfType[ENTITY_TYPE_EVIL_LEO] =  ParseXmlAttribute(*element, "leoCount", m_numEntitiesOfType[ENTITY_TYPE_EVIL_LEO]);
	m_numEntitiesOfType[ENTITY_TYPE_EVIL_ARIES] =  ParseXmlAttribute(*element, "ariesCount", m_numEntitiesOfType[ENTITY_TYPE_EVIL_ARIES]);
	m_numEntitiesOfType[ENTITY_TYPE_EVIL_SCORPIO] =  ParseXmlAttribute(*element, "scorpioCount", m_numEntitiesOfType[ENTITY_TYPE_EVIL_SCORPIO]);
	m_numEntitiesOfType[ENTITY_TYPE_EVIL_CAPRICORN] =  ParseXmlAttribute(*element, "capricornCount", m_numEntitiesOfType[ENTITY_TYPE_EVIL_CAPRICORN]);
	//m_numEntitiesOfType[ENTITY_TYPE_EVIL_] =  0;
}
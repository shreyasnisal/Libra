#include "Game/TileDefinition.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/GameCommon.hpp"

std::map<std::string, TileDefinition> TileDefinition::s_tileDefs;

void TileDefinition::InitializeTileDefitions()
{
	XmlDocument tileDefsXmlFile("Data/Definitions/TileDefinitions.xml");
	XmlResult fileLoadResult = tileDefsXmlFile.LoadFile("Data/Definitions/TileDefinitions.xml");

	if (fileLoadResult != XmlResult::XML_SUCCESS)
	{
		ERROR_AND_DIE("Could not find or open file \"Data/Definitions/TileDefinitions.xml\"");
	}

	XmlElement* tileDefinitionsXmlElement = tileDefsXmlFile.RootElement();
	XmlElement* tileDefinitionXmlElement = tileDefinitionsXmlElement->FirstChildElement();

	while (tileDefinitionXmlElement)
	{
		TileDefinition tileDef(tileDefinitionXmlElement);
		s_tileDefs[tileDef.m_typeName] = tileDef;
		tileDefinitionXmlElement = tileDefinitionXmlElement->NextSiblingElement();
	}
}

TileDefinition::TileDefinition(XmlElement const* element)
{
	m_typeName = ParseXmlAttribute(*element, "name", "INVALID_TILE_TYPE");
	IntVec2 spriteCoords = ParseXmlAttribute(*element, "spriteCoords", IntVec2::ZERO);
	m_uvs = g_terrainSpriteSheet->GetSpriteUVs(spriteCoords.x + 8 * spriteCoords.y);
	m_isSolid = ParseXmlAttribute(*element, "isSolid", false);
	m_isWater = ParseXmlAttribute(*element, "isWater", false);
	m_isDestructible = ParseXmlAttribute(*element, "isDestructible", m_isDestructible);
	m_alternateTileType = ParseXmlAttribute(*element, "alternateTileType", m_alternateTileType);
	m_maxHealth = ParseXmlAttribute(*element, "maxHealth", m_maxHealth);
	m_tint = ParseXmlAttribute(*element, "tint", Rgba8::WHITE);
	m_mapImageColor = ParseXmlAttribute(*element, "mapColor", m_mapImageColor);
}

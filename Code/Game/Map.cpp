#include "Game/Map.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Game/Player.hpp"
#include "Game/Bullet.hpp"
#include "Game/Scorpio.hpp"
#include "Game/Aries.hpp"
#include "Game/Leo.hpp"
#include "Game/Capricorn.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Explosion.hpp"

#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include <queue>

SpriteSheet* g_terrainSpriteSheet = nullptr;

Map::Map(MapDefinition mapDef)
{
	if (mapDef.m_mapExitTileCoords == IntVec2(1, 1))
	{
		mapDef.m_mapExitTileCoords = IntVec2(mapDef.m_dimensions.x - 2, mapDef.m_dimensions.y - 2);
	}
	m_mapDef = mapDef;

	LoadAssets();


	int numTriesForMapGeneration = 0;
	do
	{
		numTriesForMapGeneration++;
		PopulateTiles();

		if (!m_mapDef.m_mapImageName.empty())
		{
			ConstructMapFromImage();
		}

		GenerateHeatMaps();
		for (int tileIndex = 0; tileIndex < GetDimensions().x * GetDimensions().y; tileIndex++)
		{
			IntVec2 tileCoords(tileIndex % GetDimensions().x, tileIndex / GetDimensions().x);
			if (m_startDistanceField.GetValueAtTile(tileCoords) == SPECIAL_VALUE_FOR_HEATMAPS && !IsTileSolid(tileCoords) && !IsTileWater(tileCoords))
			{
				SetTileType(tileCoords, mapDef.m_edgeTileType);
			}
		}
		GenerateHeatMaps();
	} while (!IsMapValid() && numTriesForMapGeneration < 100);
	if (numTriesForMapGeneration == 100)
	{
		ERROR_AND_DIE(Stringf("Could not generate valid map of type %s in %d attempts!", m_mapDef.m_name.c_str(), numTriesForMapGeneration));
	}
	DebuggerPrintf("Map of type %s took %d attempts to generate!\n", m_mapDef.m_name.c_str(), numTriesForMapGeneration);

	SpawnEntities();
}

void Map::LoadAssets()
{
	
}

void Map::ConstructMapFromImage()
{
	Image mapImage = Image(m_mapDef.m_mapImageName.c_str());
	
	int numTiles = m_mapDef.m_dimensions.x * m_mapDef.m_dimensions.y;
	m_tiles.resize(numTiles);

	for (int tileY = m_mapDef.m_mapImageOffset.y; tileY < GetDimensions().y; tileY++)
	{
		for (int tileX = m_mapDef.m_mapImageOffset.x; tileX < GetDimensions().x; tileX++)
		{
			if (tileX - m_mapDef.m_mapImageOffset.x >= mapImage.GetDimensions().x || tileY - m_mapDef.m_mapImageOffset.y >= mapImage.GetDimensions().y)
			{
				continue;
			}

			Rgba8 mapImageTexelColor = mapImage.GetTexelColor(IntVec2(tileX, tileY) - m_mapDef.m_mapImageOffset);

			if (mapImageTexelColor.a == 0)
			{
				continue;
			}

			bool wasMatchingTileFound = false;
			for (auto tileDefIter = TileDefinition::s_tileDefs.begin(); tileDefIter != TileDefinition::s_tileDefs.end(); tileDefIter++) {
				if (tileDefIter->second.m_mapImageColor.r == mapImageTexelColor.r && tileDefIter->second.m_mapImageColor.g == mapImageTexelColor.g && tileDefIter->second.m_mapImageColor.b == mapImageTexelColor.b)
				{
					wasMatchingTileFound = true;
					if (g_RNG->RollRandomIntInRange(0, 254) < static_cast<int>(mapImageTexelColor.a))
					{
						SetTileType(IntVec2(tileX, tileY), tileDefIter->first);
					}
				}
			}
			if (!wasMatchingTileFound)
			{
				ERROR_AND_DIE(Stringf("Could not find Tile Definition matching image texel color at (%d, %d)", tileX, tileY));
			}
		}
	}
}

void Map::PopulateTiles()
{
	int startAreaSize = g_gameConfigBlackboard.GetValue("startAreaSize", 5);
	int endAreaSize = g_gameConfigBlackboard.GetValue("endAreaSize", 6);

	int numTiles = m_mapDef.m_dimensions.x * m_mapDef.m_dimensions.y;
	m_tiles.resize(numTiles);

	for (int tileY = 0; tileY < GetDimensions().y; tileY++)
	{
		for (int tileX = 0; tileX < GetDimensions().x; tileX++)
		{
			SetTileType(IntVec2(tileX, tileY), m_mapDef.m_fillTileType);
		}
	}

	for (int wormIndex = 0; wormIndex < m_mapDef.m_worm1Count; wormIndex++)
	{
		int wormLength = m_mapDef.m_worm1MaxLength;
		IntVec2 wormTileCoords = IntVec2(g_RNG->RollRandomIntInRange(0, GetDimensions().x - 1), g_RNG->RollRandomIntInRange(0, GetDimensions().y - 1));
		SetTileType(wormTileCoords, m_mapDef.m_worm1TileType);
		wormLength--;
		while(wormLength--)
		{
			int nextTileDirection = g_RNG->RollRandomIntInRange(0, 3);
			switch (nextTileDirection)
			{
				case 0:
				{
					wormTileCoords += IntVec2::NORTH;
					break;
				}
				case 1:
				{
					wormTileCoords += IntVec2::SOUTH;
					break;
				}
				case 2:
				{
					wormTileCoords += IntVec2::EAST;
					break;
				}
				case 3:
				{
					wormTileCoords += IntVec2::WEST;
					break;
				}
			}
			if (wormTileCoords.x < 0 || wormTileCoords.x > GetDimensions().x - 1 || wormTileCoords.y < 0 || wormTileCoords.y > GetDimensions().y - 1)
			{
				continue;
			}
			SetTileType(wormTileCoords, m_mapDef.m_worm1TileType);
		}
	}

	for (int wormIndex = 0; wormIndex < m_mapDef.m_worm2Count; wormIndex++)
	{
		int wormLength = m_mapDef.m_worm2MaxLength;
		IntVec2 wormTileCoords = IntVec2(g_RNG->RollRandomIntInRange(0, GetDimensions().x - 1), g_RNG->RollRandomIntInRange(0, GetDimensions().y - 1));
		SetTileType(wormTileCoords, m_mapDef.m_worm2TileType);
		wormLength--;
		while (wormLength--)
		{
			int nextTileDirection = g_RNG->RollRandomIntInRange(0, 3);
			switch (nextTileDirection)
			{
				case 0:
				{
					wormTileCoords += IntVec2::NORTH;
					break;
				}
				case 1:
				{
					wormTileCoords += IntVec2::SOUTH;
					break;
				}
				case 2:
				{
					wormTileCoords += IntVec2::EAST;
					break;
				}
				case 3:
				{
					wormTileCoords += IntVec2::WEST;
					break;
				}
			}
			if (wormTileCoords.x < 0 || wormTileCoords.x > GetDimensions().x - 1 || wormTileCoords.y < 0 || wormTileCoords.y > GetDimensions().y - 1)
			{
				continue;
			}
			SetTileType(wormTileCoords, m_mapDef.m_worm2TileType);
		}
	}

	for (int tileY = 0; tileY < startAreaSize + 1; tileY++)
	{
		for (int tileX = 0; tileX < startAreaSize + 1; tileX++)
		{
			SetTileType(IntVec2(tileX, tileY), m_mapDef.m_startFloorTileType);
		}
	}

	for (int tileY = GetDimensions().y - endAreaSize - 1; tileY < GetDimensions().y; tileY++)
	{
		for (int tileX = GetDimensions().x - endAreaSize - 1 ; tileX < GetDimensions().x; tileX++)
		{
			SetTileType(IntVec2(tileX, tileY), m_mapDef.m_endFloorTileType);
		}
	}

	for (int tileY = 0; tileY < startAreaSize; tileY++)
	{
		for (int tileX = 0; tileX < startAreaSize; tileX++)
		{
			if ((tileY == startAreaSize - 1 && tileX > 1 && tileX < startAreaSize) || (tileX == startAreaSize - 1 && tileY > 1 && tileY < startAreaSize))
			{
				SetTileType(IntVec2(tileX, tileY), m_mapDef.m_startBunkerTileType);
			}
		}
	}

	for (int tileY = GetDimensions().y - endAreaSize - 1; tileY < GetDimensions().y; tileY++)
	{
		for (int tileX = GetDimensions().x - endAreaSize - 1; tileX < GetDimensions().x; tileX++)
		{
			if ((tileY == GetDimensions().y - endAreaSize && GetDimensions().x - tileX < endAreaSize + 1 && tileX != GetDimensions().x - 2) || (tileX == m_mapDef.m_dimensions.x - endAreaSize) && (GetDimensions().y - tileY < endAreaSize && tileY != GetDimensions().y - 2))
			{
				SetTileType(IntVec2(tileX, tileY), m_mapDef.m_endBunkerTileType);
			}	
		}
	}

	for (int tileY = 0; tileY < GetDimensions().y; tileY++)
	{
		for (int tileX = 0; tileX < GetDimensions().x; tileX++)
		{
			if (tileX == 0 || tileX == GetDimensions().x - 1 || tileY == 0 || tileY == GetDimensions().y - 1)
			{
				SetTileType(IntVec2(tileX, tileY), m_mapDef.m_edgeTileType);
			}
		}
	}

	SetTileType(m_mapDef.m_mapEntryTileCoords, m_mapDef.m_mapEntryTileType);
	SetTileType(m_mapDef.m_mapExitTileCoords, m_mapDef.m_mapExitTileType);
}

void Map::SetTileType(IntVec2 const& tileCoords, std::string tileTypeName)
{
	int tileIndex = tileCoords.x + tileCoords.y * GetDimensions().x;
	m_tiles[tileIndex] = Tile(tileTypeName, tileCoords.x, tileCoords.y);
}

void Map::PopulateDistanceField(TileHeatMap& out_distanceField, IntVec2 const& startCoords, float maxCost, bool treatWaterAsSolid, bool treatScorpioAsSolid, bool treatDestructibleTilesAsSolid)
{
	IntVec2 mapSize = GetDimensions();

	std::vector<float> distanceFieldValues(mapSize.x * mapSize.y, maxCost);
	std::queue<IntVec2> nextTiles;

	int startTileIndex = startCoords.x + startCoords.y * mapSize.x;
	distanceFieldValues[startTileIndex] = 0;
	nextTiles.push(startCoords);

	while (!nextTiles.empty())
	{
		IntVec2 currentTile = nextTiles.front();
		int currentTileIndex = currentTile.x + currentTile.y * mapSize.x;
		nextTiles.pop();
		
		IntVec2 southTile = currentTile + IntVec2::SOUTH;
		int southTileIndex = southTile.x + southTile.y * mapSize.x;
		if (southTile.y >= 0 && distanceFieldValues[southTileIndex] > distanceFieldValues[currentTileIndex] + 1 && IsTileTraversable(southTile, treatWaterAsSolid, treatScorpioAsSolid, treatDestructibleTilesAsSolid))
		{
			distanceFieldValues[southTileIndex] = distanceFieldValues[currentTileIndex] + 1;
			nextTiles.push(southTile);
		}

		IntVec2 westTile = currentTile + IntVec2::WEST;
		int westTileIndex = westTile.x + westTile.y * mapSize.x;
		if (westTile.x >= 0 && distanceFieldValues[westTileIndex] > distanceFieldValues[currentTileIndex] + 1 && IsTileTraversable(westTile, treatWaterAsSolid, treatScorpioAsSolid, treatDestructibleTilesAsSolid))
		{
			distanceFieldValues[westTileIndex] = distanceFieldValues[currentTileIndex] + 1;
			nextTiles.push(westTile);
		}

		IntVec2 northTile = currentTile + IntVec2::NORTH;
		int northTileIndex = northTile.x + northTile.y * mapSize.x;
		if (northTile.y < GetDimensions().y && distanceFieldValues[northTileIndex] > distanceFieldValues[currentTileIndex] + 1 && IsTileTraversable(northTile, treatWaterAsSolid, treatScorpioAsSolid, treatDestructibleTilesAsSolid))
		{
			distanceFieldValues[northTileIndex] = distanceFieldValues[currentTileIndex] + 1;
			nextTiles.push(northTile);
		}

		IntVec2 eastTile = currentTile + IntVec2::EAST;
		int eastTileIndex = eastTile.x + eastTile.y * mapSize.x;
		if (eastTile.x < GetDimensions().x && distanceFieldValues[eastTileIndex] > distanceFieldValues[currentTileIndex] + 1 && IsTileTraversable(eastTile, treatWaterAsSolid, treatScorpioAsSolid, treatDestructibleTilesAsSolid))
		{
			distanceFieldValues[eastTileIndex] = distanceFieldValues[currentTileIndex] + 1;
			nextTiles.push(eastTile);
		}
	}

	out_distanceField.SetAllValues(distanceFieldValues);
}

float Map::GetMaxHeatValueExceptSpecialValueOnTileHeatMap(TileHeatMap const& heatMap, float specialValue) const
{
	float maxHeatValue = 0.f;
	for (int tileIndex = 0; tileIndex < static_cast<int>(heatMap.m_values.size()); tileIndex++)
	{
		if (heatMap.m_values[tileIndex] != specialValue && heatMap.m_values[tileIndex] > maxHeatValue)
		{
			maxHeatValue = heatMap.m_values[tileIndex];
		}
	}

	return maxHeatValue;
}

void Map::GenerateHeatMaps()
{
	m_startDistanceField = TileHeatMap(GetDimensions());
	m_solidMapForLand = TileHeatMap(GetDimensions());
	m_solidMapForAmphibian = TileHeatMap(GetDimensions());

	for (int tileIndex = 0; tileIndex < GetDimensions().x * GetDimensions().y; tileIndex++)
	{
		IntVec2 tileCoords = IntVec2(tileIndex % GetDimensions().x, tileIndex / GetDimensions().x);
		if (!IsTileSolid(tileCoords) && !IsTileWater(tileCoords))
		{
			m_solidMapForLand.SetValueAtTile(1.f, tileCoords);
		}
		else
		{
			m_solidMapForLand.SetValueAtTile(SPECIAL_VALUE_FOR_HEATMAPS, tileCoords);
		}
	}

	for (int tileIndex = 0; tileIndex < GetDimensions().x * GetDimensions().y; tileIndex++)
	{
		IntVec2 tileCoords = IntVec2(tileIndex % GetDimensions().x, tileIndex / GetDimensions().x);
		if (!IsTileSolid(tileCoords))
		{
			m_solidMapForAmphibian.SetValueAtTile(1.f, tileCoords);
		}
		else
		{
			m_solidMapForAmphibian.SetValueAtTile(SPECIAL_VALUE_FOR_HEATMAPS, tileCoords);
		}
	}

	PopulateDistanceField(m_startDistanceField, IntVec2(1, 1), SPECIAL_VALUE_FOR_HEATMAPS);
}

bool Map::IsMapValid()
{
	return m_startDistanceField.GetValueAtTile(m_mapDef.m_mapExitTileCoords) != SPECIAL_VALUE_FOR_HEATMAPS;
}

bool Map::IsEntityAlive(Entity* entity) const
{
	return entity && !entity->m_isDead;
}

Entity* Map::SpawnNewEntityOfType(EntityType type, Vec2 const& position, float orientationDegrees, float scale, float lifetimeSeconds)
{
	Entity* entity = CreateNewEntityOfType(type, position, orientationDegrees, scale, lifetimeSeconds);
	AddEntityToMap(entity);
	return entity;
}

Entity* Map::CreateNewEntityOfType(EntityType type, Vec2 const& position, float orientationDegrees, float scale, float lifetimeSeconds)
{
	switch(type)
	{
		case ENTITY_TYPE_EVIL_SCORPIO:					return new Scorpio(ENTITY_TYPE_EVIL_SCORPIO, FACTION_EVIL, position, orientationDegrees);
		case ENTITY_TYPE_EVIL_LEO:						return new Leo(ENTITY_TYPE_EVIL_LEO, FACTION_EVIL, position, orientationDegrees);
		case ENTITY_TYPE_EVIL_ARIES:					return new Aries(ENTITY_TYPE_EVIL_ARIES, FACTION_EVIL, position, orientationDegrees);
		case ENTITY_TYPE_EVIL_CAPRICORN:				return new Capricorn(ENTITY_TYPE_EVIL_CAPRICORN, FACTION_EVIL, position, orientationDegrees);
		case ENTITY_TYPE_GOOD_BULLET:					return new Bullet(ENTITY_TYPE_GOOD_BULLET, FACTION_GOOD, position, orientationDegrees);
		case ENTITY_TYPE_GOOD_BOLT:						return new Bullet(ENTITY_TYPE_GOOD_BOLT, FACTION_GOOD, position, orientationDegrees);
		case ENTITY_TYPE_EVIL_BULLET:					return new Bullet(ENTITY_TYPE_EVIL_BULLET, FACTION_EVIL, position, orientationDegrees);
		case ENTITY_TYPE_EVIL_BOLT:						return new Bullet(ENTITY_TYPE_EVIL_BOLT, FACTION_EVIL, position, orientationDegrees);
		case ENTITY_TYPE_GOOD_FLAMETHROWER_BULLET:		return new Bullet(ENTITY_TYPE_GOOD_FLAMETHROWER_BULLET, FACTION_GOOD, position, orientationDegrees);
		case ENTITY_TYPE_GOOD_PLAYER:					return new Player(ENTITY_TYPE_GOOD_PLAYER, FACTION_GOOD, position, orientationDegrees);
		case ENTITY_TYPE_NEUTRAL_EXPLOSION:				return new Explosion(ENTITY_TYPE_NEUTRAL_EXPLOSION, FACTION_NEUTRAL, position, orientationDegrees, scale, lifetimeSeconds);
	}

	return nullptr;
}

void Map::AddEntityToMap(Entity* entity)
{
	entity->m_map = this;
	AddEntityToList(entity, m_allEntities);
	AddEntityToList(entity, m_entitiesByType[entity->m_entityType]);

	if (entity->m_entityType == ENTITY_TYPE_GOOD_BULLET || entity->m_entityType == ENTITY_TYPE_GOOD_BOLT || entity->m_entityType == ENTITY_TYPE_GOOD_FLAMETHROWER_BULLET)
	{
		AddEntityToList(entity, m_bulletsByFaction[FACTION_GOOD]);
	}
	else if (entity->m_entityType == ENTITY_TYPE_EVIL_BULLET || entity->m_entityType == ENTITY_TYPE_EVIL_BOLT)
	{
		AddEntityToList(entity, m_bulletsByFaction[FACTION_EVIL]);
	}
	else
	{
		AddEntityToList(entity, m_actorsByFaction[entity->m_faction]);
	}
}

void Map::RemoveEntityFromMap(Entity* entity)
{
	entity->m_map = nullptr;
	
	if (entity->m_faction == FACTION_EVIL && m_actorsByFaction[FACTION_EVIL][m_selectedEntityIndexForHeatMapDebugDraw] == entity)
	{
		m_selectedEntityIndexForHeatMapDebugDraw = 0;
		m_currentTileHeatMap = nullptr;
	}

	RemoveEntityFromList(entity, m_allEntities);
	RemoveEntityFromList(entity, m_entitiesByType[entity->m_entityType]);
	if (entity->m_entityType == ENTITY_TYPE_GOOD_BULLET || entity->m_entityType == ENTITY_TYPE_GOOD_BOLT)
	{
		RemoveEntityFromList(entity, m_bulletsByFaction[FACTION_GOOD]);
	}
	else if (entity->m_entityType == ENTITY_TYPE_EVIL_BULLET || entity->m_entityType == ENTITY_TYPE_EVIL_BOLT)
	{
		RemoveEntityFromList(entity, m_bulletsByFaction[FACTION_EVIL]);
	}
	else
	{
		RemoveEntityFromList(entity, m_actorsByFaction[entity->m_faction]);
	}
}

Vec2 Map::GetNewEntitySpawnPosition() const
{
	Vec2 spawnPosition;
	do
	{
		int spawnTileX = g_RNG->RollRandomIntInRange(0, GetDimensions().x - 1);
		int spawnTileY = g_RNG->RollRandomIntInRange(0, GetDimensions().y - 1);
		spawnPosition = Vec2(static_cast<float>(spawnTileX) + 0.5f, static_cast<float>(spawnTileY) + 0.5f);
	} while (IsPointInSolid(spawnPosition) || IsPointInWater(spawnPosition) || IsPointInBunker(spawnPosition));

	return spawnPosition;
}

void Map::SpawnEntities()
{
	for (int entityListIndex = 0; entityListIndex < ENTITY_TYPE_NUM; entityListIndex++)
	{
		for (int entityIndex = 0; entityIndex < m_mapDef.m_numEntitiesOfType[entityListIndex]; entityIndex++)
		{
			Vec2 spawnPosition = GetNewEntitySpawnPosition();
			float spawnOrientationDegrees = g_RNG->RollRandomFloatInRange(0.f, 360.f);

			SpawnNewEntityOfType(EntityType(entityListIndex), spawnPosition, spawnOrientationDegrees);
		}
	}
}

void Map::AddEntityToList(Entity* entity, EntityList& entities)
{
	for (int entityIndex = 0; entityIndex < static_cast<int>(entities.size()); entityIndex++)
	{
		if (entities[entityIndex] == nullptr)
		{
			entities[entityIndex] = entity;
			return;
		}
	}

	entities.push_back(entity);
}

void Map::RemoveEntityFromList(Entity* entity, EntityList& entities)
{
	for (int entityIndex = 0; entityIndex < static_cast<int>(entities.size()); entityIndex++)
	{
		if (entities[entityIndex] && entities[entityIndex] == entity)
		{
			entities[entityIndex] = nullptr;
			return;
		}
	}
}

bool Map::IsPointInSolid(Vec2 const& point) const
{
	return IsTileSolid(IntVec2(RoundDownToInt(point.x), RoundDownToInt(point.y)));
}

bool Map::IsPointInWater(Vec2 const& point) const
{
	return IsTileWater(IntVec2(RoundDownToInt(point.x), RoundDownToInt(point.y)));
}

bool Map::IsPointInDestructibleTile(Vec2 const& point) const
{
	return IsTileDestructible(IntVec2(RoundDownToInt(point.x), RoundDownToInt(point.y)));
}

bool Map::IsPointInBunker(Vec2 const& point) const
{
	int startAreaSize = g_gameConfigBlackboard.GetValue("startAreaSize", 5);
	int endAreaSize = g_gameConfigBlackboard.GetValue("endAreaSize", 6);
	return ((point.x < static_cast<float>(startAreaSize)) && (point.y < static_cast<float>(startAreaSize))) || ((point.x > static_cast<float>(endAreaSize)) && (point.y > static_cast<float>(endAreaSize)));
}

bool Map::IsTileSolid(IntVec2 const& tileCoordinates) const
{
	if (tileCoordinates.x < 0 || tileCoordinates.x > GetDimensions().x - 1 || tileCoordinates.y < 0 || tileCoordinates.y > GetDimensions().y - 1)
	{
		return false;
	}

	int tileIndex = tileCoordinates.x + tileCoordinates.y * m_mapDef.m_dimensions.x;
	return m_tiles[tileIndex].IsSolid();
}

bool Map::IsTileWater(IntVec2 const& tileCoordinates) const
{
	if (tileCoordinates.x < 0 || tileCoordinates.x > GetDimensions().x - 1 || tileCoordinates.y < 0 || tileCoordinates.y > GetDimensions().y - 1)
	{
		return false;
	}
	int tileIndex = tileCoordinates.x + tileCoordinates.y * m_mapDef.m_dimensions.x;
	return m_tiles[tileIndex].IsWater();
}

bool Map::IsTileDestructible(IntVec2 const& tileCoordinates) const
{
	if (tileCoordinates.x < 0 || tileCoordinates.x > GetDimensions().x - 1 || tileCoordinates.y < 0 || tileCoordinates.y > GetDimensions().y - 1)
	{
		return false;
	}
	int tileIndex = tileCoordinates.x + tileCoordinates.y * m_mapDef.m_dimensions.x;
	return m_tiles[tileIndex].IsDestructible();
}

bool Map::IsTileTraversable(IntVec2 const& tileCoordinates, bool treatWaterAsSolid, bool treatScorpioAsSolid, bool treatDestructibleTilesAsSolid) const
{
	if (IsTileSolid(tileCoordinates))
	{
		if (!IsTileDestructible(tileCoordinates))
		{
			return false;
		}
		else
		{
			return !treatDestructibleTilesAsSolid;
		}
	}

	if (IsTileWater(tileCoordinates))
	{
		return !treatWaterAsSolid;
	}

	bool isOccupiedByScorpio = false;
	for (int scorpioIndex = 0; scorpioIndex < static_cast<int>(m_entitiesByType[ENTITY_TYPE_EVIL_SCORPIO].size()); scorpioIndex++)
	{
		if (!m_entitiesByType[ENTITY_TYPE_EVIL_SCORPIO][scorpioIndex])
		{
			continue;
		}
		Entity* const& scorpio = m_entitiesByType[ENTITY_TYPE_EVIL_SCORPIO][scorpioIndex];
		if (IntVec2(RoundDownToInt(scorpio->m_position.x), RoundDownToInt(scorpio->m_position.y)) == tileCoordinates)
		{
			isOccupiedByScorpio = true;
		}
	}

	if (isOccupiedByScorpio)
	{
		return !treatScorpioAsSolid;
	}

	return true;
}

RaycastResult2D Map::StepAndSampleRaycastVsTiles(Vec2 const& rayStartPosition, Vec2 const& rayFwdNormal, float maxRayLength) const
{
	constexpr int STEPS_PER_UNIT = 100;
	float stepSize = maxRayLength / STEPS_PER_UNIT;

	RaycastResult2D raycastResult;

	Vec2 currentPos = rayStartPosition;
	Vec2 prevPos = rayStartPosition;

	if (IsPointInSolid(rayStartPosition))
	{
		raycastResult.m_impactPosition = rayStartPosition;
		raycastResult.m_didImpact = true;
		raycastResult.m_impactNormal = -rayFwdNormal;
		raycastResult.m_impactDistance = 0.f;
	}

	for (int i = 0; static_cast<float>(i) * stepSize < maxRayLength; i++)
	{
		currentPos += rayFwdNormal * stepSize;

		if (IsPointInSolid(currentPos))
		{
			raycastResult.m_didImpact = true;
			raycastResult.m_impactPosition = currentPos;
			raycastResult.m_impactDistance = stepSize * i;
			// #TODO: impact normal
			return raycastResult;
		}
	}

	return raycastResult;
}

RaycastResult2D Map::BetterMoreConfusingRaycastVsTiles(Vec2 const& startPosition, Vec2 const& direction, float maxDistance) const
{
	RaycastResult2D raycastResult;

	if (maxDistance == 0)
	{
		return raycastResult;
	}

	int dimensionX = GetDimensions().x;
	int dimensionY = GetDimensions().y;

	IntVec2 currentTile = IntVec2(RoundDownToInt(startPosition.x), RoundDownToInt(startPosition.y));
	Vec2 rayStepSize = Vec2(direction.x != 0 ? 1.f / fabsf(direction.x) : 99999.f, direction.y != 0 ? 1.f / fabsf(direction.y) : 99999.f);
	Vec2 cumulativeRayLengthIn1D;
	IntVec2 directionXY;
	float totalRayLength = 0.f;

	float deltaRayLength = 0.f;

	if (direction.x < 0.f)
	{
		directionXY.x = -1;
		cumulativeRayLengthIn1D.x = (startPosition.x - static_cast<float>(currentTile.x)) * rayStepSize.x;
	}
	else
	{
		directionXY.x = 1;
		cumulativeRayLengthIn1D.x = (static_cast<float>(currentTile.x) + 1.f - startPosition.x) * rayStepSize.x;
	}

	if (direction.y < 0)
	{
		directionXY.y = -1;
		cumulativeRayLengthIn1D.y = (startPosition.y - static_cast<float>(currentTile.y)) * rayStepSize.y;
	}
	else
	{
		directionXY.y = 1;
		cumulativeRayLengthIn1D.y = (static_cast<float>(currentTile.y) + 1.f - startPosition.y) * rayStepSize.y;
	}

	while (totalRayLength < maxDistance)
	{
		int tileIndex = currentTile.x + dimensionX * currentTile.y;

		if (currentTile.x < 0 || currentTile.y < 0 || currentTile.x > dimensionX - 1 || currentTile.y > dimensionY - 1)
		{
			return raycastResult;
		}

		if (m_tiles[tileIndex].IsSolid())
		{
			Vec2 hitPoint = startPosition + direction * (totalRayLength - deltaRayLength);
			g_renderer->BindTexture(nullptr);
			raycastResult.m_didImpact = true;
			raycastResult.m_impactDistance = totalRayLength - deltaRayLength;
			raycastResult.m_impactPosition = hitPoint;
			return raycastResult;
		}

		if (cumulativeRayLengthIn1D.x < cumulativeRayLengthIn1D.y)
		{
			currentTile.x += directionXY.x;
			totalRayLength = cumulativeRayLengthIn1D.x;
			cumulativeRayLengthIn1D.x += rayStepSize.x;
		}
		else
		{
			currentTile.y += directionXY.y;
			totalRayLength = cumulativeRayLengthIn1D.y;
			cumulativeRayLengthIn1D.y += rayStepSize.y;
		}
	}

	raycastResult.m_didImpact = false;
	raycastResult.m_impactPosition = startPosition + direction * maxDistance;
	raycastResult.m_impactDistance = maxDistance;
	return raycastResult;
}

bool Map::HasLineOfSight(Vec2 const& startPosition, Vec2 const& endPosition) const
{
	Vec2 rayDirection = (endPosition - startPosition).GetNormalized();
	float maxRayLength = GetDistance2D(endPosition, startPosition);
	//RaycastResult2D raycastResult = StepAndSampleRaycastVsTiles(startPosition, rayDirection, maxRayLength);
	RaycastResult2D raycastResult = BetterMoreConfusingRaycastVsTiles(startPosition, rayDirection, maxRayLength);

	return !raycastResult.m_didImpact;
}

void Map::Update(float deltaSeconds)
{
	UpdateEntities(deltaSeconds);
	PushEntitiesOutOfEachOther();
	PushEntitiesOutOfWalls();
	CheckForBulletHits();
	DeleteGarbageEntities();
}

void Map::PushEntitiesOutOfEachOther()
{
	for (int entity1Index = 0; entity1Index < static_cast<int>(m_allEntities.size()); entity1Index++)
	{
		Entity*& entity1 = m_allEntities[entity1Index];
		if (!IsEntityAlive(entity1))
		{
			continue;
		}

		for (int entity2Index = entity1Index + 1; entity2Index < static_cast<int>(m_allEntities.size()); entity2Index++)
		{
			Entity*& entity2 = m_allEntities[entity2Index];
			if (!IsEntityAlive(entity2))
			{
				continue;
			}

			PushEntityOutOfEntity(*entity1, *entity2);
		}
	}
}

void Map::PushEntityOutOfEntity(Entity& entityA, Entity& entityB)
{
	bool canAPushB = entityA.m_pushesEntities && entityB.m_isPushedByEntities;
	bool canBPushA = entityB.m_pushesEntities && entityA.m_isPushedByEntities;

	if (!canAPushB && !canBPushA)
	{
		return;
	}

	if (canAPushB && canBPushA)
	{
		PushDiscsOutOfEachOther2D(entityA.m_position, entityA.m_physicsRadius, entityB.m_position, entityB.m_physicsRadius);
	}
	else if (canAPushB)
	{
		PushDiscOutOfFixedDisc2D(entityB.m_position, entityA.m_physicsRadius, entityA.m_position, entityA.m_physicsRadius);
	}
	else if (canBPushA)
	{
		PushDiscOutOfFixedDisc2D(entityA.m_position, entityA.m_physicsRadius, entityB.m_position, entityB.m_physicsRadius);
	}
}

void Map::PushEntitiesOutOfWalls()
{
	for (int entityIndex = 0; entityIndex < static_cast<int>(m_allEntities.size()); entityIndex++)
	{
		if (!IsEntityAlive(m_allEntities[entityIndex]))
		{
			continue;
		}

		if (m_allEntities[entityIndex]->m_entityType == ENTITY_TYPE_GOOD_PLAYER && g_game->m_isNoClipActive)
		{
			continue;
		}
		if (!m_allEntities[entityIndex]->m_isPushedByWalls)
		{
			continue;
		}
		PushEntityOutOfWalls(m_allEntities[entityIndex]);
	}
}

void Map::PushEntityOutOfWalls(Entity* entity)
{
	IntVec2 tileCoords = IntVec2(RoundDownToInt(entity->m_position.x), RoundDownToInt(entity->m_position.y));
	
	PushEntityOutOfTileIfSolid(entity, tileCoords + IntVec2::EAST);
	PushEntityOutOfTileIfSolid(entity, tileCoords + IntVec2::WEST);
	PushEntityOutOfTileIfSolid(entity, tileCoords + IntVec2::NORTH);
	PushEntityOutOfTileIfSolid(entity, tileCoords + IntVec2::SOUTH);

	PushEntityOutOfTileIfSolid(entity, tileCoords + IntVec2::EAST + IntVec2::NORTH);
	PushEntityOutOfTileIfSolid(entity, tileCoords + IntVec2::EAST + IntVec2::SOUTH);
	PushEntityOutOfTileIfSolid(entity, tileCoords + IntVec2::WEST + IntVec2::NORTH);
	PushEntityOutOfTileIfSolid(entity, tileCoords + IntVec2::WEST + IntVec2::SOUTH);
}

void Map::PushEntityOutOfTileIfSolid(Entity* entity, IntVec2 const& tileCoords)
{
	if (tileCoords.x < 0 || tileCoords.x > GetDimensions().x - 1 || tileCoords.y < 0 || tileCoords.y > GetDimensions().y - 1)
	{
		return;
	}

	int tileIndex = tileCoords.x + tileCoords.y * m_mapDef.m_dimensions.x;
	Tile& tile = m_tiles[tileIndex];
	if (!tile.IsSolid() && !tile.IsWater())
	{
		return;
	}
	else if (tile.IsWater() && entity->m_canSwim)
	{
		return;
	}

	bool didEntityHitTile = PushDiscOutOfFixedAABB2(entity->m_position, entity->m_physicsRadius, tile.GetBounds());

	if (didEntityHitTile && (entity->m_entityType == ENTITY_TYPE_GOOD_BULLET || entity->m_entityType == ENTITY_TYPE_GOOD_BOLT || entity->m_entityType == ENTITY_TYPE_EVIL_BULLET || entity->m_entityType == ENTITY_TYPE_EVIL_BOLT))
	{
		Bullet* bullet = dynamic_cast<Bullet*>(entity);
		IntVec2 entityTile = IntVec2(RoundDownToInt(entity->m_position.x), RoundDownToInt(entity->m_position.y));
		Vec2 impactNormal = Vec2(static_cast<float>(tileCoords.x - entityTile.x), static_cast<float>(tileCoords.y - entityTile.y)).GetNormalized();
		bullet->BounceOff(impactNormal);
	}
}

void Map::CheckForBulletHits()
{
	for (int bulletFaction = 0; bulletFaction < FACTION_NUM; bulletFaction++)
	{
		for (int entityFaction = 0; entityFaction < FACTION_NUM; entityFaction++)
		{
			if (bulletFaction == entityFaction)
			{
				// same faction
				continue;
			}

			CheckBulletListVsActorList(m_bulletsByFaction[bulletFaction], m_actorsByFaction[entityFaction]);
		}
	}
}

void Map::CheckBulletListVsActorList(EntityList& bulletList, EntityList& actorList)
{
	for (int bulletIndex = 0; bulletIndex < static_cast<int>(bulletList.size()); bulletIndex++)
	{
		Bullet* bullet = dynamic_cast<Bullet*>(bulletList[bulletIndex]);

		if (!IsEntityAlive(bullet))
		{
			continue;
		}

		for (int actorIndex = 0; actorIndex < static_cast<int>(actorList.size()); actorIndex++)
		{
			if (!IsEntityAlive(actorList[actorIndex]))
			{
				continue;
			}

			CheckBulletVsActor(*bullet, *actorList[actorIndex]);
		}
	}
}

void Map::CheckBulletVsActor(Bullet& bullet, Entity& actor)
{
	if (bullet.m_faction == actor.m_faction)
	{
		return;
	}

	if (!actor.m_isHitByBullets)
	{
		return;
	}

	if (DoDiscsOverlap(bullet.m_position, bullet.m_physicsRadius, actor.m_position, actor.m_physicsRadius))
	{
		actor.ReactToBulletHit(bullet);
	}
}

void Map::Render() const
{
	RenderTiles();

	if (m_currentTileHeatMap)
	{
		RenderHeatMap(m_currentTileHeatMap);
	}

	RenderEntities();
}

void Map::RenderTiles() const
{
	int estimatedVertexCount = 3 * 2 * m_mapDef.m_dimensions.x * m_mapDef.m_dimensions.y;
	std::vector<Vertex_PCU> tileVertexes;
	tileVertexes.reserve(estimatedVertexCount);

	for (int tileIndex = 0; tileIndex < static_cast<int>(m_tiles.size()); tileIndex++)
	{
		AddVertsForTile(tileVertexes, tileIndex, m_tiles[tileIndex].GetUVs());
	}

	g_renderer->BindTexture(g_terrainSpriteSheet->GetTexture());
	g_renderer->DrawVertexArray(tileVertexes);
}

void Map::AddVertsForTile(std::vector<Vertex_PCU>& verts, int tileIndex, AABB2 const& uvs) const
{
	Tile tile = m_tiles[tileIndex];
	AABB2 bounds = tile.GetBounds();
	Rgba8 color = tile.GetColor();
	AddVertsForAABB2(verts, bounds, color, uvs.m_mins, uvs.m_maxs);
}

void Map::UpdateEntities(float deltaSeconds)
{
	for (int entityIndex = 0; entityIndex < static_cast<int>(m_allEntities.size()); entityIndex++)
	{
		Entity*& entity = m_allEntities[entityIndex];
		if (entity)
		{
			m_allEntities[entityIndex]->Update(deltaSeconds);
		}
	}
}

void Map::RenderEntities() const
{
	for (int entityListIndex = 0; entityListIndex < ENTITY_TYPE_NUM; entityListIndex++)
	{
		EntityList const& entityList = m_entitiesByType[entityListIndex];
		for (int entityIndex = 0; entityIndex < static_cast<int>(entityList.size()); entityIndex++)
		{
			Entity* const& entity = entityList[entityIndex];
			if (entity)
			{
				entityList[entityIndex]->Render();
			}
		}
	}
}

void Map::GoToNextEntityForHeatMapDebugDraw()
{
	Entity* selectedEntityForHeatMapDebugDraw = GetNextEntityForHeatMapDebugDraw();
	if (selectedEntityForHeatMapDebugDraw == nullptr)
	{
		m_currentTileHeatMap = nullptr;
	}
	else
	{
		m_currentTileHeatMap = (selectedEntityForHeatMapDebugDraw->m_distanceFieldFromGoalPosition);
	}
}

Entity* Map::GetNextEntityForHeatMapDebugDraw()
{
	if (m_actorsByFaction[FACTION_EVIL].empty())
	{
		return nullptr;
	}

	int evilActorIndex = (m_selectedEntityIndexForHeatMapDebugDraw + 1) % static_cast<int>(m_actorsByFaction[FACTION_EVIL].size());
	
	do
	{
		Entity*& evilActor = m_actorsByFaction[FACTION_EVIL][evilActorIndex];
		if (g_game->IsEntityAlive(evilActor) && evilActor->m_entityType != ENTITY_TYPE_EVIL_SCORPIO)
		{
			m_selectedEntityIndexForHeatMapDebugDraw = evilActorIndex;
			return evilActor;
		}
		evilActorIndex = (evilActorIndex + 1) % static_cast<int>(m_actorsByFaction[FACTION_EVIL].size());
	} while(evilActorIndex != m_selectedEntityIndexForHeatMapDebugDraw + 1);

	m_selectedEntityIndexForHeatMapDebugDraw = 0;
	return nullptr;
}

void Map::GoToNextHeatMap()
{
	if (!m_currentTileHeatMap)
	{
		m_currentTileHeatMap = &m_startDistanceField;
	}
	else if (m_currentTileHeatMap == &m_startDistanceField)
	{
		m_currentTileHeatMap = &m_solidMapForLand;
	}
	else if (m_currentTileHeatMap == &m_solidMapForLand)
	{
		m_currentTileHeatMap = &m_solidMapForAmphibian;
	}
	else if (m_currentTileHeatMap == &m_solidMapForAmphibian)
	{
		GoToNextEntityForHeatMapDebugDraw();
	}
	else if (m_currentTileHeatMap == (m_actorsByFaction[FACTION_EVIL][m_selectedEntityIndexForHeatMapDebugDraw]->m_distanceFieldFromGoalPosition))
	{
		m_currentTileHeatMap = nullptr;
		m_selectedEntityIndexForHeatMapDebugDraw = 0;
	}
}

std::string Map::GetHeatMapSimpleString() const
{
	if (m_currentTileHeatMap == &m_startDistanceField)
	{
		return "Distance Field from Map Entry";
	}
	else if (m_currentTileHeatMap == &m_solidMapForLand)
	{
		return "Solid Map for Land";
	}
	else if (m_currentTileHeatMap == &m_solidMapForAmphibian)
	{
		return "Solid Map for Amphibian";
	}
	else if (m_currentTileHeatMap == (m_actorsByFaction[FACTION_EVIL][m_selectedEntityIndexForHeatMapDebugDraw]->m_distanceFieldFromGoalPosition))
	{
		return "Distance Field from Entity Goal Position";
	}

	return "";
}

void Map::RenderHeatMap(TileHeatMap const* heatmap) const
{
	std::vector<Vertex_PCU> heatMapVerts;
	heatmap->AddVertsForDebugDraw(heatMapVerts, AABB2(Vec2::ZERO, Vec2(static_cast<float>(GetDimensions().x), static_cast<float>(GetDimensions().y))), FloatRange(0.f, GetMaxHeatValueExceptSpecialValueOnTileHeatMap(*m_currentTileHeatMap, SPECIAL_VALUE_FOR_HEATMAPS)), Rgba8::BLACK, Rgba8::WHITE, SPECIAL_VALUE_FOR_HEATMAPS, Rgba8::BLUE);
	
	float textWidth = GetSimpleTriangleStringWidth(GetHeatMapSimpleString(), 0.4f);
	AddVertsForTextTriangles2D(heatMapVerts, GetHeatMapSimpleString(), g_game->m_worldCamera.GetOrthoTopRight() - Vec2(textWidth + 0.5f, 2.f), 0.4f, Rgba8::RED);

	if (!m_actorsByFaction[FACTION_EVIL].empty() && m_currentTileHeatMap == (m_actorsByFaction[FACTION_EVIL][m_selectedEntityIndexForHeatMapDebugDraw]->m_distanceFieldFromGoalPosition))
	{
		AddVertsForArrow2D(heatMapVerts, Vec2(static_cast<float>(GetDimensions().x), static_cast<float>(GetDimensions().y)), m_actorsByFaction[FACTION_EVIL][m_selectedEntityIndexForHeatMapDebugDraw]->m_position + Vec2(0.5f, 0.5f), 0.1f, 0.015f, Rgba8::MAGENTA);
	}

	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(heatMapVerts);
}

void Map::DeleteGarbageEntities()
{
	for (int entityIndex = 0; entityIndex < static_cast<int>(m_allEntities.size()); entityIndex++)
	{
		Entity*& entity = m_allEntities[entityIndex];
		if (entity && entity->m_isGarbage)
		{
			RemoveEntityFromMap(entity);
			delete entity;
		}
	}
}

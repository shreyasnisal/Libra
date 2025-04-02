#pragma once

#include "Game/Tile.hpp"
#include "Game/Entity.hpp"
#include "Game/MapDefinition.hpp"

#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/HeatMaps/TileHeatMap.hpp"
#include "Engine/Math/RaycastUtils.hpp"

#include <vector>


class Entity;
class Texture;

class Map
{
public:
	~Map() = default;
	Map(MapDefinition mapDef);
	void							Update(float deltaSeconds);
	void							Render() const;
	Entity*							SpawnNewEntityOfType(EntityType type, Vec2 const& position, float orientationDegrees, float scale = 1.f, float lifetimeSeconds = 0.f);
	void							AddEntityToMap(Entity* entity);
	void							RemoveEntityFromMap(Entity* entity);
	bool							IsEntityAlive(Entity* entity) const;
	IntVec2							GetDimensions() const { return m_mapDef.m_dimensions; }
	int								GetTileIndexFromTileCoords(int tileX, int tileY) const { return tileX + tileY * m_mapDef.m_dimensions.x; }

	bool							IsPointInSolid(Vec2 const& point) const;
	bool							IsPointInWater(Vec2 const& point) const;
	bool							IsPointInDestructibleTile(Vec2 const& point) const;
	bool							IsPointInBunker(Vec2 const& point) const;
	bool							IsTileSolid(IntVec2 const& tileCoordinates) const;
	bool							IsTileWater(IntVec2 const& tileCoordinates) const;
	bool							IsTileDestructible(IntVec2 const& tileCoordiantes) const;
	RaycastResult2D					StepAndSampleRaycastVsTiles(Vec2 const& rayStartPosition, Vec2 const& rayFwdNormal, float maxRayLength) const;
	RaycastResult2D					BetterMoreConfusingRaycastVsTiles(Vec2 const& startPosition, Vec2 const& direction, float maxDistance) const;
	bool							HasLineOfSight(Vec2 const& startPosition, Vec2 const& endPosition) const;
	void							PopulateDistanceField(TileHeatMap& out_distanceField, IntVec2 const& startCoords, float maxCost, bool treatWaterAsSolid = true, bool treatScorpioAsSolid = false, bool treatDestructibleTilesAsSolid = false);
	void							GoToNextHeatMap();
	void							GoToNextEntityForHeatMapDebugDraw();

public:

	EntityList						m_allEntities;
	EntityList						m_entitiesByType[ENTITY_TYPE_NUM];
	EntityList						m_bulletsByFaction[FACTION_NUM];
	EntityList						m_actorsByFaction[FACTION_NUM];

	std::vector<Tile>				m_tiles;

	TileHeatMap						m_solidMapForLand;
	TileHeatMap						m_solidMapForAmphibian;

private:
	Entity*							CreateNewEntityOfType(EntityType type, Vec2 const& position, float orientationDegrees, float scale = 1.f, float lifetimeSeconds = 0.f);
	void							AddEntityToList(Entity* entity, EntityList& entities);
	void							RemoveEntityFromList(Entity* entity, EntityList& entities);
	void							PopulateTiles();
	void							ConstructMapFromImage();
	void							SetTileType(IntVec2 const& tileCoords, std::string tileTypeName);
	void							LoadAssets();
	void							SpawnEntities();
	Vec2							GetNewEntitySpawnPosition() const;

	void							UpdateEntities(float deltaSeconds);

	void							PushEntitiesOutOfEachOther();
	void							PushEntityOutOfEntity(Entity& entityA, Entity& entityB);

	void							PushEntitiesOutOfWalls();
	void							PushEntityOutOfWalls(Entity* entity);
	void							PushEntityOutOfTileIfSolid(Entity* entity, IntVec2 const& tileCoords);

	void							CheckForBulletHits();
	void							CheckBulletListVsActorList(EntityList& bulletList, EntityList& actorList);
	void							CheckBulletVsActor(Bullet& bullet, Entity& actor);

	void							RenderTiles() const;
	void							RenderEntities() const;
	void							AddVertsForTile(std::vector<Vertex_PCU>& verts, int tileIndex, AABB2 const& uvs) const;
	void							RenderHeatMap(TileHeatMap const* heatmap) const;

	void							DeleteGarbageEntities();

	float							GetMaxHeatValueExceptSpecialValueOnTileHeatMap(TileHeatMap const& heatMap, float specialValue) const;
	bool							IsMapValid();
	bool							IsTileTraversable(IntVec2 const& tileCoords, bool treatWaterAsSolid, bool treatScorpioAsSolid, bool treatDestructibleTilesAsSolid) const;
	void							GenerateHeatMaps();
	Entity*							GetNextEntityForHeatMapDebugDraw();
	std::string						GetHeatMapSimpleString() const;

private:
	MapDefinition					m_mapDef;
	float							m_maxDistanceFromStart = 0;
	int								m_selectedEntityIndexForHeatMapDebugDraw = 0;

	TileHeatMap*					m_currentTileHeatMap = nullptr;
	TileHeatMap						m_startDistanceField;
};
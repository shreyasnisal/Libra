#pragma once

#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#include <vector>

class Texture;
class Map;

enum WeaponType
{
	WEAPON_TYPE_INVALID = -1,

	WEAPON_TYPE_GUN,
	WEAPON_TYPE_FLAMETHROWER,

	WEAPON_TYPE_NUM
};

class Player : public Entity
{
public:
	~Player() = default;
	Player(EntityType type, EntityFaction faction, Vec2 const& startPos, float orientationDegrees);

	void									Update(float deltaSeconds);
	void									Render() const;
	void									Die() override;
	void									ReactToBulletHit(Bullet& bullet) override;

public:
	static constexpr float					SECONDS_BETWEEN_FLAMETHROWER_BULLETS = 0.1f;

	std::vector<Vertex_PCU>					m_baseVertexes;
	std::vector<Vertex_PCU>					m_turretVertexes;

	float									m_turretOrientationDegrees = 0.f;

private:
	void									LoadAssets();
	void									InitializeVertexes();
	void									UpdateFromKeyboard(float& deltaSeconds);
	void									UpdateFromXboxController(float& deltaSeconds);
	void									UpdatePlayer(float deltaSeconds);

	void									ChangeSelectedWeapon();
	void									FireBullet(float deltaSeconds);
	void									FireFlamethrower(float deltaSeconds);

	void									RenderDebug() const override;

private:
	Vec2									m_moveIntent = Vec2::ZERO;
	float									m_goalOrientationDegrees = 0.f;
	Vec2									m_turretGoalDirection = Vec2::ZERO;
	float									m_turretGoalOrientationDegrees = 0.f;
	float									m_timeRemainingUntilNextBullet = 0.f;
	float									m_timeRemainingUntilGameOverScreen = 3.f;

	float									m_secondsUntilNextFlamethrowerBullets = SECONDS_BETWEEN_FLAMETHROWER_BULLETS;

	WeaponType								m_selectedWeapon = WeaponType::WEAPON_TYPE_GUN;							
};
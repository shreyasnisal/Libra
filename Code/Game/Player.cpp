#include "Game/Player.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Bullet.hpp"

Player::Player(EntityType type, EntityFaction faction, Vec2 const& startPosition, float orientationDegrees)
	: Entity(type, faction, startPosition, orientationDegrees)
{
	m_isPushedByWalls = true;
	m_isPushedByEntities = true;
	m_pushesEntities = true;
	m_isHitByBullets = true;
	m_physicsRadius = 0.3f;
	m_cosmeticRadius = 0.4f;
	m_turretOrientationDegrees = orientationDegrees;
	m_maxHealth = g_gameConfigBlackboard.GetValue("playerMaxHealth", 10);
	m_health = m_maxHealth;

	InitializeVertexes();
}

void Player::InitializeVertexes()
{
	AABB2 baseVertexBox = AABB2(Vec2(-m_cosmeticRadius, -m_cosmeticRadius), Vec2(m_cosmeticRadius, m_cosmeticRadius));
	AABB2 turretVertexBox = AABB2(Vec2(-m_cosmeticRadius, -m_cosmeticRadius), Vec2(m_cosmeticRadius, m_cosmeticRadius));
	AddVertsForAABB2(m_baseVertexes, baseVertexBox, Rgba8(255, 255, 255, 255));
	AddVertsForAABB2(m_turretVertexes, turretVertexBox, Rgba8(255, 255, 255, 255));
}

void Player::Update(float deltaSeconds)
{
	if (m_isDead)
	{
		if (g_game->m_gameState != GAME_STATE_GAMEOVER)
		{
			m_timeRemainingUntilGameOverScreen -= deltaSeconds;

			if (m_timeRemainingUntilGameOverScreen <= 0.f)
			{
				m_timeRemainingUntilGameOverScreen = g_gameConfigBlackboard.GetValue("perishFadeoutSeconds", 3.f);
				g_game->PlaySound(GAMEOVER_SOUND);
				g_game->m_gameState = GAME_STATE_GAMEOVER;
			}
		}

		return;
	}

	m_moveIntent = Vec2::ZERO;
	m_turretGoalDirection = Vec2::ZERO;

	UpdateFromKeyboard(deltaSeconds);
	UpdateFromXboxController(deltaSeconds);

	UpdatePlayer(deltaSeconds);

	IntVec2 playerTileCoords = IntVec2(RoundDownToInt(m_position.x), RoundDownToInt(m_position.y));
	if (playerTileCoords == m_map->GetDimensions() - IntVec2(2, 2))
	{
		g_game->AdvanceToNextMap();
	}
}

void Player::UpdateFromKeyboard(float& deltaSeconds)
{
	if (g_input->IsKeyDown('E'))
	{
		m_moveIntent += Vec2::NORTH;
	}
	if (g_input->IsKeyDown('S'))
	{
		m_moveIntent += Vec2::WEST;
	}
	if (g_input->IsKeyDown('D'))
	{
		m_moveIntent += Vec2::SOUTH;
	}
	if (g_input->IsKeyDown('F'))
	{
		m_moveIntent += Vec2::EAST;
	}
	m_moveIntent.Normalize();
	
	if (g_input->IsKeyDown('I'))
	{
		m_turretGoalDirection += Vec2::NORTH;
	}
	if (g_input->IsKeyDown('J'))
	{
		m_turretGoalDirection += Vec2::WEST;
	}
	if (g_input->IsKeyDown('K'))
	{
		m_turretGoalDirection += Vec2::SOUTH;
	}
	if (g_input->IsKeyDown('L'))
	{
		m_turretGoalDirection += Vec2::EAST;
	}
	if (g_input->WasKeyJustPressed('H'))
	{
		ChangeSelectedWeapon();
	}

	if (g_input->IsKeyDown(KEYCODE_SPACE))
	{
		switch (m_selectedWeapon)
		{
			case WEAPON_TYPE_GUN:				FireBullet(deltaSeconds);				break;
			case WEAPON_TYPE_FLAMETHROWER:		FireFlamethrower(deltaSeconds);			break;
		}
	}
	if (g_input->WasKeyJustReleased(KEYCODE_SPACE))
	{
		m_timeRemainingUntilNextBullet = 0.f;
		m_secondsUntilNextFlamethrowerBullets = 0.f;
	}
}

void Player::UpdateFromXboxController(float& deltaSeconds)
{
	UNUSED(deltaSeconds);

	XboxController controller = g_input->GetController(0);
	AnalogJoystick leftJoystick = controller.GetLeftStick();
	AnalogJoystick rightJoystick = controller.GetRightStick();

	float leftJoystickMagnitude = leftJoystick.GetMagnitude();
	float rightJoystickMagnitude = rightJoystick.GetMagnitude();

	if (leftJoystickMagnitude > 0.f)
	{
		m_moveIntent += Vec2::MakeFromPolarDegrees(leftJoystick.GetOrientationDegrees(), leftJoystickMagnitude);
	}

	if (rightJoystickMagnitude > 0.f)
	{
		m_turretGoalDirection = Vec2::MakeFromPolarDegrees(rightJoystick.GetOrientationDegrees());
	}
}

void Player::UpdatePlayer(float deltaSeconds)
{
	float orientationDeltaDegrees = 0.f;
	if (m_moveIntent != Vec2::ZERO)
	{
		m_goalOrientationDegrees = m_moveIntent.GetOrientationDegrees();
		float angularVelocity = g_gameConfigBlackboard.GetValue("playerTurnRate", 360.f);
		float m_turnedOrientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, m_goalOrientationDegrees, angularVelocity * deltaSeconds);
		orientationDeltaDegrees = m_turnedOrientationDegrees - m_orientationDegrees;
		m_orientationDegrees = m_turnedOrientationDegrees;
		m_velocity = GetForwardNormal() * m_moveIntent.GetLength() * g_gameConfigBlackboard.GetValue("playerDriveSpeed", 1.f);
		m_position += m_velocity * deltaSeconds;
	}
	else
	{
		m_velocity = Vec2::ZERO;
	}


	if (m_turretGoalDirection != Vec2::ZERO)
	{
		m_turretGoalDirection.Normalize();
		m_turretGoalOrientationDegrees = m_turretGoalDirection.GetOrientationDegrees();
		float turretAngularVelocity = g_gameConfigBlackboard.GetValue("playerGunTurnRate", 720.f);
		m_turretOrientationDegrees = GetTurnedTowardDegrees(m_turretOrientationDegrees, m_turretGoalOrientationDegrees, turretAngularVelocity * deltaSeconds);
	}
	else
	{
		m_turretOrientationDegrees += orientationDeltaDegrees;
	}
}

void Player::FireBullet(float deltaSeconds)
{
	m_timeRemainingUntilNextBullet -= deltaSeconds;

	if (m_timeRemainingUntilNextBullet <= 0.f)
	{
		g_game->PlaySound(PLAYER_SHOOT_SOUND, m_position);
		m_map->SpawnNewEntityOfType(ENTITY_TYPE_NEUTRAL_EXPLOSION, m_position + Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees, 0.35f), m_orientationDegrees, 0.3f, 0.4f);

		m_map->SpawnNewEntityOfType(ENTITY_TYPE_GOOD_BOLT, m_position, m_turretOrientationDegrees);
		m_timeRemainingUntilNextBullet = g_gameConfigBlackboard.GetValue("playerShootCooldownSeconds", 0.1f);
	}
}

void Player::FireFlamethrower(float deltaSeconds)
{
	m_secondsUntilNextFlamethrowerBullets -= deltaSeconds;

	if (m_secondsUntilNextFlamethrowerBullets <= 0.f)
	{
		int flamethrowerBulletsPerSecond = g_gameConfigBlackboard.GetValue("flamethrowerBulletsPerSeconds", 40);
		int flamethrowerBulletsThisFrame = RoundDownToInt(flamethrowerBulletsPerSecond * SECONDS_BETWEEN_FLAMETHROWER_BULLETS);

		for (int bulletIndex = 0; bulletIndex < flamethrowerBulletsThisFrame; bulletIndex++)
		{
			float orientationVariance = g_RNG->RollRandomFloatInRange(-30.f, 30.f);
			m_map->SpawnNewEntityOfType(ENTITY_TYPE_GOOD_FLAMETHROWER_BULLET, m_position + Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees, 0.4f), m_turretOrientationDegrees + orientationVariance);
		}
		m_secondsUntilNextFlamethrowerBullets = SECONDS_BETWEEN_FLAMETHROWER_BULLETS;
	}
}

void Player::Render() const
{

	if (!m_isDead)
	{
		std::vector<Vertex_PCU> worldBaseVertexes = m_baseVertexes;
		std::vector<Vertex_PCU> worldTurretVertexes = m_turretVertexes;

		TransformVertexArrayXY3D(worldBaseVertexes, m_scale, m_orientationDegrees, m_position);
		TransformVertexArrayXY3D(worldTurretVertexes, m_scale, m_turretOrientationDegrees, m_position);

		g_renderer->BindTexture(g_textures[PLAYERTANK_BASE_TEXTURE]);
		g_renderer->DrawVertexArray(worldBaseVertexes);

		g_renderer->BindTexture(g_textures[PLAYERTANK_TURRET_TEXTURE]);
		g_renderer->DrawVertexArray(worldTurretVertexes);

		if (m_health < m_maxHealth)
		{
			RenderHealthBar();
		}
	}

	if (g_game->m_drawDebug)
	{
		RenderDebug();
	}

	if (g_game->m_isPlayerInvincible)
	{
		DebugDrawRing(m_position, m_physicsRadius, 0.02f, Rgba8::WHITE);
	}
}

void Player::RenderDebug() const
{
	std::vector<Vertex_PCU> debugRenderVerts;

	float debugElementsThickness = g_gameConfigBlackboard.GetValue("debugDrawLineThickness", 0.03f);

	AddVertsForLineSegment2D(debugRenderVerts, m_position, m_position + Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees, m_cosmeticRadius), 0.2f, Rgba8(0, 0, 255, 255));
	AddVertsForLineSegment2D(debugRenderVerts, m_position + Vec2::MakeFromPolarDegrees(m_turretGoalOrientationDegrees, m_cosmeticRadius), m_position + Vec2::MakeFromPolarDegrees(m_turretGoalOrientationDegrees, m_cosmeticRadius + 0.1f), 0.2f, Rgba8(0, 0, 255, 255));

	Entity::RenderDebug();

	AddVertsForLineSegment2D(debugRenderVerts, m_position + Vec2::MakeFromPolarDegrees(m_goalOrientationDegrees, m_cosmeticRadius), m_position + Vec2::MakeFromPolarDegrees(m_goalOrientationDegrees, m_cosmeticRadius + 0.1f), debugElementsThickness, Rgba8(255, 0, 0, 255));
}

void Player::ReactToBulletHit(Bullet& bullet)
{
	if (g_game->m_isPlayerInvincible)
	{
		bullet.Die();
		return;
	}

	g_game->PlaySound(PLAYER_HIT_SOUND, m_position);
	Entity::ReactToBulletHit(bullet);
}

void Player::Die()
{
	m_map->SpawnNewEntityOfType(ENTITY_TYPE_NEUTRAL_EXPLOSION, m_position, m_orientationDegrees, 1.5f, 1.f);
	m_isDead = true;
}

void Player::ChangeSelectedWeapon()
{
	m_selectedWeapon = WeaponType((static_cast<int>(m_selectedWeapon) + 1) % static_cast<int>(WEAPON_TYPE_NUM));
}

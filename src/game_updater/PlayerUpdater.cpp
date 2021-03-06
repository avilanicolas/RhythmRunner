//	Joseph	Arhar



#include	"PlayerUpdater.h"



#include	<imgui.h>

#include	<glm/glm.hpp>

#include	<queue>

#include	<algorithm>



#include	"MovingObject.h"

#include	"DroppingPlatform.h"

#include	"InputBindings.h"

#include	"TimingConstants.h"

#include	"Octree.h"

#include	"Player.h"

#include	"CollisionCalculator.h"

#include	"DroppingPlatform.h"



#define	COLLISION_WIDTH	0.420f

#define	MAX_AERIAL_ROTATION_ANGLE	0.67f



PlayerUpdater::PlayerUpdater()	{}



PlayerUpdater::~PlayerUpdater()	{}



void	PlayerUpdater::MovePlayer(std::shared_ptr<GameState>	game_state)	{

		std::shared_ptr<Player>	player	=	game_state->GetPlayer();

		std::shared_ptr<GameCamera>	camera	=	game_state->GetCamera();



		//	the	max	width	to	allow	the	player	to	be	colliding	with	an	object

		collision_width	=	COLLISION_WIDTH;



		//	Store	player	state	before	moving

		previous_player_box	=	player->GetBoundingBox();



		//	Check	to	see	if	the	ground	is	no	longer	beneath	the	player,

		//	in	which	case	they	should	fall	down

		if	(player->GetGround())	{

				AxisAlignedBox	ground_box	=	player->GetGround()->GetBoundingBox();

				if	(previous_player_box.GetMin().x	>	ground_box.GetMax().x	||

								previous_player_box.GetMin().z	>	ground_box.GetMax().z	||

								previous_player_box.GetMax().z	<	ground_box.GetMin().z)	{

						//	Ground	is	no	longer	beneath	player

						FallOffGround(game_state);

				}

		}



		//	Jump	if	the	user	inputs	a	jump.

		//	use	basic	key	detection	when	on	the	ground,	newly	pressed	for	doublejump

		if	((player->GetGround()	&&	ImGui::GetIO().KeysDown[GLFW_KEY_SPACE])	||

						(!player->GetGround()	&&

							InputBindings::KeyNewlyPressed(GLFW_KEY_SPACE)))	{

				Jump(game_state);

		}



		//	apply	gravity

		if	(!player->GetGround())	{

				player->SetYVelocity(player->GetYVelocity()	-	PLAYER_GRAVITY);

		}



		glm::vec3	prevCameraPos	=	camera->getPosition();

		Player::DuckDir	target_duck;

		//	always	check	for	ducking	(be	ware	of	ducks)

		//	let	the	player	move	up/down	Z	(sock	it	to	me)

		if	(ImGui::GetIO().KeysDown[GLFW_KEY_LEFT]	||

						ImGui::GetIO().KeysDown[GLFW_KEY_H])	{

				target_duck	=	Player::DuckDir::LEFT;

				player->MoveDownZ();

				camera->setPosition(prevCameraPos	-

																								glm::vec3(0,	0,	PLAYER_DELTA_Z_PER_TICK));

		}	else	if	(ImGui::GetIO().KeysDown[GLFW_KEY_RIGHT]	||

													ImGui::GetIO().KeysDown[GLFW_KEY_L])	{

				target_duck	=	Player::DuckDir::RIGHT;

				player->MoveUpZ();

				camera->setPosition(prevCameraPos	+

																								glm::vec3(0,	0,	PLAYER_DELTA_Z_PER_TICK));

		}	else	{

				target_duck	=	Player::DuckDir::NONE;

		}

		if	((ImGui::GetIO().KeysDown[GLFW_KEY_LEFT_SHIFT]	||

							ImGui::GetIO().KeysDown[GLFW_KEY_RIGHT_SHIFT])	&&

						target_duck	==	Player::DuckDir::NONE)	{

				target_duck	=	Player::DuckDir::RIGHT;

		}



		if	(target_duck	!=	player->GetDucking())	{

				player->SetDucking(target_duck);

		}



		//	Update	player	position	based	on	new	velocity.

		player->SetPosition(player->GetPosition()	+

																						glm::vec3(DELTA_X_PER_TICK,	player->GetYVelocity(),

																																player->GetZVelocity()));

}



void	PlayerUpdater::AnimatePlayer(std::shared_ptr<GameState>	game_state)	{

		std::shared_ptr<Player>	player	=	game_state->GetPlayer();

		std::shared_ptr<PhysicalObject>	rear_wheel	=	player->GetRearWheel();

		std::shared_ptr<PhysicalObject>	front_wheel	=	player->GetFrontWheel();

		Player::Animation	current_animation	=	player->GetAnimation();



		//	Rotate	the	wheels

		if	(current_animation	&	Player::ANIMATION_WHEELSPIN_BIT)	{

				player->SetWheelRotationSpeed(WHEEL_ROTATION_PER_TICK);

		}	else	if	(current_animation	&	Player::ANIMATION_WHEELSPIN_SLOW_BIT)	{

				player->SetWheelRotationSpeed(player->GetWheelRotationSpeed()	*	0.98);

		}	else	{

				player->SetWheelRotationSpeed(0);

		}

		player->GetRearWheel()->SetRotationAngle(rear_wheel->GetRotationAngle()	+

																																											player->GetWheelRotationSpeed());

		player->GetFrontWheel()->SetRotationAngle(front_wheel->GetRotationAngle()	+

																																												player->GetWheelRotationSpeed());



		if	(current_animation	==	Player::Animation::FAILURE)	{

				//	go	crazy

				player->SetRotationAngle(player->GetRotationAngle()	+	0.3f);

		}



		if	(current_animation	&	Player::ANIMATION_AERIAL_TILT_BIT)	{

				//	set	player	rotation	based	on	y	velocity

				float	rotation_angle	=	std::atan(player->GetYVelocity()	*	4);

				if	(rotation_angle	>	0)	{

						player->SetRotationAngle(

										std::min(MAX_AERIAL_ROTATION_ANGLE,	rotation_angle));

				}	else	{

						player->SetRotationAngle(

										std::max(-MAX_AERIAL_ROTATION_ANGLE,	rotation_angle));

				}

		}



		//	some	animations	may	have	misaligned	the	player	with	the	ground,

		//	make	sure	the	player	is	on	the	ground	if	grounded.

		//	this	is	also	important	for	staying	attached	to	moving	platforms

		SnapToGround(game_state);

}



void	PlayerUpdater::CollisionCheck(std::shared_ptr<GameState>	game_state)	{

		std::shared_ptr<Player>	player	=	game_state->GetPlayer();



		//	Determine	colliding	objects.

		std::shared_ptr<std::unordered_set<std::shared_ptr<GameObject>>>

						collidingObjects	=	CollisionCalculator::GetCollidingObjects(

										player->GetBoundingBox(),	game_state->GetLevel()->getTree());

		std::vector<std::shared_ptr<Obstacle>>	colliding_obstacles;

		std::vector<std::shared_ptr<Collectible>>	colliding_collectibles;

		for	(std::shared_ptr<GameObject>	game_object	:	*collidingObjects)	{

				if	(game_object->GetType()	==	ObjectType::COLLECTIBLE)	{

						colliding_collectibles.push_back(

										std::static_pointer_cast<Collectible>(game_object));

				}	else	if	(game_object->GetType()	==	ObjectType::OBSTACLE)	{

						colliding_obstacles.push_back(

										std::static_pointer_cast<Obstacle>(game_object));

				}

		}



		//	Collect	the	collectibles	we	are	colliding	with.

		for	(std::shared_ptr<Collectible>	collectible	:	colliding_collectibles)	{

				if	(!collectible->GetCollected())	{

						collectible->SetCollected();

						game_state->GetPlayer()->SetScore(game_state->GetPlayer()->GetScore()	+

																																								1);

				}

		}



		//	Apply	collision	logic	for	obstacles.

		if	(!colliding_obstacles.empty())	{

				AxisAlignedBox	player_box	=	player->GetBoundingBox();

				float	player_min_y	=	previous_player_box.GetMin().y;

				//	Handle	the	collisions

				for	(std::shared_ptr<GameObject>	colliding_object	:	colliding_obstacles)	{

						//	Are	we	in	the	air,	and	is	the	object	we	are	colliding	with	below	us?

						//	If	so,	then	set	it	as	our	current	ground

						AxisAlignedBox	colliding_box	=	colliding_object->GetBoundingBox();

						float	colliding_max_y	=	colliding_object->GetBoundingBox().GetMax().y;

						float	y_velocity_tolerance	=	player->GetYVelocity();



						//	If	the	player's	bounding	box	was	"above"	the	platform's	box	before

						//	collision,	then	we	consider	the	collision	"landing"	on	the	platform.

						//	collision_width	+	y_velocity	are	acceptable	error.

						if	((player_min_y	>	colliding_max_y)	||

										(std::abs(player_min_y	-	colliding_max_y)	<

											collision_width	+	y_velocity_tolerance))	{

								//	above	colliding	box,	we	are	grounded	on	this	platform

								Land(game_state,	colliding_object);

								player_min_y	=	player->GetBoundingBox().GetMin().y;

								//	If	the	ground	is	now	a	dropping	platform	drop	it

								if	(player->GetGround()->GetSecondaryType()	==

												SecondaryType::DROPPING_PLATFORM)	{

										std::shared_ptr<gameobject::DroppingPlatform>	dropping	=

														std::static_pointer_cast<gameobject::DroppingPlatform>(

																		player->GetGround());

										dropping->SetDropping();

								}

						}	else	{

								//	The	collision	was	not	from	above,	so	reset	the	game

								Death(game_state);

								return;

						}

				}

		}



		//	Check	to	see	if	the	player	fell	out	of	the	world.

		if	(previous_player_box.GetMin().y	<

						game_state->GetLevel()->getTree()->GetKillZone())	{

				Death(game_state);

		}

}



void	PlayerUpdater::Jump(std::shared_ptr<GameState>	game_state)	{

		std::shared_ptr<Player>	player	=	game_state->GetPlayer();



		if	(player->GetGround()	||	player->GetDoubleJump())	{

				player->SetDoubleJump(player->GetGround()	?	true	:	false);

				ChangeAnimation(game_state,	Player::Animation::JUMPING);

				//	if	we	are	on	a	moving	platform,	then	add	its	velocity	to	the	jump

				player->SetYVelocity(PLAYER_JUMP_VELOCITY	+

																									std::max(player->GetGroundYVelocity(),	0.0f));

				player->SetZVelocity(0);

				player->RemoveGround();



				//	TODO(jarhar):	create	a	particle	effect	here?

		}

}



void	PlayerUpdater::Land(std::shared_ptr<GameState>	game_state,

																									std::shared_ptr<GameObject>	ground)	{

		std::shared_ptr<Player>	player	=	game_state->GetPlayer();



		player->SetGround(ground);

		player->SetYVelocity(0);

		player->SetZVelocity(0);

		player->SetRotationAngle(0);

		ChangeAnimation(game_state,	Player::Animation::GROUNDED);

}



void	PlayerUpdater::ChangeAnimation(std::shared_ptr<GameState>	game_state,

																																				Player::Animation	animation)	{

		std::shared_ptr<Player>	player	=	game_state->GetPlayer();

		std::shared_ptr<PhysicalObject>	rear_wheel	=	player->GetRearWheel();

		std::shared_ptr<PhysicalObject>	front_wheel	=	player->GetFrontWheel();



		player->SetAnimationStartTick(game_state->GetElapsedTicks()	-

																																1);		//	TODO(jarhar):	this	is	hacky

		player->SetAnimation(animation);



		rear_wheel->SetRotationAxis(glm::vec3(0,	0,	-1));

		front_wheel->SetRotationAxis(glm::vec3(0,	0,	-1));



		if	(animation	&	Player::ANIMATION_ENDGAME_BIT)	{

				//	do	fun	spins	after	the	game	ends

				//	TODO(jarhar):	add	particle/sound	effects	here?

				player->SetRotationAxis(glm::vec3(1,	0,	0));

		}	else	{

				//	rock	fowards/backwards	for	jumping

				player->SetRotationAxis(glm::vec3(0,	0,	1));

		}

}



void	PlayerUpdater::SnapToGround(std::shared_ptr<GameState>	game_state)	{

		std::shared_ptr<Player>	player	=	game_state->GetPlayer();



		if	(!player->GetGround())	{

				return;

		}



		//	After	this,	player's	min	y	should	be	Player::PLATFORM_SPACING	above

		//	ground's	max	y



		AxisAlignedBox	bounding_box	=	player->GetBoundingBox();

		AxisAlignedBox	ground_box	=	player->GetGround()->GetBoundingBox();

		glm::vec3	player_position	=	player->GetPosition();



		player->SetPosition(

						glm::vec3(player_position.x,

																ground_box.GetMax().y	+	Player::PLATFORM_SPACING	+

																				(bounding_box.GetMax().y	-	bounding_box.GetMin().y)	/	2	+

																				(player_position.y	-	bounding_box.GetCenter().y),

																player_position.z));

}



void	PlayerUpdater::Death(std::shared_ptr<GameState>	game_state)	{

		std::shared_ptr<Player>	player	=	game_state->GetPlayer();



		//	TODO(jarhar):	add	a	particle	effect	here?



		game_state->SetPlayingState(GameState::PlayingState::FAILURE);

		ChangeAnimation(game_state,	Player::Animation::FAILURE);

}



void	PlayerUpdater::FallOffGround(std::shared_ptr<GameState>	game_state)	{

		std::shared_ptr<Player>	player	=	game_state->GetPlayer();



		player->SetYVelocity(std::max(player->GetGroundYVelocity(),	0.0f));

		player->RemoveGround();

		ChangeAnimation(game_state,	Player::Animation::JUMPING);

}

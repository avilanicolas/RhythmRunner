//	Joseph	Arhar



#ifndef	TIMING_CONSTANTS_H_

#define	TIMING_CONSTANTS_H_



//	Supreme	constants

#define	DELTA_X_PER_SECOND	10		//	"horizontal"	player	speed

#define	TICKS_PER_SECOND	120.0



//	Conversions

#define	MS_PER_SECOND	1000.0

#define	MICROS_PER_SECOND	1000000.0

#define	TICKS_PER_MS	(TICKS_PER_SECOND	/	MS_PER_SECOND)

#define	TICKS_PER_MICRO	(TICKS_PER_SECOND	/	MICROS_PER_SECOND)

#define	SECONDS_PER_TICK	(1.0	/	TICKS_PER_SECOND)

#define	DELTA_X_PER_MS	(DELTA_X_PER_SECOND	/	1000.0)

#define	MS_PER_TICK	(SECONDS_PER_TICK	*	MS_PER_SECOND)

#define	MICROS_PER_TICK	(SECONDS_PER_TICK	*	MICROS_PER_SECOND)

#define	DELTA_X_PER_TICK	(DELTA_X_PER_SECOND	*	SECONDS_PER_TICK)



//	Platforms

#define	MS_PER_PLATFORM	600

//	Spacing	between	platform	positions	-	no	regard	for	platform	size/scale

#define	PLATFORM_X_DELTA	(DELTA_X_PER_MS	*	MS_PER_PLATFORM)

#define	PLATFORM_Y_DELTA	1.0



//	Player	Movement

#define	PLAYER_JUMP_DELTA_Y_PER_SECOND	10.0		//	Jump	velocity

#define	PLAYER_JUMP_DELTA_Y_PER_TICK	\

		(PLAYER_JUMP_DELTA_Y_PER_SECOND	*	SECONDS_PER_TICK)

#define	PLAYER_JUMP_VELOCITY	PLAYER_JUMP_DELTA_Y_PER_TICK

#define	PLAYER_GRAVITY	(PLAYER_JUMP_DELTA_Y_PER_TICK	/	40.0)		//	dy^2	per	tick

//	tag	yourself	as	a	one	man	team	you're	a	one	club	man

#define	PLAYER_DELTA_Z_PER_TICK	0.05f



//	Amount	of	seconds	before	the	music	starts

#define	PREGAME_SECONDS	1



#endif

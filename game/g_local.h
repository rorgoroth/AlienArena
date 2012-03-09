/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// g_local.h -- local definitions for game module

#include "q_shared.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "game.h"

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"data1"

// protocol bytes that can be directly added to messages
#define	svc_muzzleflash		1
#define	svc_muzzleflash2	2
#define	svc_temp_entity		3
#define	svc_layout			4
#define	svc_inventory		5
#define svc_nop				6
#define	svc_stufftext		11

//==================================================================

// view pitching times
#define DAMAGE_TIME		0.5
#define	FALL_TIME		0.3


// edict->spawnflags
// these are set with checkboxes on each entity in the map editor
#define	SPAWNFLAG_NOT_EASY			0x00000100
#define	SPAWNFLAG_NOT_MEDIUM		0x00000200
#define	SPAWNFLAG_NOT_HARD			0x00000400
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00000800

// edict->flags
#define	FL_FLY					0x00000001
#define	FL_SWIM					0x00000002	// implied immunity to drowining
#define FL_IMMUNE_LASER			0x00000004
#define	FL_INWATER				0x00000008
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define FL_IMMUNE_SLIME			0x00000040
#define FL_IMMUNE_LAVA			0x00000080
#define	FL_PARTIALGROUND		0x00000100	// not all corners are valid
#define	FL_WATERJUMP			0x00000200	// player jumping out of water
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_POWER_ARMOR			0x00001000	// power armor (if any) is active
#define FL_RESPAWN				0x80000000	// used for item respawning


#define	FRAMETIME		0.1

//unlagged - true ping
#define NUM_PING_SAMPLES 64
//unlagged - true ping

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level


#define MELEE_DISTANCE	80

// Anti-camp defaults
/** @brief Default anti-camp frames
 *
 * Default value for the ac_frames anti-camp CVar.
 */
#define G_ANTICAMP_FRAMES	30

/** @brief Default anti-camp threshold
 *
 * Default value for the ac_threshold anti-camp CVar.
 */
#define G_ANTICAMP_THRESHOLD	100


#define BODY_QUEUE_SIZE		8

typedef enum
{
	DAMAGE_NO,
	DAMAGE_YES,			// will take damage if hit
	DAMAGE_AIM			// auto targeting recognizes this
} damage_t;

typedef enum
{
	WEAPON_READY,
	WEAPON_ACTIVATING,
	WEAPON_DROPPING,
	WEAPON_FIRING
} weaponstate_t;

typedef enum
{
	AMMO_BULLETS,
	AMMO_SHELLS,
	AMMO_ROCKETS,
	AMMO_GRENADES,
	AMMO_CELLS,
	AMMO_SLUGS,
	AMMO_SEEKERS
} ammo_t;

//teams
typedef struct teamcensus_s
{ // see p_client.c::TeamCensus()
	int total;
	int real;
	int bots;
	int red;
	int blue;
	int real_red;
	int real_blue;
	int bots_red;
	int bots_blue;
	int team_for_real;
	int team_for_bot;
} teamcensus_t;

#define TEAM_GAME ( (dmflags->integer & (DF_SKINTEAMS)) \
		|| ctf->integer || tca->integer || cp->integer )

#define RED_TEAM				0
#define BLUE_TEAM				1
#define NO_TEAM					2

//clientinfo origins
#define INGAME					0
#define SPAWN					1
#define CONNECT					2

//deadflag
#define DEAD_NO					0
#define DEAD_DYING				1
#define DEAD_DEAD				2
#define DEAD_RESPAWNABLE		3

//range
#define RANGE_MELEE				0
#define RANGE_NEAR				1
#define RANGE_MID				2
#define RANGE_FAR				3

//gib types
#define GIB_ORGANIC				0
#define GIB_METALLIC			1

//monster ai flags
#define AI_STAND_GROUND			0x00000001
#define AI_TEMP_STAND_GROUND	0x00000002
#define AI_SOUND_TARGET			0x00000004
#define AI_LOST_SIGHT			0x00000008
#define AI_PURSUIT_LAST_SEEN	0x00000010
#define AI_PURSUE_NEXT			0x00000020
#define AI_PURSUE_TEMP			0x00000040
#define AI_HOLD_FRAME			0x00000080
#define AI_GOOD_GUY				0x00000100
#define AI_BRUTAL				0x00000200
#define AI_NOSTEP				0x00000400
#define AI_DUCKED				0x00000800
#define AI_COMBAT_POINT			0x00001000
#define AI_MEDIC				0x00002000
#define AI_RESURRECTING			0x00004000
#define AI_NPC					0x00008000	// FIXME: use AI_GOOD_GUY instead?

//monster attack state
#define AS_STRAIGHT				1
#define AS_SLIDING				2
#define	AS_MELEE				3
#define	AS_MISSILE				4

// armor types
#define ARMOR_NONE				0
#define ARMOR_JACKET			1
#define ARMOR_COMBAT			2
#define ARMOR_BODY				3
#define ARMOR_SHARD				4

// handedness values
#define RIGHT_HANDED			0
#define LEFT_HANDED				1
#define CENTER_HANDED			2


// game.serverflags values
#define SFL_CROSS_TRIGGER_1		0x00000001
#define SFL_CROSS_TRIGGER_2		0x00000002
#define SFL_CROSS_TRIGGER_3		0x00000004
#define SFL_CROSS_TRIGGER_4		0x00000008
#define SFL_CROSS_TRIGGER_5		0x00000010
#define SFL_CROSS_TRIGGER_6		0x00000020
#define SFL_CROSS_TRIGGER_7		0x00000040
#define SFL_CROSS_TRIGGER_8		0x00000080
#define SFL_CROSS_TRIGGER_MASK	0x000000ff


// noise types for PlayerNoise
#define PNOISE_SELF				0
#define PNOISE_WEAPON			1
#define PNOISE_IMPACT			2


// edict->movetype values
typedef enum
{
MOVETYPE_NONE,			// never moves
MOVETYPE_NOCLIP,		// origin and angles change with no interaction
MOVETYPE_PUSH,			// no clip to world, push on box contact
MOVETYPE_STOP,			// no clip to world, stops on box contact

MOVETYPE_WALK,			// gravity
MOVETYPE_STEP,			// gravity, special edge handling
MOVETYPE_FLY,
MOVETYPE_TOSS,			// gravity
MOVETYPE_FLYMISSILE,	// extra size to monsters
MOVETYPE_BOUNCE
} movetype_t;


typedef struct
{
	char	command[128]; //limit to prevent malicious buffer overflows
	int		yay;
	int		nay;
	float	starttime;
	float	time;
	qboolean called;
} g_vote_t;

typedef struct
{
	int		base_count;
	int		max_count;
	float	normal_protection;
	float	energy_protection;
	int		armor;
} gitem_armor_t;


// gitem_t->flags
#define	IT_WEAPON		1		// use makes active weapon
#define	IT_AMMO			2
#define IT_ARMOR		4
#define IT_KEY			8
#define IT_POWERUP		16

// gitem_t->weapmodel for weapons indicates model index
#define WEAP_BLASTER			1
#define WEAP_SMARTGUN			2
#define WEAP_CHAINGUN			3
#define WEAP_GRAPPLE			4
#define WEAP_FLAMETHROWER		5
#define WEAP_ROCKETLAUNCHER		8
#define WEAP_DISRUPTOR			9
#define WEAP_BEAMGUN			10
#define WEAP_VAPORIZER			11
#define WEAP_VIOLATOR			12
#define WEAP_MINDERASER			13
#define WEAP_BOMBER				14
#define WEAP_STRAFER			15
#define WEAP_DEATHBALL			16
#define WEAP_HOVER				17



typedef struct gitem_s
{
	char		*classname;	// spawning name
	qboolean	(*pickup)(struct edict_s *ent, struct edict_s *other);
	void		(*use)(struct edict_s *ent, struct gitem_s *item);
	void		(*drop)(struct edict_s *ent, struct gitem_s *item);
	void		(*weaponthink)(struct edict_s *ent);
	char		*pickup_sound;
	char		*world_model;
	int			world_model_flags;
	char		*view_model;

	// client side info
	char		*icon;
	char		*pickup_name;	// for printing on pickup
	int			count_width;		// number of digits to display by icon

	int			quantity;		// for ammo how much, for weapons how much is used per shot
	char		*ammo;			// for weapons
	int			flags;			// IT_* flags

	int			weapmodel;		// weapon model index (for weapons)

	void		*info;
	int			tag;

	char		*precaches;		// string of all models, sounds, and images this item will use
} gitem_t;



//
// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
typedef struct
{
	char		helpmessage1[512];
	char		helpmessage2[512];
	int			helpchanged;	// flash F1 icon if non 0, play sound
								// and increment only if 1, 2, or 3

	gclient_t	*clients;		// [maxclients]

	// store latched cvars here that we want to get at often
	int			maxclients;
	int			maxentities;

	// cross level triggers
	int			serverflags;

	// items
	int			num_items;

	qboolean	autosaved;
} game_locals_t;


//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
typedef struct
{
	int			framenum;
	float		time;
	int			previousTime; //in milleseconds

	char		level_name[MAX_QPATH];	// the descriptive name (Outer Base, etc)
	char		mapname[MAX_QPATH];		// the server name (base1, etc)
	char		nextmap[MAX_QPATH];		// go here when fraglimit is hit

	// intermission state
	float		intermissiontime;		// time the intermission was started
	char		*changemap;
	int			exitintermission;
	vec3_t		intermission_origin;
	vec3_t		intermission_angle;

	edict_t		*sight_client;	// changed once each frame for coop games

	edict_t		*sight_entity;
	int			sight_entity_framenum;
	edict_t		*sound_entity;
	int			sound_entity_framenum;
	edict_t		*sound2_entity;
	int			sound2_entity_framenum;

	int			pic_health;

	int			total_secrets;
	int			found_secrets;

	int			total_goals;
	int			found_goals;

	int			total_monsters;
	int			killed_monsters;

	edict_t		*current_entity;	// entity running from G_RunFrame
	int			body_que;			// dead bodies

	//unlagged - backward reconciliation #4
	// actual time this server frame started
	float		frameStartTime;
	//unlagged - backward reconciliation #4

} level_locals_t;


// spawn_temp_t is only used to hold entity field values that
// can be set from the editor, but aren't actualy present
// in edict_t during gameplay
typedef struct
{
	// world vars
	char		*sky;
	float		skyrotate;
	vec3_t		skyaxis;
	char		*nextmap;

	int			lip;
	int			distance;
	int			height;
	char		*noise;
	float		pausetime;
	char		*item;
	char		*gravity;

	float		minyaw;
	float		maxyaw;
	float		minpitch;
	float		maxpitch;
} spawn_temp_t;


typedef struct
{
	// fixed data
	vec3_t		start_origin;
	vec3_t		start_angles;
	vec3_t		end_origin;
	vec3_t		end_angles;

	int			sound_start;
	int			sound_middle;
	int			sound_end;

	float		accel;
	float		speed;
	float		decel;
	float		distance;

	float		wait;

	// state data
	int			state;
	vec3_t		dir;
	float		current_speed;
	float		move_speed;
	float		next_speed;
	float		remaining_distance;
	float		decel_distance;
	void		(*endfunc)(edict_t *);
} moveinfo_t;


typedef struct
{
	void	(*aifunc)(edict_t *self, float dist);
	float	dist;
	void	(*thinkfunc)(edict_t *self);
} mframe_t;

typedef struct
{
	int			firstframe;
	int			lastframe;
	mframe_t	*frame;
	void		(*endfunc)(edict_t *self);
} mmove_t;

typedef struct
{
	mmove_t		*currentmove;
	int			aiflags;
	int			nextframe;
	float		scale;

	void		(*stand)(edict_t *self);
	void		(*idle)(edict_t *self);
	void		(*search)(edict_t *self);
	void		(*walk)(edict_t *self);
	void		(*run)(edict_t *self);
	void		(*dodge)(edict_t *self, edict_t *other, float eta);
	void		(*attack)(edict_t *self);
	void		(*melee)(edict_t *self);
	void		(*sight)(edict_t *self, edict_t *other);
	qboolean	(*checkattack)(edict_t *self);

	float		pausetime;
	float		attack_finished;

	vec3_t		saved_goal;
	float		search_time;
	float		trail_time;
	vec3_t		last_sighting;
	int			attack_state;
	int			lefty;
	float		idle_time;
	int			linkcount;

} monsterinfo_t;



extern	game_locals_t	game;
extern	level_locals_t	level;
extern	game_import_t	gi;
extern	game_export_t	globals;
extern	spawn_temp_t	st;
extern	g_vote_t		playervote;

extern	int	sm_meat_index;

extern  int red_team_score;
extern  int blue_team_score;
extern  int reddiff;
extern	int bluediff;
extern	int redwinning;
extern	int print1;
extern  int print2;
extern  int print3;

// means of death
#define MOD_UNKNOWN			0
#define MOD_BLASTER			1
#define MOD_VIOLATOR		2
#define MOD_CGALTFIRE		3
#define MOD_CHAINGUN		5
#define MOD_FLAME			6
#define MOD_ROCKET			8
#define MOD_R_SPLASH		9
#define MOD_BEAMGUN			10
#define MOD_DISRUPTOR		11
#define MOD_SMARTGUN		12
#define MOD_VAPORIZER		13
#define MOD_VAPORALTFIRE	14
#define MOD_PLASMA_SPLASH	16
#define MOD_WATER			17
#define MOD_SLIME			18
#define MOD_LAVA			19
#define MOD_CRUSH			20
#define MOD_TELEFRAG		21
#define MOD_FALLING			22
#define MOD_SUICIDE			23
#define MOD_CAMPING     	24
#define MOD_EXPLOSIVE		25
#define MOD_BARREL			26
#define MOD_BOMB			27
#define MOD_EXIT			28
#define MOD_SPLASH			29
#define MOD_TARGET_LASER	30
#define MOD_TRIGGER_HURT	31
#define MOD_DEATHRAY		32
#define MOD_TARGET_BLASTER	33
#define MOD_GRAPPLE			34
#define MOD_HEADSHOT		35
#define MOD_MINDERASER		36
#define MOD_FRIENDLY_FIRE   0x8000000

extern	int	meansOfDeath;

extern	edict_t			*g_edicts;

#define FOFS(x)(ptrdiff_t)&(((edict_t *)0)->x)
#define STOFS(x)(ptrdiff_t)&(((spawn_temp_t *)0)->x)
#define LLOFS(x)(ptrdiff_t)&(((level_locals_t *)0)->x)
#define CLOFS(x)(ptrdiff_t)&(((gclient_t *)0)->x)

#define random() ((rand() & 0x7fff) / ((float)0x7fff))
#define crandom() (2.0 * (random() - 0.5))

extern	cvar_t	*maxentities;
extern	cvar_t	*deathmatch;
extern  cvar_t  *ctf;
extern  cvar_t  *tca;
extern  cvar_t  *cp;
extern	cvar_t	*dmflags;
extern	cvar_t	*skill;
extern	cvar_t	*fraglimit;
extern	cvar_t	*timelimit;
extern	cvar_t	*password;
extern	cvar_t	*spectator_password;
extern	cvar_t	*needpass;
extern	cvar_t	*g_select_empty;
extern	cvar_t	*g_dedicated;
extern	cvar_t	*motdfile;
extern	cvar_t	*motdforce;

extern	cvar_t	*filterban;

extern	cvar_t	*sv_gravity;
extern	cvar_t	*sv_maxvelocity;

extern	cvar_t	*gun_x, *gun_y, *gun_z;
extern	cvar_t	*sv_rollspeed;
extern	cvar_t	*sv_rollangle;

extern	cvar_t	*run_pitch;
extern	cvar_t	*run_roll;
extern	cvar_t	*bob_up;
extern	cvar_t	*bob_pitch;
extern	cvar_t	*bob_roll;

extern	cvar_t	*sv_cheats;
extern	cvar_t	*g_maxclients;
extern	cvar_t	*maxspectators;

extern	cvar_t	*flood_msgs;
extern	cvar_t	*flood_persecond;
extern	cvar_t	*flood_waitdelay;

extern	cvar_t	*sv_maplist;

extern  cvar_t  *g_background_music;

extern  cvar_t  *sv_botkickthreshold;
extern  cvar_t	*sv_custombots;

//mutators
extern  cvar_t  *instagib;
extern  cvar_t  *rocket_arena;
extern  cvar_t *insta_rockets;
extern  cvar_t  *low_grav;
extern  cvar_t  *regeneration;
extern  cvar_t  *vampire;
extern  cvar_t  *excessive;
extern  cvar_t  *grapple;
extern  cvar_t  *classbased;

//duel mode
extern	cvar_t	*g_duel;

extern	cvar_t	*g_losehealth;
extern	cvar_t	*g_losehealth_num;

//weapons
extern	cvar_t	*wep_selfdmgmulti;

//health/max health/max ammo
extern	cvar_t	*g_spawnhealth;
extern	cvar_t	*g_maxhealth;
extern	cvar_t	*g_maxbullets;
extern	cvar_t	*g_maxshells;
extern	cvar_t	*g_maxrockets;
extern	cvar_t	*g_maxgrenades;
extern	cvar_t	*g_maxcells;
extern	cvar_t	*g_maxslugs;
extern	cvar_t	*g_maxseekers;

//quick weapon change
extern  cvar_t  *quickweap;

//anti-camp
extern  cvar_t  *anticamp;
extern  cvar_t  *camptime;
extern  cvar_t	*ac_frames;
extern  cvar_t	*ac_threshold;

extern	cvar_t  *g_randomquad;

//warmup time
extern cvar_t   *warmuptime;

//spawn protection
extern cvar_t   *g_spawnprotect;

//jousting
extern cvar_t   *joustmode;

//map voting
extern cvar_t	*g_mapvote;
extern cvar_t	*g_voterand;
extern cvar_t	*g_votemode;
extern cvar_t	*g_votesame;

//call voting
extern cvar_t	*g_callvote;

//forced autobalanced teams
extern cvar_t	*g_autobalance;

//reward point threshold
extern cvar_t	*g_reward;

//show player lights for visibility even in non-team games
extern cvar_t	*g_dmlights;

#define world	(&g_edicts[0])

// item spawnflags
#define ITEM_TRIGGER_SPAWN		0x00000001
#define ITEM_NO_TOUCH			0x00000002
// 6 bits reserved for editor flags
// 8 bits used as power cube id bits for coop games
#define DROPPED_ITEM			0x00010000
#define	DROPPED_PLAYER_ITEM		0x00020000
#define ITEM_TARGETS_USED		0x00040000

//
// fields are needed for spawning from the entity string
// and saving / loading games
//
#define FFL_SPAWNTEMP		1
#define FFL_NOSPAWN			2

typedef enum {
	F_INT,
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_EDICT,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory
	F_FUNCTION,
	F_MMOVE,
	F_IGNORE
} fieldtype_t;

typedef struct
{
	char	*name;
	int		ofs;
	fieldtype_t	type;
	int		flags;
} field_t;


extern	field_t fields[];
extern	gitem_t	itemlist[];

//mind eraser globals
extern float mindEraserTime;

//
// g_cmds.c
//
void Cmd_Help_f (edict_t *ent);
void Cmd_Score_f (edict_t *ent);
void Cmd_VoiceTaunt_f (edict_t *ent);

//
// g_items.c
//
void PrecacheItem (gitem_t *it);
void InitItems (void);
void SetItemNames (void);
gitem_t	*FindItem (char *pickup_name);
gitem_t	*FindItemByClassname (char *classname);
#define	ITEM_INDEX(x) ((x)-itemlist)
edict_t *Drop_Item (edict_t *ent, gitem_t *item);
void SetRespawn (edict_t *ent, float delay);
void ChangeWeapon (edict_t *ent);
void SpawnItem (edict_t *ent, gitem_t *item);
void DoRespawn (edict_t *ent);
void Think_Weapon (edict_t *ent);
int ArmorIndex (edict_t *ent);
gitem_t	*GetItemByIndex (int index);
qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count, qboolean weapon, qboolean dropped);
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

//CTF
void CTFDeadDropFlag(edict_t *self, edict_t *other);
void CTFResetFlag(int team);
void CTFEffects(edict_t *player);
void CTFScoreboardMessage (edict_t *ent, edict_t *killer, int mapvote);
void CTFPrecache(void);
void CTFFlagSetup (edict_t *ent);
qboolean CTFPickup_Flag (edict_t *ent, edict_t *other);
void CTFDrop_Flag(edict_t *ent, gitem_t *item);

//Vehicles
void Reset_player(edict_t *ent);
void Jet_Explosion(edict_t *ent);
qboolean Jet_Active(edict_t *ent);
void Jet_ApplyJet( edict_t *ent, usercmd_t *ucmd );
qboolean Jet_AvoidGround( edict_t *ent );
void VehicleDeadDrop(edict_t *self);
void VehicleSetup (edict_t *ent);
qboolean Get_in_vehicle (edict_t *ent, edict_t *other);
void Leave_vehicle(edict_t *ent, gitem_t *item);

//deathball
void ResetDeathball();
void DeadDropDeathball(edict_t *self);
void DeathballSetup (edict_t *ent);
qboolean Pickup_deathball (edict_t *ent, edict_t *other);
void DeathballDrop(edict_t *ent, gitem_t *item);

//
// g_utils.c
//
qboolean	KillBox (edict_t *ent);
void	G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
edict_t *G_Find (edict_t *from, int fieldofs, char *match);
edict_t *findradius (edict_t *from, vec3_t org, float rad);
edict_t *G_PickTarget (char *targetname);
void	G_UseTargets (edict_t *ent, edict_t *activator);
void	G_SetMovedir (vec3_t angles, vec3_t movedir);

void	G_InitEdict (edict_t *e);
edict_t	*G_Spawn (void);
void	G_FreeEdict (edict_t *e);

void	G_TouchTriggers (edict_t *ent);
void	G_TouchSolids (edict_t *ent);

char	*G_CopyString (const char *in);
void	G_CleanPlayerName ( const char *source, char *dest );

float	*tv (float x, float y, float z);
char	*vtos (vec3_t v);

float vectoyaw (vec3_t vec);
// void vectoangles (vec3_t vec, vec3_t angles);

//
// g_combat.c
//
qboolean OnSameTeam (edict_t *ent1, edict_t *ent2);
qboolean CanDamage (edict_t *targ, edict_t *inflictor);
void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod);
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod, int weapon);

// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002	// armour does not protect from this damage
#define DAMAGE_ENERGY			0x00000004	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK		0x00000008	// do not affect velocity, just view angles
#define DAMAGE_BULLET			0x00000010  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION	0x00000020  // armor, shields, invulnerability, and godmode have no effect

#define DEFAULT_BULLET_HSPREAD	300
#define DEFAULT_BULLET_VSPREAD	300
#define DEFAULT_SHOTGUN_HSPREAD	1000
#define DEFAULT_SHOTGUN_VSPREAD	500
#define DEFAULT_DEATHMATCH_SHOTGUN_COUNT	12
#define DEFAULT_SHOTGUN_COUNT	12
#define DEFAULT_SSHOTGUN_COUNT	15

//
// g_monster.c
//
void M_droptofloor (edict_t *ent);
void monster_think (edict_t *self);
void walkmonster_start (edict_t *self);
void swimmonster_start (edict_t *self);
void flymonster_start (edict_t *self);
void AttackFinished (edict_t *self, float time);
void monster_death_use (edict_t *self);
void M_CatagorizePosition (edict_t *ent);
qboolean M_CheckAttack (edict_t *self);
void M_FlyCheck (edict_t *self);
void M_CheckGround (edict_t *ent);
void M_SetEffects (edict_t *ent);

//
// g_misc.c
//
void ThrowClientHead (edict_t *self, int damage);
void ThrowGib (edict_t *self, char *gibname, int damage, int type, int effects);
void BecomeExplosion1 (edict_t *self);

//
// g_ai.c
//
void AI_SetSightClient (void);

void ai_stand (edict_t *self, float dist);
void ai_move (edict_t *self, float dist);
void ai_walk (edict_t *self, float dist);
void ai_turn (edict_t *self, float dist);
void ai_still (edict_t *self, float dist);
void ai_run (edict_t *self, float dist);
void ai_charge (edict_t *self, float dist);
int range (edict_t *self, edict_t *other);

void FoundTarget (edict_t *self);
qboolean infront (edict_t *self, edict_t *other);
qboolean visible (edict_t *self, edict_t *other);
qboolean FacingIdeal(edict_t *self);

//
// g_weapon.c
//
void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin);
void ThrowDebris2 (edict_t *self, char *modelname, float speed, vec3_t origin);

void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
void fire_blaster (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect, qboolean hyper);
void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_homingrocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_plasma (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);
void fire_bomb (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, float timer);
void fire_blaster_beam (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, qboolean detonate);
void fire_floater (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, float timer);
void fire_floater_alien (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, float timer);
void fire_flamethrower(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);
void fire_hover_beam (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, qboolean detonate);
void fire_minderaser (edict_t *self, vec3_t start, vec3_t dir, float timer);

//new weapon code
void fire_energy_field (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);
void fire_blasterball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper);
void fire_prox (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, float timer);
void fire_fireball (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float damage_radius, int radius_damage);
void fire_violator(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int alt);

//deathball
void fire_deathball (edict_t *self, vec3_t start, vec3_t dir, int speed);

//grapple
typedef enum {
	CTF_GRAPPLE_STATE_FLY,
	CTF_GRAPPLE_STATE_PULL,
	CTF_GRAPPLE_STATE_HANG
} ctfgrapplestate_t;

#define CTF_GRAPPLE_SPEED					1650 // speed of grapple in flight
#define CTF_GRAPPLE_PULL_SPEED				1650	// speed player is pulled at

// GRAPPLE
void CTFWeapon_Grapple (edict_t *ent);
void CTFPlayerResetGrapple(edict_t *ent);
void CTFGrapplePull(edict_t *self);
void CTFResetGrapple(edict_t *self);

//
// g_ptrail.c
//
void PlayerTrail_Init (void);
void PlayerTrail_Add (vec3_t spot);
void PlayerTrail_New (vec3_t spot);
edict_t *PlayerTrail_PickFirst (edict_t *self);
edict_t *PlayerTrail_PickNext (edict_t *self);
edict_t	*PlayerTrail_LastSpot (void);

//
// p_client.c
//
void respawn (edict_t *ent);
void BeginIntermission (edict_t *targ);
void EndIntermission (void);
void PutClientInServer (edict_t *ent);
void InitClientPersistant (gclient_t *client);
void InitClientResp (gclient_t *client);
void InitBodyQue (void);
void ClientBeginServerFrame (edict_t *ent);
void Q2_FindFile (char *filename, FILE **file);
void ClientPlaceInQueue(edict_t *ent);
void ClientCheckQueue(edict_t *ent);
void MoveClientsDownQueue(edict_t *ent);
void SaveClientData (void);
void FetchClientEntData (edict_t *ent);
void TeamCensus( teamcensus_t* team_census );

//unlagged - g_unlagged.c
void G_ResetHistory( edict_t *ent );
void G_StoreHistory( edict_t *ent );
void G_TimeShiftAllClients( int time, edict_t *skip );
void G_UnTimeShiftAllClients( edict_t *skip );
void G_DoTimeShiftFor( edict_t *ent );
void G_UndoTimeShiftFor( edict_t *ent );
void G_UnTimeShiftClient( edict_t *ent );
void G_PredictPlayerMove( edict_t *ent, float frametime );
//unlagged - g_unlagged.c

//
// g_player.c
//
void player_pain (edict_t *self, edict_t *other, float kick, int damage);
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

//
// g_svcmds.c
//
void	ServerCommand (void);
qboolean SV_FilterPacket (char *from);

//
// p_view.c
//
void ClientEndServerFrame (edict_t *ent);

//
// p_hud.c
//
void MoveClientToIntermission (edict_t *client);
void G_SetStats (edict_t *ent);
void G_SetSpectatorStats (edict_t *ent);
void G_CheckChaseStats (edict_t *ent);
void ValidateSelectedItem (edict_t *ent);
void DeathmatchScoreboardMessage (edict_t *client, edict_t *killer, int mapvote);

//
// p_weapon.c
//
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));
void PlayerNoise(edict_t *who, vec3_t where, int type);

//
// m_move.c
//
qboolean M_CheckBottom (edict_t *ent);
qboolean M_walkmove (edict_t *ent, float yaw, float dist);
void M_MoveToGoal (edict_t *ent, float dist);
void M_ChangeYaw (edict_t *ent);

//
// g_phys.c
//
void G_RunEntity (edict_t *ent);


//
// g_chase.c
//
void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);
void GetChaseTarget(edict_t *ent);

//
// g_main.c
//
qboolean G_NameMatch( const char* netname, const char *kickname );


//============================================================================

// client_t->anim_priority
#define	ANIM_BASIC		0		// stand / run
#define	ANIM_WAVE		1
#define	ANIM_JUMP		2
#define	ANIM_PAIN		3
#define	ANIM_ATTACK		4
#define	ANIM_DEATH		5
#define	ANIM_REVERSE	6

#define BASE_ROCKETS	10
#define BASE_SHELLS		10
#define BASE_CELLS		50
#define BASE_SLUGS		10
#define BASE_GRENADES	50
#define BASE_BULLETS	50
#define BASE_SEEKERS	1


// client data that stays across multiple level loads
typedef struct
{
	char		userinfo[MAX_INFO_STRING];

	char netname[(PLAYERNAME_GLYPHS*3)+1]; // name from cvar
			// room for "rainbow" name, but is truncated to PLAYERNAME_SIZE

	int			hand;

	qboolean	connected;			// a loadgame will leave valid entities that
									// just don't have a connection yet

	// values saved and restored from edicts when changing levels
	int			health;
	int			max_health;
	int			savedFlags;

	int			selected_item;
	int			inventory[MAX_ITEMS];

	// ammo capacities
	int			max_bullets;
	int			max_shells;
	int			max_rockets;
	int			max_grenades;
	int			max_cells;
	int			max_slugs;
	int			max_seekers;

	gitem_t		*weapon;
	gitem_t		*lastweapon;
	gitem_t     *lastfailedswitch;
	float       failedswitch_framenum;

	int			game_helpchanged;
	int			helpchanged;

	int			spectator;			// client is a spectator
	int	queue;				// client queue position for duel mode

	//unlagged - client options
	// these correspond with variables in the userinfo string
	int			delag;
	int			debugDelag;
	int			cmdTimeNudge;
//unlagged - client options
//unlagged - lag simulation #2
	int			latentSnaps;
	int			latentCmds;
	int			plOut;
	usercmd_t	cmdqueue[MAX_LATENT_CMDS];
	int			cmdhead;
//unlagged - lag simulation #2
//unlagged - true ping
	int			realPing;
	int			pingsamples[NUM_PING_SAMPLES];
	int			samplehead;
//unlagged - true ping

} client_persistant_t;

//unlagged - backward reconciliation #1
// the size of history we'll keep
#define NUM_CLIENT_HISTORY 17

// everything we need to know to backward reconcile
typedef struct {
	vec3_t		mins, maxs;
	vec3_t		currentOrigin;
	int			leveltime;
} clientHistory_t;
//unlagged - backward reconciliation #1

// client data that stays across deathmatch respawns
typedef struct
{
	int			enterframe;			// level.framenum the client entered the game
	int			score;				// frags, etc
	int			deaths;				// deaths, etc
	vec3_t		cmd_angles;			// angles sent over in the last command

	//bot score info
	int botnum;
	bot_t bots[100];

	//weapon accuracy info
	int weapon_shots[9];
	int weapon_hits[9];

	//reward points
	int reward_pts;
	qboolean powered;

	//voting
	qboolean voted;

	int			spectator;			// client is a spectator
} client_respawn_t;

// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
struct gclient_s
{
	// known to server
	player_state_t	ps;				// communicated by server to clients
	int				ping;

	// private to game
	client_persistant_t	pers;
	client_respawn_t	resp;
	pmove_state_t		old_pmove;	// for detecting out-of-pmove changes

	qboolean	showscores;			// set layout stat
	qboolean	showinventory;		// set layout stat
	qboolean	showhelp;
	qboolean	showhelpicon;

	/** @brief "Message of the day" frames left
	 *
	 * Prevent centerprinting until the MOTD has been displayed
	 * for some time. This makes sure players can actually read
	 * the MOTD.
	 */
	int		motd_frames;

	//stuff for bots
	qboolean	is_bot;
	/* --------------------------*/
	/*
	 * this is identical to declarations in edict_t
	 * it appears this data in gclient_s is not used.
	 */
	int			skill;
	char		faveweap[64];
	float		weapacc[10];
	float		accuracy;
	float		awareness;
	char		chatmsg1[128];
	char		chatmsg2[128];
	char		chatmsg3[128];
	char		chatmsg4[128];
	char		chatmsg5[128];
	char		chatmsg6[128];
	char		chatmsg7[128];
	char		chatmsg8[128];
	/*----------------------------*/

	int			ammo_index;

	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	qboolean	weapon_thunk;

	gitem_t		*newweapon;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation

	float		killer_yaw;			// when dead, look at killer

	weaponstate_t	weaponstate;
	vec3_t		kick_angles;	// weapon kicks
	vec3_t		kick_origin;
	float		v_dmg_roll, v_dmg_pitch, v_dmg_time;	// damage kicks
	float		fall_time, fall_value;		// for view drop on fall
	float		damage_alpha;
	float		bonus_alpha;
	vec3_t		damage_blend;
	vec3_t		v_angle;			// aiming direction
	float		bobtime;			// so off-ground doesn't change it
	vec3_t		oldviewangles;
	vec3_t		oldvelocity;

	float		next_drown_time;
	int			old_waterlevel;
	int			breather_sound;

	int			machinegun_shots;	// for weapon raising

	// animation vars
	int			anim_end;
	int			anim_priority;
	qboolean	anim_duck;
	qboolean	anim_run;

	// powerup timers
	float		quad_framenum;
	float		invincible_framenum;
	float		haste_framenum;
	float		sproing_framenum;
	float		regen_framenum;
	float		invis_framenum;

	float		losehealth_framenum;

	qboolean	grenade_blew_up;
	float		grenade_time;
	int			silencer_shots;
	int			weapon_sound;

	float		pickup_msg_time;

	float		flood_locktill;		// locked from talking
	float		flood_when[10];		// when messages were said
	int			flood_whenhead;		// head pointer for when said

	float		respawn_time;		// can respawn when time > this

	edict_t		*chase_target;		// player we are chasing
	qboolean	update_chase;		// need to update chase info?

	//team
	int			dmteam;

	//chasecam
	int             chasetoggle;
    edict_t         *chasecam;
    edict_t         *oldplayer;

	//vehicles
	qboolean in_vehicle;
	float	Jet_framenum;   /*burn out time when jet is activated*/
    float	Jet_remaining;  /*remaining fuel time*/
    float	Jet_next_think;

	//deathball
	qboolean in_deathball;

	//grapple
	void		*ctf_grapple;		// entity of grapple
	int			ctf_grapplestate;		// true if pulling
	float		ctf_grapplereleasetime;	// time of grapple release

	//kill streaks
	int kill_streak; //obviously this *will* get cleared at each respawn in most cases

	//joust attempts - for limiting how many times you can joust at a time
	int joustattempts;

	//homing rocket limits
	int	homing_shots;

	//movetime - used for special moves
	float lastmovetime;
	int lastsidemove;
	int dodge;
	int moved;
	float lastdodge;

	float spawnprotecttime;
	qboolean spawnprotected;

	//map voting
	int mapvote;

	//taunt message
	float lasttaunttime;

	//deathray immunity
	qboolean rayImmunity;
	float rayTime;

	//unlagged - backward reconciliation #1
	// the serverTime the button was pressed
	// (stored before pmove_fixed changes serverTime)
	int			attackTime;
	// the head of the history queue
	int			historyHead;
	// the history queue
	clientHistory_t	history[NUM_CLIENT_HISTORY];
	// the client's saved position
	clientHistory_t	saved;			// used to restore after time shift
	// an approximation of the actual server time we received this
	// command (not in 50ms increments)
	int			frameOffset;
//unlagged - backward reconciliation #1

//unlagged - smooth clients #1
	// the last frame number we got an update from this client
	int			lastUpdateFrame;
//unlagged - smooth clients #1

};


struct edict_s
{
	entity_state_t	s;
	struct gclient_s	*client;	// NULL if not a player
									// the server expects the first part
									// of gclient_s to be a player_state_t
									// but the rest of it is opaque

	qboolean	inuse;
	int			linkcount;

	// FIXME: move these fields to a server private sv_entity_t
	link_t		area;				// linked to a division node or leaf

	int			num_clusters;		// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			headnode;			// unused if num_clusters != -1
	int			areanum, areanum2;

	//================================

	int			svflags;
	vec3_t		mins, maxs;
	vec3_t		absmin, absmax, size;
	solid_t		solid;
	int			clipmask;
	edict_t		*owner;


	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!

	//================================
	int			movetype;
	int			flags;

	char		*model;
	float		freetime;			// sv.time when the object was freed

	//
	// only used locally in game, not by server
	//
	char		*message;
	char		*classname;
	int			spawnflags;

	float		timestamp;

	float		angle;			// set in qe3, -1 = up, -2 = down
	char		*target;
	char		*targetname;
	char		*killtarget;
	char		*team;
	char		*pathtarget;
	char		*deathtarget;
	char		*combattarget;
	edict_t		*target_ent;

	float		speed, accel, decel;
	vec3_t		movedir;
	vec3_t		pos1, pos2;

	vec3_t		velocity;
	vec3_t		avelocity;
	int			mass;
	float		air_finished;
	float		gravity;		// per entity gravity multiplier (1.0 is normal)
								// use for lowgrav artifact, flares

	edict_t		*goalentity;
	edict_t		*movetarget;
	float		yaw_speed;
	float		ideal_yaw;

	float		nextthink;
	void		(*prethink) (edict_t *ent);
	void		(*think)(edict_t *self);
	void		(*blocked)(edict_t *self, edict_t *other);	//move to moveinfo?
	void		(*touch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
	void		(*use)(edict_t *self, edict_t *other, edict_t *activator);
	void		(*pain)(edict_t *self, edict_t *other, float kick, int damage);
	void		(*die)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

	float		touch_debounce_time;		// are all these legit?  do we need more/less of them?
	float		pain_debounce_time;
	float		damage_debounce_time;
	float		fly_sound_debounce_time;	//move to clientinfo
	float		last_move_time;

	int			health;
	int			max_health;
	int			gib_health;
	int			deadflag;
	qboolean	show_hostile;

	char		*map;			// target_changelevel

	int			viewheight;		// height above origin where eyesight is determined
	int			takedamage;
	int			dmg;
	int			radius_dmg;
	float		dmg_radius;
	int			sounds;			//make this a spawntemp var?
	int			count;

	edict_t		*chain;
	edict_t		*enemy;
	edict_t		*oldenemy;
	edict_t		*activator;
	edict_t		*groundentity;
	int			groundentity_linkcount;
	edict_t		*teamchain;
	edict_t		*teammaster;

	edict_t		*mynoise;		// can go in client only
	edict_t		*mynoise2;

	int			noise_index;
	int			noise_index2;
	float		volume;
	float		attenuation;

	// timing variables
	float		wait;
	float		delay;			// before firing targets
	float		random;

	float		teleport_time;

	int			watertype;
	int			waterlevel;

	vec3_t		move_origin;
	vec3_t		move_angles;

	// move this to clientinfo?
	int			light_level;

	int			style;			// also used as areaportal number

	gitem_t		*item;			// for bonus items

	// common data blocks
	moveinfo_t		moveinfo;
	monsterinfo_t	monsterinfo;

	edict_t		*flashlight;
	
	edict_t		*replaced_weapon;

	int			dmteam;
	int			teamset;

// ACEBOT_ADD
	qboolean is_bot;
	qboolean is_jumping;

	// For movement
	vec3_t move_vector;
	float next_move_time;
	float wander_timeout;
	float suicide_timeout;

	// For node code
	int current_node; // current node
	int goal_node; // current goal node
	int next_node; // the node that will take us one step closer to our goal
	int node_timeout;
	int last_node;
	int tries;
	int state;

	//for config
	int skill;
	char faveweap[64];
	float weapacc[10];
	float accuracy;
	float awareness;
	char		chatmsg1[128];
	char		chatmsg2[128];
	char		chatmsg3[128];
	char		chatmsg4[128];
	char		chatmsg5[128];
	char		chatmsg6[128];
	char		chatmsg7[128];
	char		chatmsg8[128];

// ACEBOT_END
	//chasecam
	int chasedist1;
	int chasedist2;

	// for deathcam to remember position of player's death
	vec3_t death_origin;

	//vehicles
	qboolean in_vehicle;

	//deathball
	qboolean in_deathball;

	//Flamethrower and fire vars
	float	 FlameDelay;
	int	 Flames;
	int	 FlameDamage;
	edict_t *orb;
	
	//Prox mines and grenades
	int		nade_timer;

	//alt-fires
	qboolean altfire;

	//power nodes
	qboolean powered;

	//class(human = 0; alien = 1; robot = 2;)
	int ctype;

	//model specific gibs
	int usegibs;
	char head[64];
	char leg[64];
	char arm[64];
	char body[64];

	// Anti-camp velocity accumulation
	
	/** @brief Velocities history
	 *
	 * At each server frame, a client's velocity will be stored inside
	 * this array. While the array may contain up to 100 vectors, the
	 * ac_frames CVar will determine how many of these vectors will be
	 * used.
	 */
	vec3_t old_velocities[ 100 ];

	/** @brief Velocity accumulator
	 *
	 * This vector contains the sum of velocities for the past frames.
	 * The amount of frames that are indeed accumulated depends on the
	 * value of the ac_frames CVar.
	 */
	vec3_t velocity_accum;

	/** @brief Velocities history index
	 *
	 * The last updated vector in the old_velocities array.
	 */
	int old_velocities_current;

	/** @brief Stored velocity vectors
	 *
	 * The actual amount of vectors currently stored in the old_velocities
	 * array.
	 */
	int old_velocities_count;

};

struct gmapvote_s
{
	char	mapname[32]; //ugg, end the pointer madness
	int		tally;
} votedmap[4];

extern char *winningmap;

extern void CheckDeathcam_Viewent(edict_t *ent);
extern void DeathcamRemove (edict_t *ent, char *opt);
extern void DeathcamStart (edict_t *ent);
extern void DeathcamTrack (edict_t *ent);

//IP information
typedef struct
{
	unsigned	mask;
	unsigned	compare;
} ipfilter_t;

#define	MAX_IPFILTERS	1024

ipfilter_t	ipfilters[MAX_IPFILTERS];
int			numipfilters;
extern qboolean StringToFilter (char *s, ipfilter_t *f);

//unlagged
extern	cvar_t	*g_antilag;
extern	cvar_t	*g_antilag_latency;
extern  cvar_t	*g_antilagdebug;

// ACEBOT_ADD
#include "acesrc/acebot.h"
// ACEBOT_END


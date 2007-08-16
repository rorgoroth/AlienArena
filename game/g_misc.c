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
// g_misc.c

#include "g_local.h"


/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.
*/

//=====================================================

void Use_Areaportal (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->count ^= 1;		// toggle state
//	gi.dprintf ("portalstate: %i = %i\n", ent->style, ent->count);
	gi.SetAreaPortalState (ent->style, ent->count);
}

/*QUAKED func_areaportal (0 0 0) ?

This is a non-visible object that divides the world into
areas that are seperated when this portal is not activated.
Usually enclosed in the middle of a door.
*/
void SP_func_areaportal (edict_t *ent)
{
	ent->use = Use_Areaportal;
	ent->count = 0;		// always start closed;
}

//=====================================================


/*
=================
Misc functions
=================
*/
void VelocityForDamage (int damage, vec3_t v)
{
	v[0] = 100.0 * crandom();
	v[1] = 100.0 * crandom();
	v[2] = 200.0 + 100.0 * random();

	if (damage < 50)
		VectorScale (v, 0.7, v);
	else 
		VectorScale (v, 1.2, v);
}

void ClipGibVelocity (edict_t *ent)
{
	if (ent->velocity[0] < -300)
		ent->velocity[0] = -300;
	else if (ent->velocity[0] > 300)
		ent->velocity[0] = 300;
	if (ent->velocity[1] < -300)
		ent->velocity[1] = -300;
	else if (ent->velocity[1] > 300)
		ent->velocity[1] = 300;
	if (ent->velocity[2] < 200)
		ent->velocity[2] = 200;	// always some upwards
	else if (ent->velocity[2] > 500)
		ent->velocity[2] = 500;
}


/*
=================
gibs
=================
*/
void gib_think (edict_t *self)
{
	self->s.frame++;
	self->nextthink = level.time + FRAMETIME;

	if (self->s.frame == 10)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 8 + random()*10;
	}
}

void gib_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	normal_angles, right;

	if (!self->groundentity)
		return;

	self->touch = NULL;

	if (plane)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/fhit3.wav"), 1, ATTN_NORM, 0);

		vectoangles (plane->normal, normal_angles);
		AngleVectors (normal_angles, NULL, right, NULL);
		vectoangles (right, self->s.angles);

		if (self->s.modelindex == sm_meat_index)
		{
			self->s.frame = 0;//++;
			self->think = gib_think;
			self->nextthink = level.time + FRAMETIME;
		}
	}
}

void gib_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict (self);
}

void ThrowGib (edict_t *self, char *gibname, int damage, int type, int effects)
{
	edict_t *gib;
	vec3_t	vd;
	vec3_t	origin;
	vec3_t	size;
	float	vscale;

	gib = G_Spawn();

	VectorScale (self->size, 0.5, size);
	VectorAdd (self->absmin, size, origin);
	gib->s.origin[0] = origin[0] + crandom() * size[0];
	gib->s.origin[1] = origin[1] + crandom() * size[1];
	gib->s.origin[2] = origin[2] + crandom() * size[2];

	gi.setmodel (gib, gibname);
	gib->solid = SOLID_NOT;
	gib->s.effects = effects;
	gib->flags |= FL_NO_KNOCKBACK;
	gib->takedamage = DAMAGE_YES;
	gib->die = gib_die;

	if (type == GIB_ORGANIC)
	{
		gib->movetype = MOVETYPE_BOUNCE;
		gib->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		gib->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}

	VelocityForDamage (damage, vd);
	VectorMA (self->velocity, vscale, vd, gib->velocity);
	ClipGibVelocity (gib);
	gib->avelocity[0] = random()*600;
	gib->avelocity[1] = random()*600;
	gib->avelocity[2] = random()*600;

	gib->think = G_FreeEdict;
	gib->nextthink = level.time + 10 + random()*10;

	gi.linkentity (gib);
}

void ThrowBloodStain (edict_t *self, char *gibname)
{
	edict_t *blood;

	blood = G_Spawn();

	blood->s.origin[0] = self->s.origin[0];
	blood->s.origin[1] = self->s.origin[1];
	blood->s.origin[2] = self->s.origin[2] - 16;

	gi.setmodel (blood, gibname);
	blood->solid = SOLID_NOT;
	
	blood->takedamage = DAMAGE_NO;
	blood->die = gib_die;

	blood->movetype = MOVETYPE_NONE;
	
	blood->think = G_FreeEdict;
	blood->nextthink = level.time + 10 + random()*10;

	gi.linkentity (blood);
}

void ThrowHead (edict_t *self, char *gibname, int damage, int type, int effects)
{
	vec3_t	vd;
	float	vscale;

	self->s.skinnum = 0;
	self->s.frame = 0;
	VectorClear (self->mins);
	VectorClear (self->maxs);

	self->s.modelindex2 = 0;
	gi.setmodel (self, gibname);
	self->solid = SOLID_NOT;
	self->s.effects = effects;
	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;
	self->svflags &= ~SVF_MONSTER;
	self->takedamage = DAMAGE_YES;
	self->die = gib_die;

	if (type == GIB_ORGANIC)
	{
		self->movetype = MOVETYPE_TOSS;
		self->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		self->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}

	VelocityForDamage (damage, vd);
	VectorMA (self->velocity, vscale, vd, self->velocity);
	ClipGibVelocity (self);

	self->avelocity[YAW] = crandom()*600;

	self->think = G_FreeEdict;
	self->nextthink = level.time + 10 + random()*10;

	gi.linkentity (self);
}

void ThrowClientHead (edict_t *self, int damage)
{
	vec3_t	vd;
	char	*gibname;

	if (rand()&1)
	{
		gibname = "models/objects/gibs/sm_meat/tris.md2";
		self->s.skinnum = 0;		// second skin is player
	}
	else
	{
		gibname = "models/objects/gibs/sm_meat/tris.md2";
		self->s.skinnum = 0;
	}

	self->s.origin[2] += 32;
	self->s.frame = 0;
	gi.setmodel (self, gibname);
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 16);

	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->s.effects = EF_GIB;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;

	self->movetype = MOVETYPE_BOUNCE;
	VelocityForDamage (damage, vd);
	VectorAdd (self->velocity, vd, self->velocity);

	if (self->client)	// bodies in the queue don't have a client anymore
	{
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = self->s.frame;
	}
	else
	{
		self->think = NULL;
		self->nextthink = 0;
	}

	gi.linkentity (self);
}


/*
=================
debris
=================
*/
void debris_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict (self);
}

void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin)
{ 
	edict_t	*chunk;
	vec3_t	v;

	chunk = G_Spawn();
	VectorCopy (origin, chunk->s.origin);
	gi.setmodel (chunk, modelname);
	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
	VectorMA (self->velocity, speed, v, chunk->velocity);
	chunk->movetype = MOVETYPE_BOUNCE;
	chunk->solid = SOLID_NOT;
	chunk->avelocity[0] = random()*600;
	chunk->avelocity[1] = random()*600;
	chunk->avelocity[2] = random()*600;
	chunk->s.effects |= EF_ROCKET;
	chunk->think = G_FreeEdict;
	chunk->nextthink = level.time + 5 + random()*5;
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "debris";
	chunk->takedamage = DAMAGE_YES;
	chunk->die = debris_die;
	gi.linkentity (chunk);
}

void ThrowDebris2 (edict_t *self, char *modelname, float speed, vec3_t origin)
{ 
	edict_t	*chunk;
	vec3_t	v;

	chunk = G_Spawn();
	VectorCopy (origin, chunk->s.origin);
	gi.setmodel (chunk, modelname);
	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
	VectorMA (self->velocity, speed, v, chunk->velocity);
	chunk->movetype = MOVETYPE_BOUNCE;
	chunk->solid = SOLID_NOT;
	chunk->avelocity[0] = random()*600;
	chunk->avelocity[1] = random()*600;
	chunk->avelocity[2] = random()*600;
	chunk->think = G_FreeEdict;
	chunk->nextthink = level.time + 5 + random()*5;
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "debris";
	chunk->takedamage = DAMAGE_YES;
	chunk->die = debris_die;
	gi.linkentity (chunk);
}

void BecomeExplosion1 (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict (self);
}


void BecomeExplosion2 (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION2);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict (self);
}


/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT
Target: next path corner
Pathtarget: gets used when an entity that has
	this path_corner targeted touches it
*/

void path_corner_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		v;
	edict_t		*next;

	if (other->movetarget != self)
		return;
	
	if (other->enemy)
		return;

	if (self->pathtarget)
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		G_UseTargets (self, other);
		self->target = savetarget;
	}

	if (self->target)
		next = G_PickTarget(self->target);
	else
		next = NULL;

	if ((next) && (next->spawnflags & 1))
	{
		VectorCopy (next->s.origin, v);
		v[2] += next->mins[2];
		v[2] -= other->mins[2];
		VectorCopy (v, other->s.origin);
		next = G_PickTarget(next->target);
		other->s.event = EV_OTHER_TELEPORT;
	}

	other->goalentity = other->movetarget = next;

	if (self->wait)
	{
		other->monsterinfo.pausetime = level.time + self->wait;
		other->monsterinfo.stand (other);
		return;
	}

	if (!other->movetarget)
	{
		other->monsterinfo.pausetime = level.time + 100000000;
		other->monsterinfo.stand (other);
	}
	else
	{
		VectorSubtract (other->goalentity->s.origin, other->s.origin, v);
		other->ideal_yaw = vectoyaw (v);
	}
}

void SP_path_corner (edict_t *self)
{
	if (!self->targetname)
	{
		gi.dprintf ("path_corner with no targetname at %s\n", vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	self->solid = SOLID_TRIGGER;
	self->touch = path_corner_touch;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	self->svflags |= SVF_NOCLIENT;
	gi.linkentity (self);
}


/*QUAKED point_combat (0.5 0.3 0) (-8 -8 -8) (8 8 8) Hold
Makes this the target of a monster and it will head here
when first activated before going after the activator.  If
hold is selected, it will stay here.
*/
void point_combat_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*activator;

	if (other->movetarget != self)
		return;

	if (self->target)
	{
		other->target = self->target;
		other->goalentity = other->movetarget = G_PickTarget(other->target);
		if (!other->goalentity)
		{
			gi.dprintf("%s at %s target %s does not exist\n", self->classname, vtos(self->s.origin), self->target);
			other->movetarget = self;
		}
		self->target = NULL;
	}
	else if ((self->spawnflags & 1) && !(other->flags & (FL_SWIM|FL_FLY)))
	{
		other->monsterinfo.pausetime = level.time + 100000000;
		other->monsterinfo.aiflags |= AI_STAND_GROUND;
		other->monsterinfo.stand (other);
	}

	if (other->movetarget == self)
	{
		other->target = NULL;
		other->movetarget = NULL;
		other->goalentity = other->enemy;
		other->monsterinfo.aiflags &= ~AI_COMBAT_POINT;
	}

	if (self->pathtarget)
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		if (other->enemy && other->enemy->client)
			activator = other->enemy;
		else if (other->oldenemy && other->oldenemy->client)
			activator = other->oldenemy;
		else if (other->activator && other->activator->client)
			activator = other->activator;
		else
			activator = other;
		G_UseTargets (self, activator);
		self->target = savetarget;
	}
}

void SP_point_combat (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	self->solid = SOLID_TRIGGER;
	self->touch = point_combat_touch;
	VectorSet (self->mins, -8, -8, -16);
	VectorSet (self->maxs, 8, 8, 16);
	self->svflags = SVF_NOCLIENT;
	gi.linkentity (self);
};

/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for spotlights, etc.
*/
void SP_info_null (edict_t *self)
{
	G_FreeEdict (self);
};


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for lightning.
*/
void SP_info_notnull (edict_t *self)
{
	VectorCopy (self->s.origin, self->absmin);
	VectorCopy (self->s.origin, self->absmax);
};


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) START_OFF
Non-displayed light.
Default light value is 300.
Default style is 0.
If targeted, will toggle between on and off.
Default _cone value is 10 (used to set size of light for spotlights)
*/

#define START_OFF	1

static void light_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & START_OFF)
	{
		gi.configstring (CS_LIGHTS+self->style, "m");
		self->spawnflags &= ~START_OFF;
	}
	else
	{
		gi.configstring (CS_LIGHTS+self->style, "a");
		self->spawnflags |= START_OFF;
	}
}

void SP_light (edict_t *self)
{
	// no targeted lights in deathmatch, because they cause global messages
	if (!self->targetname || deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if (self->style >= 32)
	{
		self->use = light_use;
		if (self->spawnflags & START_OFF)
			gi.configstring (CS_LIGHTS+self->style, "a");
		else
			gi.configstring (CS_LIGHTS+self->style, "m");
	}
}


/*QUAKED func_wall (0 .5 .8) ? TRIGGER_SPAWN TOGGLE START_ON ANIMATED ANIMATED_FAST
This is just a solid wall if not inhibited

TRIGGER_SPAWN	the wall will not be present until triggered
				it will then blink in to existance; it will
				kill anything that was in it's way

TOGGLE			only valid for TRIGGER_SPAWN walls
				this allows the wall to be turned on and off

START_ON		only valid for TRIGGER_SPAWN walls
				the wall will initially be present
*/

void func_wall_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
	{
		self->solid = SOLID_BSP;
		self->svflags &= ~SVF_NOCLIENT;
		KillBox (self);
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}
	gi.linkentity (self);

	if (!(self->spawnflags & 2))
		self->use = NULL;
}

void SP_func_wall (edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel (self, self->model);

	if (self->spawnflags & 8)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 16)
		self->s.effects |= EF_ANIM_ALLFAST;

	// just a wall
	if ((self->spawnflags & 7) == 0)
	{
		self->solid = SOLID_BSP;
		gi.linkentity (self);
		return;
	}

	// it must be TRIGGER_SPAWN
	if (!(self->spawnflags & 1))
	{
//		gi.dprintf("func_wall missing TRIGGER_SPAWN\n");
		self->spawnflags |= 1;
	}

	// yell if the spawnflags are odd
	if (self->spawnflags & 4)
	{
		if (!(self->spawnflags & 2))
		{
			gi.dprintf("func_wall START_ON without TOGGLE\n");
			self->spawnflags |= 2;
		}
	}

	self->use = func_wall_use;
	if (self->spawnflags & 4)
	{
		self->solid = SOLID_BSP;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}
	gi.linkentity (self);
}


/*QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
This is solid bmodel that will fall if it's support it removed.
*/

void func_object_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// only squash thing we fall on top of
	if (!plane)
		return;
	if (plane->normal[2] < 1.0)
		return;
	if (other->takedamage == DAMAGE_NO)
		return;
	T_Damage (other, self, self, vec3_origin, self->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void func_object_release (edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->touch = func_object_touch;
}

void func_object_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox (self);
	func_object_release (self);
}

void SP_func_object (edict_t *self)
{
	gi.setmodel (self, self->model);

	self->mins[0] += 1;
	self->mins[1] += 1;
	self->mins[2] += 1;
	self->maxs[0] -= 1;
	self->maxs[1] -= 1;
	self->maxs[2] -= 1;

	if (!self->dmg)
		self->dmg = 100;

	if (self->spawnflags == 0)
	{
		self->solid = SOLID_BSP;
		self->movetype = MOVETYPE_PUSH;
		self->think = func_object_release;
		self->nextthink = level.time + 2 * FRAMETIME;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->movetype = MOVETYPE_PUSH;
		self->use = func_object_use;
		self->svflags |= SVF_NOCLIENT;
	}

	if (self->spawnflags & 2)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 4)
		self->s.effects |= EF_ANIM_ALLFAST;

	self->clipmask = MASK_MONSTERSOLID;

	gi.linkentity (self);
}


/*QUAKED func_explosive (0 .5 .8) ? Trigger_Spawn ANIMATED ANIMATED_FAST
Any brush that you want to explode or break apart.  If you want an
ex0plosion, set dmg and it will do a radius explosion of that amount
at the center of the bursh.

If targeted it will not be shootable.

health defaults to 100.

mass defaults to 75.  This determines how much debris is emitted when
it explodes.  You get one large chunk per 100 of mass (up to 8) and
one small chunk per 25 of mass (up to 16).  So 800 gives the most.
*/
void func_explosive_explode (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t	origin;
	vec3_t	chunkorigin;
	vec3_t	size;
	int		count;
	int		mass;

	// bmodel origins are (0 0 0), we need to adjust that here
	VectorScale (self->size, 0.5, size);
	VectorAdd (self->absmin, size, origin);
	VectorCopy (origin, self->s.origin);

	self->takedamage = DAMAGE_NO;

	if (self->dmg)
		T_RadiusDamage (self, attacker, self->dmg, NULL, self->dmg+40, MOD_EXPLOSIVE, -1);

	VectorSubtract (self->s.origin, inflictor->s.origin, self->velocity);
	VectorNormalize (self->velocity);
	VectorScale (self->velocity, 150, self->velocity);

	// start chunks towards the center
	VectorScale (size, 0.5, size);

	mass = self->mass;
	if (!mass)
		mass = 75;

	// big chunks
	if (mass >= 100)
	{
		count = mass / 100;
		if (count > 8)
			count = 8;
		while(count--)
		{
			chunkorigin[0] = origin[0] + crandom() * size[0];
			chunkorigin[1] = origin[1] + crandom() * size[1];
			chunkorigin[2] = origin[2] + crandom() * size[2];
			ThrowDebris (self, "models/objects/debris1/tris.md2", 1, chunkorigin);
		}
	}

	// small chunks
	count = mass / 25;
	if (count > 16)
		count = 16;
	while(count--)
	{
		chunkorigin[0] = origin[0] + crandom() * size[0];
		chunkorigin[1] = origin[1] + crandom() * size[1];
		chunkorigin[2] = origin[2] + crandom() * size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", 2, chunkorigin);
	}

	G_UseTargets (self, attacker);

	if (self->dmg)
		BecomeExplosion1 (self);
	else
		G_FreeEdict (self);
}

void func_explosive_use(edict_t *self, edict_t *other, edict_t *activator)
{
	func_explosive_explode (self, self, other, self->health, vec3_origin);
}

void func_explosive_spawn (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox (self);
	gi.linkentity (self);
}

void SP_func_explosive (edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;

	gi.modelindex ("models/objects/debris1/tris.md2");
	gi.modelindex ("models/objects/debris2/tris.md2");

	gi.setmodel (self, self->model);

	if (self->spawnflags & 1)
	{
		self->svflags |= SVF_NOCLIENT;
		self->solid = SOLID_NOT;
		self->use = func_explosive_spawn;
	}
	else
	{
		self->solid = SOLID_BSP;
		if (self->targetname)
			self->use = func_explosive_use;
	}

	if (self->spawnflags & 2)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 4)
		self->s.effects |= EF_ANIM_ALLFAST;

	if (self->use != func_explosive_use)
	{
		if (!self->health)
			self->health = 100;
		self->die = func_explosive_explode;
		self->takedamage = DAMAGE_YES;
	}

	gi.linkentity (self);
}


/*QUAKED misc_explobox (0 .5 .8) (-16 -16 0) (16 16 40)
Large exploding box.  You can override its mass (100),
health (80), and dmg (150).
*/

void barrel_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)

{
	float	ratio;
	vec3_t	v;

	if ((!other->groundentity) || (other->groundentity == self))
		return;

	ratio = (float)other->mass / (float)self->mass;
	VectorSubtract (self->s.origin, other->s.origin, v);
	M_walkmove (self, vectoyaw(v), 20 * ratio * FRAMETIME);
}

void barrel_explode (edict_t *self)
{
	vec3_t	org;
	float	spd;
	vec3_t	save, size;

	T_RadiusDamage (self, self->activator, self->dmg, NULL, self->dmg+40, MOD_BARREL, -1);

	VectorCopy (self->s.origin, save);
	VectorMA (self->absmin, 0.5, self->size, self->s.origin);
	VectorScale (self->size, 0.5, size);

	// a few big chunks
	spd = 1.5 * (float)self->dmg / 200.0;
	org[0] = self->s.origin[0] + crandom() * size[0];
	org[1] = self->s.origin[1] + crandom() * size[1];
	org[2] = self->s.origin[2] + crandom() * size[2];
	ThrowDebris (self, "models/objects/debris1/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * size[0];
	org[1] = self->s.origin[1] + crandom() * size[1];
	org[2] = self->s.origin[2] + crandom() * size[2];
	ThrowDebris (self, "models/objects/debris1/tris.md2", spd, org);

	// bottom corners
	spd = 1.75 * (float)self->dmg / 200.0;
	VectorCopy (self->absmin, org);
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);
	VectorCopy (self->absmin, org);
	org[0] += self->size[0];
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);
	VectorCopy (self->absmin, org);
	org[1] += self->size[1];
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);
	VectorCopy (self->absmin, org);
	org[0] += self->size[0];
	org[1] += self->size[1];
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);

	// a bunch of little chunks
	spd = 2 * self->dmg / 200;
	org[0] = self->s.origin[0] + crandom() * size[0];
	org[1] = self->s.origin[1] + crandom() * size[1];
	org[2] = self->s.origin[2] + crandom() * size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * size[0];
	org[1] = self->s.origin[1] + crandom() * size[1];
	org[2] = self->s.origin[2] + crandom() * size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * size[0];
	org[1] = self->s.origin[1] + crandom() * size[1];
	org[2] = self->s.origin[2] + crandom() * size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * size[0];
	org[1] = self->s.origin[1] + crandom() * size[1];
	org[2] = self->s.origin[2] + crandom() * size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * size[0];
	org[1] = self->s.origin[1] + crandom() * size[1];
	org[2] = self->s.origin[2] + crandom() * size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * size[0];
	org[1] = self->s.origin[1] + crandom() * size[1];
	org[2] = self->s.origin[2] + crandom() * size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * size[0];
	org[1] = self->s.origin[1] + crandom() * size[1];
	org[2] = self->s.origin[2] + crandom() * size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * size[0];
	org[1] = self->s.origin[1] + crandom() * size[1];
	org[2] = self->s.origin[2] + crandom() * size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);

	VectorCopy (save, self->s.origin);
	if (self->groundentity)
		BecomeExplosion2 (self);
	else
		BecomeExplosion1 (self);
}

void barrel_delay (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + 2 * FRAMETIME;
	self->think = barrel_explode;
	self->activator = attacker;
}

//=================================================================================

void teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*dest;
	int			i;

	if (!other->client)
		return;
	dest = G_Find (NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf ("Couldn't find destination\n");
		return;
	}

	//ZOID
	CTFPlayerResetGrapple(other);
	//ZOID

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	VectorCopy (dest->s.origin, other->s.origin);
	VectorCopy (dest->s.origin, other->s.old_origin);
	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);
	other->client->ps.pmove.pm_time = 160>>3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->owner->s.event = EV_PLAYER_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i=0 ; i<3 ; i++)
	{
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);
	}

	VectorClear (other->s.angles);
	VectorClear (other->client->ps.viewangles);
	VectorClear (other->client->v_angle);

	// kill anything at the destination
	KillBox (other);

	gi.linkentity (other);
}

/*QUAKED misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16)
Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.
*/
void SP_misc_teleporter (edict_t *ent)
{
	edict_t		*trig;

	if (!ent->target)
	{
		gi.dprintf ("teleporter without a target.\n");
		G_FreeEdict (ent);
		return;
	}

	gi.setmodel (ent, "models/objects/dmspot/tris.md2");
	ent->s.skinnum = 1;
	ent->s.effects = EF_TELEPORTER;
	ent->solid = SOLID_BBOX;

	VectorSet (ent->mins, -32, -32, -24);
	VectorSet (ent->maxs, 32, 32, -16);
	gi.linkentity (ent);

	trig = G_Spawn ();
	trig->touch = teleporter_touch;
	trig->solid = SOLID_TRIGGER;
	trig->target = ent->target;
	trig->owner = ent;
	VectorCopy (ent->s.origin, trig->s.origin);
	VectorSet (trig->mins, -8, -8, 8);
	VectorSet (trig->maxs, 8, 8, 24);
	gi.linkentity (trig);
	
}

/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
*/
void SP_misc_teleporter_dest (edict_t *ent)
{
	ent->s.modelindex = 0;
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -32, -32, -24);
	VectorSet (ent->maxs, 32, 32, -16);
	gi.linkentity (ent);
}

void SP_misc_etube (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;

	
	ent->s.modelindex = gi.modelindex ("models/misc/tube/tris.md2"); 
	ent->s.modelindex2 = gi.modelindex ("models/misc/tube/cover.md2");

	ent->s.effects = EF_COLOR_SHELL;

	ent->s.renderfx = (RF_SHELL_BLUE | RF_FULLBRIGHT | RF_GLOW | RF_NOSHADOWS);
	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 64);

	ent->s.frame = 0;
	gi.linkentity (ent);

	ent->think = NULL;
}
void misc_receptor_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 13;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_receptor (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/misc/receptor/tris.md2");
	ent->s.modelindex2 = gi.modelindex ("models/misc/receptor/bulb.md2");

	ent->solid = SOLID_BBOX;
	ent->s.effects = EF_COLOR_SHELL;

	ent->s.renderfx = (RF_SHELL_BLUE | RF_FULLBRIGHT | RF_GLOW | RF_NOSHADOWS);

	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 48);

	ent->s.frame = rand() % 13;
	gi.linkentity (ent);

	ent->think = misc_receptor_think;
	ent->nextthink = level.time + FRAMETIME;
}
void SP_misc_lamp (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	
	ent->s.modelindex = gi.modelindex ("models/misc/lamp/tris.md2"); 
	
	ent->s.renderfx = (RF_FULLBRIGHT | RF_GLOW | RF_NOSHADOWS);
	VectorSet (ent->mins, -8, -8, 0);
	VectorSet (ent->maxs, 8, 8, 24);

	ent->s.frame = 0;
	gi.linkentity (ent);

	ent->think = NULL;
}
void SP_misc_lamp2 (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;

	
	ent->s.modelindex = gi.modelindex ("models/misc/lamp2/tris.md2"); 
	
	ent->s.renderfx = (RF_FULLBRIGHT | RF_GLOW | RF_NOSHADOWS);
	VectorSet (ent->mins, -8, -8, 0);
	VectorSet (ent->maxs, 8, 8, 56);
	
	ent->health = 50;
	ent->die = barrel_delay;
	ent->takedamage = DAMAGE_YES;
	
	ent->s.frame = 0;
	gi.linkentity (ent);

	ent->think = NULL;
}
void SP_misc_electrode (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;

	
	ent->s.modelindex = gi.modelindex ("models/misc/electrode/tris.md2"); 
	ent->s.modelindex2 = gi.modelindex ("models/misc/electrode/cover.md2");

	ent->s.renderfx = (RF_FULLBRIGHT | RF_GLOW | RF_NOSHADOWS);
	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 64);

	ent->health = 50;
	ent->die = barrel_delay;
	ent->takedamage = DAMAGE_YES;

	ent->s.frame = 0;
	gi.linkentity (ent);

	ent->think = NULL;
}
void SP_misc_scope (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;

	
	ent->s.modelindex = gi.modelindex ("models/misc/scope/tris.md2"); 
	ent->s.modelindex2 = gi.modelindex ("models/misc/scope/cover.md2");

	ent->s.renderfx = (RF_FULLBRIGHT | RF_GLOW | RF_NOSHADOWS);
	VectorSet (ent->mins, -32, -32, 0);
	VectorSet (ent->maxs, 32, 32, 128);

	ent->health = 500;
	ent->die = barrel_delay;
	ent->takedamage = DAMAGE_YES;

	ent->s.frame = 0;
	gi.linkentity (ent);

	ent->think = NULL;
}
void SP_misc_pod (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	
	ent->s.modelindex = gi.modelindex ("models/misc/pod/tris.md2"); 
	
	ent->s.renderfx = (RF_FULLBRIGHT | RF_GLOW | RF_NOSHADOWS);
	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 64);

	ent->s.frame = 0;
	gi.linkentity (ent);

	ent->think = NULL;
}
void spidervolts (edict_t *self)
{
	
	vec3_t	start;
	vec3_t	end;
	int		damage = 30;
	int		x;
	int		aim;					     
 		
	VectorCopy (self->s.origin, start);
	VectorCopy (start, end);
	start[2] = start[2] + 128;
	for(x = 0; x<20; x++)
	{
			
		if(random() < .5)
			aim = -1000;
		else
			aim = 1000;
		end[0] = end[0] + random()*aim;
		if(random() < .5)
			aim = -1000;
		else
			aim = 1000;
		end[1] = end[1] + random()*aim;
		if(random() < .5)
			aim = 0;
		else
			aim = 1000;
		end[2] = end[2] + random()*aim;
		

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_LIGHTNING);
		gi.WritePosition (start);
		gi.WritePosition (end);
		gi.multicast (start, MULTICAST_PHS); 

		T_RadiusDamage(self, self, 300, NULL, 800, MOD_R_SPLASH, -1);
		
	}
	gi.sound (self, CHAN_AUTO, gi.soundindex("weapons/electroball.wav"), 1, ATTN_NONE, 0);
}	
void misc_spiderpod_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 13;

	//fire random bursts of electricicy
	if(ent->s.frame == 10)
		if(random() > 0.7)
			spidervolts(ent);

	ent->nextthink = level.time + FRAMETIME;
}
void SP_misc_spiderpod (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	
	ent->s.modelindex = gi.modelindex ("models/misc/spiderpod/tris.md2"); 
	
	ent->s.renderfx = (RF_FULLBRIGHT | RF_GLOW | RF_NOSHADOWS);
	VectorSet (ent->mins, -64, -64, 0);
	VectorSet (ent->maxs, 64, 64, 128);
	ent->think = misc_spiderpod_think;
	ent->nextthink = level.time + FRAMETIME;

	gi.linkentity (ent);
	M_droptofloor (ent);
}

//items for Team Core Assault mode
void rednode_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{

	if(other->dmteam == NO_TEAM || other->movetype == MOVETYPE_FLYMISSILE)
		return; //do not allow projectiles to turn anything off

	//if off, and red player, turn on
	if(!ent->powered && other->client && other->dmteam == RED_TEAM) {
		ent->powered = true;
		red_team_score++;
		if(other->client)
			other->client->resp.score+=2;
		//play sound, print message
		gi.sound (other, CHAN_AUTO, gi.soundindex("misc/redpnenabled.wav"), 1, ATTN_NONE, 0);
		safe_centerprintf(other, "Red Powernode Enabled!\n");
	}

	//if on, and blue player, turn off
	if(ent->powered && other->client && other->dmteam == BLUE_TEAM) {
		ent->powered = false;
		red_team_score--;
		if(other->client)
			other->client->resp.score+=5;
		if(red_team_score == 1) {
			gi.sound (other, CHAN_AUTO, gi.soundindex("misc/redvulnerable.wav"), 1, ATTN_NONE, 0);
			safe_centerprintf(other, "Red Spider Node Vulnerable!");
		}
		else {
			gi.sound (other, CHAN_AUTO, gi.soundindex("misc/redpndisabled.wav"), 1, ATTN_NONE, 0);
			safe_centerprintf(other, "Red Powernode Disabled!\n");
		}
	}
	
	//if off, and blue player, nothing
	//if on, and red player, nothing
}
void rednode_think (edict_t *ent)
{
	vec3_t start, end;

	if(ent->powered){
		
		VectorCopy(ent->s.origin, start);
		start[2] -= 24;
		VectorCopy(ent->s.origin, end);
		end[2] += 256;

		//draw a beam into the sky
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_REDLASER);
		gi.WritePosition (start);
		gi.WritePosition (end);
		gi.multicast (start, MULTICAST_PHS);   
	}
	
	ent->nextthink = level.time + FRAMETIME;
}
void SP_misc_rednode (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->takedamage = DAMAGE_NO;
	ent->s.modelindex = gi.modelindex ("models/objects/dmspot/tris.md2"); 
	
	ent->s.renderfx = (RF_FULLBRIGHT | RF_GLOW | RF_NOSHADOWS);
	VectorSet (ent->mins, -32, -32, -24);
	VectorSet (ent->maxs, 32, 32, -8);

	ent->s.frame = 0;
	ent->powered = true; //start on

	ent->touch = rednode_touch;
	ent->think = rednode_think;
	ent->nextthink = level.time + FRAMETIME;
	
	gi.linkentity (ent);	
	M_droptofloor (ent);
}
void bluenode_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{

	if(other->dmteam == NO_TEAM || other->movetype == MOVETYPE_FLYMISSILE)
		return; //do not allow projectiles to turn anything off

	//if off, and blue player, turn on
	if(!ent->powered && other->client && other->dmteam == BLUE_TEAM) {
		ent->powered = true;
		blue_team_score++;
		if(other->client)
			other->client->resp.score+=2;
		gi.sound (other, CHAN_AUTO, gi.soundindex("misc/bluepnenabled.wav"), 1, ATTN_NONE, 0);
		safe_centerprintf(other, "Blue Powernode Enabled!\n");
	}

	//if on, and red player, turn off
	if(ent->powered && other->client && other->dmteam == RED_TEAM) {
		ent->powered = false;
		blue_team_score--;
		if(other->client)
			other->client->resp.score+=5;
		if(blue_team_score == 1) {
			gi.sound (other, CHAN_AUTO, gi.soundindex("misc/bluevulnerable.wav"), 1, ATTN_NONE, 0);
			safe_centerprintf(other, "Blue Spider Node Vulnerable!\n");
		}
		else {
			gi.sound (other, CHAN_AUTO, gi.soundindex("misc/bluepndisabled.wav"), 1, ATTN_NONE, 0);
			safe_centerprintf(other, "Blue Powernode Disabled!\n");
		}
	}

}
void bluenode_think (edict_t *ent)
{
	vec3_t start, end;

	if(ent->powered){
		
		VectorCopy(ent->s.origin, start);
		start[2] -= 24;
		VectorCopy(ent->s.origin, end);
		end[2] += 256;

		//draw a beam into the sky
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTERBEAM);
		gi.WritePosition (start);
		gi.WritePosition (end);
		gi.multicast (start, MULTICAST_PHS);   
	}
	
	ent->nextthink = level.time + FRAMETIME;
}
void SP_misc_bluenode (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->takedamage = DAMAGE_NO;

	ent->s.modelindex = gi.modelindex ("models/objects/dmspot/tris.md2"); 
	
	ent->s.renderfx = (RF_FULLBRIGHT | RF_GLOW | RF_NOSHADOWS);
	VectorSet (ent->mins, -32, -32, -32);
	VectorSet (ent->maxs, 32, 32, -16);

	ent->s.frame = 0;
	ent->powered = true; //start on
	
	gi.linkentity (ent);
	ent->touch = bluenode_touch;
	ent->think = bluenode_think;
	ent->nextthink = level.time + FRAMETIME;
		
	M_droptofloor (ent);
}
void redspidernode_think (edict_t *ent)
{
	//just sits there pulsing
	if(red_team_score < 2) //now it can take damage
		ent->takedamage = DAMAGE_YES;
	else
		ent->takedamage = DAMAGE_NO;

	ent->s.frame = (ent->s.frame + 1) % 13;
	ent->nextthink = level.time + FRAMETIME;
}
void red_roundend(edict_t *ent) {
	red_team_score = 0;
}
void redspidernode_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->activator = attacker;
	
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	if(attacker->dmteam == BLUE_TEAM && attacker->client)
		attacker->client->resp.score+=50;
	
	gi.sound (self, CHAN_AUTO, gi.soundindex("players/martiancyborg/death1.wav"), 1, ATTN_NONE, 0);
	self->think = red_roundend;
	self->nextthink = level.time + 2;
}
void SP_misc_redspidernode (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->takedamage = DAMAGE_NO;

	ent->s.modelindex = gi.modelindex ("models/misc/spiderpod/tris.md2"); 
	
	ent->s.renderfx = (RF_FULLBRIGHT | RF_GLOW | RF_NOSHADOWS);
	VectorSet (ent->mins, -64, -64, 0);
	VectorSet (ent->maxs, 64, 64, 128);
	ent->health = 600;
	ent->die = redspidernode_die;
	ent->think = redspidernode_think;
	ent->nextthink = level.time + FRAMETIME;

	gi.linkentity (ent);
	M_droptofloor (ent);
}
void bluespidernode_think (edict_t *ent)
{
	//just sits there pulsing
	if(blue_team_score < 2) //now it can take damage
		ent->takedamage = DAMAGE_YES;
	else
		ent->takedamage = DAMAGE_NO;

	ent->s.frame = (ent->s.frame + 1) % 13;
	ent->nextthink = level.time + FRAMETIME;
}
void blue_roundend(edict_t *self) {
	blue_team_score = 0;
}
void bluespidernode_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->activator = attacker;
	
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	if(attacker->dmteam == RED_TEAM && attacker->client)
		attacker->client->resp.score+=50;
	
	gi.sound (self, CHAN_AUTO, gi.soundindex("players/martiancyborg/death1.wav"), 1, ATTN_NONE, 0);
	self->think = blue_roundend;
	self->nextthink = level.time + 2;
}
void SP_misc_bluespidernode (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->takedamage = DAMAGE_NO;

	ent->s.modelindex = gi.modelindex ("models/misc/spiderpod/tris.md2"); 
	
	ent->s.renderfx = (RF_FULLBRIGHT | RF_GLOW | RF_NOSHADOWS);
	VectorSet (ent->mins, -64, -64, 0);
	VectorSet (ent->maxs, 64, 64, 128);
	ent->health = 600;
	ent->die = bluespidernode_die;
	ent->think = bluespidernode_think;
	ent->nextthink = level.time + FRAMETIME;

	gi.linkentity (ent);
	M_droptofloor (ent);
}


#
# CodeRED Makefile
#

# Start of configurable options.

# Which compiler to use.
CC?=gcc

# Enable compilation optimizations when "yes".
OPTIMIZED_CFLAGS?=yes

# Integer from 0 to 3, controls GCC's -O CFLAG.
OPTIM_LVL?=2

# Path to standard libraries (e.g. jpeg).
LOCALBASE?=/usr/local

# Path to X libraries (e.g. GL).
X11BASE?=/usr/X11R6

# Build binary that uses SDL for sound when "1".
SDLSOUND?=1

# Selects the component to build; ALL, GAME, or DEDICATED
BUILD?=ALL

# End of configurable options.

VERSION=		1.40

ARCH:=			$(shell uname -m)
OSTYPE:=		$(shell uname -s | tr A-Z a-z)

MOUNT_DIR=		./

BUILD_RELEASE_DIR=	release
BUILD_DEBUG_DIR=	debug
CLIENT_DIR=		$(MOUNT_DIR)/client
SERVER_DIR=		$(MOUNT_DIR)/server
REF_GL_DIR=		$(MOUNT_DIR)/ref_gl
COMMON_DIR=		$(MOUNT_DIR)/qcommon
UNIX_DIR=		$(MOUNT_DIR)/unix
GAME_DIR=		$(MOUNT_DIR)/game
NULL_DIR=		$(MOUNT_DIR)/null
ARENA_DIR=		$(GAME_DIR)

ifeq ($(ARCH),x86_64)
	_LIB := lib64
else
	_LIB := lib
endif

BASE_CFLAGS=$(CFLAGS) -Dstricmp=strcasecmp -D_stricmp=strcasecmp -I$(X11BASE)/include -fno-strict-aliasing

RELEASE_CFLAGS=$(BASE_CFLAGS)

ifeq ($(strip $(OPTIMIZED_CFLAGS)),yes)
RELEASE_CFLAGS+=-O$(OPTIM_LVL) -fomit-frame-pointer \
		-ftree-vectorize -fexpensive-optimizations

ifeq ($(OSTYPE),linux)
ifeq ($(ARCH),x86_64)
	RELEASE_CFLAGS += -march=k8
else
ifeq ($(ARCH),i686)
	RELEASE_CFLAGS += -march=i686
else
ifeq ($(ARCH),i586)
	RELEASE_CFLAGS += -march=i586
else
ifeq ($(ARCH),i486)
	RELEASE_CFLAGS += -march=i486
else
ifeq ($(ARCH),i386)
	RELEASE_CFLAGS += -march=i386
endif
endif
endif
endif
endif
endif
endif

ARENA_CFLAGS=-DARENA

DEBUG_CFLAGS=$(BASE_CFLAGS) -g -ggdb

LDFLAGS+=-lm
# In FreeBSD, dlopen() is in libc.
ifeq ($(OSTYPE),linux)
	LDFLAGS+=-ldl
endif

GLXLDFLAGS=-L$(X11BASE)/$(_LIB) -L$(LOCALBASE)/$(_LIB) -lX11 -lXext -lXxf86dga -lXxf86vm -lm -ljpeg -lGL -lGLU

SDLCFLAGS=$(shell sdl-config --cflags)
SDLLDFLAGS=$(shell sdl-config --libs)

SHLIBEXT=so
SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

DO_CC=$(CC) $(CFLAGS) -o $@ -c $<
DO_DEBUG_CC=$(CC) $(DEBUG_CFLAGS) -o $@ -c $<
DO_DED_CC=$(CC) $(CFLAGS) -DDEDICATED_ONLY -o $@ -c $<
DO_DED_DEBUG_CC=$(CC) $(DEBUG_CFLAGS) -DDEDICATED_ONLY -o $@ -c $<
DO_O_CC=$(CC) $(CFLAGS) -O -o $@ -c $<
DO_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_SHLIB_O_CC=$(CC) $(CFLAGS) -O $(SHLIBCFLAGS) -o $@ -c $<
DO_SHLIB_DEBUG_CC=$(CC) $(DEBUG_CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_GL_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_GL_SHLIB_O_CC=$(CC) $(CFLAGS) -O $(SHLIBCFLAGS) -o $@ -c $<
DO_AS=$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<
DO_SHLIB_AS=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<

DO_ARENA_SHLIB_CC=$(CC) $(CFLAGS) $(ARENA_CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

#############################################################################
# SETUP AND BUILD
#############################################################################

TARGETS=$(BUILDDIR)/game.$(SHLIBEXT) \
	$(BUILDDIR)/arena/game.$(SHLIBEXT)

ifeq ($(strip $(BUILD)),ALL)
	TARGETS+=$(BUILDDIR)/crded \
		$(BUILDDIR)/crx
endif

ifeq ($(strip $(BUILD)),DEDICATED)
	SDLSOUND=0
	TARGETS+=$(BUILDDIR)/crded
endif

ifeq ($(strip $(BUILD)),GAME)
	TARGETS+=$(BUILDDIR)/crx
endif

ifeq ($(strip $(SDLSOUND)),1)
	TARGETS+=$(BUILDDIR)/crx.sdl
endif

build-release:
	@mkdir -p $(BUILD_RELEASE_DIR) \
		$(BUILD_RELEASE_DIR)/client \
		$(BUILD_RELEASE_DIR)/ded \
		$(BUILD_RELEASE_DIR)/ref_gl \
		$(BUILD_RELEASE_DIR)/game \
		$(BUILD_RELEASE_DIR)/arena
	$(MAKE) targets BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"

build-debug:
	@mkdir -p $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/client \
		$(BUILD_DEBUG_DIR)/ded \
		$(BUILD_DEBUG_DIR)/ref_gl \
		$(BUILD_DEBUG_DIR)/game \
		$(BUILD_DEBUG_DIR)/arena
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

all: build-release build-debug

targets: $(TARGETS)

#############################################################################
# CLIENT/SERVER
#############################################################################

CODERED_OBJS = \
	$(BUILDDIR)/client/cl_cin.o \
	$(BUILDDIR)/client/cl_ents.o \
	$(BUILDDIR)/client/cl_fx.o \
	$(BUILDDIR)/client/cl_input.o \
	$(BUILDDIR)/client/cl_inv.o \
	$(BUILDDIR)/client/cl_main.o \
	$(BUILDDIR)/client/cl_newfx.o \
	$(BUILDDIR)/client/cl_parse.o \
	$(BUILDDIR)/client/cl_pred.o \
	$(BUILDDIR)/client/cl_tent.o \
	$(BUILDDIR)/client/cl_scrn.o \
	$(BUILDDIR)/client/cl_view.o \
	$(BUILDDIR)/client/console.o \
	$(BUILDDIR)/client/keys.o \
	$(BUILDDIR)/client/menu.o \
	$(BUILDDIR)/client/snd_dma.o \
	$(BUILDDIR)/client/snd_mem.o \
	$(BUILDDIR)/client/snd_mix.o \
	$(BUILDDIR)/client/qmenu.o \
	\
	$(BUILDDIR)/client/cmd.o \
	$(BUILDDIR)/client/cmodel.o \
	$(BUILDDIR)/client/common.o \
	$(BUILDDIR)/client/crc.o \
	$(BUILDDIR)/client/cvar.o \
	$(BUILDDIR)/client/files.o \
	$(BUILDDIR)/client/mdfour.o \
	$(BUILDDIR)/client/net_chan.o \
	\
	$(BUILDDIR)/client/sv_ccmds.o \
	$(BUILDDIR)/client/sv_ents.o \
	$(BUILDDIR)/client/sv_game.o \
	$(BUILDDIR)/client/sv_init.o \
	$(BUILDDIR)/client/sv_main.o \
	$(BUILDDIR)/client/sv_send.o \
	$(BUILDDIR)/client/sv_user.o \
	$(BUILDDIR)/client/sv_world.o \
	\
	$(BUILDDIR)/client/cd_unix.o \
	$(BUILDDIR)/client/q_shunix.o \
	$(BUILDDIR)/client/vid_menu.o \
	$(BUILDDIR)/client/vid_so.o \
	$(BUILDDIR)/client/sys_unix.o \
	$(BUILDDIR)/client/rw_unix.o \
	$(BUILDDIR)/client/glob.o \
	$(BUILDDIR)/client/net_udp.o \
	\
	$(BUILDDIR)/client/q_shared.o \
	$(BUILDDIR)/client/pmove.o \
	\
	$(BUILDDIR)/ref_gl/r_bloom.o \
	$(BUILDDIR)/ref_gl/r_draw.o \
	$(BUILDDIR)/ref_gl/r_image.o \
	$(BUILDDIR)/ref_gl/r_light.o \
	$(BUILDDIR)/ref_gl/r_main.o \
	$(BUILDDIR)/ref_gl/r_mesh.o \
	$(BUILDDIR)/ref_gl/r_misc.o \
	$(BUILDDIR)/ref_gl/r_model.o \
	$(BUILDDIR)/ref_gl/r_refl.o \
	$(BUILDDIR)/ref_gl/r_math.o \
	$(BUILDDIR)/ref_gl/r_script.o \
	$(BUILDDIR)/ref_gl/r_surf.o \
	$(BUILDDIR)/ref_gl/r_vlights.o \
	$(BUILDDIR)/ref_gl/r_warp.o \
	\
	$(BUILDDIR)/ref_gl/qgl_unix.o


REF_GL_GLX_OBJS = \
	$(BUILDDIR)/ref_gl/gl_glx.o

SOUND_OSS_OBJS = \
	$(BUILDDIR)/client/snd_unix.o

SOUND_SDL_OBJS = \
	$(BUILDDIR)/client/snd_sdl.o

CODERED_AS_OBJS = \
	$(BUILDDIR)/client/snd_mixa.o

$(BUILDDIR)/crx : $(CODERED_OBJS) $(SOUND_OSS_OBJS) $(CODERED_AS_OBJS) $(REF_GL_OBJS) $(REF_GL_GLX_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CODERED_OBJS) $(SOUND_OSS_OBJS) $(CODERED_AS_OBJS) $(LDFLAGS) $(REF_GL_OBJS) $(REF_GL_GLX_OBJS) $(GLXLDFLAGS)

$(BUILDDIR)/crx.sdl : $(CODERED_OBJS) $(SOUND_SDL_OBJS) $(CODERED_AS_OBJS) $(REF_GL_OBJS) $(REF_GL_GLX_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CODERED_OBJS) $(SOUND_SDL_OBJS) $(CODERED_AS_OBJS) $(LDFLAGS) $(REF_GL_OBJS) $(REF_GL_GLX_OBJS) $(GLXLDFLAGS) $(SDLLDFLAGS)

$(BUILDDIR)/client/cl_cin.o :     $(CLIENT_DIR)/cl_cin.c
	$(DO_CC)

$(BUILDDIR)/client/cl_ents.o :    $(CLIENT_DIR)/cl_ents.c
	$(DO_CC)

$(BUILDDIR)/client/cl_fx.o :      $(CLIENT_DIR)/cl_fx.c
	$(DO_CC)

$(BUILDDIR)/client/cl_input.o :   $(CLIENT_DIR)/cl_input.c
	$(DO_CC)

$(BUILDDIR)/client/cl_inv.o :     $(CLIENT_DIR)/cl_inv.c
	$(DO_CC)

$(BUILDDIR)/client/cl_main.o :    $(CLIENT_DIR)/cl_main.c
	$(DO_CC)

$(BUILDDIR)/client/cl_newfx.o :    $(CLIENT_DIR)/cl_newfx.c
	$(DO_CC)

$(BUILDDIR)/client/cl_parse.o :   $(CLIENT_DIR)/cl_parse.c
	$(DO_CC)

$(BUILDDIR)/client/cl_pred.o :    $(CLIENT_DIR)/cl_pred.c
	$(DO_CC)

$(BUILDDIR)/client/cl_tent.o :    $(CLIENT_DIR)/cl_tent.c
	$(DO_CC)

$(BUILDDIR)/client/cl_scrn.o :    $(CLIENT_DIR)/cl_scrn.c
	$(DO_CC)

$(BUILDDIR)/client/cl_view.o :    $(CLIENT_DIR)/cl_view.c
	$(DO_CC)

$(BUILDDIR)/client/console.o :    $(CLIENT_DIR)/console.c
	$(DO_CC)

$(BUILDDIR)/client/keys.o :       $(CLIENT_DIR)/keys.c
	$(DO_CC)

$(BUILDDIR)/client/menu.o :       $(CLIENT_DIR)/menu.c
	$(DO_CC)

$(BUILDDIR)/client/snd_dma.o :    $(CLIENT_DIR)/snd_dma.c
	$(DO_CC)

$(BUILDDIR)/client/snd_mem.o :    $(CLIENT_DIR)/snd_mem.c
	$(DO_CC)

$(BUILDDIR)/client/snd_mix.o :    $(CLIENT_DIR)/snd_mix.c
	$(DO_CC)

$(BUILDDIR)/client/qmenu.o :      $(CLIENT_DIR)/qmenu.c
	$(DO_CC)

$(BUILDDIR)/client/cmd.o :        $(COMMON_DIR)/cmd.c
	$(DO_CC)

$(BUILDDIR)/client/cmodel.o :     $(COMMON_DIR)/cmodel.c
	$(DO_CC)

$(BUILDDIR)/client/common.o :     $(COMMON_DIR)/common.c
	$(DO_CC)

$(BUILDDIR)/client/crc.o :        $(COMMON_DIR)/crc.c
	$(DO_CC)

$(BUILDDIR)/client/cvar.o :       $(COMMON_DIR)/cvar.c
	$(DO_CC)

$(BUILDDIR)/client/files.o :      $(COMMON_DIR)/files.c
	$(DO_CC)

$(BUILDDIR)/client/mdfour.o :     $(COMMON_DIR)/mdfour.c
	$(DO_CC)

$(BUILDDIR)/client/net_chan.o :   $(COMMON_DIR)/net_chan.c
	$(DO_CC)

$(BUILDDIR)/client/q_shared.o :   $(GAME_DIR)/q_shared.c
	$(DO_DEBUG_CC)

$(BUILDDIR)/client/pmove.o :      $(COMMON_DIR)/pmove.c
	$(DO_CC)

$(BUILDDIR)/client/sv_ccmds.o :   $(SERVER_DIR)/sv_ccmds.c
	$(DO_CC)

$(BUILDDIR)/client/sv_ents.o :    $(SERVER_DIR)/sv_ents.c
	$(DO_CC)

$(BUILDDIR)/client/sv_game.o :    $(SERVER_DIR)/sv_game.c
	$(DO_CC)

$(BUILDDIR)/client/sv_init.o :    $(SERVER_DIR)/sv_init.c
	$(DO_CC)

$(BUILDDIR)/client/sv_main.o :    $(SERVER_DIR)/sv_main.c
	$(DO_CC)

$(BUILDDIR)/client/sv_send.o :    $(SERVER_DIR)/sv_send.c
	$(DO_CC)

$(BUILDDIR)/client/sv_user.o :    $(SERVER_DIR)/sv_user.c
	$(DO_CC)

$(BUILDDIR)/client/sv_world.o :   $(SERVER_DIR)/sv_world.c
	$(DO_CC)

$(BUILDDIR)/client/cd_unix.o :   $(UNIX_DIR)/cd_unix.c
	$(DO_CC)

$(BUILDDIR)/client/q_shunix.o :  $(UNIX_DIR)/q_shunix.c
	$(DO_O_CC)

$(BUILDDIR)/client/vid_menu.o :   $(CLIENT_DIR)/vid_menu.c
	$(DO_CC)

$(BUILDDIR)/client/vid_so.o :     $(UNIX_DIR)/vid_so.c
	$(DO_CC)

$(BUILDDIR)/client/snd_unix.o :  $(UNIX_DIR)/snd_unix.c
	$(DO_CC)

$(BUILDDIR)/client/snd_mixa.o :   $(UNIX_DIR)/snd_mixa.s
	$(DO_AS)

$(BUILDDIR)/client/sys_unix.o :  $(UNIX_DIR)/sys_unix.c
	$(DO_CC)

$(BUILDDIR)/client/rw_unix.o :  $(UNIX_DIR)/rw_unix.c
	$(DO_O_CC)

$(BUILDDIR)/client/glob.o :       $(UNIX_DIR)/glob.c
	$(DO_CC)

$(BUILDDIR)/client/net_udp.o :    $(UNIX_DIR)/net_udp.c
	$(DO_CC)

$(BUILDDIR)/ref_gl/r_bloom.o :        $(REF_GL_DIR)/r_bloom.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/r_draw.o :        $(REF_GL_DIR)/r_draw.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/r_light.o :       $(REF_GL_DIR)/r_light.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/r_main.o :       $(REF_GL_DIR)/r_main.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/r_misc.o :       $(REF_GL_DIR)/r_misc.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/r_script.o :       $(REF_GL_DIR)/r_script.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/r_surf.o :       $(REF_GL_DIR)/r_surf.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/r_warp.o :        $(REF_GL_DIR)/r_warp.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/r_image.o :       $(REF_GL_DIR)/r_image.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/r_mesh.o :        $(REF_GL_DIR)/r_mesh.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/r_model.o :       $(REF_GL_DIR)/r_model.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/r_refl.o :       $(REF_GL_DIR)/r_refl.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/r_math.o :	    $(REF_GL_DIR)/r_math.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/r_vlights.o:     $(REF_GL_DIR)/r_vlights.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/qgl_unix.o :      $(UNIX_DIR)/qgl_unix.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/gl_glx.o :      $(UNIX_DIR)/gl_glx.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/client/snd_sdl.o :     $(UNIX_DIR)/snd_sdl.c
	$(DO_CC) $(SDLCFLAGS)

#############################################################################
# DEDICATED SERVER
#############################################################################

CRDED_OBJS = \
	\
	$(BUILDDIR)/ded/cmd.o \
	$(BUILDDIR)/ded/cmodel.o \
	$(BUILDDIR)/ded/common.o \
	$(BUILDDIR)/ded/crc.o \
	$(BUILDDIR)/ded/cvar.o \
	$(BUILDDIR)/ded/files.o \
	$(BUILDDIR)/ded/mdfour.o \
	$(BUILDDIR)/ded/net_chan.o \
	\
	$(BUILDDIR)/ded/sv_ccmds.o \
	$(BUILDDIR)/ded/sv_ents.o \
	$(BUILDDIR)/ded/sv_game.o \
	$(BUILDDIR)/ded/sv_init.o \
	$(BUILDDIR)/ded/sv_main.o \
	$(BUILDDIR)/ded/sv_send.o \
	$(BUILDDIR)/ded/sv_user.o \
	$(BUILDDIR)/ded/sv_world.o \
	\
	$(BUILDDIR)/ded/q_shunix.o \
	$(BUILDDIR)/ded/sys_unix.o \
	$(BUILDDIR)/ded/glob.o \
	$(BUILDDIR)/ded/net_udp.o \
	\
	$(BUILDDIR)/ded/q_shared.o \
	$(BUILDDIR)/ded/pmove.o \
	\
	$(BUILDDIR)/ded/cl_null.o \
	$(BUILDDIR)/ded/cd_null.o

$(BUILDDIR)/crded : $(CRDED_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CRDED_OBJS) $(LDFLAGS)

$(BUILDDIR)/ded/cmd.o :        $(COMMON_DIR)/cmd.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/cmodel.o :     $(COMMON_DIR)/cmodel.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/common.o :     $(COMMON_DIR)/common.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/crc.o :        $(COMMON_DIR)/crc.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/cvar.o :       $(COMMON_DIR)/cvar.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/files.o :      $(COMMON_DIR)/files.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/mdfour.o :        $(COMMON_DIR)/mdfour.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/net_chan.o :   $(COMMON_DIR)/net_chan.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/q_shared.o :   $(GAME_DIR)/q_shared.c
	$(DO_DED_DEBUG_CC)

$(BUILDDIR)/ded/pmove.o :      $(COMMON_DIR)/pmove.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_ccmds.o :   $(SERVER_DIR)/sv_ccmds.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_ents.o :    $(SERVER_DIR)/sv_ents.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_game.o :    $(SERVER_DIR)/sv_game.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_init.o :    $(SERVER_DIR)/sv_init.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_main.o :    $(SERVER_DIR)/sv_main.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_send.o :    $(SERVER_DIR)/sv_send.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_user.o :    $(SERVER_DIR)/sv_user.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_world.o :   $(SERVER_DIR)/sv_world.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/q_shunix.o :  $(UNIX_DIR)/q_shunix.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sys_unix.o :  $(UNIX_DIR)/sys_unix.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/glob.o :       $(UNIX_DIR)/glob.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/net_udp.o :    $(UNIX_DIR)/net_udp.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/cd_null.o     : $(NULL_DIR)/cd_null.c    
	$(DO_DED_CC)

$(BUILDDIR)/ded/cl_null.o     : $(NULL_DIR)/cl_null.c    
	$(DO_DED_CC)

#############################################################################
# GAME
#############################################################################

GAME_OBJS = \
	$(BUILDDIR)/game/acebot_ai.o \
	$(BUILDDIR)/game/acebot_cmds.o \
	$(BUILDDIR)/game/acebot_compress.o \
	$(BUILDDIR)/game/acebot_items.o \
	$(BUILDDIR)/game/acebot_movement.o \
	$(BUILDDIR)/game/acebot_nodes.o \
	$(BUILDDIR)/game/acebot_spawn.o \
	$(BUILDDIR)/game/c_cam.o \
	$(BUILDDIR)/game/cow.o \
	$(BUILDDIR)/game/ctf.o \
	$(BUILDDIR)/game/q_shared.o \
	$(BUILDDIR)/game/g_ai.o \
	$(BUILDDIR)/game/g_chase.o \
	$(BUILDDIR)/game/g_cmds.o \
	$(BUILDDIR)/game/g_combat.o \
	$(BUILDDIR)/game/g_func.o \
	$(BUILDDIR)/game/g_items.o \
	$(BUILDDIR)/game/g_main.o \
	$(BUILDDIR)/game/g_misc.o \
	$(BUILDDIR)/game/g_monster.o \
	$(BUILDDIR)/game/g_phys.o \
	$(BUILDDIR)/game/g_save.o \
	$(BUILDDIR)/game/g_spawn.o \
	$(BUILDDIR)/game/g_svcmds.o \
	$(BUILDDIR)/game/g_target.o \
	$(BUILDDIR)/game/g_trigger.o \
	$(BUILDDIR)/game/g_utils.o \
	$(BUILDDIR)/game/g_weapon.o \
	$(BUILDDIR)/game/m_move.o \
	$(BUILDDIR)/game/p_client.o \
	$(BUILDDIR)/game/p_hud.o \
	$(BUILDDIR)/game/p_light.o \
	$(BUILDDIR)/game/p_trail.o \
	$(BUILDDIR)/game/p_view.o \
	$(BUILDDIR)/game/p_weapon.o \
	$(BUILDDIR)/game/vehicles.o



$(BUILDDIR)/game.$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS)


$(BUILDDIR)/game/acebot_ai.o :        $(GAME_DIR)/acesrc/acebot_ai.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/acebot_cmds.o :        $(GAME_DIR)/acesrc/acebot_cmds.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/acebot_compress.o :        $(GAME_DIR)/acesrc/acebot_compress.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/acebot_items.o :        $(GAME_DIR)/acesrc/acebot_items.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/acebot_movement.o :        $(GAME_DIR)/acesrc/acebot_movement.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/acebot_nodes.o :        $(GAME_DIR)/acesrc/acebot_nodes.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/acebot_spawn.o :        $(GAME_DIR)/acesrc/acebot_spawn.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/c_cam.o :        $(GAME_DIR)/c_cam.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/cow.o :        $(GAME_DIR)/cow.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/ctf.o :        $(GAME_DIR)/ctf.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_ai.o :        $(GAME_DIR)/g_ai.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_chase.o :    $(GAME_DIR)/g_chase.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_cmds.o :      $(GAME_DIR)/g_cmds.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_combat.o :    $(GAME_DIR)/g_combat.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_func.o :      $(GAME_DIR)/g_func.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_items.o :     $(GAME_DIR)/g_items.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_main.o :      $(GAME_DIR)/g_main.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_misc.o :      $(GAME_DIR)/g_misc.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_monster.o :   $(GAME_DIR)/g_monster.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_phys.o :      $(GAME_DIR)/g_phys.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_save.o :      $(GAME_DIR)/g_save.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_spawn.o :     $(GAME_DIR)/g_spawn.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_svcmds.o :    $(GAME_DIR)/g_svcmds.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_target.o :    $(GAME_DIR)/g_target.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_trigger.o :   $(GAME_DIR)/g_trigger.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_utils.o :     $(GAME_DIR)/g_utils.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_weapon.o :    $(GAME_DIR)/g_weapon.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/m_move.o :      $(GAME_DIR)/m_move.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/p_client.o :    $(GAME_DIR)/p_client.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/p_hud.o :       $(GAME_DIR)/p_hud.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/p_light.o :       $(GAME_DIR)/p_light.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/p_trail.o :     $(GAME_DIR)/p_trail.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/p_view.o :      $(GAME_DIR)/p_view.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/p_weapon.o :    $(GAME_DIR)/p_weapon.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/vehicles.o :    $(GAME_DIR)/vehicles.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/q_shared.o :    $(GAME_DIR)/q_shared.c
	$(DO_SHLIB_DEBUG_CC)



#############################################################################
# ALIEN ARENA
#############################################################################

ARENA_OBJS = \
	$(BUILDDIR)/arena/acebot_ai.o \
	$(BUILDDIR)/arena/acebot_cmds.o \
	$(BUILDDIR)/arena/acebot_compress.o \
	$(BUILDDIR)/arena/acebot_items.o \
	$(BUILDDIR)/arena/acebot_movement.o \
	$(BUILDDIR)/arena/acebot_nodes.o \
	$(BUILDDIR)/arena/acebot_spawn.o \
	$(BUILDDIR)/arena/q_shared.o \
	$(BUILDDIR)/arena/c_cam.o \
	$(BUILDDIR)/arena/cow.o \
	$(BUILDDIR)/arena/ctf.o \
	$(BUILDDIR)/arena/g_ai.o \
	$(BUILDDIR)/arena/g_chase.o \
	$(BUILDDIR)/arena/g_cmds.o \
	$(BUILDDIR)/arena/g_combat.o \
	$(BUILDDIR)/arena/g_func.o \
	$(BUILDDIR)/arena/g_items.o \
	$(BUILDDIR)/arena/g_main.o \
	$(BUILDDIR)/arena/g_misc.o \
	$(BUILDDIR)/arena/g_monster.o \
	$(BUILDDIR)/arena/g_phys.o \
	$(BUILDDIR)/arena/g_save.o \
	$(BUILDDIR)/arena/g_spawn.o \
	$(BUILDDIR)/arena/g_svcmds.o \
	$(BUILDDIR)/arena/g_target.o \
	$(BUILDDIR)/arena/g_trigger.o \
	$(BUILDDIR)/arena/g_utils.o \
	$(BUILDDIR)/arena/g_weapon.o \
	$(BUILDDIR)/arena/m_move.o \
	$(BUILDDIR)/arena/p_client.o \
	$(BUILDDIR)/arena/p_hud.o \
	$(BUILDDIR)/arena/p_light.o \
	$(BUILDDIR)/arena/p_trail.o \
	$(BUILDDIR)/arena/p_view.o \
	$(BUILDDIR)/arena/p_weapon.o \
	$(BUILDDIR)/arena/vehicles.o

$(BUILDDIR)/arena/game.$(SHLIBEXT) : $(ARENA_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(ARENA_OBJS)


$(BUILDDIR)/arena/acebot_ai.o :        $(ARENA_DIR)/acesrc/acebot_ai.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/acebot_cmds.o :        $(ARENA_DIR)/acesrc/acebot_cmds.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/acebot_compress.o :        $(ARENA_DIR)/acesrc/acebot_compress.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/acebot_items.o :        $(ARENA_DIR)/acesrc/acebot_items.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/acebot_movement.o :        $(ARENA_DIR)/acesrc/acebot_movement.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/acebot_nodes.o :        $(ARENA_DIR)/acesrc/acebot_nodes.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/acebot_spawn.o :        $(ARENA_DIR)/acesrc/acebot_spawn.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/c_cam.o :        $(ARENA_DIR)/c_cam.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/cow.o :        $(ARENA_DIR)/cow.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/ctf.o :        $(ARENA_DIR)/ctf.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_ai.o :        $(ARENA_DIR)/g_ai.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_chase.o :    $(ARENA_DIR)/g_chase.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_cmds.o :      $(ARENA_DIR)/g_cmds.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_combat.o :    $(ARENA_DIR)/g_combat.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_func.o :      $(ARENA_DIR)/g_func.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_items.o :     $(ARENA_DIR)/g_items.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_main.o :      $(ARENA_DIR)/g_main.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_misc.o :      $(ARENA_DIR)/g_misc.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_monster.o :   $(ARENA_DIR)/g_monster.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_phys.o :      $(ARENA_DIR)/g_phys.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_save.o :      $(ARENA_DIR)/g_save.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_spawn.o :     $(ARENA_DIR)/g_spawn.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_svcmds.o :    $(ARENA_DIR)/g_svcmds.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_target.o :    $(ARENA_DIR)/g_target.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_trigger.o :   $(ARENA_DIR)/g_trigger.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_utils.o :     $(ARENA_DIR)/g_utils.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/g_weapon.o :    $(ARENA_DIR)/g_weapon.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/m_move.o :      $(ARENA_DIR)/m_move.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/p_client.o :    $(ARENA_DIR)/p_client.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/p_hud.o :       $(ARENA_DIR)/p_hud.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/p_light.o :       $(ARENA_DIR)/p_light.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/p_trail.o :     $(ARENA_DIR)/p_trail.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/p_view.o :      $(ARENA_DIR)/p_view.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/p_weapon.o :    $(ARENA_DIR)/p_weapon.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/vehicles.o :    $(ARENA_DIR)/vehicles.c
	$(DO_ARENA_SHLIB_CC)

$(BUILDDIR)/arena/q_shared.o :    $(ARENA_DIR)/q_shared.c
	$(DO_SHLIB_DEBUG_CC)


#############################################################################
# MISC
#############################################################################
#Needs work.
clean: clean-debug clean-release

clean-debug:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean-release:
	$(MAKE) clean2 BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:
	-rm -f \
	$(CODERED_OBJS) \
	$(CRDED_OBJS) \
	$(CODERED_AS_OBJS) \
	$(GAME_OBJS) \
	$(ARENA_OBJS) \
	$(REF_GL_OBJS) \
	$(REF_GL_GLX_OBJS) \
	$(SOUND_OSS_OBJS) \
	$(SOUND_SDL_OBJS)

distclean: clean
	rm -rf $(BUILD_RELEASE_DIR)
	rm -rf $(BUILD_DEBUG_DIR)

install:
	cp $(BUILD_RELEASE_DIR)/cr* ../
	cp $(BUILD_RELEASE_DIR)/game.so ../data1/
	cp $(BUILD_RELEASE_DIR)/game.so ../arena/
	strip ../crded
	strip ../crx
	strip ../crx.sdl
	strip ../arena/game.so
	strip ../data1/game.so

install-debug:
	cp $(BUILD_DEBUG_DIR)/cr* ../
	cp $(BUILD_DEBUG_DIR)/game.so ../data1/
	cp $(BUILD_DEBUG_DIR)/game.so ../arena/

uninstall:
	rm -f ../data1/game.so
	rm -f ../arena/game.so
	rm -f ../crx
	rm -f ../crx.sdl
	rm -f ../crded

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

#include "qcommon.h"
#include "../unix/glob.h"

// define this to dissalow any data but the demo pak file
//#define	NO_ADDONS

// if a packfile directory differs from this, it is assumed to be hacked
// Full version
#define	PAK0_CHECKSUM	0x40e614e0
// Demo
//#define	PAK0_CHECKSUM	0xb2c6d7ea
// OEM
//#define	PAK0_CHECKSUM	0x78e135c

/*
=============================================================================

QUAKE FILESYSTEM

=============================================================================
*/


//
// in memory
//

typedef struct
{
	char	name[MAX_QPATH];
	int		filepos, filelen;
} packfile_t;

typedef struct pack_s
{
	char	filename[MAX_OSPATH];
	FILE	*handle;
	int		numfiles;
	packfile_t	*files;
} pack_t;

char	fs_gamedir[MAX_OSPATH];
cvar_t	*fs_basedir;
cvar_t	*fs_gamedirvar;

typedef struct filelink_s
{
	struct filelink_s	*next;
	char	*from;
	int		fromlength;
	char	*to;
} filelink_t;

filelink_t	*fs_links;

typedef struct searchpath_s
{
	char	filename[MAX_OSPATH];
	pack_t	*pack;		// only one of filename / pack will be used
	struct searchpath_s *next;
} searchpath_t;

searchpath_t	*fs_searchpaths;
searchpath_t	*fs_base_searchpaths;	// without gamedirs


/*

All of Quake's data access is through a hierchal file system, but the contents of the file system can be transparently merged from several sources.

The "base directory" is the path to the directory holding the quake.exe and all game directories.  The sys_* files pass this to host_init in quakeparms_t->basedir.  This can be overridden with the "-basedir" command line parm to allow code debugging in a different directory.  The base directory is
only used during filesystem initialization.

The "game directory" is the first tree on the search path and directory that all generated files (savegames, screenshots, demos, config files) will be saved to.  This can be overridden with the "-game" command line parameter.  The game directory can never be changed while quake is executing.  This is a precacution against having a malicious server instruct clients to write files over areas they shouldn't.

*/


/*
================
FS_filelength
================
*/
int FS_filelength (FILE *f)
{
	int		pos;
	int		end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}


/*
============
FS_CreatePath

Creates any directories needed to store the given filename
============
*/
void	FS_CreatePath (char *path)
{
	char	*ofs;

	for (ofs = path+1 ; *ofs ; ofs++)
	{
		if (*ofs == '/')
		{	// create the directory
			*ofs = 0;
			Sys_Mkdir (path);
			*ofs = '/';
		}
	}
}


/*
==============
FS_FCloseFile

For some reason, other dll's can't just cal fclose()
on files returned by FS_FOpenFile...
==============
*/
void FS_FCloseFile (FILE *f)
{
	fclose (f);
}

/*
===========
FS_FOpenFile

Finds the file in the search path.
returns filesize and an open FILE *
Used for streaming data out of either a pak file or
a seperate file.
===========
*/
int file_from_pak = 0;
#ifndef NO_ADDONS
int FS_FOpenFile (char *filename, FILE **file)
{
	searchpath_t	*search;
	char			netpath[MAX_OSPATH];
	pack_t			*pak;
	int				i;
	filelink_t		*link;

	file_from_pak = 0;

	// check for links first
	for (link = fs_links ; link ; link=link->next)
	{
		if (!strncmp (filename, link->from, link->fromlength))
		{
			Com_sprintf (netpath, sizeof(netpath), "%s%s",link->to, filename+link->fromlength);
			*file = fopen (netpath, "rb");
			if (*file)
			{
				Com_DPrintf ("link file: %s\n",netpath);
				return FS_filelength (*file);
			}
			return -1;
		}
	}

//
// search through the path, one element at a time
//
	for (search = fs_searchpaths ; search ; search = search->next)
	{
	// is the element a pak file?
		if (search->pack)
		{
		// look through all the pak file elements
			pak = search->pack;
			for (i=0 ; i<pak->numfiles ; i++)
				if (!Q_strcasecmp (pak->files[i].name, filename))
				{	// found it!
					file_from_pak = 1;
					Com_DPrintf ("PackFile: %s : %s\n",pak->filename, filename);
				// open a new file on the pakfile
					*file = fopen (pak->filename, "rb");
					if (!*file)
						Com_Error (ERR_FATAL, "Couldn't reopen %s", pak->filename);
					fseek (*file, pak->files[i].filepos, SEEK_SET);
					return pak->files[i].filelen;
				}
		}
		else
		{
	// check a file in the directory tree

			Com_sprintf (netpath, sizeof(netpath), "%s/%s",search->filename, filename);

			*file = fopen (netpath, "rb");
			if (!*file)
				continue;

			Com_DPrintf ("FindFile: %s\n",netpath);

			return FS_filelength (*file);
		}

	}

	Com_DPrintf ("FindFile: can't find %s\n", filename);

	*file = NULL;
	return -1;
}

#else

// this is just for demos to prevent add on hacking

int FS_FOpenFile (char *filename, FILE **file)
{
	searchpath_t	*search;
	char			netpath[MAX_OSPATH];
	pack_t			*pak;
	int				i;

	file_from_pak = 0;

	// get config from directory, everything else from pak
	if (!strcmp(filename, "config.cfg") || !strncmp(filename, "players/", 8))
	{
		Com_sprintf (netpath, sizeof(netpath), "%s/%s",FS_Gamedir(), filename);

		*file = fopen (netpath, "rb");
		if (!*file)
			return -1;

		Com_DPrintf ("FindFile: %s\n",netpath);

		return FS_filelength (*file);
	}

	for (search = fs_searchpaths ; search ; search = search->next)
		if (search->pack)
			break;
	if (!search)
	{
		*file = NULL;
		return -1;
	}

	pak = search->pack;
	for (i=0 ; i<pak->numfiles ; i++)
		if (!Q_strcasecmp (pak->files[i].name, filename))
		{	// found it!
			file_from_pak = 1;
			Com_DPrintf ("PackFile: %s : %s\n",pak->filename, filename);
		// open a new file on the pakfile
			*file = fopen (pak->filename, "rb");
			if (!*file)
				Com_Error (ERR_FATAL, "Couldn't reopen %s", pak->filename);
			fseek (*file, pak->files[i].filepos, SEEK_SET);
			return pak->files[i].filelen;
		}

	Com_DPrintf ("FindFile: can't find %s\n", filename);

	*file = NULL;
	return -1;
}

#endif


/*
=================
FS_ReadFile

Properly handles partial reads
=================
*/
#define	MAX_READ	0x10000		// read in blocks of 64k
void FS_Read (void *buffer, int len, FILE *f)
{
	int		block, remaining;
	int		read;
	byte	*buf;
	int		tries;

	buf = (byte *)buffer;

	// read in chunks for progress bar
	remaining = len;
	tries = 0;
	while (remaining)
	{
		block = remaining;
		if (block > MAX_READ)
			block = MAX_READ;
		read = fread (buf, 1, block, f);
		if (read == 0)
		{
			Com_Error (ERR_FATAL, "FS_Read: 0 bytes read");
		}

		if (read == -1)
			Com_Error (ERR_FATAL, "FS_Read: -1 bytes read");

		// do some progress bar thing here...

		remaining -= read;
		buf += read;
	}
}

/*
==================
FS_TolowerPath

Makes the path to the given file lowercase on case-sensitive systems.
Kludge for case-sensitive and case-insensitive systems inteoperability.
Background: Some people may extract game paths/files as uppercase onto their
            HDD (while their system is case-insensitive, so game will work, but
            will case trouble for case-sensitive systems if they are acting
            as servers [propagating their maps etc. in uppercase]). Indeed the
            best approach here would be to make fopen()ing always case-
            insensitive, but due to resulting complexity and fact that
            Linux people will always install the game files with correct
            name casing, this is just fine.
-JR / 20050802 / 1
==================
*/
char *FS_TolowerPath (char *path)
{
	int	i = 0;
	static char buf[MAX_OSPATH]; // path can be const char *, so thats why

	do
		buf[i] = tolower(path[i]);
	while (path[i++]);

	return buf;
}

/*
============
FS_LoadFile

Filename are reletive to the quake search path
a null buffer will just return the file length without loading
============
*/
int FS_LoadFile (char *path, void **buffer)
{
	FILE	*h;
	byte	*buf;
	int		len;

	buf = NULL;	// quiet compiler warning

// look for it in the filesystem or pack files
	len = FS_FOpenFile (path, &h);

	//-JR
	if (!h)
	{
		path = FS_TolowerPath (path);
		len = FS_FOpenFile (path, &h);
	}

	if (!h)
	{
		if (buffer)
			*buffer = NULL;
		return -1;
	}

	if (!buffer)
	{
		fclose (h);
		return len;
	}

	buf = Z_Malloc(len+1);
	buf[len] = 0;
	*buffer = buf;

	FS_Read (buf, len, h);

	fclose (h);

	return len;
}

/*
=============
FS_FreeFile
=============
*/
void FS_FreeFile (void *buffer)
{
	Z_Free (buffer);
}

/*
=================
FS_LoadPackFile

Takes an explicit (not game tree related) path to a pak file.

Loads the header and directory, adding the files at the beginning
of the list so they override previous pack files.
=================
*/
pack_t *FS_LoadPackFile (char *packfile)
{
	dpackheader_t	header;
	int				i;
	packfile_t		*newfiles;
	int				numpackfiles;
	pack_t			*pack;
	FILE			*packhandle;
	dpackfile_t		info[MAX_FILES_IN_PACK];
	unsigned		checksum;

	packhandle = fopen(packfile, "rb");

	//-JR
	if (!packhandle)
	{
		packfile = FS_TolowerPath (packfile);
		packhandle = fopen(packfile, "rb");
	}

	if (!packhandle)
		return NULL;

	fread (&header, 1, sizeof(header), packhandle);
	if (LittleLong(header.ident) != IDPAKHEADER)
		Com_Error (ERR_FATAL, "%s is not a packfile", packfile);
	header.dirofs = LittleLong (header.dirofs);
	header.dirlen = LittleLong (header.dirlen);

	numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK)
		Com_Error (ERR_FATAL, "%s has %i files", packfile, numpackfiles);

	newfiles = Z_Malloc (numpackfiles * sizeof(packfile_t));

	fseek (packhandle, header.dirofs, SEEK_SET);
	fread (info, 1, header.dirlen, packhandle);

// crc the directory to check for modifications
	checksum = Com_BlockChecksum ((void *)info, header.dirlen);

#ifdef NO_ADDONS
	if (checksum != PAK0_CHECKSUM)
		return NULL;
#endif
// parse the directory
	for (i=0 ; i<numpackfiles ; i++)
	{
		strcpy (newfiles[i].name, info[i].name);
		newfiles[i].filepos = LittleLong(info[i].filepos);
		newfiles[i].filelen = LittleLong(info[i].filelen);
	}

	pack = Z_Malloc (sizeof (pack_t));
	strcpy (pack->filename, packfile);
	pack->handle = packhandle;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;

	Com_Printf ("Added packfile %s (%i files)\n", packfile, numpackfiles);
	return pack;
}

/*
 * ========= FS_FileExists ========
 */
qboolean
FS_FileExists(char *path)
{
	FILE*	f;

	FS_FOpenFile(path, &f);

	if (f != 0) {
		FS_FCloseFile(f);
		return (true);
	}
	return (false);
}

/*
================
FS_AddGameDirectory

Sets fs_gamedir, adds the directory to the head of the path,
then loads and adds pak1.pak pak2.pak ...
================
*/
void FS_AddGameDirectory (char *dir)
{
	int				i;
	searchpath_t	*search;
	pack_t			*pak;
	char			pakfile[MAX_OSPATH];
#ifdef _WINDOWS
	strcpy (fs_gamedir, dir);
#endif
	//
	// add the directory to the search path
	//
	search = Z_Malloc (sizeof(searchpath_t));
#ifdef __unix__
	strncpy (search->filename, dir, sizeof(search->filename)-1);
	search->filename[sizeof(search->filename)-1] = 0;
#else
	strcpy (search->filename, dir);
#endif
	search->next = fs_searchpaths;
	fs_searchpaths = search;

	//
	// add any pak files in the format pak0.pak pak1.pak, ...
	//
	for (i=0; i<10; i++)
	{
		Com_sprintf (pakfile, sizeof(pakfile), "%s/pak%i.pak", dir, i);
		pak = FS_LoadPackFile (pakfile);
		if (!pak)
			continue;
		search = Z_Malloc (sizeof(searchpath_t));
		search->pack = pak;
		search->next = fs_searchpaths;
		fs_searchpaths = search;
	}


}

#ifdef __unix__
/*
================
FS_AddHomeAsGameDirectory

Use ~/.codered/dir as fs_gamedir
================
*/
void FS_AddHomeAsGameDirectory (char *dir)
{
	char gdir[MAX_OSPATH];
	char *homedir = getenv("COR_GAME");
	int len;

	if (homedir)
	{
		len = snprintf (gdir, sizeof(gdir), "%s/%s/", homedir, dir);
	}
	else
	{
		homedir = getenv("HOME");
		if (!homedir)
			return;
		len = snprintf(gdir,sizeof(gdir),"%s/.codered/%s/", homedir, dir);
	}

	Com_Printf("using %s for writing\n",gdir);
	FS_CreatePath (gdir);

	if ((len > 0) && (len < sizeof(gdir)) && (gdir[len-1] == '/'))
		gdir[len-1] = 0;

	strncpy(fs_gamedir,gdir,sizeof(fs_gamedir)-1);
	fs_gamedir[sizeof(fs_gamedir)-1] = 0;

	FS_AddGameDirectory (gdir);
 }
#endif

/*
============
FS_Gamedir

Called to find where to write a file (demos, savegames, etc)
============
*/
char *FS_Gamedir (void)
{
#ifdef __unix__
	return fs_gamedir;
#else
	if (*fs_gamedir)
		return fs_gamedir;
	else
		return BASEDIRNAME;
#endif
}

/*
=============
FS_ExecAutoexec
=============
*/
void FS_ExecAutoexec (void)
{
#ifdef __unix__
	searchpath_t *s, *end;
#else
	char *dir;
#endif
	char name [MAX_QPATH];
#ifdef __unix__
	if (fs_searchpaths == fs_base_searchpaths)
		end = NULL;
#else
	dir = Cvar_VariableString("gamedir");
	if (*dir)
		Com_sprintf(name, sizeof(name), "%s/%s/autoexec.cfg", fs_basedir->string, dir);
#endif
	else
#ifdef __unix__
		end = fs_base_searchpaths;

	// search through all the paths for an autoexec.cfg file
	for (s = fs_searchpaths ; s != end ; s = s->next)
	{
		if (s->pack)
			continue;

		Com_sprintf(name, sizeof(name), "%s/autoexec.cfg", s->filename);

		if (Sys_FindFirst(name, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM))
		{
			Cbuf_AddText ("exec autoexec.cfg\n");
			Sys_FindClose();
			break;
		}
		Sys_FindClose();
	}
#else
		Com_sprintf(name, sizeof(name), "%s/%s/autoexec.cfg", fs_basedir->string, BASEDIRNAME);
	if (Sys_FindFirst(name, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM))
		Cbuf_AddText ("exec autoexec.cfg\n");
	Sys_FindClose();
#endif
}


/*
================
FS_SetGamedir

Sets the gamedir and path to a different directory.
================
*/
void FS_SetGamedir (char *dir)
{
	searchpath_t	*next;

	if (strstr(dir, "..") || strstr(dir, "/")
		|| strstr(dir, "\\") || strstr(dir, ":") )
	{
		Com_Printf ("Gamedir should be a single filename, not a path\n");
		return;
	}

	//
	// free up any current game dir info
	//
	while (fs_searchpaths != fs_base_searchpaths)
	{
		if (fs_searchpaths->pack)
		{
			fclose (fs_searchpaths->pack->handle);
			Z_Free (fs_searchpaths->pack->files);
			Z_Free (fs_searchpaths->pack);
		}
		next = fs_searchpaths->next;
		Z_Free (fs_searchpaths);
		fs_searchpaths = next;
	}

	//
	// flush all data, so it will be forced to reload
	//
	if (dedicated && !dedicated->value)
		Cbuf_AddText ("vid_restart\nsnd_restart\n");
#ifdef _WINDOWS
	Com_sprintf (fs_gamedir, sizeof(fs_gamedir), "%s/%s", fs_basedir->string, dir);
#endif
	if (!strcmp(dir,BASEDIRNAME) || (*dir == 0))
	{
		Cvar_FullSet ("gamedir", "", CVAR_SERVERINFO|CVAR_NOSET);
		Cvar_FullSet ("game", "", CVAR_LATCH|CVAR_SERVERINFO);
	}
	else
	{
		Cvar_FullSet ("gamedir", dir, CVAR_SERVERINFO|CVAR_NOSET);
#ifdef DATADIR
		FS_AddGameDirectory (va("%s/%s", DATADIR, dir) );
#endif
#ifdef LIBDIR
		FS_AddGameDirectory (va("%s/%s", LIBDIR, dir) );
#endif
		FS_AddGameDirectory (va("%s/%s", fs_basedir->string, dir) );
#ifdef __unix__
		FS_AddHomeAsGameDirectory(dir);
#endif
	}
}


/*
================
FS_Link_f

Creates a filelink_t
================
*/
void FS_Link_f (void)
{
	filelink_t	*l, **prev;

	if (Cmd_Argc() != 3)
	{
		Com_Printf ("USAGE: link <from> <to>\n");
		return;
	}

	// see if the link already exists
	prev = &fs_links;
	for (l=fs_links ; l ; l=l->next)
	{
		if (!strcmp (l->from, Cmd_Argv(1)))
		{
			Z_Free (l->to);
			if (!strlen(Cmd_Argv(2)))
			{	// delete it
				*prev = l->next;
				Z_Free (l->from);
				Z_Free (l);
				return;
			}
			l->to = CopyString (Cmd_Argv(2));
			return;
		}
		prev = &l->next;
	}

	// create a new link
	l = Z_Malloc(sizeof(*l));
	l->next = fs_links;
	fs_links = l;
	l->from = CopyString(Cmd_Argv(1));
	l->fromlength = strlen(l->from);
	l->to = CopyString(Cmd_Argv(2));
}

/*
** FS_ListFiles
**
** IMPORTANT: does not count the guard in returned "numfiles" anymore, to
** avoid adding/subtracting 1 all the time.
*/
char **FS_ListFiles( char *findname, int *numfiles, unsigned musthave, unsigned canthave )
{
	char *s;
	int nfiles = 0;
	char **list = 0;

	s = Sys_FindFirst( findname, musthave, canthave );
	while ( s )
	{
		if ( s[strlen(s)-1] != '.' )
			nfiles++;
		s = Sys_FindNext( musthave, canthave );
	}
	Sys_FindClose ();

	if ( !nfiles )
		return NULL;

	*numfiles = nfiles;
	nfiles++; // add space for a guard

	list = malloc( sizeof( char * ) * nfiles );
	memset( list, 0, sizeof( char * ) * nfiles );

	s = Sys_FindFirst( findname, musthave, canthave );
	nfiles = 0;
	while ( s )
	{
		if ( s[strlen(s)-1] != '.' )
		{
			list[nfiles] = strdup( s );
#ifdef _WIN32
			strlwr( list[nfiles] );
#endif
			nfiles++;
		}
		s = Sys_FindNext( musthave, canthave );
	}
	Sys_FindClose ();

	return list;
}

void FS_FreeFileList (char **list, int n) // jit
{
	int i;

	for (i = 0; i < n; i++)
	{
		free(list[i]);
		list[i] = 0;
	}

	free(list);
}

/*
 * CompareAttributesPack
 *
 * Compare file attributes (musthave and canthave) in packed files. If
 * "output" is not NULL, "size" is greater than zero and the file matches the
 * attributes then a copy of the matching string will be placed there (with
 * SFF_SUBDIR it changes).
 *
 * Returns a boolean value, true if the attributes match the file.
 */
qboolean
ComparePackFiles(const char *findname, const char *name,
    unsigned musthave, unsigned canthave, char *output, int size)
{
	qboolean	 retval;
	char		*ptr;
	char		 buffer[MAX_OSPATH];

	strncpy(buffer, name, sizeof(buffer)-1);
	buffer[sizeof(buffer)-1] = '\0';

	if ((canthave & SFF_SUBDIR) && name[strlen(name)-1] == '/')
		return (false);

	if (musthave & SFF_SUBDIR) {
		if ((ptr = strrchr(buffer, '/')) != NULL)
			*ptr = '\0';
		else
			return (false);
	}

	if ((musthave & SFF_HIDDEN) || (canthave & SFF_HIDDEN)) {
		if ((ptr = strrchr(buffer, '/')) == NULL)
			ptr = buffer;
		if (((musthave & SFF_HIDDEN) && ptr[1] != '.') ||
		    ((canthave & SFF_HIDDEN) && ptr[1] == '.'))
			return (false);
	}

	if (canthave & SFF_RDONLY)
		return (false);

	retval = glob_match((char *)findname, buffer);

	if (retval && output != NULL) {
		strncpy(output, buffer, size-1);
		buffer[size-1] = '\0';
	}

	return (retval);
}

/*
 * FS_ListFilesInFS
 *
 * Create a list of files that match a criteria.
 *
 * Searchs are relative to the game directory and use all the search paths
 * including .pak and .pk3 files.
 */
char **
FS_ListFilesInFS(char *findname, int *numfiles, unsigned musthave,
    unsigned canthave)
{
	searchpath_t	*search;		/* Search path. */
	int		i, j;			/* Loop counters. */
	int		nfiles;			/* Number of files found. */
	int		tmpnfiles;		/* Temp number of files. */
	char		**tmplist;		/* Temporary list of files. */
	char		**list;			/* List of files found. */
	char		path[MAX_OSPATH];	/* Temporary path. */

	nfiles = 0;
	list = malloc(sizeof(char *));

	for (search = fs_searchpaths; search != NULL; search = search->next) {
		if (search->pack != NULL) {
			if (canthave & SFF_INPACK)
				continue;

			for (i = 0, j = 0; i < search->pack->numfiles; i++)
				if (ComparePackFiles(findname,
				    search->pack->files[i].name,
				    musthave, canthave, NULL, 0))
					j++;
			if (j == 0)
				continue;
			nfiles += j;
			list = realloc(list, nfiles * sizeof(char *));
			for (i = 0, j = nfiles - j;
			    i < search->pack->numfiles;
			    i++)
				if (ComparePackFiles(findname,
				    search->pack->files[i].name,
				    musthave, canthave, path, sizeof(path)))
					list[j++] = strdup(path);
		} else if (search->filename != NULL) {
			if (musthave & SFF_INPACK)
				continue;

			Com_sprintf(path, sizeof(path), "%s/%s",
			    search->filename, findname);
			tmplist = FS_ListFiles(path, &tmpnfiles, musthave,
			    canthave);
			if (tmplist != NULL) {
				nfiles += tmpnfiles;
				list = realloc(list, nfiles * sizeof(char *));
				for (i = 0, j = nfiles - tmpnfiles;
				    i < tmpnfiles;
				    i++, j++)
					list[j] = strdup(tmplist[i] +
					    strlen(search->filename) + 1);
				FS_FreeFileList(tmplist, tmpnfiles);
			}
		}
	}

	/* Delete duplicates. */
	tmpnfiles = 0;
	for (i = 0; i < nfiles; i++) {
		if (list[i] == NULL)
			continue;
		for (j = i + 1; j < nfiles; j++)
			if (list[j] != NULL &&
			    strcmp(list[i], list[j]) == 0) {
				free(list[j]);
				list[j] = NULL;
				tmpnfiles++;
			}
	}

	if (tmpnfiles > 0) {
		nfiles -= tmpnfiles;
		tmplist = malloc(nfiles * sizeof(char *));
		for (i = 0, j = 0; i < nfiles + tmpnfiles; i++)
			if (list[i] != NULL)
				tmplist[j++] = list[i];
		free(list);
		list = tmplist;
	}

	/* Add a guard. */
	if (nfiles > 0) {
		nfiles++;
		list = realloc(list, nfiles * sizeof(char *));
		list[nfiles - 1] = NULL;
	} else {
		free(list);
		list = NULL;
	}

	/* IMPORTANT: Don't count the guard when returning nfiles. */
	nfiles--;

	*numfiles = nfiles;

	return (list);
}

/*
** FS_Dir_f
*/
void FS_Dir_f( void )
{
	char	*path = NULL;
	char	findname[1024];
	char	wildcard[1024] = "*.*";
	char	**dirnames;
	int		ndirs;

	if ( Cmd_Argc() != 1 )
	{
		strcpy( wildcard, Cmd_Argv( 1 ) );
	}

	while ( ( path = FS_NextPath( path ) ) != NULL )
	{
		char *tmp = findname;

		Com_sprintf( findname, sizeof(findname), "%s/%s", path, wildcard );

		while ( *tmp != 0 )
		{
			if ( *tmp == '\\' )
				*tmp = '/';
			tmp++;
		}
		Com_Printf( "Directory of %s\n", findname );
		Com_Printf( "----\n" );

		if ( ( dirnames = FS_ListFiles( findname, &ndirs, 0, 0 ) ) != 0 )
		{
			int i;

			for ( i = 0; i < ndirs; i++ )
			{
				if ( strrchr( dirnames[i], '/' ) )
					Com_Printf( "%s\n", strrchr( dirnames[i], '/' ) + 1 );
				else
					Com_Printf( "%s\n", dirnames[i] );

				free( dirnames[i] );
			}
			free( dirnames );
		}
		Com_Printf( "\n" );
	};
}

/*
============
FS_Path_f

============
*/
void FS_Path_f (void)
{
	searchpath_t	*s;
	filelink_t		*l;

	Com_Printf ("Current search path:\n");
	for (s=fs_searchpaths ; s ; s=s->next)
	{
		if (s == fs_base_searchpaths)
			Com_Printf ("----------\n");
		if (s->pack)
			Com_Printf ("%s (%i files)\n", s->pack->filename, s->pack->numfiles);
		else
			Com_Printf ("%s\n", s->filename);
	}

	Com_Printf ("\nLinks:\n");
	for (l=fs_links ; l ; l=l->next)
		Com_Printf ("%s : %s\n", l->from, l->to);
}

/*
================
FS_NextPath

Allows enumerating all of the directories in the search path
================
*/
char *FS_NextPath (char *prevpath)
{
	searchpath_t	*s;
	char			*prev;

#ifdef __unix__
	prev = NULL; /* fs_gamedir is the first directory in the searchpath */
#else
	if (!prevpath)
		return fs_gamedir;

	prev = fs_gamedir;
#endif
	for (s=fs_searchpaths ; s ; s=s->next)
	{
		if (s->pack)
			continue;
#ifdef __unix__
		if (prevpath == NULL)
			return s->filename;
#endif
		if (prevpath == prev)
			return s->filename;
		prev = s->filename;
	}

	return NULL;
}


/*
================
FS_InitFilesystem
================
*/
void FS_InitFilesystem (void)
{
	Cmd_AddCommand ("path", FS_Path_f);
	Cmd_AddCommand ("link", FS_Link_f);
	Cmd_AddCommand ("dir", FS_Dir_f );

	//
	// DATADIR / LIBDIR support.
	// Both directories are used to load data and libraries from, allowing
	// different OSes to have them where they want, according to their
	// conventions.
	//
#ifdef DATADIR
	FS_AddGameDirectory (va("%s/"BASEDIRNAME, DATADIR));
#endif
#ifdef LIBDIR
	FS_AddGameDirectory (va("%s/"BASEDIRNAME, LIBDIR));
#endif

	//
	// basedir <path>
	// allows the game to run from outside the data tree
	//
	fs_basedir = Cvar_Get ("basedir", ".", CVAR_NOSET);

	//
	// start up with baseq2 by default
	//
	FS_AddGameDirectory (va("%s/"BASEDIRNAME, fs_basedir->string) );

#ifdef __unix__
	FS_AddHomeAsGameDirectory(BASEDIRNAME);
#endif
	// any set gamedirs will be freed up to here
	fs_base_searchpaths = fs_searchpaths;

	// check for game override
	fs_gamedirvar = Cvar_Get ("game", "", CVAR_LATCH|CVAR_SERVERINFO);
	if (fs_gamedirvar->string[0])
		FS_SetGamedir (fs_gamedirvar->string);
	else
		FS_SetGamedir ("arena");
}

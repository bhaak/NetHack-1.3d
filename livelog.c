/* Write live game progress changes to a log file
 * Needs xlog-v3 patch. */

#include "hack.h"

#ifdef LIVELOGFILE

extern char plname[], pl_character[];

/* Keep the last xlog "achieve" value to be able to compare */
long last_achieve_int;

/* Generic buffer for snprintf */
#define STRBUF_LEN (4096)
char strbuf[STRBUF_LEN];
char prefixbuf[STRBUF_LEN];

/* Locks the live log file and writes 'buffer' */
void livelog_write_string(char* buffer) {
	FILE* livelogfile;
	if(!(livelogfile = fopen(LIVELOGFILE, "a"))) {
		pline("Cannot open live log file!");
	} else {
		fprintf(livelogfile, "%s", buffer);
		(void) fclose(livelogfile);
	}
}

static
char *livelog_prefix() {
	snprintf(prefixbuf, STRBUF_LEN,
			"version=%s-%s:"
			"player=%s:turns=%ld:starttime=%ld:"
			"currenttime=%ld:"
			"dlev=%d:maxlvl=%d:"
			"hp=%d:maxhp=%d:"
			"role=%s:"
			"gender=%s:"
			"explvl=%d:exp=%ld",
			GAME_SHORT_NAME, VERSION,
			plname,
			moves,
			(long)u.ubirthday,
			(long)current_epoch(),
			dlevel, maxdlevel,
			u.uhp, u.uhpmax,
			pl_character,
			flags.female ? "Fem" : "Mal",
			u.ulevel,u.uexp);
	return prefixbuf;
}

/* Writes changes in the achieve structure to the live log.
 * Called from various places in the NetHack source,
 * usually where xlog's achieve is set. */
#if 0
void livelog_achieve_update() {
	long achieve_int, achieve_diff;

	achieve_int = encodeachieve();
	achieve_diff = last_achieve_int ^ achieve_int;

	/* livelog_achieve_update is sometimes called when there's
	 * no actual change. */
	if(achieve_diff == 0) {
		return;
	}

	snprintf(strbuf, STRBUF_LEN,
		"%s:type=achievements:achieve=0x%lx:achieve_diff=0x%lx\n",
		livelog_prefix(),
		achieve_int,
		achieve_diff);
	livelog_write_string(strbuf);

	last_achieve_int = achieve_int;
}
#endif

/* Reports wishes */
void
livelog_wish(item)
char *item;
{
	snprintf(strbuf, STRBUF_LEN,
		"%s:type=wish:wish=%s\n",
		livelog_prefix(),
		item);
	livelog_write_string(strbuf);
}

/* Shout */
#ifdef LIVELOG_SHOUT

int 
doshout()
{
	char buf[BUFSZ], qbuf[QBUFSZ];
	char* p;
	
	Sprintf(qbuf,"Shout what?");
	getlin(qbuf, buf);
	
	if (strlen(buf) == 0) {
		Strcpy(buf, "*@&%!!");
		You("shout profanities into the void!");
	} else {
		You("shout into the void: %s", buf);
	}

	/* filter livelog delimiter */
	for (p = buf; *p != 0; p++)
		if( *p == ':' )
			*p = ' ';

	livelog_generic("shout", buf);

	return 0;
}

#endif /* LIVELOG_SHOUT */

#ifdef LIVELOG_BONES_KILLER
void
livelog_bones_killed(mtmp)
struct monst *mtmp;
{
	char *name = NAME(mtmp);

	if (name &&
	    mtmp->former_rank && strlen(mtmp->former_rank) > 0) {
		/* $player killed the $bones_monst of $bones_killed the former
		 * $bones_rank on $turns on dungeon level $dlev! */
		snprintf(strbuf, STRBUF_LEN,
				"%s:type=bones_killed:bones_killed=%s:bones_rank=%s:bones_monst=%s\n",
				livelog_prefix(),
				name,
				mtmp->former_rank,
				mtmp->data->mname);
		livelog_write_string(strbuf);
	} else if (mtmp->data->geno & G_UNIQ) {
		char *n = noit_mon_nam(mtmp);
		/* $player killed a uniq monster */
		livelog_generic("killed_uniq", n);
	} else if (mtmp->isshk) {
		char *n = noit_mon_nam(mtmp);
		/* $player killed a shopkeeper */
		livelog_generic("killed_shopkeeper", n);
	}
}
#endif /* LIVELOG_BONES_KILLER */

/** Reports shoplifting */
void
livelog_shoplifting(shk_name, shop_name, total)
const char* shk_name;
const char* shop_name;
long total;
{
	/* shopkeeper: Name of the shopkeeper (e.g. Kopasker)
	   shop:       Name of the shop (e.g. general store)
	   shoplifted: Merchandise worth this many Zorkmids was stolen */
	snprintf(strbuf, STRBUF_LEN,
		"%s:type=shoplifting:shopkeeper=%s:shop=%s:shoplifted=%ld\n",
		livelog_prefix(),
		shk_name,
		shop_name,
		total);
	livelog_write_string(strbuf);
}

/** Livelog method for reporting the starting/resuming of a game. */
void
livelog_game_started(verb, role)
const char* verb;
const char* role;
{
	snprintf(strbuf, STRBUF_LEN,
		"%s:type=%s:game_action=%s:character=%s\n",
		livelog_prefix(),
		verb,
		verb,
		role);
	livelog_write_string(strbuf);
}

/** Livelog method for reporting saving, quitting, etc. */
void
livelog_game_action(verb)
const char* verb;
{
	snprintf(strbuf, STRBUF_LEN,
		"%s:type=%s:game_action=%s\n",
		livelog_prefix(),
		verb,
		verb);
	livelog_write_string(strbuf);
}

/** Livelog method for reporting generic events with one customizable field. */
void
livelog_generic(field, text)
const char* field;
const char* text;
{
	snprintf(strbuf, STRBUF_LEN,
		"%s:type=%s:%s=%s\n",
		livelog_prefix(),
		field,
		field,
		text);
	livelog_write_string(strbuf);
}

/** Livelog method for reporting monster genocides. */
void
livelog_genocide(genocided_monster, level_wide)
const char* genocided_monster;
int level_wide;
{
	snprintf(strbuf, STRBUF_LEN,
		"%s:type=genocide:genocided_monster=%s:dungeon_wide=%s\n",
		livelog_prefix(),
		genocided_monster,
		level_wide ? "no" : "yes");
	livelog_write_string(strbuf);
}

#endif /* LIVELOGFILE */

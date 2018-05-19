/*	SCCS Id: @(#)end.c	3.5	2010/03/08	*/
/* Copyright (c) Patric Mueller.			*/
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* originally from end.c */
#ifdef DUMP_LOG
#ifdef DUMP_FN
char dump_fn[] = DUMP_FN;
#endif
#endif /* DUMP_LOG */

#ifdef DUMP_LOG

# ifdef HAVE_UTIME_H
#  include <utime.h>
#  include <errno.h>
# endif
extern char msgs[][BUFSZ];
extern int lastmsg;
void do_vanquished(int, int);
extern char plname[];

char* html_escape_character(const char c);
void dump_html(const char *format, const char *str);
#endif

#ifdef DUMP_LOG
FILE *dump_fp = (FILE *)0;  /**< file pointer for text dumps */
char dump_path[BUFSIZ];
FILE *html_dump_fp = (FILE *)0;  /**< file pointer for html dumps */
char html_dump_path[BUFSIZ];
/* TODO:
 * - escape unmasked characters in html (done for map)
 * - started/ended date at the top
 */

#define Sprintf sprintf
static
char*
get_dump_filename()
{
    char *buf = (char *) alloc((unsigned)(BUFSIZ+1));
    long ubirthday = u.ubirthday;
    const char *fp = dump_fn;
    char *bp = buf;
    int slen, len = 0;
    char tmpbuf[BUFSZ];
    char verbuf[BUFSZ];
    long uid;
    time_t now;

    now = time(NULL);
    uid = (long) getuid();

    /*
     * Note: %t and %T assume that time_t is a 'long int' number of
     * seconds since some epoch value.  That's quite iffy....  The
     * unit of time might be different and the datum size might be
     * some variant of 'long long int'.  [Their main purpose is to
     * construct a unique file name rather than record the date and
     * time; violating the 'long seconds since base-date' assumption
     * may or may not interfere with that usage.]
     */

    while (fp && *fp && len < BUFSZ-1) {
        if (*fp == '%') {
            fp++;
            switch (*fp) {
            default:
                goto finish;
            case '\0': /* fallthrough */
            case '%':  /* literal % */
                Sprintf(tmpbuf, "%%");
                break;
            case 't': /* game start, timestamp */
                Sprintf(tmpbuf, "%lu", (unsigned long) ubirthday);
                break;
            case 'T': /* current time, timestamp */
                Sprintf(tmpbuf, "%lu", (unsigned long) now);
                break;
            case 'd': /* game start, YYYYMMDDhhmmss */
                Sprintf(tmpbuf, "%08ld%06ld",
                        yyyymmdd(ubirthday), hhmmss(ubirthday));
                break;
            case 'D': /* current time, YYYYMMDDhhmmss */
                Sprintf(tmpbuf, "%08ld%06ld", yyyymmdd(now), hhmmss(now));
                break;
#if 0
            case 'v': /* version, eg. "3.6.2-0" */
                Sprintf(tmpbuf, "%s", version_string(verbuf));
                break;
#endif
            case 'u': /* UID */
                Sprintf(tmpbuf, "%ld", uid);
                break;
            case 'n': /* player name */
            case 's': /* player name */
                Sprintf(tmpbuf, "%s", *plname ? plname : "unknown");
                break;
            case 'N': /* first character of player name */
                Sprintf(tmpbuf, "%c", *plname ? *plname : 'u');
                break;
            }

            slen = strlen(tmpbuf);
            if (len + slen < BUFSZ-1) {
                len += slen;
                Sprintf(bp, "%s", tmpbuf);
                bp += slen;
                if (*fp) fp++;
            } else
                break;
        } else {
            *bp = *fp;
            bp++;
            fp++;
            len++;
        }
    }
 finish:
    *bp = '\0';
    return buf;
}

void
dump_init()
{
  if (dump_fn[0]) {
    char *new_dump_fn = get_dump_filename();

#ifdef DUMP_TEXT_LOG
    strncpy(dump_path, new_dump_fn, BUFSIZ-1);
    dump_fp = fopen(new_dump_fn, "w");
    if (!dump_fp) {
	pline("Can't open %s for output.", new_dump_fn);
	pline("Dump file not created.");
    }
#endif
#ifdef DUMP_HTML_LOG
    strncpy(html_dump_path, strcat(new_dump_fn, ".html"), BUFSIZ-1);
    html_dump_fp = fopen(html_dump_path, "w");
    if (!html_dump_fp) {
	pline("Can't open %s for output.", new_dump_fn);
	pline("Html dump file not created.");
    }
#endif
    if (new_dump_fn) free(new_dump_fn);
  }
}
#endif

#ifdef DUMP_LOG
/** Set a file's access and modify time to u.udeathday. */
static void
adjust_file_timestamp(fpath)
const char* fpath;
{
# ifdef HAVE_UTIME_H
	if (u.udeathday > 0) {
		struct utimbuf tv;
		tv.actime = u.udeathday;
		tv.modtime = u.udeathday;
		if (utime(fpath, &tv)) {
			paniclog("adjust_file_timestamp: utime failed: ", strerror(errno));
		}
	}
# endif
}
#endif

void
dump_exit()
{
#ifdef DUMP_LOG
	if (dump_fp) {
		fclose(dump_fp);
		dump_fp = NULL;
		adjust_file_timestamp(dump_path);
	}
	if (html_dump_fp) {
		dump_html("</body>\n</html>\n","");
		fclose(html_dump_fp);
		html_dump_fp = NULL;
		adjust_file_timestamp(html_dump_path);
	}
#endif
}

void
dump(pre, str)
const char *pre, *str;
{
#ifdef DUMP_LOG
  if (dump_fp)
    fprintf(dump_fp, "%s%s\n", pre, str);
  if (html_dump_fp)
    fprintf(html_dump_fp, "%s%s\n", pre, str);
#endif
}

/** Outputs a string only into the html dump. */
void
dump_html(format, str)
const char *format, *str;
{
#ifdef DUMP_LOG
  if (html_dump_fp)
    fprintf(html_dump_fp, format, str);
#endif
}

/** Outputs a string only into the text dump. */
void
dump_text(format, str)
const char *format, *str;
{
#ifdef DUMP_LOG
  if (dump_fp)
    fprintf(dump_fp, format, str);
#endif
}

/** Dumps one line as is. */
void
dump_line(pre, str)
const char *pre, *str;
{
#ifdef DUMP_LOG
  if (dump_fp)
    fprintf(dump_fp, "%s%s\n", pre, str);
  if (html_dump_fp)
    fprintf(html_dump_fp, "%s%s<br />\n", pre, str);
#endif
}

#ifdef MENU_COLOR
extern boolean get_menu_coloring(const char *str, int *color, int *attr);
#endif

static char tmp_html_link[BUFSZ];
/** Return a link to nethackwiki . */
char *
html_link(link_name, name)
const char *link_name;
const char *name;
{
	snprintf(tmp_html_link, BUFSZ,
		"<a href=\"http://nethackwiki.com/wiki/%s\">%s</a>",
		link_name, name);
	return tmp_html_link;
}

/** Dumps an object from the inventory. */
void
dump_object(c, obj, str)
const char c;
const struct obj *obj;
const char *str;
{
#ifdef DUMP_LOG
	if (dump_fp)
		fprintf(dump_fp, "  %c - %s\n", c, str);
	if (html_dump_fp) {
		char *link = ""; /* html_link(dump_typename(obj->otyp), str); */
#ifdef MENU_COLOR
# ifdef TTY_GRAPHICS
		int color;
		int attr;
		if (iflags.use_menu_color &&
		    get_menu_coloring(str, &color, &attr)) {
			fprintf(html_dump_fp, "<span class=\"nh_color_%d\"><span class=\"nh_item_letter\">%c</span> - %s</span><br />\n", color, c, link);
		} else
# endif
#endif
		fprintf(html_dump_fp, "<span class=\"nh_item_letter\">%c</span> - %s<br />\n", c, link);
	}
#endif
}

/** Dumps a secondary title. */
void
dump_subtitle(str)
const char *str;
{
#ifdef DUMP_LOG
  dump_text("  %s\n", str);
  dump_html("<h3>%s</h3>\n", str);
#endif
}

/** Dump a title. Strips : from the end of str. */
void
dump_title(str)
char *str;
{
#ifdef DUMP_LOG
	int len = strlen(str);
	if (str[len-1] == ':') {
		str[len-1] = '\0';
	}
	if (dump_fp)
		fprintf(dump_fp, "%s\n", str);
	if (html_dump_fp)
		fprintf(html_dump_fp, "<h2>%s</h2>\n", str);
#endif
}

/** Starts a list in the dump. */
void
dump_list_start()
{
#ifdef DUMP_LOG
	if (html_dump_fp)
		fprintf(html_dump_fp, "<ul>\n");
#endif
}

/** Dumps a linked list item. */
void
dump_list_item_link(link, str)
const char *link;
const char *str;
{
#ifdef DUMP_LOG
	if (dump_fp)
		fprintf(dump_fp, "  %s\n", str);
	if (html_dump_fp)
		fprintf(html_dump_fp, "<li>%s</li>\n", html_link(link, str));
#endif
}

/** Dumps an object as list item. */
void
dump_list_item_object(obj)
struct obj *obj;
{
#ifdef DUMP_LOG
	if (dump_fp)
		fprintf(dump_fp, "  %s\n", doname(obj));
	if (html_dump_fp) {
		const char* str = doname(obj);
		char *link = ""; /* html_link(dump_typename(obj->otyp), str); */
#ifdef MENU_COLOR
# ifdef TTY_GRAPHICS
		int color;
		int attr;
		if (iflags.use_menu_color &&
		    get_menu_coloring(str, &color, &attr)) {
			fprintf(html_dump_fp, "<li class=\"nh_color_%d\">%s</li>\n", color, link);
		} else
# endif
#endif
		fprintf(html_dump_fp, "<li>%s</li>\n", link);
	}
#endif
}

/** Dumps a list item. */
void
dump_list_item(str)
const char *str;
{
#ifdef DUMP_LOG
	if (dump_fp)
		fprintf(dump_fp, "  %s\n", str);
	if (html_dump_fp)
		fprintf(html_dump_fp, "<li>%s</li>\n", str);
#endif
}

/** Ends a list in the dump. */
void
dump_list_end()
{
	dump_html("</ul>\n","");
}

/** Starts a blockquote in the dump. */
void
dump_blockquote_start()
{
#ifdef DUMP_LOG
	if (html_dump_fp)
		fprintf(html_dump_fp, "<blockquote>\n");
#endif
}

/** Ends a blockquote in the dump. */
void
dump_blockquote_end()
{
#ifdef DUMP_LOG
	dump_text("\n", "");
	dump_html("</blockquote>\n", "");
#endif
}

/** Starts a definition list in the dump. */
void
dump_definition_list_start()
{
#ifdef DUMP_LOG
	if (html_dump_fp)
		fprintf(html_dump_fp, "<dl>\n");
#endif
}

/** Dumps a definition list item. */
void
dump_definition_list_dt(str)
const char *str;
{
#ifdef DUMP_LOG
	if (dump_fp)
		fprintf(dump_fp, "  %s\n", str);
	if (html_dump_fp) {
		fprintf(html_dump_fp, "<dt>");
		while (*str != '\0')
			fprintf(html_dump_fp, "%s", html_escape_character(*str++));
		fprintf(html_dump_fp, "</dt>\n");
	}
#endif
}

/** Dumps a definition list item. */
void
dump_definition_list_dd(str)
const char *str;
{
#ifdef DUMP_LOG
	if (dump_fp)
		fprintf(dump_fp, "  %s\n", str);
	if (html_dump_fp)
		fprintf(html_dump_fp, "<dd>%s</dd>\n", str);
#endif
}

/** Ends a list in the dump. */
void
dump_definition_list_end()
{
	dump_html("</dl>\n","");
}

#ifdef DUMP_HTML_CSS_FILE
# ifdef DUMP_HTML_CSS_EMBEDDED
static
void
dump_html_css_file(const char *filename)
{
#  ifdef DUMP_HTML_LOG
	FILE *css = fopen(filename, "r");
	if (!css) {
		pline("Can't open %s for input.", filename);
		pline("CSS file not included.");
	} else if (css && html_dump_fp) {
		int c=0;
		while ((c=fgetc(css))!=EOF) {
			fputc(c, html_dump_fp);
		}
		fclose(css);
	}
#  endif
}
# endif
#endif


/** Dumps the HTML header. */
void
dump_header_html(title)
const char *title;
{
	dump_html("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n","");
	dump_html("<html xmlns=\"http://www.w3.org/1999/xhtml\">\n", "");
	dump_html("<head>\n", "");
	dump_html("<title>NetHack " VERSION ": %s</title>\n", title);
	dump_html("<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n", "");
	dump_html("<meta name=\"generator\" content=\"NetHack " VERSION "\" />\n", "");
	dump_html("<meta name=\"date\" content=\"%s\" />\n", (const char *)iso8601(0));
#ifdef DUMP_HTML_CSS_FILE
# ifndef DUMP_HTML_CSS_EMBEDDED
	dump_html("<link rel=\"stylesheet\" type=\"text/css\" href=\"" DUMP_HTML_CSS_FILE "\" />\n", "");
# else
	dump_html("<style type=\"text/css\">\n", "");
	dump_html_css_file(DUMP_HTML_CSS_FILE);
	dump_html("</style>\n", "");
# endif
#endif
	dump_html("</head>\n", "");
	dump_html("<body>\n", "");
}

static char html_escape_buf[BUFSZ];
/** Escape a single character for HTML. */
char* html_escape_character(const char c) {
	switch (c) {
		case '<':
			return "&lt;";
		case '>':
			return "&gt;";
		case '&':
			return "&amp;";
		case '\"':
			return "&quot;";
		case '\'':
			return "&#39;"; /* not &apos; */
		default:
			sprintf(html_escape_buf, "%c", c);
			return html_escape_buf;
	}
}

/*dump.c*/

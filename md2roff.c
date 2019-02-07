/*
*	md2roff.c
*	A utility to convert markdown documents to troff.
*
*	Copyright (C) 2017, Nicholas Christopoulos (mailto:nereus@freemail.gr)
*
*	License GPL3+
*	CC: std C99
*	history: 20017-05-08, created
*
*	This program is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License.
*	See LICENSE for details.
*/

#include <u.h>
#include <libc.h>

typedef int bool;
#define true 1
#define false 0

// buffers size in KB
#define MD_BUF_SZ	64

// options
bool opt_use_mm = false;
bool opt_use_mdoc = false;
bool opt_use_man = true;
bool opt_use_mom = false;

/*
 */
char *strdup(const char *s)
{
	char *d = (char *) malloc(strlen(s) + 1); 
    if ( d )
		strcpy(d, s);
    return d;
}

/*
*	squeeze (& strdup)
*/
char *sqzdup(const char *source)
{
	char *rp, *p, *d;
	int lc = 0;

	rp = malloc(strlen(source) + 1);
	p = (char *) source;
	d = rp;

	while ( isspace(*p) ) p ++;

	while ( *p ) {
		if ( isspace(*p) ) {
			if ( !lc ) {
				lc = 1;
				if ( p > source ) {
					if ( isalnum(*(p - 1)) )
						*d ++ = ' ';
					else {
						char *nc = p;
						while ( isspace(*nc) )
							nc ++;
						if ( isalnum(*nc) )
							*d ++ = ' ';
						}
					}
				}
			}
		else {
			lc = 0;
			*d ++ = *p;
			}
		p ++;
		}

	*d = '\0';
	if ( d > rp ) {
		if ( isspace(*(d - 1)) ) 
			*(d - 1) = '\0';
		}
	
	return rp;
}

/*
 * Loads the `filename` file into memory and return a pointer to its contents.
 * The pointer must freed by the user.
 */
char *loadfile(const char *filename)
{
	int len = -1, c, alloc;
	int fd;
	char *buf;

	if ( filename == nil ) {
		len = 0;
		alloc = 1024;
		buf = (char *) malloc(alloc);
		while ( (c = fgetc(0)) != EOF ) {
			buf[len] = c;
			len ++;
			if ( len >= alloc ) {
				alloc += 1024;
				buf = (char *) realloc(buf, alloc);
				}
			}
		}
	else {
		fp = fopen(filename, "r");
		if ( !fp ) {
			fprintf(stderr, "Unable to open '%s': %s\n", filename, strerror(errno));
			exit(EXIT_FAILURE);
			}
		if ( fseek(fp, 0L, SEEK_END) == -1 ) {
			fprintf(stderr, "fseek failed: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
			}
		if ( (len = ftell(fp)) == -1 ) {
			fprintf(stderr, "ftell failed: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
			}
		if ( fseek(fp, 0L, SEEK_SET) == -1 ) {
			fprintf(stderr, "fseek failed: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
			}
		buf = (char *) malloc(len+1);
		if ( fread(buf, len, 1, fp) == -1 ) {
			fprintf(stderr, "fread failed: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
			}
		buf[len] = '\0';
		fclose(fp);
		}

	return buf;
}

/*
 * adds the string 'str' to buffer 'buf' and returns
 * a pointer to new position in buf.
 */
char *stradd(char *buf, const char *str)
{
	const char *p = str;
	char *d = buf;

	while ( *p )
		*d ++ = *p ++;
	return d;
}

/*
 * prints the whole line of 'src' and returns pointer
 * to the next character (the first of the next line).
 */
const char *println(const char *src)
{
	const char *p = src;

	while ( *p ) {
		putchar(*p);
		p ++;
		if ( *(p-1) == '\n' )
			break;
		}
	return p;
}

/*
*	types of elements
*/
enum { none,
		par_end, ln_brk,
		cblock_end, cblock_open,
		li_open, li_end,
		ol_open, ul_open, lst_close,
		man_ref, ol, ul,
		bq_open, bq_close,
		box_open, box_close,
		url_mark,
		new_sh, new_ss, new_s4 };

/*
*	list (enumeration/itemize) stack
*/
#define	MAX_LIST_SIZE	32
int		stk_list[MAX_LIST_SIZE];	// type of list
int		stk_count[MAX_LIST_SIZE];	// counter of item
int		stk_list_p = 0;				// top pointer, always points to first free

/*
*	write the roff code of 'type'
*/
void roff(int type, ...)
{
	va_list	ap;
	char	*title, *link;

	va_start(ap, type);
	switch ( type ) {

	// new paragraph
	case par_end:
		if ( opt_use_mdoc )
			puts(".Pp");
		else
			puts(".PP");
		break;

	// line break
	case ln_brk:
		if ( opt_use_mom )
			puts(".BR");	// or .br or .EL or .LINEBREAK ????
		else
			puts(".br");
		break;

	// link
	case url_mark:
		title = va_arg(ap, char *);
		link = va_arg(ap, char *);
		if ( opt_use_man ) {
			if ( strchr(link, '@') )
				printf(".MT %s\n%s\n.ME\n", link, title);
			else
				printf(".UR %s\n%s\n.UE\n", link, title);
			}
		else if ( opt_use_mdoc ) {
			if ( strchr(link, '@') )
				printf(".An %s Aq Mt %s\n", title, link);
			else
				printf(".Lk %s \"%s\"\n", link, title);
			}
		else if ( opt_use_mm ) // there is no such thing...
			printf("%s <%s>\n", title, link);
		else if ( opt_use_mom ) // ??? 
			printf("%s \\*[UL]%s\\*[ULX]\n", title, link);
		break;
		
	// cartouche top
	case box_open:
		if ( opt_use_mom )
			puts(".DRH");
		else if ( opt_use_man )
			puts(".B");
		else
			puts(".FT B");
		break;

	// cartouche bottom
	case box_close:
		if ( opt_use_mom )
			puts(".DRH");
		else if ( opt_use_man )
			break;
//			puts(".P");
		else 
			puts(".FT P");
		break;

	// code block - begin
	case cblock_open:
		if ( opt_use_mom )
			printf(".CODE\n");
		else if ( opt_use_mdoc )
			printf(".Bd -literal -offset indent\n");
		else
			printf(".RS 4\n.EX\n");
		break;

	// code block - end
	case cblock_end:
		if ( opt_use_mom )
			printf(".CODE OFF\n");
		else if ( opt_use_mdoc )
			printf(".Ed\n");
		else
			printf("\n.EE\n.RE\n");
		break;

	// ordered list (1..2..3..)
	case ol_open:
		stk_list[stk_list_p] = ol;
		stk_count[stk_list_p] = 1;
		stk_list_p ++;
		if ( opt_use_mom ) {
			switch ( stk_list_p ) {
			case 1: puts(".LIST DIGIT"); break;
			case 2: puts(".LIST ALPHA"); break;
			case 3: puts(".LIST DIGIT"); break;
			case 4:	puts(".LIST alpha"); break;
			default:
				puts(".LIST DIGIT");
				};
			}
		else if ( opt_use_mdoc )
			puts(".Bl -enum -offset indent");
		else if ( opt_use_mm ) 
			puts(".AL");
		break;

	// unordered list (bullets)
	case ul_open:
		stk_list[stk_list_p] = ul;
		stk_count[stk_list_p] = 1;
		stk_list_p ++;
		if ( opt_use_mom ) {
			if ( stk_list_p % 2 ) 
				puts(".LIST BULLET");
			else
				puts(".LIST DASH");
			}
		else if ( opt_use_mdoc ) {
			if ( stk_list_p % 2 ) 
				puts(".Bl -bullet -offset indent");
			else
				puts(".Bl -dash -offset indent");
			}
		else if ( opt_use_mm ) 
			puts(".BL");
		break;

	// close list
	case lst_close:
		if ( opt_use_mom )
			puts(".LIST OFF");
		else if ( opt_use_mdoc )
			puts(".El");
		break;

	// list item - begin
	case li_open:
		if ( opt_use_mom )
			puts(".ITEM");
		else if ( opt_use_mdoc )
			puts(".It");
		else if ( opt_use_man ) {
			if ( stk_list_p ) {
				if ( stk_list[stk_list_p-1] == ul )
					puts(".IP \\(bu 4");
				else {
					printf(".IP %d. 4\n", stk_count[stk_list_p-1]);
					stk_count[stk_list_p-1] ++;
					}
				}
			}
		else
			puts(".LI");
		break;
		
	// list item - end
	case li_end:
		if ( opt_use_mom ) break;
		if ( opt_use_mdoc ) break;
		if ( opt_use_man ) break;
		puts(".LE");
		break;

	// new big header/section
	case new_sh:
		if ( opt_use_mom )
			printf(".HEADING 1 \"");
		else if ( opt_use_mdoc )
			printf(".Sh ");
		else
			printf(".SH ");
		break;

	// new medium header/secrtion
	case new_ss:
		if ( opt_use_mom )
			printf(".HEADING 2 \"");
		else if ( opt_use_mdoc )
			printf(".Ss ");
		else
			printf(".SS ");
		break;

	// new small header/secrtion
	case new_s4:
		if ( opt_use_mom )
			printf(".HEADING 3 \"");
		else if ( opt_use_mdoc )
			printf(".Ss ");
		else
			printf(".SS ");
		break;

	// reference to man page
	case man_ref:
		link = va_arg(ap, char *);
		if ( opt_use_mdoc )
			printf(".Xr %s\n", link);
		else if ( opt_use_man ) {
			char *tmp = strdup(link);
			char *p = strchr(tmp, ' ');
			if ( p ) {
				*p = '\0';
				printf("\\fB%s\\fP(%s)\n", tmp, p+1);
				}
			else
				printf("\\fB%s\\fP\n", link);
			free(tmp);
			}
		else
			printf("%s\n", link);
		break;
		}
	
	va_end(ap);
}

/*
 *  write buffer and reset
 */
char *flushln(char *d, char *bf)
{
	if ( d > bf ) {
		*d = '\0';
		d = bf;
		while ( isspace(*d) )
			d ++;
		if ( *d ) {
			char *z = sqzdup(d);
			puts(z);
			free(z);
			}
		}
	return bf;
}

/*
 *	this converts the file 'docname',
 *	that is loaded in 'source', to *-roff.
 */
void md2roff(const char *docname, const char *source)
{
	const char *p = source, *pnext, *pstart;
	char	*dest, *d;
	bool	bline = true, bcode = false;
	bool	bold = false, italics = false;
	bool	inside_list = false;
	bool	title_level = 0;

	stk_list_p = 0; // reset stack
	
	puts(".\\\" x-roff document");
	if ( opt_use_mm )
		puts(".do mso m.tmac"); // mm package, AL BL DL LI LE
	if ( opt_use_mdoc )
		puts(".do mso mdoc.tmac"); // BSD man
	if ( opt_use_man )
		puts(".do mso man.tmac"); // LNX man
	if ( opt_use_mom )
		puts(".do mso mom.tmac"); // mom

	if ( opt_use_mom ) {
		printf(".TITLE \"%s\"\n", docname);
		printf(".AUTHOR \"md2roff\"\n");
		printf(".PAPER A4\n");
		printf(".PRINTSTYLE TYPESET\n");
		printf(".START\n");
		}
	if ( opt_use_man || opt_use_mdoc ) {
		if ( *p == '#' && isspace(*(p+1)) ) {
			printf(".TH ");
			p = println(p+2);
			}
		else {
			time_t tt = time(0);   // get time now
			struct tm *t = localtime(&tt);
			printf(".TH %s 7 %d-%02d-%02d document\n",
				   docname,
				   t->tm_year+1900, t->tm_mon+1, t->tm_mday);
			}
		}

	dest = (char *) malloc(MD_BUF_SZ*1024);
	d = dest;
	while ( *p ) {

		//////////////////////////////////
		// inside code block
		//////////////////////////////////
		if ( bcode ) {
			d = flushln(d, dest); // we dont care
			
			if ( strncmp(p, "```", 3) == 0 ) { // end of code-block
				p += 3;
				bcode = false;
				roff(cblock_end);
				d = flushln(d, dest);
				continue;
				}
			else {
				bool xchg_dot = false;
				if ( *p == '.' ) {
					if ( opt_use_mom )
						puts(".ESC_CHAR !");
					else
						puts(".cc !");
					xchg_dot = true;
					}
				p = println(p);
				if ( xchg_dot ) {
					if ( opt_use_mom )
						puts(".ESC_CHAR .");
					else
						puts("!cc .");
					}
				continue;
				}
			}

		//////////////////////////////////
		// ignore escape characters
		if ( *p == '\\' ) {
			p ++;
			switch (*p) {
			case 'n': *d ++ = '\n'; break;
			case 'r': *d ++ = '\r'; break;
			case 't': *d ++ = '\t'; break;
			case 'f': *d ++ = '\f'; break;
			case 'b': *d ++ = '\b'; break;
			case 'a': *d ++ = '\a'; break;
			case 'e': *d ++ = '\033'; break;
			default:
				*d ++ = *p;
				}
			p ++;
			bline = false;
			continue;
			}
		
		//////////////////////////////////
		// beginning of line
		//////////////////////////////////
		if ( bline ) {
			bline = false;
			
			if ( *p == '\n' ) { // empty line
				d = flushln(d, dest);
				
				if ( stk_list_p ) {
					roff(li_end);
					roff(lst_close);
					stk_list_p --;
					}
				roff(par_end);
				bline = true;
				p ++;
				continue;
				}
			else if ( *p == '#' ) { // header
				d = flushln(d, dest);
				
				pnext = strchr(p, '\n');
				if ( pnext ) {
					if ( *(pnext-1) != '#' ) {
						int	level = 0;
						while ( *p == '#' ) { level ++; p ++; }
						while ( *p == ' ' || *p == '\t' ) p ++;
						switch ( level ) {
						case 1: roff(new_sh); break; // TH?
						case 2: roff(new_sh); break;
						case 3: roff(new_ss); break;
						case 4:
						default:
							if ( opt_use_man ) {
								printf(".TP\n\\fB");
								p = println(p);
								printf("\\fR");
								}
							else
								roff(new_s4);
							continue;
							}
						p = println(p);
						}
					else {
						roff(box_open);
						roff(ln_brk);
						p = println(p);
						roff(ln_brk);
						roff(box_close);
						continue;
						}
					}
				}
			else if ( (*(p+1) == ' ' || *(p+1) == '\t')
				&& (*p == '*' || *p == '+' || *p == '-') ) { // unordered list
				d = flushln(d, dest);
				if ( stk_list_p )
					roff(li_end);
				else
					roff(ul_open);
				roff(li_open);
				p ++;
				continue;
				}
			else if ( isdigit(*p) ) { // ordered list
				char	num[16], *n;
				const char *pstub = p;

				n = num;
				while ( isdigit(*p) )
					*n ++ = *p ++;
				*n = '\0';
				if ( *p == '.' ) {
					d = flushln(d, dest);
					if ( stk_list_p )
						roff(li_end);
					else
						roff(ol_open);
					stk_count[stk_list_p-1] = atoi(num);
					roff(li_open);
					p ++;
					while ( *p == ' ' || *p == '\t' ) p ++;
					continue;
					}
				p = pstub;
				}
			else if ( strncmp(p, "```", 3) == 0 ) { // open code-block
				bcode = true;
				p += 3;
				d = flushln(d, dest);
				roff(cblock_open);
				continue;
				}
			} // inside if ( beginning of line )

		//////////////////////////////////
		// in line
		//////////////////////////////////
		if ( *p == '\n' ) {
			if ( strncmp(p+1, "===", 3) == 0
				|| strncmp(p+1, "---", 3) == 0
				|| strncmp(p+1, "***", 3) == 0 ) {
				char rc = *(p+1);
				char	*prevln;
				p = strchr(p+1, '\n');
				if ( !p )
					return;
				if ( d == dest ) {
					p ++;
					continue;
					}

				// this is ruler or section
				*d = '\0';
				prevln = strrchr(dest, '\n');
				if ( prevln ) {
					*prevln = '\0';
					if ( prevln > dest )
						puts(dest);
					prevln ++;
					roff(new_sh);
					printf("%s\n", prevln);
					d = dest;
					}
				else {
					roff(new_sh);
					d = flushln(d, dest);
					}
				
				p ++;
				continue;
				}
			else
				*d ++ = ' ';

			bline = true;
			}
		else if (
			( *p == '*' && *(p+1) == '*' ) ||
			( *p == '_' && *(p+1) == '_' ) ) { // strong
			if ( bold ) {
				bold = false;
				if ( opt_use_mom )
					d = stradd(d, "\\*[PREV]");
				else
					d = stradd(d, "\\fP");
				}
			else {
				char pc = (p > source) ? *(p-1) : ' ';
				if ( strchr("({[,.;`'\" \t\n", pc) != NULL ) {
					bold = true;
					if ( opt_use_mom )
						d = stradd(d, "\\*[BD]");
					else
						d = stradd(d, "\\fB");
					}
				else {
					*d ++ = *p;
					*d ++ = *(p+1);
					}
				}
			p += 2;
			continue;
			}
		else if ( *p == '*' ||  *p == '_' ) { // emphasis
			if ( italics ) {
				italics = false;
				if ( opt_use_mom )
					d = stradd(d, "\\*[PREV]");
				else
					d = stradd(d, "\\fP");
				}
			else {
				char pc = (p > source) ? *(p-1) : ' ';
				if ( strchr("({[,.;`'\" \t\n", pc) != NULL ) {
					italics = true;
					if ( opt_use_mom )
						d = stradd(d, "\\*[IT]");
					else
						d = stradd(d, "\\fI");
					}
				else
					*d ++ = *p;
				}
			p ++;
			continue;
			}
		else if ( *p == '`' ) { // inline code
			p ++;
			if ( opt_use_mom )
				d = stradd(d, "`\\*[CODE]");
			else
				d = stradd(d, "`\\f[CR]");
			
			while ( *p != '`' ) {
				if ( *p == '\0' ) {
					fprintf(stderr, "%s", "inline code (`) didnt closed.");
					exit(EXIT_FAILURE);
					}
				*d ++ = *p ++;
				}

			if ( opt_use_mom )
				d = stradd(d, "\\*[CODE OFF]'");
			else
				d = stradd(d, "\\fP'");
			}

		//
		//	Markdown link:
		//
		//	generic link syntax  [text](link)
		//	image link syntax	![text](link)
		//	man page syntax      [page section](man)
		//
		else if ( *p == '[' || (*p == '!' && *(p+1) == '[') ) { // markdown link
			const char *pfin;
			bool bimg = false;
			if ( *p == '!' ) {
				p ++;
				bimg = true;
				}
			pstart = p + 1;
			pnext = strchr(pstart, ']');
			if ( pnext
					 && ( *(pnext+1) == '(' )
						 && ((pfin = strchr(pnext+2, ')')) != NULL)
			   ) {
				char *left = strdup(pstart);
				char *rght = strdup(pnext+2);
				char *cp;

				d = flushln(d, dest);
				
				cp = strchr(left, ']');	*cp = '\0';
				cp = strchr(rght, ')');	*cp = '\0';
				
//				if ( bimg ) // RTFM
				if ( strcmp(rght, "man") == 0 )
					roff(man_ref, left);
				else
					roff(url_mark, left, rght);
				
				// finish
				free(left);
				free(rght);
				p = pfin + 1;
				continue;
				}
			else {
				*d ++ = *p ++;
				continue;
				}
			}
		else {
			*d = *p;
			d ++;
			}

		p ++;
		}
	d = flushln(d, dest);

	free(dest);
}

/*
 * --- main() ---
 */
static char *usage ="\
usage: md2roff [options] [file1 .. [fileN]]\n\
\t-n, --man\n\t\tuse man package (default)\n\
\t-d, --mdoc\n\t\tuse mdoc package (BSD man-pages)\n\
\t-m, --mm\n\t\tuse mm package\n\
\t-o, --mom\n\t\tuse mom package\n\
\t-h, --help\n\t\tprint this screen\n\
\t-v, --version\n\t\tprint version information\n\
";

static char *version ="\
md2roff, version 1.0\n\
Copyright (C) 2017 Free Software Foundation, Inc.\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
";

int main(int argc, char *argv[])
{
	int files[64];
	int fc = 0;
	
	for ( int i = 1; i < argc; i ++ ) {
		if ( argv[i][0] == '-' ) {
			if ( argv[i][1] == '\0' ) { // read from stdin
				char *buf = loadfile(NULL);
				md2roff("stdin", buf);
				free(buf);
				}
			else if ( strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 )
				printf("%s", usage);
			else if ( strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0 )
				printf("%s", version);
			else if ( strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--man") == 0 ) {
				opt_use_mm = false;
				opt_use_man = true;
				opt_use_mdoc = false;
				opt_use_mom = false;
				}
			else if ( strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mm") == 0 ) {
				opt_use_mm = true;
				opt_use_man = false;
				opt_use_mdoc = false;
				opt_use_mom = false;
				}
			else if ( strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--mdoc") == 0 ) {
				opt_use_mm = false;
				opt_use_man = false;
				opt_use_mdoc = true;
				opt_use_mom = false;
				}
			else if ( strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--mom") == 0 ) {
				opt_use_mm = false;
				opt_use_man = false;
				opt_use_mdoc = false;
				opt_use_mom = true;
				}
			else
				fprintf(stderr, "unknown option: [%s]\n", argv[i]);
			}
		else {
			files[fc] = i;
			fc ++;
			}
		}
		
	for ( int i = 0; i < fc; i ++ ) {
		char *buf = loadfile(argv[files[i]]);
		md2roff(argv[files[i]], buf);
		free(buf);
		}

	return EXIT_SUCCESS;
}

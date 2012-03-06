/* ------------------------------------------------------------------------- */
/* bdf2fnt.c

   Convert X11 BDF font to MicroSoft .fnt format for inclusion in .fon file
   Copyright (C) Angus J. C. Duggan, 1995-1999

   Modified for variable-width fonts
   Copyright (C) 2009 grischka@users.sf.net

   Released under the terms of GNU General Public License
   (GPL) version 2 (See: http://www.fsf.org/licenses/gpl.html)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include "fontstruc.h"

#undef VGA_RESOLUTION

/* Define windows font versions */
#define WINDOWS_2       0x200
#define WINDOWS_3_0     0x300
#define WINDOWS_3_1     0x30a

static int verbose = 1 ;
static char *program ;

static void usage(void)
{
  fprintf(stderr,
    "bdf2fnt: Convert .bdf to .fnt for windows\n"
    "Copyright (C) Angus J. C. Duggan 1995-1999.\n"
    "\n"
    "Modified for variable-width fonts\n"
    "Copyright (C) 2009 grischka@users.sf.net\n"
    "\n"
    "Usage: bdf2fon [-q] [-c] [infile [outfile [fontname]]]\n"
    "\n"
    "Options:\n"
    " -q\t\tQuiet; do not print progress (not currently used)\n"
    " -c\t\tForce OEM (console) character set\n"
    "\n"
    "Files:\n"
    " infile\t\tName of input BDF file (stdin if none)\n"
    " outfile\tName of output FNT file (stdout if none)\n"
    "\n"
    "Source code is available from:\n"
    " http://bb4win.sourceforge.net/bblean/awiz.htm\n"
    "\n"
    "Original source code is available from:\n"
    " ftp.dcs.ed.ac.uk:/pub/ajcd/bdftofon.tar.gz and\n"
    " ftp.tardis.ed.ac.uk:/users/ajcd/bdftofon.tar.gz\n"
    "\n"
    );
  fflush(stderr);
  exit(1);
}

#define MAX_LINE 512

typedef struct {
  int xvec, yvec ;
  int bbox[4] ;
  int size ;
  unsigned int *bitmap ;
} FontChar ;

typedef struct {
  char *name ;
  char *xlfd[14] ;
  int bbox[4] ;
  int ascent ;
  int descent ;
  int pixels ;
  int defaultch ;
  int firstch ;
  int lastch ;
  int nchars ;
  int thischar ;
  int bmwidth ;
  char copyright[60];
  FontChar *chars[256+1] ;
} Font ;

int imin (int a, int b)
{
  return a < b ? a : b;
}

int imax (int a, int b)
{
  return a > b ? a : b;
}

static void *xalloc(size_t num, size_t size)
{
  void *mem ;

  if ( (mem = calloc(num, size)) == (void *)0 ) {
    fprintf(stderr, "%s: memory exhausted\n", program);
    fflush(stderr);
    exit(1);
  }
  return mem ;
}

static Font *newfont(void)
{
  Font *fnt = (Font *)xalloc(1, sizeof(Font)) ;

  fnt->ascent = -1 ;
  fnt->descent = -1 ;
  fnt->defaultch = -1 ;
  fnt->thischar = -1 ;
  fnt->lastch = -1 ;
  fnt->firstch = 256 ;
  fnt->nchars = 0 ;

  return fnt ;
}

int bdfignore(char *line, FILE *in, Font *fnt)
{
  return 1 ;
}

int bdffontbb(char *line, FILE *in, Font *fnt)
{
  return sscanf(line, "%d %d %d %d\n", &(fnt->bbox[0]),
                &(fnt->bbox[1]), &(fnt->bbox[2]), &(fnt->bbox[3])) == 4 ;
}

int bdffont(char *line, FILE *in, Font *fnt)
{
  char name[MAX_LINE] ;

  if ( sscanf(line, "%s\n", name) != 1 )
    return 0 ;

  if ( ! fnt->name ) {
    fnt->name = (char *)xalloc(strlen(line) + 1, sizeof(char)) ;
    strcpy(fnt->name, name) ;
  }

  if ( name[0] == '-' ) {       /* split out parts of XLFD */
    int index = 0 ;
    char *start = name ;
    char *end = start ;

    do {
      ++start ;
      do {
        ++end ;
      } while ( *end && *end != '-' ) ;
      fnt->xlfd[index] = (char *)xalloc(end - start + 1, sizeof(char)) ;
      strncpy(fnt->xlfd[index], start, end - start) ;
      fnt->xlfd[index][end - start] = '\0' ;
      start = end ;
    } while ( *end && ++index < 14 ) ;
  }

  return 1 ;
}

int bdfascent(char *line, FILE *in, Font *fnt)
{
  return sscanf(line, "%d\n", &(fnt->ascent)) == 1 ;
}

int bdfdescent(char *line, FILE *in, Font *fnt)
{
  return sscanf(line, "%d\n", &(fnt->descent)) == 1 ;
}

int bdfdefault(char *line, FILE *in, Font *fnt)
{
  return sscanf(line, "%d\n", &(fnt->defaultch)) == 1 ;
}

int bdfnchars(char *line, FILE *in, Font *fnt)
{
  return sscanf(line, "%d\n", &(fnt->nchars)) == 1 ;
}

int bdfpixels(char *line, FILE *in, Font *fnt)
{
  return sscanf(line, "%d\n", &(fnt->pixels)) == 1 ;
}

int bdfcopyright(char *line, FILE *in, Font *fnt)
{
  return 1 == sscanf(line, " \"%59[^\"]\"\n", fnt->copyright);
}

int bdfencode(char *line, FILE *in, Font *fnt)
{
  int thischar ;

  if ( sscanf(line, "%d\n", &thischar) != 1 )
    return 0 ;
  if (thischar > 255)
    return 0;
  fnt->thischar = thischar ;
  if ( thischar > fnt->lastch )
    fnt->lastch = thischar ;
  if ( thischar < fnt->firstch )
    fnt->firstch = thischar ;

  fnt->chars[thischar] = (FontChar *)xalloc(1, sizeof(FontChar)) ;

  return 1 ;
}

int bdfwidth(char *line, FILE *in, Font *fnt)
{
  FontChar *ch ;

  if ( fnt->thischar < 0 || (ch = fnt->chars[fnt->thischar]) == (FontChar *)0 )
    return 0 ;

  return sscanf(line, "%d %d\n", &(ch->xvec), &(ch->yvec)) == 2 ;
}

int bdfcharbb(char *line, FILE *in, Font *fnt)
{
  FontChar *ch ;
  int result ;

  if ( fnt->thischar < 0 || (ch = fnt->chars[fnt->thischar]) == (FontChar *)0 )
    return 0 ;

  result = (sscanf(line, "%d %d %d %d\n", &(ch->bbox[0]), &(ch->bbox[1]),
                   &(ch->bbox[2]), &(ch->bbox[3])) == 4) ;

  if ( result ) {
    if ( ch->bbox[0] > fnt->bbox[0] )
      fnt->bbox[0] = ch->bbox[0] ;
    if ( ch->bbox[1] > fnt->bbox[1] )
      fnt->bbox[1] = ch->bbox[1] ;
  }

  return result ;
}

int bdfbitmap(char *line, FILE *in, Font *fnt)
{
  FontChar *ch ;
  int bmwidth, bmheight ;
  unsigned int *row;
  char buf[MAX_LINE] ;

  if ( fnt->thischar < 0 || (ch = fnt->chars[fnt->thischar]) == (FontChar *)0 )
    return 0 ;

  //bmwidth = (ch->bbox[0] + imax(0, ch->bbox[2]) + 7) >> 3 ;
  bmwidth = (ch->xvec + 7) >> 3;
  bmheight = ch->bbox[1] ;

  if ( bmwidth > fnt->bmwidth )
    fnt->bmwidth = bmwidth ;

  ch->size = bmheight ;
  if (!bmheight)
    return 1;
  ch->bitmap = row = (unsigned int *)xalloc(ch->size, sizeof(unsigned int)) ;

  while ( bmheight-- ) {
    char *hex = buf ;
    unsigned int val = 0;
    int n = 8;

    if ( ! fgets(buf, MAX_LINE, in) )
      return 0 ;

    while (n--) {
      val <<= 4;
      switch ( *hex ) {
      case ' ': case '\t' : case '\n' : case '\r' : case '\0':
        break;
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        val += *hex++ - '0' ;
        break ;
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        val +=  *hex++ - 'a' + 10 ;
        break ;
      case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        val +=  *hex++ - 'A' + 10 ;
        break ;
      default:
        return 0 ;
      }
    }
    *row++ = val;
  }
  return 1 ;
}

struct {
  char *name ;
  int (*function)(char *, FILE *, Font *) ;
} dispatch[] = {
  { "STARTFONT", bdfignore },
  { "FONT", bdffont },
  { "SIZE", bdfignore },
  { "FONTBOUNDINGBOX", bdffontbb },
  { "STARTPROPERTIES", bdfignore },
  { "FONT_ASCENT", bdfascent },
  { "FONT_DESCENT", bdfdescent },
  { "PIXEL_SIZE", bdfpixels },
  { "DEFAULT_CHAR", bdfdefault },
  { "COPYRIGHT", bdfcopyright },
  { "ENDPROPERTIES", bdfignore },
  { "CHARS", bdfnchars },
  { "STARTCHAR", bdfignore },
  { "ENCODING", bdfencode },
  { "SWIDTH", bdfignore },
  { "DWIDTH", bdfwidth },
  { "BBX", bdfcharbb },
  { "BITMAP", bdfbitmap },
  { "ENDCHAR", bdfignore },
  { "ENDFONT", bdfignore },
  { (char *)0, bdfignore },
} ;

int readbdf(FILE *in, Font *fnt)
{
  static char line[MAX_LINE] ;
  while ( fgets(line, MAX_LINE, in) ) {
    int index, len ;
    char *eow ;

    for ( eow = line; *eow && ! isspace(*eow) ; eow++ ) ;
    len = eow - line ;

    for ( index = 0 ; dispatch[index].name ; index++ ) {
      if ( strlen(dispatch[index].name) == len &&
           strncmp(dispatch[index].name, line, len) == 0 ) {
        if ( ! (*(dispatch[index].function))(eow, in, fnt) ) {
          fprintf(stderr, "%s: can't parse line %s\n", program, line);
          fflush(stderr);
          exit(1);
        }
        break ;
      }
    }
    
  }
  return 1 ;
}

struct writefntopt {
  int oem ;     /* Force oem charset? */
} ;

/* ------------------------------------------------------------------------- */

int writefnt(FILE *out, Font *fnt, int version, char *name, struct writefntopt *options)
{
  FONTFILEHEADER *fhead = (FONTFILEHEADER *)xalloc(1, sizeof(FONTFILEHEADER)) ;
  FONTINFO *finfo = &(fhead->dffi) ;
  long rastersz = 0 ;
  long headersz = 0 ;
  long tablesz = 0 ;
  char *xlfd ;
  int maxwidth = 0 ;
  int avgwidth = 0 ;
  int totwidth = 0 ;
  int samewidth = 1 ;
  int i, f, w, h, rs;
  FontChar *ch;
/*
  unsigned db[] = { 0xC0000000, 0xC0000000, 0xC0000000, 0xC0000000,
                    0xC0000000, 0xC0000000, 0xC0000000, 0xC0000000 };
  FontChar dc = { 3, 0, {2, 8, 1, -1}, 8, db };
*/

  f = 129;

  i = fnt->defaultch + fnt->firstch;
  if ( i < fnt->firstch || i > fnt->lastch)
      i = '?';
  fnt->chars[f] = fnt->chars[i];

  //fnt->chars[f] = &dc;

  fnt->defaultch = f - fnt->firstch;
  if (f > fnt->lastch)
      fnt->lastch = f;

  //fnt->lastch = 255;

  fnt->chars[fnt->lastch + 1] = fnt->chars[32];
  fnt->nchars = fnt->lastch + 1 - fnt->firstch;

  w = fnt->bmwidth;
  h = fnt->bbox[1];
  rs = w * h;

  /* Fill in gaps from first to last character, and calculate raster size */
  for ( i = fnt->firstch ; i <= fnt->lastch ; i++ ) {
    int width;
    ch = fnt->chars[i];
    if ( ch == NULL )
      fnt->chars[i] = ch = fnt->chars[fnt->firstch + fnt->defaultch] ;

    width = ch->xvec ;
    if (maxwidth && width != maxwidth)
        samewidth = 0 ;
    maxwidth = imax(width, maxwidth);
    totwidth += width ;
  }
  avgwidth = totwidth / fnt->nchars ;

  if ( name == NULL && fnt->xlfd[1] && *(fnt->xlfd[1]) ) 
    name = fnt->xlfd[1] ;
  if ( name == NULL ) {
    fprintf(stderr, "no font name\n");
    return 0;
  }

  printf("%s: %d/%d\n", name, avgwidth, h);

  headersz = (char *)&(finfo->dfFlags) - (char *)fhead;
  tablesz = (fnt->nchars + 1) * sizeof(RASTERGLYPHENTRY) ;
  rastersz = (fnt->nchars + 1) * rs ;

  fhead->dfVersion = version ;
  fhead->dfSize = headersz + tablesz + rastersz + 
    strlen(name) + 1 ; /* size of entire file in bytes */
  strcpy(fhead->dfCopyright, 
    fnt->copyright[0] 
    ? fnt->copyright
    : "Converted by bd2fnt, (C) AJCD 1995 (C) 2009 grischka") ;
  finfo->dfType = PF_RASTER_TYPE ;
  finfo->dfPoints = fnt->xlfd[6] ? atoi(fnt->xlfd[6]) : fnt->ascent ;   /* well, it's near enough */
#ifdef VGA_RESOLUTION
  finfo->dfVertRes = 96 ; /* Standard VGA */
  finfo->dfHorizRes = 96 ; /* Standard VGA */
#else
  finfo->dfVertRes = fnt->xlfd[8] ? atoi(fnt->xlfd[8]) : fnt->xlfd[9] ? atoi(fnt->xlfd[9]) : 96 ;
  finfo->dfHorizRes = fnt->xlfd[9] ? atoi(fnt->xlfd[9]) : fnt->xlfd[8] ? atoi(fnt->xlfd[8]) : 96 ;
#endif
  finfo->dfAscent = fnt->ascent ;
  finfo->dfInternalLeading = 1 ;
  finfo->dfExternalLeading = 0 ;
  finfo->dfItalic = (xlfd = fnt->xlfd[3]) &&
    (strcmp(xlfd, "i") == 0 || strcmp(xlfd, "o") == 0) ;
  finfo->dfUnderline = 0 ;
  finfo->dfStrikeOut = 0 ;
  finfo->dfWeight = (xlfd = fnt->xlfd[2]) ?
    (strcmp(xlfd, "medium") == 0 ? 400 :
     strcmp(xlfd, "bold") == 0 ? 700 :
     strcmp(xlfd, "light") == 0 ? 200 : 400) : 400 ;
  finfo->dfCharSet = !options->oem &&
    (xlfd = fnt->xlfd[12]) && strcmp(xlfd, "iso8859") == 0 ?
    DF_CHARSET_ANSI : DF_CHARSET_OEM ;
  finfo->dfPixWidth = 0;
  finfo->dfPixHeight = h;
  finfo->dfPitchAndFamily = samewidth ? FF_MODERN : FF_SWISS | FF_VARIABLE ;
  finfo->dfAvgWidth = avgwidth ;
  finfo->dfMaxWidth = maxwidth ;
  finfo->dfFirstChar = fnt->firstch ;
  finfo->dfLastChar = fnt->lastch ;
  finfo->dfDefaultChar = fnt->defaultch ;
  finfo->dfBreakChar = 0 ; /* relative to firstchar */
  finfo->dfWidthBytes = fnt->bmwidth * fnt->nchars ; /* gr: ??? */
  finfo->dfDevice = 0 ;
  finfo->dfFace = headersz + tablesz + rastersz ; /* offset to face name */
  finfo->dfBitsPointer = 0 ;
  finfo->dfBitsOffset = headersz + tablesz ; /* offset to bitmap */
  finfo->dfReserved = 0xFF;

  /* write font header struct */
  if ( fwrite((void *)fhead, headersz, 1, out) < 1 )
    return 0 ;

  /* char width table */
  {
    short offset = (short)(headersz + tablesz) ;
    for ( i = fnt->firstch ; i <= fnt->lastch + 1 ; i++ ) {
      RASTERGLYPHENTRY glyph ;
      ch = fnt->chars[i] ;
      if (ch) {
        glyph.rgeWidth = ch->xvec;
        glyph.rgeOffset = offset ;
        offset += rs;
      } else {
        glyph.rgeWidth = 0;
        glyph.rgeOffset = 0;
      }
      if ( fwrite((void *)&glyph, sizeof(RASTERGLYPHENTRY), 1, out) < 1 )
        return 0 ;
    }
  }

  /* write bitmap data */
  for ( i = fnt->firstch ; i <= fnt->lastch + 1 ; i++ ) {
    char tmp[MAX_LINE];
    int r, s, c, v_offs, h_offs;
    unsigned b, *p;

    ch = fnt->chars[i] ;
    if (ch) {
        p = ch->bitmap;
        s = ch->size;
        v_offs = imax(0, (fnt->bbox[1] + fnt->bbox[3]) - (ch->bbox[1] + ch->bbox[3]));
        h_offs = imax(0, ch->bbox[2]);

        memset(tmp, 0, rs);
        for (r = 0; r < s; ++r) {
          b = *p++ >> h_offs;
          for (c = 0; c < w; ++c) {
            tmp[r + v_offs + c*h] = (b >> 8*(3-c)) & 255;
          }
        }
        fwrite(tmp, rs, 1, out);
    }
  }

  /* write face name */
  if ( fwrite((void *)name, strlen(name) + 1, 1, out) < 1 )
    return 0 ;

  (void)free(fhead) ;

  return 1 ;
}

/* ------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  FILE *infile = stdin;
  FILE *outfile = stdout;
  Font *thisfont = newfont() ;
  char *name = NULL ;
  int version = WINDOWS_2 ;
  struct writefntopt woptions = { 0 } ;

  if (argc <= 1) {
      usage();
      exit(1);
  }

  for (program = *argv++ ; --argc ; argv++) {
    if (argv[0][0] == '-') {
      switch (argv[0][1]) {
      case 'q': /* quiet */
        verbose = 0;
        break;
      case 'c': /* OEM (console) charset */
        woptions.oem = 1 ;
        break;
      case '2': /* windows 2.0 */
        if ( argv[0][2] == '\0' || strcmp(argv[0], "-2.0") == 0 )
          version = WINDOWS_2 ;
        else
          usage() ;
        break;
      case '3': /* windows 3.0 */
        if ( argv[0][2] == '\0' || strcmp(argv[0], "-3.0") == 0 )
          version = WINDOWS_3_0 ;
        else if ( strcmp(argv[0], "-3.1") == 0 )
          version = WINDOWS_3_1 ;
        else
          usage() ;
        break;
      case 's':
        if (!argc--) {
          usage();
        }
        printf("char %c = %d\n", argv[1][0], argv[1][0] & 255);
        exit(0);
      default:
        usage();
      }
    } else if (infile == stdin) {
      if ((infile = fopen(*argv, "r")) == NULL) {
        fprintf(stderr, "%s: can't open input file %s\n", program, *argv);
        fflush(stderr);
        exit(1);
      }
    } else if (outfile == stdout) {
      if ((outfile = fopen(*argv, "wb")) == NULL) {
        fprintf(stderr, "%s: can't open output file %s\n", program, *argv);
        fflush(stderr);
        exit(1);
      }
    } else if (name == NULL) {
      name = *argv ;
    } else usage();
  }
  if ( outfile == stdout ) {
    int fd = fileno(stdout) ;
    if ( setmode(fd, O_BINARY) < 0 ) {
      fprintf(stderr, "%s: can't reset stdout to binary mode\n", program);
      exit(1);
    }
  }

  if ( ! readbdf(infile, thisfont) ) {
    fprintf(stderr, "%s: problem reading BDF font file\n", program);
    exit(1);
  }

  if ( ! writefnt(outfile, thisfont, version, name, &woptions) ) {
    fprintf(stderr, "%s: problem writing FON font file\n", program);
    exit(1);
  }

  fclose(stdin) ;
  fclose(stdout) ;
  return 0;
}

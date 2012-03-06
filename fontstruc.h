/*
 * fontstruc.h
 */

typedef unsigned char CHAR;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;
typedef short SHORT;
typedef short INT16;
typedef WORD HANDLE16;
typedef DWORD FARPROC16;

/*
 * NE Header FORMAT FLAGS
 */
#define NE_FFLAGS_SINGLEDATA    0x0001
#define NE_FFLAGS_MULTIPLEDATA  0x0002
#define NE_FFLAGS_WIN32         0x0010
#define NE_FFLAGS_BUILTIN       0x0020  /* Wine built-in module */
#define NE_FFLAGS_FRAMEBUF      0x0100  /* OS/2 fullscreen app */
#define NE_FFLAGS_CONSOLE       0x0200  /* OS/2 console app */
#define NE_FFLAGS_GUI           0x0300  /* right, (NE_FFLAGS_FRAMEBUF | NE_FFLAGS_CONSOLE) */
#define NE_FFLAGS_SELFLOAD      0x0800
#define NE_FFLAGS_LINKERROR     0x2000
#define NE_FFLAGS_CALLWEP       0x4000
#define NE_FFLAGS_LIBMODULE     0x8000

/*
 * NE Header OPERATING SYSTEM
 */
#define NE_OSFLAGS_UNKNOWN      0x01
#define NE_OSFLAGS_WINDOWS      0x04

/*
 * NE Header ADDITIONAL FLAGS
 */
#define NE_AFLAGS_WIN2_PROTMODE 0x02
#define NE_AFLAGS_WIN2_PROFONTS 0x04
#define NE_AFLAGS_FASTLOAD      0x08

/*
 * Segment Flags
 */
#define NE_SEGFLAGS_DATA        0x0001
#define NE_SEGFLAGS_ALLOCATED   0x0002
#define NE_SEGFLAGS_LOADED      0x0004
#define NE_SEGFLAGS_ITERATED    0x0008
#define NE_SEGFLAGS_MOVEABLE    0x0010
#define NE_SEGFLAGS_SHAREABLE   0x0020
#define NE_SEGFLAGS_PRELOAD     0x0040
#define NE_SEGFLAGS_EXECUTEONLY 0x0080
#define NE_SEGFLAGS_READONLY    0x0080
#define NE_SEGFLAGS_RELOC_DATA  0x0100
#define NE_SEGFLAGS_SELFLOAD    0x0800
#define NE_SEGFLAGS_DISCARDABLE 0x1000
#define NE_SEGFLAGS_32BIT       0x2000

/*
 * Resource Types
 */
#define NE_RSCTYPE_CURSOR             0x8001
#define NE_RSCTYPE_BITMAP             0x8002
#define NE_RSCTYPE_ICON               0x8003
#define NE_RSCTYPE_MENU               0x8004
#define NE_RSCTYPE_DIALOG             0x8005
#define NE_RSCTYPE_STRING             0x8006
#define NE_RSCTYPE_FONTDIR            0x8007
#define NE_RSCTYPE_FONT               0x8008
#define NE_RSCTYPE_ACCELERATOR        0x8009
#define NE_RSCTYPE_RCDATA             0x800a
#define NE_RSCTYPE_GROUP_CURSOR       0x800c
#define NE_RSCTYPE_GROUP_ICON         0x800e
#define NE_RSCTYPE_SCALABLE_FONTPATH  0x80cc   /* Resource found in .fot files */

#pragma pack(push,2)

typedef struct _IMAGE_DOS_HEADER {
        WORD e_magic;
        WORD e_cblp;
        WORD e_cp;
        WORD e_crlc;
        WORD e_cparhdr;
        WORD e_minalloc;
        WORD e_maxalloc;
        WORD e_ss;
        WORD e_sp;
        WORD e_csum;
        WORD e_ip;
        WORD e_cs;
        WORD e_lfarlc;
        WORD e_ovno;
        WORD e_res[4];
        WORD e_oemid;
        WORD e_oeminfo;
        WORD e_res2[10];
        LONG e_lfanew;
} IMAGE_DOS_HEADER,*PIMAGE_DOS_HEADER;

typedef struct _IMAGE_OS2_HEADER {
        WORD ne_magic;
        CHAR ne_ver;
        CHAR ne_rev;
        WORD ne_enttab;
        WORD ne_cbenttab;
        LONG ne_crc;
        WORD ne_flags;
        WORD ne_autodata;
        WORD ne_heap;
        WORD ne_stack;
        LONG ne_csip;
        LONG ne_sssp;
        WORD ne_cseg;
        WORD ne_cmod;
        WORD ne_cbnrestab;
        WORD ne_segtab;
        WORD ne_rsrctab;
        WORD ne_restab;
        WORD ne_modtab;
        WORD ne_imptab;
        LONG ne_nrestab;
        WORD ne_cmovent;
        WORD ne_align;
        WORD ne_cres;
        BYTE ne_exetyp;
        BYTE ne_flagsothers;
        WORD ne_pretthunks;
        WORD ne_psegrefbytes;
        WORD ne_swaparea;
        WORD ne_expver;
} IMAGE_OS2_HEADER,*PIMAGE_OS2_HEADER;

/*
 * Resource table structures.
 */
typedef struct
{
    WORD     offset;
    WORD     length;
    WORD     flags;
    WORD     id;
    HANDLE16 handle;
    WORD     usage;
} NE_NAMEINFO;

typedef struct
{
    WORD        type_id;   /* Type identifier */
    WORD        count;     /* Number of resources of this type */
    FARPROC16   resloader; /* SetResourceHandler() */
    /*
     * Name info array.
     */
} NE_TYPEINFO;

#pragma pack(pop)

#pragma pack(push,1)

typedef struct
{
    INT16 dfType;
    INT16 dfPoints;
    INT16 dfVertRes;
    INT16 dfHorizRes;
    INT16 dfAscent;
    INT16 dfInternalLeading;
    INT16 dfExternalLeading;
    CHAR  dfItalic;
    CHAR  dfUnderline;
    CHAR  dfStrikeOut;
    INT16 dfWeight;
    BYTE  dfCharSet;
    INT16 dfPixWidth;
    INT16 dfPixHeight;
    CHAR  dfPitchAndFamily;
    INT16 dfAvgWidth;
    INT16 dfMaxWidth;
    CHAR  dfFirstChar;
    CHAR  dfLastChar;
    CHAR  dfDefaultChar;
    CHAR  dfBreakChar;
    INT16 dfWidthBytes;
    LONG  dfDevice;
    LONG  dfFace;
    LONG  dfBitsPointer;
    LONG  dfBitsOffset;
    CHAR  dfReserved;
    /* Fields, introduced for Windows 3.x fonts */
    LONG  dfFlags;
    INT16 dfAspace;
    INT16 dfBspace;
    INT16 dfCspace;
    LONG  dfColorPointer;
    LONG  dfReserved1[4];
} FONTINFO, *LPFONTINFO;

typedef struct tagVECTORGLYPHENTRY {
    short rgeOffset;   /* offset to vectors relative to segment start */
    short rgeWidth;    /* width of character in pixels */
} VECTORGLYPHENTRY;

typedef struct tagFONTFILEHEADER {
    short dfVersion;
    long  dfSize;
    char  dfCopyright[60];
    FONTINFO dffi;
} FONTFILEHEADER;

typedef struct tabRASTERGLYPHENTRY {
  short rgeWidth;
  short rgeOffset;
} RASTERGLYPHENTRY ;

typedef struct tabRASTERGLYPHENTRY3 {
  short rgeWidth;
  long rgeOffset;
} RASTERGLYPHENTRY3 ;

#pragma pack(pop)

#define PF_RASTER_TYPE (0x0000)
#define PF_VECTOR_TYPE (0x0001)
#define PF_BITS_IS_ADDRESS (0x0004)
#define PF_DEVICE_REALIZED (0x0080)

#define DF_CHARSET_ANSI 0
#define DF_CHARSET_SYMBOL 2
#define DF_CHARSET_OEM 255

#define FF_VARIABLE 0x01
#ifndef FF_ROMAN
#define FF_ROMAN (0x10)
#define FF_SWISS (0x20)
#define FF_MODERN (0x30)
#define FF_SCRIPT (0x40)
#define FF_DECORATIVE (0x50)
#endif
#define FSF_FIXED (0x0001)
#define FSF_PROPORTIONAL (0x0002)
#define FSF_ABCFIXED (0x0004)
#define FSF_ABCPROPORTIONAL (0x0008)
#define FSF_1COLOR (0x0010)
#define FSF_16COLOR (0x0020)
#define FSF_256COLOR (0x0040)
#define FSF_RGBCOLOR (0x0080)

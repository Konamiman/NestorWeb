#ifndef __MSXDOS_H
#define __MSXDOS_H

//MSX-DOS 2 function calls

#define F_TERM0   0x00
#define F_CONIN   0x01
#define F_CONOUT  0x02
#define F_AUXIN   0x03
#define F_AUXOUT  0x04
#define F_LSTOUT  0x05
#define F_DIRIO   0x06
#define F_DIRIN   0x07
#define F_INNOE   0x08
#define F_STROUT  0x09
#define F_BUFIN   0x0A
#define F_CONST   0x0B
#define F_CPMVER  0x0C
#define F_DSKRST  0x0D
#define F_SELDSK  0x0E
#define F_FOPEN   0x0F
#define F_FCLOSE  0x10
#define F_SFIRST  0x11
#define F_SNEXT   0x12
#define F_FDEL    0x13
#define F_RDSEQ   0x14
#define F_WRSEQ   0x15
#define F_FMAKE   0x16
#define F_FREN    0x17
#define F_LOGIN   0x18
#define F_CURDRV  0x19
#define F_SETDTA  0x1A
#define F_ALLOC   0x1B
#define F_RDRND   0x21
#define F_WRRND   0x22
#define F_FSIZE   0x23
#define F_SETRND  0x24
#define F_WRBLK   0x26
#define F_RDBLK   0x27
#define F_WRZER   0x28
#define F_GDATE   0x2A
#define F_SDATE   0x2B
#define F_GTIME   0x2C
#define F_STIME   0x2D
#define F_VERIFY  0x2E
#define F_RDABS   0x2F
#define F_WRABS   0x30
#define F_DPARM   0x31
#define F_FFIRST  0x40
#define F_FNEXT   0x41
#define F_FNEW    0x42
#define F_OPEN    0x43
#define F_CREATE  0x44
#define F_CLOSE   0x45
#define F_ENSURE  0x46
#define F_DUP     0x47
#define F_READ    0x48
#define F_WRITE   0x49
#define F_SEEK    0x4A
#define F_IOCTL   0x4B
#define F_HTEST   0x4C
#define F_DELETE  0x4D
#define F_RENAME  0x4E
#define F_MOVE    0x4F
#define F_ATTR    0x50
#define F_FTIME   0x51
#define F_HDELETE 0x52
#define F_HRENAME 0x53
#define F_HMOVE   0x54
#define F_HATTR   0x55
#define F_HFTIME  0x56
#define F_GETDTA  0x57
#define F_GETVFY  0x58
#define F_GETCD   0x59
#define F_CHDIR   0x5A
#define F_PARSE   0x5B
#define F_PFILE   0x5C
#define F_CHKCHR  0x5D
#define F_WPATH   0x5E
#define F_FLUSH   0x5F
#define F_FORK    0x60
#define F_JOIN    0x61
#define F_TERM    0x62
#define F_DEFAB   0x63
#define F_DEFER   0x64
#define F_ERROR   0x65
#define F_EXPLAIN 0x66
#define F_FORMAT  0x67
#define F_RAMD    0x68
#define F_BUFFER  0x69
#define F_ASSIGN  0x6A
#define F_GENV    0x6B
#define F_SENV    0x6C
#define F_FENV    0x6D
#define F_DSKCHK  0x6E
#define F_DOSVER  0x6F
#define F_REDIR   0x70
#define F_RDDRV   0x73
#define F_WRDRV   0x74
#define F_RALLOC  0x75
#define F_DSPACE  0x76
#define F_LOCK    0x77
#define F_GDLI    0x79
#define F_GPART   0x7A
#define F_Z80MODE 0x7D

//MSX-DOS 2 error codes

#define ERR_NCOMP  0xFF
#define ERR_WRERR  0xFE
#define ERR_DISK   0xFD
#define ERR_NRDY   0xFC
#define ERR_VERFY  0xFB
#define ERR_DATA   0xFA
#define ERR_RNF    0xF9
#define ERR_WPROT  0xF8
#define ERR_UFORM  0xF7
#define ERR_NDOS   0xF6
#define ERR_WDISK  0xF5
#define ERR_WFILE  0xF4
#define ERR_SEEK   0xF3
#define ERR_IFAT   0xF2
#define ERR_NOUPB  0xF1
#define ERR_IFORM  0xF0
#define ERR_INTER  0xDF
#define ERR_NORAM  0xDE
#define ERR_IBDOS  0xDC
#define ERR_IDRV   0xDB
#define ERR_IFNM   0xDA
#define ERR_IPATH  0xD9
#define ERR_PLONG  0xD8
#define ERR_NOFIL  0xD7
#define ERR_NODIR  0xD6
#define ERR_DRFUL  0xD5
#define ERR_DKFUL  0xD4
#define ERR_DUPF   0xD3
#define ERR_DIRE   0xD2
#define ERR_FILRO  0xD1
#define ERR_DIRNE  0xD0
#define ERR_IATTR  0xCF
#define ERR_DOT    0xCE
#define ERR_SYSX   0xCD
#define ERR_DIRX   0xCC
#define ERR_FILEX  0xCB
#define ERR_FOPEN  0xCA
#define ERR_OV64K  0xC9
#define ERR_FILE   0xC8
#define ERR_EOF    0xC7
#define ERR_ACCV   0xC6
#define ERR_IPROC  0xC5
#define ERR_NHAND  0xC4
#define ERR_IHAND  0xC3
#define ERR_NOPEN  0xC2
#define ERR_IDEV   0xC1
#define ERR_IENV   0xC0
#define ERR_ELONG  0xBF
#define ERR_IDATE  0xBE
#define ERR_ITIME  0xBD
#define ERR_RAMDX  0xBC
#define ERR_NRAMD  0xBB
#define ERR_HDEAD  0xBA
#define ERR_EOL    0xB9
#define ERR_ISBFN  0xB8
#define ERR_IFCB   0xB7
#define ERR_IDRVR  0xB6
#define ERR_IDEVL  0xB5
#define ERR_IPART  0xB4
#define ERR_PUSED  0xB3
#define ERR_FMNT   0xB2
#define ERR_STOP   0x9F
#define ERR_CTRLC  0x9E
#define ERR_ABORT  0x9D
#define ERR_OUTERR 0x9C
#define ERR_INERR  0x9B
#define ERR_BADCOM 0x8F
#define ERR_BADCMD 0x8E
#define ERR_BUFUL  0x8D
#define ERR_OKCMD  0x8C
#define ERR_IPARM  0x8B
#define ERR_INP    0x8A
#define ERR_NOPAR  0x89
#define ERR_IOPT   0x88
#define ERR_BADNO  0x87
#define ERR_NOHELP 0x86
#define ERR_BADVER 0x85
#define ERR_NOCAT  0x84
#define ERR_BADEST 0x83
#define ERR_COPY   0x82
#define ERR_OVDEST 0x81
#define ERR_BATEND 0x80
#define ERR_INSDSK 0x7F
#define ERR_PRAK   0x7E

// Disk errors are those that trigger an "Abort, Retry, Ignore?" prompt by default
#define MIN_DISK_ERROR_CODE 0xF0

// File attributes
#define FILE_ATTR_DIRECTORY (1 << 4)

//Open file flags
#define FILE_OPEN_NO_WRITE (1 << 0)
#define FILE_OPEN_NO_READ (1 << 1)
#define FILE_OPEN_INHERITABLE (1 << 2)


// Data structures

typedef struct {
    byte alwaysFF;
    char filename[13];
    byte attributes;
    uint timeOfModification;
    uint dateOfModification;
    unsigned int startCluster;
    unsigned long fileSize;
    byte logicalDrive;
    byte internal[38];
} fileInfoBlock;

#endif

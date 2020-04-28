/*************** type.h file ************************/
#ifndef _TYPEH_
#define _TYPEH_
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER* sp;
GD* gp;
INODE* ip;
DIR* dp;

// Block number of EXT2 FS on FD
#define SUPERBLOCK	1
#define GDBLOCK		2
#define ROOT_INODE	2

#define FREE        0
#define READY       1

#define BLKSIZE  1024
#define NMINODE   128
#define NMTABLE	   10
#define NMOUNT	   10
#define NFD        16
#define NPROC       2

#define OWNER  000700
#define GROUP  000070
#define OTHER  000007

typedef struct minode {
	INODE INODE;
	int dev, ino;
	int refCount;
	int dirty;
	int mounted;
	struct mntable* mptr;
}MINODE;

typedef struct oft {
	int  mode;
	int  refCount;
	MINODE* mptr;
	int  offset;
}OFT;

typedef struct proc {
	struct proc* next;
	int          pid;
	int          status;
	int          uid, gid;
	MINODE* cwd;
	OFT* fd[NFD];
}PROC;

// Mount Table structure
typedef struct mtable {
	int dev; // device number; 0 for FREE
	int ninodes; // from superblock
	int nblocks;
	int free_blocks; // from superblock and GD
	int free_inodes;
	int bmap; // from group descriptor
	int imap;
	int iblock; // inodes start block
	MINODE* mntDirPtr; // mount point DIR pointer
	char devName[64]; //device name
	char mntName[64]; // mount point DIR name
}MTABLE;
#endif

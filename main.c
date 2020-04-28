/****************************************************************************
*                   KCW  Implement ext2 file system                         *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>

#include <sys/stat.h>
#include <sys/types.h>


#include <errno.h>
#include <time.h>

#include "type.h"

// global variables
MINODE minode[NMINODE];
MTABLE mp[NMTABLE];
MINODE* root;
SUPER* sp;

PROC   proc[NPROC], * running;

char gpath[128]; // global for tokenized components
char* name[32];  // assume at most 32 components in pathname
int   n;         // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start; // disk parameters
char buf[BLKSIZE];

#include "util.c"
#include "cd_ls_pwd.c"
#include "mkdir_creat.c"
#include "rmdir.c"
#include "link_unlink.c"
#include "symlink.c"
#include "open_close_lseek.c"
#include "read_cat.c"
#include "write_cp.c"
#include "mount_umount.c"
#include "access_macess.c"

int init()
{
	int i, j;
	MINODE* mip;
	MTABLE* mtab;
	PROC* p;

	printf("init()\n");

	running = malloc(sizeof(PROC));

	for (i = 0; i < NMINODE; i++) {
		mip = &minode[i];
		mip->dev = mip->ino = 0;
		mip->refCount = 0;
		mip->mounted = 0;
		mip->mptr = 0;
	}

	for (i = 0; i < NPROC; i++) {
		p = &proc[i];
		p->pid = i; // pid = 0 to NPROC-1
		p->uid = i; // P0 is a superuser process
		p->cwd = 0;
		p->status = FREE;
		for (j = 0; j < NFD; j++)
			p->fd[j] = 0; // all file descriptors are NULL
	}
	proc[NPROC - 1].next = &proc[0]; // circular list

	for (i = 0; i < NMTABLE; i++) { // initialize mtables as FREE
		mtab = &mp[i];
		mtab->dev = 0;
		mtab->ninodes = 0; // from superblock
		mtab->nblocks = 0;
		mtab->free_blocks = 0; // from superblock and GD
		mtab->free_inodes = 0;
		mtab->bmap = 0; // from group descriptor
		mtab->imap = 0;
		mtab->iblock = 0;
	}

	printf("init done\n");
}

// load root INODE and set root pointer to it
int mount_root(char *disk)
{
	int user;
	char line[128], userline[128];

	printf("mount_root()\n");
	printf("checking EXT2 FS ....");
	if ((fd = open(disk, O_RDWR)) < 0) {
		printf("open %s failed\n", disk);
		exit(1);
	}
	dev = fd;    // fd is the global dev 

	/********** read super block  ****************/
	get_block(dev, 1, buf);
	sp = (SUPER*)buf;

	/* verify it's an ext2 file system ***********/
	if (sp->s_magic != 0xEF53) {
		printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
		exit(1);
	}
	printf("EXT2 FS OK\n");
	// fill mount table mtable[0] with rootdev information
	mp->dev = dev;
	// copy super block info into mtable[0]
	ninodes = mp->ninodes = sp->s_inodes_count;
	nblocks = mp->nblocks = sp->s_blocks_count;
	get_block(dev, 2, buf);
	gp = (GD*)buf;

	printf("test\n");
	bmap = mp->bmap = gp->bg_block_bitmap;
	imap = mp->imap = gp->bg_inode_bitmap;
	inode_start = gp->bg_inode_table;
	strcpy(mp->devName, disk);
	strcpy(mp->mntName, "/");
	printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);
	root = iget(dev, 2);
	mp->mntDirPtr = root; // double link

	//printf("running on proc[0] or proc[1]? (0 / 1)\n");
	//fgets(userline, 128, stdin);
	//userline[strlen(userline) - 1] = 0;
	//if (userline[0] == 0)
	//	continue;
	//sscanf(userline, "%d", user);

	//if (user == 0) {
		printf("creating P0 as running process\n");
		running = &proc[0];
		running->status = READY;
		running->cwd = iget(dev, 2);
	//}

	//else if (user == 1) {
	//	printf("creating P1 as running process\n");
	//	running = &proc[1];
	//	running->status = READY;
	//	running->cwd = iget(dev, 2);
	//}

	printf("mount : %s mounted on / \n", disk);
	return 0;
}

int main(int argc, char* argv[])
{
	int ino, user;
	char line[128], cmd[32], pathname[128], pathname2[128];

	init();
	mount_root(argv[1]);
	printf("root refCount = %d\n", root->refCount);	

	// WRTIE code here to create P1 as a USER process
	while (1) {
		printf("input command : [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink|open|close|lseek|read|write|cat|cp|mv|pfd|mount|umount|quit] ");
		fgets(line, 128, stdin);
		line[strlen(line) - 1] = 0;

		if (line[0] == 0)
			continue;
		pathname[0] = 0;

		sscanf(line, "%s %s %s", cmd, pathname, pathname2);

		if (strcmp(cmd, "ls") == 0)
			ls(pathname);
		else if (strcmp(cmd, "cd") == 0)
			chdir(pathname);
		else if (strcmp(cmd, "pwd") == 0)
			pwd(running->cwd);
		else if (strcmp(cmd, "mkdir") == 0)
			make_dir(pathname);
		else if (strcmp(cmd, "creat") == 0)
			creat_file(pathname);
		else if (strcmp(cmd, "rmdir") == 0)
			rmdir(pathname);
		else if (strcmp(cmd, "link") == 0)
			link(pathname, pathname2);
		else if (strcmp(cmd, "unlink") == 0)
			unlink(pathname);
		else if (strcmp(cmd, "symlink") == 0)
			symlink(pathname, pathname2);
		else if (strcmp(cmd, "open") == 0)
			open_file(pathname, pathname2);
		else if (strcmp(cmd, "close") == 0)
			close_file(pathname);
		else if (strcmp(cmd, "lseek") == 0)
			mylseek(pathname, pathname2);
		else if (strcmp(cmd, "read") == 0)
			read_file(pathname, pathname2);
		else if (strcmp(cmd, "write") == 0)
			write_file(pathname);
		else if (strcmp(cmd, "cat") == 0)
			mycat(pathname);
		else if (strcmp(cmd, "cp") == 0)
			cp_file(pathname, pathname2);
		else if (strcmp(cmd, "pfd") == 0)
			pfd();
		else if (strcmp(cmd, "mount") == 0)
			mount(pathname, pathname2);
		else if (strcmp(cmd, "umount") == 0)
			umount(pathname);
		else if (strcmp(cmd, "quit") == 0)
			quit();
	}
}

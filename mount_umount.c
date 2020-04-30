// mount <filesys> <mount_point>
// mounts a filesystem to a mount_point directory
// allows the file system to include other file sys as parts of the existing file sys
int mount(char* filesys, char* mount_point)    /*  Usage: mount filesys mount_point OR mount */
{
	int ino, dv, fd, i, md;
	char buf[BLKSIZE];
	MINODE* mip, *mtdip;
	SUPER* sp;
	MTABLE* mt;
	GD* gp;

	printf("filesys: %s	mount_point: %s\n", filesys, mount_point);

	// 1. Ask for filesys (a pathname) and mount_point (a pathname also).
	// If mount with no parameters: display current mounted filesystems.
	if (filesys[0] == 0)
		printf("Current Mount\n");

	// 2. Check whether filesys is already mounted: 
	for (i = 0; i < NMOUNT; i++) {

		if (strcmp(mp[i].devName, filesys) == 0) { // If already mounted, reject;
			printf("MOUNT FAILED: '%s' ALREADY MOUNTED\n", filesys);
			return -1;
		}

	}

	for (i = 0; i < NMOUNT; i++) {
		if (mp[i].dev == 0) { // allocate a free MOUNT table entry (whose dev=0 means FREE).
			md = i;
			break;
		}
	}

	fd = open(filesys, O_RDWR);
	printf("DEV: %d\n", fd);

	if (fd < 0) {
		printf("MOUNT: unable to open %s\n", filesys);
		return -1;
	}

	// Check whether it's an EXT2 file system: if not, reject.
	get_block(fd, SUPERBLOCK, buf);  // SUPERBLOCK = 1
	sp = (SUPER*)buf;

	if (sp->s_magic != 0xEF53)
	{
		printf("ERROR not an EXT2 File Sys!\n");
		return -1;
	}
	printf("passed s_magic\n");

	get_block(fd, GDBLOCK, buf); // getting GDBLOCK
	gp = (GD*)buf;

	// 4. For mount_point: find its ino, then get its minode:
	ino = getino(mount_point);
	mip = iget(dev, ino);

	// 5. Check mount_point is a DIR.  
	if (!S_ISDIR(mip->INODE.i_mode)) {
		printf("ERROR: mount_point %s is not a DIR\n");
		return -1;
	}

	// 6. Record new DEV in the MOUNT table entry;
	mt = &mp[md];
	mt->dev = fd;
	mt->ninodes = sp->s_inodes_count;
	mt->nblocks = sp->s_blocks_count;
	mt->bmap = gp->bg_block_bitmap;
	mt->imap = gp->bg_inode_bitmap;
	mt->iblock = gp->bg_inode_table;
	mt->mntDirPtr = mip;
	strcpy(mt->devName, filesys);
	strcpy(mt->mntName, mount_point);

	// 7. Mark mount_point's minode as being mounted on and let it point at the
	// MOUNT table entry, which points back to the mount_point minode.
	mip->mounted = 1;
	mip->mptr = mt;

	//close(dev2);


	printf("MOUNT: mounted %s on %s\n", filesys, mount_point);
	return 0; // for SUCCESS;
}


int umount(char* filesys)
{
	int i, dv;
	MTABLE* mt;

	// 1. Search the MOUNT table to check filesys is indeed mounted.
	for (i = 0; i < NMOUNT; i++) {
		if (strcmp(mp[i].devName, filesys) == 0) { // If already mounted, reject;
			printf("MOUNT FAILED: '%s' ALREADY MOUNTED\n", filesys);
			return -1;
		}
	}

	dv = mp[i].dev;
	// 2. Check whether any file is still active in the mounted filesys;
	for (i = 0; i < NMINODE; i++) {
		if (mp[i].dev == dv && minode[i].ino != 2) {
			printf("INODE: %d\n", minode[i].ino);
			return 1;
		}
	}

	// 3. Find the mount_point's inode (which should be in memory while it's mounted on). 
	mt = &mp[i];

	iput(mt->mntDirPtr);
	my_close(mt->dev);

	mt->dev = 0;
	mt->ninodes = 0;
	mt->nblocks = 0;
	mt->bmap = 0;
	mt->imap = 0;
	mt->iblock = 0;
	mt->mntDirPtr = 0;
	memset(mt->devName, 0, 64);
	memset(mt->mntName, 0, 64);

	// 4. return 0 for SUCCESS;
	return 0;
}
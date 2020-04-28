// mount <filesys> <mount_point>
// mounts a filesystem to a mount_point directory
// allows the file system to include other file sys as parts of the existing file sys
int mount(char* filesys, char* mount_point)    /*  Usage: mount filesys mount_point OR mount */
{
	int ino, dev2, i, md;
	char buf[BLKSIZE];
	MINODE* mip;
	SUPER* sp;
	MOUNT* mt;
	GD* gp;

	printf("filesys: %s	mount_point: %s\n", filesys, mount_point);

	// 1. Ask for filesys (a pathname) and mount_point (a pathname also).
	// If mount with no parameters: display current mounted filesystems.
	if (filesys[0] == 0)
		printf("just display curr mount filesystems\n");

	// 2. Check whether filesys is already mounted: 
	for (i = 0; i < NMOUNT; i++) {

		if (strcmp(mtable[i].devName, filesys) == 0) { // If already mounted, reject;
			printf("MOUNT FAILED: '%s' ALREADY MOUNTED\n", filesys);
			return -1;
		}

	}

	for (i = 0; i < NMOUNT; i++) {
		if (mtable[i].dev == 0) { // allocate a free MOUNT table entry (whose dev=0 means FREE).
			md = i;
			break;
		}
	}

	dev2 = open_file(filesys, "0");

	if (dev < 0)
		printf("MOUNT: unable to open %s\n", filesys);

	// Check whether it's an EXT2 file system: if not, reject.
	get_block(dev, SUPERBLOCK, buf);  // SUPERBLOCK = 1
	sp = (SUPER*)buf;
	if (sp->s_magic != 0xEF53)
	{
		printf("ERROR not an EXT2 File Sys!\n");
		return -1;
	}
	printf("passed s_magic\n");

	// 4. For mount_point: find its ino, then get its minode:
	ino = getino(dev2, mount_point);
	mip = iget(dev, ino);

	// 5. Check mount_point is a DIR.  
	if (!S_ISDIR(mip->INODE.i_mode)) {
		printf("ERROR: mount_point %s is not a DIR\n");
		return -1;
	}

	memset(buf, 0, BLKSIZE);

	// 6. Record new DEV in the MOUNT table entry;
	get_block(dev2, GDBLOCK, buf); // getting GDBLOCK
	gp = (GD*)buf;

	mt = &mtable[md];
	mt->dev = dev2;
	mt->ninodes = sp->s_inodes_count;
	mt->nblocks = sp->s_blocks_count;
	mt->bmap = gp->bg_block_bitmap;
	mt->imap = gp->bg_inode_bitmap;
	mt->iblock = gp->bg_inode_table;
	mt->mntDirPtr = mip;
	strcpy(mt->devname, filesys);
	strcpy(mt->mntName, mount_point);

	// 7. Mark mount_point's minode as being mounted on and let it point at the
	// MOUNT table entry, which points back to the mount_point minode.
	mip->mounted = 1;
	mip->mptr = mt;

	if (mip)
		iput(mip);
	if (dev2 > 0)
		myclose(dev2);

	printf("MOUNT: mounted %s on %s\n", filesys, mount_point);
	return 0; // for SUCCESS;
}


int umount(char* filesys)
{
	int i, dv;
	MOUNT* mt;

	// 1. Search the MOUNT table to check filesys is indeed mounted.
	for (i = 0; i < NMOUNT; i++) {
		if (strcmp(mtable[i].devName, filesys) == 0) { // If already mounted, reject;
			printf("MOUNT FAILED: '%s' ALREADY MOUNTED\n", filesys);
			return -1;
		}
	}

	dv = mtable[i].dev;
	// 2. Check whether any file is still active in the mounted filesys;
	for (i = 0; i < NMINODES; i++) {
		if (minode[i]->dev == dv && minode[i]->ino != 2) {
			printf("INODE: %d\n", minode[i].ino);
			return 1;
		}
	}

	// 3. Find the mount_point's inode (which should be in memory while it's mounted on). 
	mt = &mtable[i];

	iput(mt->mntDirPtr);
	myclose(mt->dev);

	mt->dev = 0;
	mt->ninodes = 0;
	mt->nblocks = 0;
	mt->bmap = 0;
	mt->imap = 0;
	mt->iblock = 0;
	mt->mntDirPtr = 0;
	memset(mt->devname, 0, 64);
	memset(mt->mntName, 0, 64);

	// 4. return 0 for SUCCESS;
	return 0;
}
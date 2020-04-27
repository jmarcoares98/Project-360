int mount()    /*  Usage: mount filesys mount_point OR mount */
{	
	int ino;
	char buf[BLKSIZE];
	MINODE *mip;
	SUPER* sp;
	printf("filesys: %s	mount_point: %s\n", filesys, mount_point);

	// 1. Ask for filesys (a pathname) and mount_point (a pathname also).
	// If mount with no parameters: display current mounted filesystems.
	if (filesys[0] == 0)
		printf("just display curr mount filesystems\n");
	// 2. Check whether filesys is already mounted: 
	// (you may store the name of mounted filesys in the MOUNT table entry). 
	// If already mounted, reject;
	// else: allocate a free MOUNT table entry (whose dev=0 means FREE).

	// 3. open filesys for RW; use its fd number as the new DEV;
	dev = open_file(filesys, "0");
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
	printf("past s_magic\n");
	// 4. For mount_point: find its ino, then get its minode:
	ino  = getino(dev, mount_point); 
	mip  = iget(dev, ino);    

	// 5. Check mount_point is a DIR.  
	// Check mount_point is NOT busy (e.g. can't be someone's CWD)
	if (!S_ISDIR(mip->INODE.i_mode))
	{
		printf("ERROR: mount_point %s is not a DIR\n");
		return -1;
	}
	// 6. Record new DEV in the MOUNT table entry;

	// (For convenience, store the filesys name in the Mount table, and also its
	// ninodes, nblocks, bitmap blocks, inode_start block, etc. for quick reference)

	// 7. Mark mount_point's minode as being mounted on and let it point at the
	// MOUNT table entry, which points back to the mount_point minode.

	printf("MOUNT: mounted %s on %s\n", filesys, mount_point);
	return 0; // for SUCCESS;
}
  

int umount(char *filesys)
{

	// 1. Search the MOUNT table to check filesys is indeed mounted.

	// 2. Check whether any file is still active in the mounted filesys;
	// e.g. someone's CWD or opened files are still there,
	// if so, the mounted filesys is BUSY ==> cannot be umounted yet.
	// HOW to check?      ANS: by checking all minode[].dev

	// 3. Find the mount_point's inode (which should be in memory while it's mounted on). 
	// Reset it to "not mounted"; then iput() the minode.  
	// (because it was iget()ed during mounting)

	// 4. return 0 for SUCCESS;

}  

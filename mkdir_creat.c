int tst_bit(char* buf, int bit)
{
	return buf[bit / 8] & (1 << (bit % 8)); // in Chapter 11.3.1
}

int set_bit(char* buf, int bit)
{
	return buf[bit / 8] |= (1 << (bit % 8)); // in Chapter 11.3.1
}

int clr_bit(char* buf, int bit) 
{
	return buf[bit / 8] &= ~(1 << (bit % 8));
}

int ialloc(int dev)  // allocate an inode number from inode_bitmap
{
	int  i;
	char buf[BLKSIZE];

	// read inode_bitmap block
	get_block(dev, imap, buf);

	for (i = 0; i < ninodes; i++) {
		if (tst_bit(buf, i) == 0) {
			set_bit(buf, i);
			put_block(dev, imap, buf);
			printf("allocated ino = %d\n", i + 1);
			return i + 1;
		}
	}
	return 0;
}

int balloc(int dev)
{
	int  i, n;
	char buf[BLKSIZE];

	// read inode_bitmap block
	get_block(dev, bmap, buf);

	for (i = 0; i < nblocks ; i++) {

		if (tst_bit(buf, i) == 0) {
			set_bit(buf, i);
			put_block(dev, bmap, buf);
			printf("allocated bno = %d\n", i + 1);
			return i + 1;
		}
	}

	return 0;
}

/************* mkdir **************/
int make_dir(char* name)
{
	MINODE* pip;

	// 1. pahtname = "/a/b/c" start = root;         dev = root->dev;
	//             =  "a/b/c" start = running->cwd; dev = running->cwd->dev;
	char pathname[128], parent[128], child[128], path[128];
	int ino = 0, start, mk;

	strcpy(pathname, name);
	printf("%s\n", pathname);
	
	if(pathname[0] == '/')
		dev = root->dev;
	else
		dev = running->cwd->dev;

	// 2. Let  
	//    parent = dirname(pathname);   parent = "/a/b" OR "a/b"
	//    child = basename(pathname);  child = "c"
	// WARNING: strtok(), dirname(), basename() destroy pathname
	strcpy(path, pathname);
	strcpy(parent, dirname(path));
	printf("parent: %s\n", parent);

	strcpy(path, pathname);
	strcpy(child, basename(path));
	printf("child: %s\n", child);

	// 3. get minode of parent
	ino = getino(dev, parent);
	pip = iget(dev, ino);

	// check that parent INODE is a dir and that child does not exist in par dir
	// 4. call mymkdir(pip, child);
	mymkdir(pip, child);

	// 6. iput(pip);
	iput(pip);
}

int mymkdir(MINODE* pip, char* name)
{
	MINODE* mip;
	char* cp;
	DIR* dp;
	char buf[BLKSIZE];
	int ino, bno, parent;

	// 1. pip points at the parent minode[] of "/a/b", name is a string "c"
	parent = pip->dev;

	// 2. allocate an inode and a disk block for the new directory;
	// DO NOT WORK IN THE DARK : PRINT OUT THESE NUMBERS!!!
	ino = ialloc(parent);
	bno = balloc(parent);
    printf("ino: %d bno: %d\n", ino, bno);

	// 3. mip = iget(dev, ino);  load the inode into a minode[] (in order to
	// wirte contents to the INODE in memory.
	mip = iget(parent, ino);

	// 4. Write contents to mip->INODE to make it a DIR INODE.
    mip->INODE.i_mode = 0x41ED;		// DIR type and permissions
	mip->INODE.i_uid = running->uid;	// Owner uid
	mip->INODE.i_gid = running->gid;	// Group Id
	mip->INODE.i_size = BLKSIZE;		// Size in bytes
	mip->INODE.i_links_count = 2;	        // Links count=2 because of . and ..
	mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);  // set to current time
	mip->INODE.i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks
	mip->INODE.i_block[0] = bno;             // new DIR has one data block

	for (int i = 1; i < 15; i++) //  i_block[1] to i_block[14] = 0;
		mip->INODE.i_block[i] = 0;

	mip->dirty = 1;               // mark minode dirty
	mip->ino = ino;

	// 5. iput(mip); which should write the new INODE out to disk.
	iput(mip);                    // write INODE to disk

	// 6. Write . and .. entries to a buf[ ] of BLKSIZE
	memset(buf, 0, BLKSIZE);

	dp = (DIR*)buf;
	cp = buf;

	dp->inode = ino;
	strncpy(dp->name, ".", 1); // for . entries
	dp->name_len = 1;
	dp->rec_len = 12;

	cp += dp->rec_len;
	dp = (DIR*)cp;

	dp->inode = pip->ino;
	dp->name_len = 2;
	dp->file_type = (u8)EXT2_FT_DIR; //EXT2 dit type
	strncpy(dp->name, "..", 2); // for .. entries
	dp->rec_len = BLKSIZE - 12;

	put_block(parent, bno, buf);

	// 7. Finally, enter name ENTRY into parent's directory
	enter_name(pip, mip->ino, name);
}

int enter_name(MINODE* pip, int myino, char* myname)
{
	INODE* ip = &(pip->INODE);
	char buf[BLKSIZE];
	char* cp;
	DIR* dp;
	int idea_len = 0, need_len = 0, remain = 0, bnum = 0, i = 0;
	int name_len = strlen(myname);

	//For each data block of parent DIR do { // assume: only 12 direct blocks
	for (i = 0; i < ip->i_size / BLKSIZE; i++) {
		if (ip->i_block[i] == 0)
			break;

		// Step to the last entry in a data block 
		get_block(dev, ip->i_block[i], buf);
		need_len = 4 * ((8 + dp->name_len + 3) / 4);  // a multiple of 4

		// step to LAST entry in block: int blk = parent->INODE.i_block[i];
		while ((dp->rec_len + cp) < buf + BLKSIZE) {
			cp += dp->rec_len;
			dp = (DIR*)cp;
		}

		cp = (char*)dp;

		// Let remain = LAST entry's rec_len - its IDEAL_LENGTH;
		remain = dp->rec_len - need_len;

		if (remain >= need_len) {
			dp->rec_len = need_len;

			cp += dp->rec_len;
			dp = (DIR*)cp;

			dp->rec_len = remain;
			dp->inode = myino;
			dp->name_len = name_len;
			dp->file_type = EXT2_FT_DIR;
			strncpy(dp->name, myname, dp->name_len);

			put_block(dev, pip->INODE.i_block[0], buf);
			return 1;
		}
	}

	// Reach here means: NO space in existing data block(s)
	// Allocate a new data block; IN parent's by BLKSIZE;
	bnum = balloc(dev);
	ip->i_block[i] = bnum;

	get_block(dev, bnum, buf);

	dp = (DIR*)buf;
	cp = buf;

	dp->inode = myino;
	dp->name_len = name_len;
	dp->rec_len = remain;
	dp->file_type = EXT2_FT_DIR;
	strncpy(dp->name, myname, dp->name_len);

	// write data block to disk
	put_block(dev, bnum, buf);

	return 1;
}

/************* creat **************/
int creat_file(char* name)
{
	MINODE* pip;

	char pathname[128], parent[128], child[128], path[128];
	int ino = 0, start, mk;

	strcpy(pathname, name);
	printf("%s\n", pathname);

	if (pathname[0] == '/')
		dev = root->dev;
	else
		dev = running->cwd->dev;

	printf("dev = %d\n", dev);

	strcpy(path, pathname);
	strcpy(parent, dirname(path));
	printf("parent: %s\n", parent);

	strcpy(path, pathname);
	strcpy(child, basename(path));
	printf("child: %s\n", child);

	//get minode of parent
	ino = getino(dev, parent);
	pip = iget(dev, ino);

	// check that parent INODE is a dir and that child does not exist in par dir
	// 4. call mymkdir(pip, child);
	mycreat(pip, child);

	// 6. iput(pip);
	iput(pip);
}

int mycreat(MINODE* pip, char* name) {
	MINODE* mip;
	char* cp;
	DIR* dp;
	int ino, bno, parent, mk;

	parent = pip->dev;

	ino = ialloc(parent);
	bno = balloc(parent);
	printf("ino: %d bno: %d\n", ino, bno);

	mip = iget(parent, ino);

	mip->INODE.i_mode = 0x81A4;
	mip->INODE.i_uid = running->uid;	// Owner uid
	mip->INODE.i_gid = running->gid;	// Group Id
	mip->INODE.i_size = BLKSIZE;		// Size in bytes
	mip->INODE.i_links_count = 2;	        // Links count=2 because of . and ..
	mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);  // set to current time
	mip->INODE.i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks
	mip->INODE.i_block[0] = bno;             // new DIR has one data block

	for (int i = 1; i < 15; i++) //  i_block[1] to i_block[14] = 0;
		mip->INODE.i_block[i] = 0;


	mip->dirty = 1;               // mark minode dirty
	mip->ino = ino;

	// 5. iput(mip); which should write the new INODE out to disk.
	iput(mip);                    // write INODE to disk

	// 6. Write . and .. entries to a buf[ ] of BLKSIZE
	memset(buf, 0, 1024);

	put_block(parent, bno, buf);

	ino = mip->ino;

	enter_name(pip, ino, name);
}

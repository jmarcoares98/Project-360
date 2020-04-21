int tst_bit(char* buf, int bit)
{
	if (buf[bit / 8] & (1 << (bit % 8))) // in Chapter 11.3.1
		return 1;

	return 0;
}

void set_bit(char* buf, int bit)
{
	buf[bit / 8] |= (1 << (bit % 8)); // in Chapter 11.3.1
}

// this function helps reduce of how many free inodes there are
void decFreeInodes(int dev)
{
	char buf[BLKSIZE];

	//dec free inodes count in SUPER and Group Descriptor block
	get_block(dev, 1, buf);
	sp = (SUPER*)buf;
	sp->s_free_inodes_count--;
	put_block(dev, 1, buf); 
	get_block(dev, 2, buf);
	gp = (GD*)buf;
	gp->bg_free_inodes_count--;
	put_block(dev, 2, buf);
}

int ialloc(int dev)  // allocate an inode number from inode_bitmap
{
	int  i;
	char buf[BLKSIZE];

	// read imap block
	get_block(dev, imap, buf);

	for (i = 0; i < ninodes; i++) {
		if (tst_bit(buf, i) == 0) {
			set_bit(buf, i);
			decFreeInodes(dev); // update free inode count in SUPER and GD
			put_block(dev, imap, buf);
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
			decFreeInodes(dev);
			put_block(dev, bmap, buf);
			return i - 1;
		}
	}

	return 0;
}

/************* mkdir **************/
int make_dir(char* name)
{
	MINODE* pip; // parent minode

	// 1. pahtname = "/a/b/c" start = root;         dev = root->dev;
	//             =  "a/b/c" start = running->cwd; dev = running->cwd->dev;
	char parent[128], child[128], path1[128], path2[128];
	int ino = 0, start, mk;

	// 2. Let  
	//    parent = dirname(pathname);   parent = "/a/b" OR "a/b"
	strcpy(path1, name);
	strcpy(parent, dirname(path1));
	printf("parent: %s\n", parent);

	//    child = basename(pathname);  child = "c"
	strcpy(path2, name);
	strcpy(child, basename(path2));
	printf("child: %s\n", child);

	// 3. get minode of parent
	ino = getino(running->cwd, parent);
	pip = iget(dev, ino);

	// check that parent INODE is a dir and that child does not exist in par dir
	if (!pip) {
		printf("PARENT DOES NOT EXIST!\n");
		return;
	}

	if (!S_ISDIR(pip->INODE.i_mode)) {
		printf("PARENT NOT A DIRECTPORY!\n");
		return;
	}

	if (getino(running->cwd, name) != 0)
	{
		printf("INVALID: %s ALREADY EXISTS!\n", name);
		return;
	}

	// 4. call mymkdir(pip, child);
	mymkdir(pip, child);

	pip->INODE.i_links_count++;
	pip->INODE.i_atime = time(0L);
	pip->dirty = 1;

	// 6. iput(pip);
	iput(pip);
}

int mymkdir(MINODE* pip, char* name)
{
	MINODE* mip;
	INODE* ip;
	char* cp;
	DIR* dp;
	char buf[BLKSIZE];
	int ino, bno;

	// 2. allocate an inode and a disk block for the new directory;
	// DO NOT WORK IN THE DARK : PRINT OUT THESE NUMBERS!!!
	ino = ialloc(dev);
	bno = balloc(dev);
	printf("DEV: %d\n", dev);
    printf("INO: %d BNO: %d\n", ino, bno);

	// 3. mip = iget(dev, ino);  load the inode into a minode[] (in order to
	// wirte contents to the INODE in memory.
	mip = iget(dev, ino);
	ip = &mip->INODE;

	// 4. Write contents to mip->INODE to make it a DIR INODE.
    ip->i_mode = 0x41ED;		// DIR type and permissions
	ip->i_uid = running->uid;	// Owner uid
	ip->i_gid = running->gid;	// Group Id
	ip->i_size = BLKSIZE;		// Size in bytes
	ip->i_links_count = 2;	        // Links count=2 because of . and ..
	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
	ip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks
	ip->i_block[0] = bno;             // new DIR has one data block

	for (int i = 1; i < 15; i++) //  i_block[1] to i_block[14] = 0;
		ip->i_block[i] = 0;

	mip->dirty = 1;               // mark minode dirty

	printf("INO: %d\n", mip->ino);
	// 5. iput(mip); which should write the new INODE out to disk.
	iput(mip);                    // write INODE to disk

	// 6. Write . and .. entries to a buf[ ] of BLKSIZE
	get_block(dev, bno, buf);

	dp = (DIR*)buf;
	cp = buf;

	dp->inode = ino;
	strncpy(dp->name, ".", 1); // for . entries
	dp->file_type = (u8)EXT2_FT_DIR; // EXT2 dit type
	dp->name_len = 1;
	dp->rec_len = 12;

	cp += dp->rec_len;
	dp = (DIR*)cp;

	dp->inode = pip->ino;
	dp->name_len = 2;
	dp->file_type = (u8)EXT2_FT_DIR; // EXT2 dit type
	strncpy(dp->name, "..", 2); // for .. entries
	dp->rec_len = BLKSIZE - 12;

	put_block(dev, bno, buf);

	// 7. Finally, enter name ENTRY into parent's directory
	enter_name(pip, ino, name);

	return 1;
}

int enter_name(MINODE* pip, int myino, char* myname)
{
	INODE* ip = &pip->INODE;
	char buf[BLKSIZE];
	char* cp;
	DIR* dp;
	int idea_len = 0, need_len = 0, remain = 0, bnum = 0, i = 0, j = 0, newrec = 0;
	int name_len = strlen(myname);


	//For each data block of parent DIR do { // assume: only 12 direct blocks
	for (i = 0; i < 12; i++) {
		// if (i_block[i]==0) BREAK;
		if (ip->i_block[i] == 0) 
			break;

		// Step to the last entry in a data block 
		bnum = ip->i_block[i];

		
		get_block(dev, bnum, buf);
		cp = buf;
		dp = (DIR*)cp;

		// step to LAST entry in block: int blk = parent->INODE.i_block[i];
		while ((dp->rec_len + cp) < buf + BLKSIZE) {
			cp += dp->rec_len;
			dp = (DIR*)cp;
		}

		printf("LAST ENTRY: %s...\n", dp->name);
		cp = (char*)dp;

		need_len = 4 * ((8 + name_len + 3) / 4);  // a multiple of 4

		// Each DIR entry has rec_len, name_len. Each entry's ideal length is 
		idea_len = 4 * ((8 + dp->name_len + 3) / 4);

		// Let remain = LAST entry's rec_len - its IDEAL_LENGTH;
		remain = dp->rec_len - idea_len;
		printf("REMAINING: %d...\n", remain);

		if (remain >= need_len) {
			dp->rec_len = idea_len;

			cp += dp->rec_len;
			dp = (DIR*)cp;

			dp->rec_len = BLKSIZE - ((u32)cp - (u32)buf);
			dp->inode = myino;
			dp->name_len = name_len;
			dp->file_type = EXT2_FT_DIR;
			strcpy(dp->name, myname);

			put_block(dev, bnum, buf);
			return 1;
		}
	}

	// Reach here means: NO space in existing data block(s)
	// Allocate a new data block; IN parent's by BLKSIZE;
	bnum = balloc(dev);
	ip->i_block[i] = bnum;

	ip->i_size += BLKSIZE;
	pip->dirty = 1;

	get_block(dev, bnum, buf);

	dp = (DIR*)buf;
	cp = buf;

	dp->inode = myino;
	dp->name_len = name_len;
	dp->rec_len = BLKSIZE;
	//dp->file_type = EXT2_FT_DIR;
	strncpy(dp->name, myname, dp->name_len);

	// write data block to disk
	put_block(pip->dev, bnum, buf);

	return 1;
}

/************* creat **************/
int creat_file(char* name)
{
	MINODE* mip;

	char parent[128], child[128], path1[128], path2[128];
	int ino = 0, start;

	strcpy(path1, name);
	strcpy(parent, dirname(path1));
	printf("parent: %s\n", parent);

	strcpy(path2, name);
	strcpy(child, basename(path2));
	printf("child: %s\n", child);

	//get minode of parent
	ino = getino(running->cwd, parent);
	mip = iget(dev, ino);

	// check that parent INODE is a dir and that child does not exist in par dir
	if (!mip) {
		printf("PARENT DOES NOT EXIST\n");
		return;
	}

	if (!S_ISDIR(mip->INODE.i_mode)) {
		printf("PARENT NOT A DIRECTORY!\n");
		return;
	}

	if (getino(running->cwd, name) != 0)
	{
		printf("INVALID: %s ALREADY EXISTS!\n", name);
		return;
	}

	// 4. call mycreat(pip, child);
	mycreat(mip, child);

	mip->INODE.i_atime = time(0L);
	mip->dirty = 1;

	// 6. iput(pip);
	iput(mip);
}

int mycreat(MINODE* pip, char* name) {
	MINODE* mip;
	INODE* ip;
	int ino = ialloc(dev);
	printf("DEV: %d\n", dev);
	printf("INO: %d\n", ino);

	mip = iget(dev, ino);
	ip = &mip->INODE;

	ip->i_mode = 0x81A4;			// INODE's file type = 0x81A4 OR 0100644
	ip->i_uid = running->uid;		// Owner uid
	ip->i_gid = running->gid;		// Group Id
	ip->i_size = 0;					// No data block, set to zero
	ip->i_links_count = 1;	        // Links count=1 links to parent directory
	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
	ip->i_blocks = 0;               // LINUX: Blocks count in 512-byte chunks

	for (int i = 0; i < 15; i++)	//  i_block[1] to i_block[14] = 0;
		ip->i_block[i] = 0;


	mip->dirty = 1;					// mark minode dirty

	printf("INO: %d\n", mip->ino);
	// 5. iput(mip); which should write the new INODE out to disk.
	iput(mip);                    // write INODE to disk

	enter_name(pip, ino, name);

	return;
}

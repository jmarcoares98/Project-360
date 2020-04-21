/*********** util.c file ****************/
int get_block(int dev, int blk, char* buf)
{
	lseek(dev, (long)blk * BLKSIZE, 0);
	read(dev, buf, BLKSIZE);
}
int put_block(int dev, int blk, char* buf)
{
	lseek(dev, (long)blk * BLKSIZE, 0);
	write(dev, buf, BLKSIZE);
}

MINODE* mialloc() // allocate a FREE minode for use
{
	int i;
	MINODE* mp;

	for (i = 0; i < NMINODE; i++) {
		mp = &minode[i];
		if (mp->refCount == 0) {
			mp->refCount = 1;
			return mp;
		}
	}
	printf("FS panic : out of minodes\n");
	return 0;
}

int midalloc(MINODE* mip) // release a used minode
{
	mip->refCount = 0;
}

int tokenize(char* pathname)
{
	int i, n = 0;
	char* s;
	printf("tokenize %s\n", pathname);

	strcpy(gpath, pathname);   // tokens are in global gpath[ ]

	s = strtok(gpath, "/");
	while (s) {
		name[n] = s;
		n++;
		s = strtok(0, "/");
	}

	//for (i = 0; i < n; i++)
	//	printf("%s  ", name[i]);
	//printf("\n");

	return n;
}

// return minode pointer to loaded INODE
MINODE* iget(int dev, int ino)
{
	int i;
	MINODE* mip;
	char buf[BLKSIZE];
	int blk, offset;
	INODE* ip;

	for (i = 0; i < NMINODE; i++) {
		mip = &minode[i];
		if (mip->dev == dev && mip->ino == ino) {
			mip->refCount++;
			//printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
			return mip;
		}
	}

	for (i = 0; i < NMINODE; i++) {
		mip = &minode[i];
		if (mip->refCount == 0) {
			//printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
			mip = mialloc();
			mip->refCount++;
			mip->dev = dev;
			mip->ino = ino;

			// get INODE of ino into buf[ ]    
			blk = (ino - 1) / 8 + inode_start;
			offset = (ino - 1) % 8;

			get_block(dev, blk, buf);
			ip = (INODE*)buf + offset;
			// copy INODE to mp->INODE
			mip->INODE = *ip;
			return mip;
		}
	}
	printf("PANIC: no more free minodes\n");
	return 0;
}

void iput(MINODE* mip)
{
	int blk, offset;
	char buf[BLKSIZE];
	int ino = mip->ino;

	INODE* ip;

	if (mip == 0) return;

	if (mip->refCount == 0)		// minode is still in use
		return;
	if (mip->dirty == 0)        // INODE has not changed; no need to write back
		return;

	mip->refCount--;			// dec refCount by 1

	// write INODE back to disk
	blk = (ino - 1) / 8 + inode_start;
	offset = (ino - 1) % 8;

	get_block(dev, blk, buf);

	ip = (INODE*)buf + offset;	// ip points at INODE
	*ip = mip->INODE;			// copy mp->INODE to INODE
	put_block(dev, blk, buf); 	// write back to disk
	midalloc(mip);				// mip->refCount = 0;
	mip->dirty = 0;
}

int search(MINODE* mip, char* name)
{
	char* cp, buf[BLKSIZE], temp[256];
	int i;
	DIR* dp;
	INODE* ip;

	//printf("search for %s in MINODE = [%d, %d]\n", name, mip->dev, mip->ino);
	ip = &(mip->INODE);

	/*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/
	for (i = 0; i < 12; i++) {
		if (ip->i_block[i] == 0)
			break;

		get_block(dev, ip->i_block[i], buf);
		cp = buf;
		dp = (DIR*)cp;
		//printf("  ino   rlen  nlen  name\n");

		while (cp < buf + BLKSIZE) {
			strncpy(temp, dp->name, dp->name_len);
			temp[dp->name_len] = 0;
			//printf("%4d  %4d  %4d    %s\n",
				//dp->inode, dp->rec_len, dp->name_len, temp);
			if (strcmp(temp, name) == 0) {
				printf("found %s : ino = %d\n", temp, dp->inode);
				return dp->inode;
			}
			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
	} 
	printf("%s DOES NOT EXIST...\n", name);
	return 0;
}

int getino(int dev, char* pathname)
{
	int i, ino, blk, disp;
	char buf[BLKSIZE];
	INODE* ip;
	MINODE* mip;

	//printf("getino: pathname=%s\n", pathname);
	if (strcmp(pathname, "/") == 0)
		return 2;

	// starting mip = root OR CWD
	if (pathname[0] == '/')
		mip = root;
	else
		mip = running->cwd;

	mip->refCount++;         // because we iput(mip) later

	n = tokenize(pathname);

	for (i = 0; i < n; i++) {
		//printf("===========================================\n");
		//printf("getino: i=%d name[%d]=%s\n", i, i, name[i]);
		if (!S_ISDIR(mip->INODE.i_mode)) { // check DIR type
			printf("%s NOT A DIRECTORY\n", name[i]);
			iput(mip);
			return 0;
		}

		ino = search(mip, name[i]);

		if (!ino) {
			iput(mip);
			return 0;
		}
		iput(mip); // release current minode
		mip = iget(dev, ino); // switch to new minode
	}

	iput(mip);
	return ino;
}

int findmyname(MINODE* parent, u32 myino, char* myname)
{
	// WRITE YOUR code here:
	// search parent's data block for myino;
	// copy its name STRING to myname[ ];
}

int findino(MINODE* mip, u32* myino) // myino = ino of . return ino of ..
{
	char buf[BLKSIZE], * cp;
	DIR* dp;

	get_block(mip->dev, mip->INODE.i_block[0], buf);
	cp = buf;
	dp = (DIR*)buf;
	*myino = dp->inode;
	cp += dp->rec_len;
	dp = (DIR*)cp;
	return dp->inode;
}

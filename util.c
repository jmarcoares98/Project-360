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

int tokenize(char* pathname)
{
	int i, n;
	char* s;
	printf("tokenize %s\n", pathname);

	strcpy(gpath, pathname);   // tokens are in global gpath[ ]
	n = 0;

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

	// get INODE of ino into buf[ ]    
	blk = (ino - 1) / 8 + inode_start;
	offset = (ino - 1) % 8;

	//printf("iget: ino=%d blk=%d offset=%d\n", ino, blk, offset);

	get_block(dev, blk, buf);
	ip = (INODE*)buf + offset;
	// copy INODE to mp->INODE

	for (i = 0; i < NMINODE; i++) {
		mip = &minode[i];
		if (mip->refCount == 0) {
			//printf("\nallocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
			mip->refCount++;
			mip->dev = dev;
			mip->ino = ino;
			mip->INODE = *ip;
			return mip;
		}
	}

	printf("PANIC: no more free minodes\n");
	return 0;
}

void iput(MINODE* mip)
{
	int i, block, offset;
	char buf[BLKSIZE];
	INODE* ip;

	mip->refCount--;

	if (mip->refCount > 0)  // minode is still in use
		return;
	if (!mip->dirty)        // INODE has not changed; no need to write back
		return;

	/* write INODE back to disk */
	/***** NOTE *******************************************
	 For mountroot, we never MODIFY any loaded INODE
					so no need to write it back
	 FOR LATER WROK: MUST write INODE back to disk if refCount==0 && DIRTY

	 Write YOUR code here to write INODE back to disk
	********************************************************/
	block = (mip->ino - 1) / 8 + inode_start;
	offset = (mip->ino) % 8;

	get_block(mip->dev, block, (char*)buf);
	ip = ((INODE*)buf + offset);
	memcpy(ip, &mip->INODE, sizeof(INODE));
	put_block(mip->dev, block, buf);

	return;
}

int search(MINODE* mip, char* name)
{
	char* cp, c, sbuf[BLKSIZE], temp[256];
	DIR* dp;
	INODE* ip;

	//printf("search for %s in MINODE = [%d, %d]\n", name, mip->dev, mip->ino);
	ip = &(mip->INODE);

	/*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

	get_block(dev, ip->i_block[0], sbuf);
	dp = (DIR*)sbuf;
	cp = sbuf;
	//printf("  ino   rlen  nlen  name\n");

	while (cp < sbuf + BLKSIZE) {
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

		ino = search(mip, name[i]);

		if (ino == 0) {
			iput(mip);
			printf("name %s does not exist\n", name[i]);
			return 0;
		}
		iput(mip);                // release current mip
		mip = iget(dev, ino);     // get next mip
	}

	iput(mip);                   // release mip  
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

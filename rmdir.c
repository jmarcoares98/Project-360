int idalloc(int dev, int ino)  // deallocate an ino number
{
	int i;
	char buf[BLKSIZE];

	if (ino > ninodes) {
		printf("inumber %d out of range\n", ino);
		return 0;
	}

	// get inode bitmap block
	get_block(dev, imap, buf);
	clr_bit(buf, ino - 1);

	// write buf back
	put_block(dev, imap, buf);
}

int bdalloc(int dev, int blk) // deallocate a blk number
{
	int i;
	char buf[BLKSIZE];

	// get block bitmap block
	get_block(dev, bmap, buf);
	clr_bit(buf, blk - 1);

	// write buf back
	put_block(dev, bmap, buf);
}

int rmdir(char* pathname)
{
	int ino, pino;
	MINODE* mip, * pip;
	INODE* ip;
	char parent[256], child[256], path[256];

	// get inumber of pathname
	ino = getino(dev, pathname);

	// get its minode[ ] pointer:
	mip = iget(dev, ino);
	ip = &mip->INODE;

	// check ownership
	// check DIR type (HOW?), not BUSY (HOW?), is empty:
	if (!S_ISDIR(ip->i_mode)) {
		iput(mip->dev, mip);
		return -1;
	}

	if (ip->i_links_count > 2) {
		iput(mip->dev, mip);
		return -1;
	}

	// ASSUME passed the above checks.
	// get parent DIR's ino and Minode (pointed by pip);
	for (int i = 0; i < 12; i++) {
		if (ip->i_block[i] == 0)
			continue;
		bdealloc(mip->dev, mip->INODE.i_block[i]);
	}
	idalloc(mip->dev, mip->ino);
	iput(mip->dev,mip); // (which clears mip->refCount = 0);

	strcpy(path, pathname);
	strcpy(parent, dirname(pathname));
	strcpy(child, basename(pathname));

	pino = getino(dev, pathname);
	pip = iget(dev, ino);
	ip = &pip->INODE;

	// remove child's entry from parent directory
	rm_child(pip, pathname);

	// decrement pip's link_count by 1; 
	ip->i_links_count -= 1;

	// touch pip's atime, mtime fields;
	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);

	// mark pip dirty;
	pip->dirty = 1;

	// iput(pip);
	iput(pip->dev, pip);

	// return SUCCESS;
	return 1;
}

int rm_child(MINODE* parent, char* name)
{
	INODE* ip = &parent->INODE;
	char* cp = buf, * lastcp = buf;
	DIR* dp = (DIR*)buf, * lastdp = (DIR*)buf;

	// find last entry block
	while ((lastcp + lastdp->rec_len) < buf + BLKSIZE) {
		lastcp += dp->rec_len;
		lastdp = (DIR*)lastcp;
	}

	// Search parent INODE's data block(s) for the entry of name
	for (int i = 0; i < 12; i++) {
		get_block(parent->dev, ip->i_block[i], buf);
		cp = buf;
		dp = (DIR*)buf;

		while (cp < buf + BLKSIZE) {
			// Delete name entry from parent directory by
			// if (first and only entry in a data block){
			if (dp->rec_len == BLKSIZE) {
				// deallocate the data block
				bdalloc(parent->dev, ip->i_block[i]);

				// compact parent�s i_block[] array to eliminate the deleted entry if it�s
				// between nonzero entries
				ip->i_block[i] = 0;
				ip->i_blocks--;
			}

			// else if LAST entry in block
			else if (cp == lastcp) {
				lastdp->rec_len += dp->rec_len;
			}

			// else: entry is first but not the only entry or in the middle of a block
			else {
				dp = (DIR*)lastcp;
				lastdp->rec_len += dp->rec_len;
				memcpy(cp, cp + dp->rec_len, (buf + BLKSIZE) - (cp + dp->rec_len));
			}

			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
		// Write the parent's data block back to disk
		// mark parent minode DIRTY for write - back
		put_block(parent->dev, ip->i_block[i], buf);
	}
}

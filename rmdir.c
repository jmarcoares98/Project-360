void clr_bit(char* buf, int bit)
{
	buf[bit / 8] &= ~(1 << (bit % 8));
}

int incFreeInodes(int dev)
{
	char buf[BLKSIZE];
	// inc free inodes count in SUPER and GD
	get_block(dev, 1, buf);
	sp = (SUPER*)buf;
	sp->s_free_inodes_count++;
	put_block(dev, 1, buf);
	get_block(dev, 2, buf);
	gp = (GD*)buf;
	gp->bg_free_inodes_count++;
	put_block(dev, 2, buf);
}

int idalloc(int dev, int ino)  // deallocate an ino number
{
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

	// update free inode count in SUPER and GD
	incFreeInodes(dev);
}

int bdalloc(int dev, int blk) // deallocate a blk number
{
	char buf[BLKSIZE];

	// get block bitmap block
	get_block(dev, bmap, buf);
	clr_bit(buf, blk - 1);

	// write buf back
	put_block(dev, bmap, buf);

	// update free inode count in SUPER and GD
	incFreeInodes(dev);
}

int rmdir(char* pathname)
{
	int ino, tino;
	MINODE* mip, * tip;
	INODE* ip;
	char parent[256], child[256], path[256], name[256];
	char* cp;
	DIR* dp;

	// get inumber of pathname
	ino = getino(dev, pathname);

	// get its minode[ ] pointer:
	mip = iget(dev, ino);
	ip = &(mip->INODE);

	// check ownership
	// check DIR type (HOW?), not BUSY (HOW?), is empty:
	if (!S_ISDIR(ip->i_mode)) {
		printf("NOT A DIRECTORY\n");
		return 1;
	}

	if (ip->i_links_count > 2) {
		printf("DIRECTORY IS NOT EMPTY!\n");
		return 1;

	}

	// ASSUME passed the above checks.
	// get parent DIR's ino and Minode (pointed by pip);
	for (int i = 0; i < 12; i++) {
		if (ip->i_block[i] != 0)
			bdalloc(mip->dev, ip->i_block[i]);
	}
	idalloc(mip->dev, mip->ino);

	strcpy(path, pathname);
	strcpy(parent, dirname(pathname));
	strcpy(child, basename(pathname));

	tino = getino(mip->dev, parent);
	tip = iget(mip->dev, tino);
	ip = &(tip->INODE);
	iput(mip); // (which clears mip->refCount = 0);

	// remove child's entry from parent directory
	rm_child(tip, child);

	// decrement pip's link_count by 1; 
	ip->i_links_count -= 1;

	// touch pip's atime, mtime fields;
	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);

	// mark pip dirty;
	tip->dirty = 1;

	// iput(pip);
	iput(tip);

	// return SUCCESS;
	return 1;
}

int rm_child(MINODE* parent, char* name)
{
	INODE* ip = &(parent->INODE);
	char buf[BLKSIZE], temp[256];
	memset(buf, 0, BLKSIZE);
	char* cp, * lastcp;
	DIR* dp, * lastdp, * prev;
	int good = 0, size;

	for (int i = 0; i < 12; i++) {
		// getting data block assuming there is only 12
		get_block(dev, parent->INODE.i_block[i], buf);
		cp = buf;
		dp = (DIR*)cp;
		lastcp = buf;
		lastdp = (DIR*)lastcp;

		// Search parent INODE's data block(s) for the entry of name
		while (cp < buf + BLKSIZE) {
			strncpy(temp, dp->name, dp->name_len);
			temp[dp->name_len] = 0;
			printf("%4d  %4d  %4d    %s\n",
				dp->inode, dp->rec_len, dp->name_len, temp);
			if (strcmp(name, temp) == 0) {
				//Delete name entry from parent directory by
				//if (first and only entry in a data block){
				if (dp->rec_len == BLKSIZE) {
					//deallocate the data block
					bdalloc(parent->dev, ip->i_block[i]);

					//compact parent�s i_block[] array to eliminate the deleted entry if it�s
					//between nonzero entries
					ip->i_block[i] = 0;
					ip->i_blocks--;
				}

				else {
					// look for last entry in block
					while ((lastdp->rec_len + lastcp) < buf + BLKSIZE) {
						prev = lastdp; // get the prev dp
						lastcp += lastdp->rec_len;
						lastdp = (DIR*)lastcp;
					}

					// if last entry in block
					if (lastdp == dp) {
						prev->rec_len += dp->rec_len;
						good = 1;
					}

					// if in the middle of a block{
					else {
						// get the size
						size = buf + BLKSIZE - (cp + dp->rec_len);
						lastdp->rec_len += dp->rec_len;

						memcpy(cp, (cp + dp->rec_len), size);
						good = 1;
					}
				}

			}

			cp += dp->rec_len;
			dp = (DIR*)cp;
		}

		if (good) {
			put_block(parent->dev, ip->i_block[i], buf);
			return 1;
		}
	}

}
/************* cd **************/
int chdir(char* pathname)
{
	int ino;
	MINODE* mip;

	// (1). int ino = getino(pathname); // return error if ino=0
	ino = getino(dev, pathname);

	if (ino == 0)
		return -1;

	// (2).MINODE* mip = iget(dev, ino);
	mip = iget(dev, ino);

	// (3).Verify mip->INODE is a DIR // return error if not DIR
	if (!S_ISDIR(mip->INODE.i_mode)) {
		printf("NOT A DIRECTORY\n");
		iput(mip);
		return -1;
	}
	// (4).iput(running->cwd); // release old cwd
	iput(running->cwd);

	// (5).running->cwd = mip; // change cwd to mip
	running->cwd = mip;
}

/************* ls **************/
int ls_file(MINODE* mip, char *name)
{
	INODE* ip = &mip->INODE;
	u16 type = ip->i_mode & 0xF000;

	if (type == 0x4000)			// prints out that its a directory
		printf("dir");
	else if (type == 0x8000)	// prints out that its a regular file
		printf("reg");
	else if (type == 0xA000)	// prints out that its a link
		printf("lnk");
	else						// prints default
		printf("???");
	
	printf("%4d	%4d	%4d	%8d	%s", ip->i_links_count, ip->i_gid, ip->i_uid, ip->i_size, name);

	// shows whats it point to when file a symlink
	if ((ip->i_mode & 0120000) == 0120000)
		printf(" => %s\n", (char*)(mip->INODE.i_block));
	else
		printf("\n");
}

int ls_dir(MINODE*mip)
{
	INODE* ip = &mip->INODE;
	MINODE* tmip;
	char* temp[256], buf[BLKSIZE], *cp;
	DIR* dp;
	int i;

	// assuming there is only 12 blocks
	for (i = 0; i < 12; i++)
	{
		// if (i_block[i]==0) BREAK;
		if (ip->i_block[i] == 0)
			break;

		// rom the minode of a directory, step through the dir_entries in the data blocks of the minode
		get_block(dev, ip->i_block[i], buf);
		cp = buf;
		dp = (DIR*)cp;

		while (cp < buf + BLKSIZE) {
			// get the name of the blocks
			strncpy(temp, dp->name, dp->name_len);
			temp[dp->name_len] = 0; //set the last one to null

			// For each dir_entry, use iget() to get its minode, as in MINODE *mip = iget(dev, ino);
			tmip = iget(dev, dp->inode);

			// Then, call ls_file(mip, name).
			if (tmip) { // check if it exists
				ls_file(tmip, temp);
				iput(tmip);
			}
			else
				printf("INVALID\n");

			memset(temp, 0, 256);
			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
	}
	printf("\n");
}


int ls(char* pathname)
{
	MINODE* mip;
	INODE* ip;
	int ino;

	printf("mode  lnk  gid     uid	    size        name\n");
	// goes through current working directory
	if (strcmp(pathname, "") == 0) {
		mip = iget(running->cwd->dev, running->cwd->ino);
		ls_dir(mip);
	}

	else if (pathname) {
		if (pathname[0] == '/')
			mip = root;

		ino = getino(mip, pathname);
		mip = iget(dev, ino);
		ip = &mip->INODE;

		if (!S_ISDIR(ip->i_mode)) {
			printf("NOT A DIRECTORY\n");
			iput(mip);
			return;
		}

		ls_dir(mip);
		iput(mip);
	}
	else // just ls_dir the root
		ls_dir(root);

	iput(mip);
	return;

}

/************* pwd **************/
int pwd(MINODE* wd)
{
	printf("***** pwd *****\n");
	rpwd(running->cwd);
	printf("\n");
}

// (1). if (wd==root) return;
// (2). from wd->INODE.i_block[0], get my_ino and parent_ino
// (3). pip = iget(dev, parent_ino);
// (4). from pip->INODE.i_block[]: get my_name string by my_ino as LOCAL
// (5). rpwd(pip); // recursive call rpwd(pip) with parent minode
// (6). print "/$s", my_name;
void rpwd(MINODE* wd)
{
	MINODE* pip;
	INODE* ip;
	char buf[BLKSIZE], * cp, pathname[64];
	DIR* dp = (DIR*)buf;
	int my_ino, parent_ino;
	ip = &wd->INODE;


	if (wd->ino == root->ino) {
		printf("/");
		return;
	}

	get_block(dev, ip->i_block[0], buf);
	cp = buf + dp->rec_len;
	dp = (DIR*)cp;

	my_ino = search(wd, dp->name);
	pip = iget(dev, my_ino);
	rpwd(pip); // recursive call rpwd(pip) with parent minode

	ip = &pip->INODE;

	for (int i = 0; i < 12; i++)
	{

		if (ip->i_block[i] == 0) { break; }

		get_block(pip->dev, ip->i_block[i], buf);
		cp = buf;
		dp = (DIR*)buf;
		while (cp < buf + BLKSIZE)
		{
			if (wd->ino == dp->inode)
			{
				strncpy(pathname, dp->name, dp->name_len);
			}
			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
	}
	pathname[dp->name_len] = '\0';
	printf("%s/", pathname);

}

/************* quit **************/
int quit()
{
	int i;
	MINODE* mip;
	for (i = 0; i < NMINODE; i++) {
		mip = &minode[i];
		if (mip->refCount > 0)
			iput(mip);
	}
	exit(0);
}

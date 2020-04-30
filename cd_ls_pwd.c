/************* cd **************/
int chdir(char* pathname)
{
	int ino;
	MINODE* mip;

	// (1). int ino = getino(pathname); // return error if ino=0
	ino = getino(pathname);

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

	printf("mode  cnt  gid     uid	    size        name\n");
	// goes through current working directory
	if (strcmp(pathname, "") == 0) {
		dev = running->cwd->dev;
		ino = running->cwd->ino;
		mip = iget(dev, ino);
		ls_dir(mip);
	}

	else if (pathname) {
		ino = getino(pathname);
		mip = iget(dev, ino);

		//if (!S_ISDIR(ip->i_mode)) {
		//	printf("NOT A DIRECTORY\n");
		//	iput(mip);
		//	return;
		//}

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
	if (wd == root)
		printf("/");
	else {
		printf("/");
		rpwd(wd);
	}
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
	char buf[BLKSIZE], * cp, my_name[64];
	DIR* dp = (DIR*)buf;
	int my_ino, parent_ino;

	//  if (wd==root) return;
	if (wd->ino == root->ino)
		return;
	
	// from wd->INODE.i_block[0], get my_ino and parent_ino
	parent_ino = findino(wd, &my_ino);
	pip = iget(dev, parent_ino);

	// from pip->INODE.i_block[]: get my_name string by my_ino as LOCAL
	findmyname(pip, my_ino, my_name);

	rpwd(pip); // recursive call rpwd(pip) with parent minode
	
	printf("%s/", my_name);
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

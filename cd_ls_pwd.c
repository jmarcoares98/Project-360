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
int ls_file(MINODE* mip, char* name)
{
	int r, i;
	char ftime[64], fname[64], t1[64], t2[64];

	// from lab4
	if ((mip->INODE.i_mode & 0xF000) == 0x8000) // if (S_ISREG())
	{
		printf("%c", '-');
	}

	else if ((mip->INODE.i_mode & 0xF000) == 0x4000) // if (S_ISDIR())
	{
		printf("%c", 'd');
	}

	else if ((mip->INODE.i_mode & 0xF000) == 0xA000) // if (S_ISLNK())
	{
		printf("%c", 'l');
	}

	for (i = 8; i >= 0; i--)
	{
		if (mip->INODE.i_mode & (1 << i)) // print r|w|x
			printf("%c", t1[i]);
		else
			printf("%c", t2[i]); // or print -
	}

	printf("%s ", ftime); // print name
	printf("%s", basename(fname)); // print file basename // print -> linkname if symbolic file

	if ((mip->INODE.i_mode & 0xF000) == 0xA000)
	{
		char* linkname = "";
		r = readlink(fname, linkname, 16);
		printf(" -> %s", linkname);
	}
	printf("\n");
}

int ls_dir(MINODE* mip)
{
	char temp[256];
	char* cp;
	DIR* dp;
	char buf[BLKSIZE];

	memset(buf, 0, 1024);

	// Assume DIR has only one data block i_block[0]
	get_block(dev, mip->INODE.i_block[0], buf);
	dp = (DIR*)buf;
	cp = buf;

	while (cp < buf + BLKSIZE) {
		memset(temp, 0, 256);
		strncpy(temp, dp->name, dp->name_len);
		temp[dp->name_len] = 0;

		printf("%4d %4d %4d %s\n", dp->inode, dp->rec_len, dp->name_len, temp); // print [inode# name]

		cp += dp->rec_len;
		dp = (DIR*)cp;
	}
	printf("\n");
}

// ALGORITHM OF LS
// (1). from the minode of a directory, step through the dir_entries in the data blocks
// of the minode.INODE. Each dir_entry contains the inode number, ino, and name of a file.
// For each dir_entry, use iget() to get its minode, as in MINODE *mip = iget(dev, ino);
// Then, call ls_file(mip, name).
// (2). ls_file(MINODE (mip, char *name): use mip->INODE and name to list the file info
int ls(char* pathname)
{
	char* temp_dir[128];
	char* temp_cwd;
	char* token = strtok(pathname, " ");
	int i = 0;

	//strcpy(temp_cwd, running->fname);
	if (token)
	{
		printf("in if pathname: \n");
		while (token != NULL)
		{
			printf("%s\n", token);
			temp_dir[i] = token;
			printf("char buf: %s\n", temp_dir[i]);
			token = strtok(NULL, " ");
			i++;
			if (token == NULL)
			{
				chdir(temp_dir[i - 1]);
				ls_dir(running->cwd);
				chdir("..");
			}
		}
	}
	else
	{
		printf("else\n");
		ls_dir(running->cwd);
	}
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
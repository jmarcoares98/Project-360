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
		iput(dev, mip);
		return -1;
	}
	// (4).iput(running->cwd); // release old cwd
	iput(dev, running->cwd);

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
	char temp_cwd[256];
	char* token = strtok(pathname, " ");
	int i = 0;

	while (token != NULL)
	{
		printf("%s\n", token);
		temp_dir[i] = token;
		printf("char buf: %s\n", temp_dir[i]);
		token = strtok(NULL, " ");
		i++;
	}
	ls_dir(running->cwd);
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
			iput(dev, mip);
	}
	exit(0);
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
	iput(dev, pip);
}

int mymkdir(MINODE* pip, char* name)
{
	MINODE* mip;
	INODE* ip;
	int ino, bno, parent, mk;

	// 1. pip points at the parent minode[] of "/a/b", name is a string "c"
	parent = pip->dev;

	// 2. allocate an inode and a disk block for the new directory;
	// DO NOT WORK IN THE DARK : PRINT OUT THESE NUMBERS!!!
	ino = ialloc(parent);
	bno = balloc(parent);
    printf("ino: %d bno: %d\n", ino, bno);

	// 3. mip = iget(dev, ino);  load the inode into a minode[] (in order to
	// wirte contents to the INODE in memory.
	mip = iget(dev, ino);
	ip = &mip->INODE;

	// 4. Write contents to mip->INODE to make it a DIR INODE.
    ip->i_mode = 040755;		// DIR type and permissions
	ip->i_uid = running->uid;	// Owner uid
	ip->i_gid = running->gid;	// Group Id
	ip->i_size = BLKSIZE;		// Size in bytes
	ip->i_links_count = 2;	        // Links count=2 because of . and ..
	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
	ip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks
	ip->i_block[0] = bno;             // new DIR has one data block

	for (int i = 0; i < 15; i++) //  i_block[1] to i_block[14] = 0;
		ip->i_block[i] = 0;

	mip->dirty = 1;               // mark minode dirty

	// 5. iput(mip); which should write the new INODE out to disk.
	iput(mip->dev, mip);                    // write INODE to disk

	// 6. Write . and .. entries to a buf[ ] of BLKSIZE
	memset(buf, 0, 1024);

	dp = (DIR*)buf;
	dp->inode = ino;
	strncpy(dp->name, ".", 1); // for . entries
	dp->name_len = 1;
	dp->rec_len = 12;

	cp = buf + 12;
	dp = (DIR*)cp;

	dp->inode = pip->ino;
	dp->name_len = 2;
	strncpy(dp->name, "..", 2); // for .. entries
	dp->rec_len = BLKSIZE - 12;

	put_block(dev, bno, buf);

	ino = mip->ino;
	// 7. Finally, enter name ENTRY into parent's directory
	enter_name(pip, ino, name);
}

int enter_name(MINODE* pip, int myino, char* myname)
{
	int idea_len, need_len, remain, i, bnum;
	int name_len = strlen(myname);
	INODE* ip = &pip->INODE;

	// update
	memset(buf, 0, BLKSIZE);

	// Step to the last entry in a data block 
	get_block(pip->dev, ip->i_block[0], buf);
	cp = buf;
	dp = (DIR*)cp;

	// step to LAST entry in block: int blk = parent->INODE.i_block[i];
	while ((dp->rec_len + cp) < buf + BLKSIZE)
	{
		cp += dp->rec_len;
		dp = (DIR*)cp;
	}
	// To enter a new entry of name with n_len, the needed length is
	need_len = 4 * ((8 + dp->name_len + 3) / 4);  // a multiple of 4

	// Let remain = LAST entry's rec_len - its IDEAL_LENGTH;
	remain = dp->rec_len - need_len;

	if (remain >= need_len) {
		dp->rec_len = need_len;

		cp += dp->rec_len;
		dp = (DIR*)cp;

		dp->rec_len = remain;
		dp->inode = myino;
		dp->name_len = name_len;
		strncpy(dp->name, myname, dp->name_len);

		put_block(pip->dev, ip->i_block[0], buf);
	}
	else {
		// For each data block of parent DIR do // assume: only 12 direct blocks
		for (i = 0; i < 12; i++) {
			if (ip->i_block[i] == 0) {
				ip->i_block[i] = balloc(dev);
				pip->refCount = 0;
				memset(buf, 0, 1024);
				cp = buf;
				dp = (DIR*)cp;
			}

			// Step to the last entry in a data block 
			get_block(pip->dev, ip->i_block[i], buf);
			cp = buf;
			dp = (DIR*)cp;

			// step to LAST entry in block: int blk = parent->INODE.i_block[i];
			while ((cp + dp->rec_len) < buf + BLKSIZE)
			{
				cp += dp->rec_len;
				dp = (DIR*)cp;
			}

			// Each DIR entry has rec_len, name_len. Each entry's ideal length is 
			idea_len = 4 * ((8 + dp->name_len + 3) / 4);

			// Let remain = LAST entry's rec_len - its IDEAL_LENGTH;
			remain = dp->rec_len - need_len;

			if (remain >= need_len) {
				dp->rec_len = idea_len;
				cp += dp->rec_len;
				dp = (DIR*)cp;
			}
		}

		// Reach here means: NO space in existing data block(s)
		// Allocate a new data block; IN parent's by BLKSIZE;
		dp->inode = myino;
		dp->name_len = name_len;
		dp->rec_len = remain;
		strncpy(dp->name, myname, dp->name_len);

		// write data block to disk
		put_block(pip->dev, ip->i_block[i], buf);
	}

	pip->dirty = 1;
	ip->i_links_count++;
	ip->i_atime = time(0);
}

int tst_bit(char* buf, int bit)
{
	return buf[bit / 8] & (1 << (bit % 8)); // in Chapter 11.3.1
}

int set_bit(char* buf, int bit)
{
	return buf[bit / 8] |= (1 << (bit % 8)); // in Chapter 11.3.1
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
	int  i;
	char buf[BLKSIZE];

	// read inode_bitmap block
	get_block(dev, bmap, buf);

	for (i = 0; i < BLKSIZE; i++) {
		if (tst_bit(buf, i) == 0) {
			set_bit(buf, i);
			put_block(dev, bmap, buf);
			printf("allocated bno = %d\n", i + 1);
			return i + 1;
		}
	}

	return 0;
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
	iput(dev, pip);
}

int mycreat(MINODE* pip, char* name) {
	MINODE* mip;
	INODE* ip;
	int ino, bno, parent, mk;

	parent = pip->dev;

	ino = ialloc(parent);
	bno = balloc(parent);
	printf("ino: %d bno: %d\n", ino, bno);

	mip = iget(dev, ino);
	ip = &mip->INODE;

	ip->i_mode = 0x81A4;		
	ip->i_uid = running->uid;	// Owner uid
	ip->i_gid = running->gid;	// Group Id
	ip->i_size = BLKSIZE;		// Size in bytes
	ip->i_links_count = 2;	        // Links count=2 because of . and ..
	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
	ip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks
	ip->i_block[0] = bno;             // new DIR has one data block

	for (int i = 0; i < 15; i++) //  i_block[1] to i_block[14] = 0;
		ip->i_block[i] = 0;

	mip->dirty = 1;               // mark minode dirty

	// 5. iput(mip); which should write the new INODE out to disk.
	iput(dev, mip);                    // write INODE to disk

	// 6. Write . and .. entries to a buf[ ] of BLKSIZE
	memset(buf, 0, 1024);

	dp = (DIR*)buf;
	dp->inode = ino;
	strncpy(dp->name, ".", 1); // for . entries
	dp->name_len = 1;
	dp->rec_len = 12;

	cp = buf + 12;
	dp = (DIR*)cp;

	dp->inode = pip->ino;
	dp->name_len = 2;
	strncpy(dp->name, "..", 2); // for .. entries
	dp->rec_len = BLKSIZE - 12;

	put_block(dev, bno, buf);

	enter_name(pip, ino, name);
}
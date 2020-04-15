// creates a file newFileName which has the SAME inode (number) as that of oldFileName
int link(char* old_file, char* new_file)
{
    printf("PRINTING OLD FILE: %s\n", old_file);
    printf("PRINTING NEW FILE: %s\n", new_file);
	int oino, pino;
	char *child, *parent;
	MINODE *omip, *pmip;
	// (1). get the INODE of /a/b/c into memory: mip->minode[ ]
    printf("check\n");
	oino = getino(dev,old_file);
	if (oino == 0)
	{
		return -1;
	}
    printf("check\n");
	omip = iget(dev, oino);
	// (2). check /a/b/c is a REG or LNK file (link to DIR is NOT allowed).
	if (S_ISDIR(omip->INODE.i_mode))
	{
		return -1;
	}
	// (3). creat new_file with the same inode number of old_file
    printf("check\n");
	if (getino(dev,new_file) == 0)
	{
        printf("IN if statement\n");
		parent = dirname(new_file);
        printf("parent: %s\n", parent);
		child = basename(new_file);
        printf("child: %s\n", child);
		pino = getino(dev,parent);
		pmip = iget(dev, pino);
		// create entry in new parent DIR with same inode number of old_file
		enter_name(pmip, oino, child);
	}
	else
	{
		return -1;
	}
    printf("in here\n");
	// (5). increment the i_links_count of INODE by 1
	omip->INODE.i_links_count++;
	omip->dirty = 1;
	// (6). write INODE back to disk      
	iput(omip);
	iput(pmip);
}

// unlinks a file
int unlink(char* filename)
{
	int ino, pino;
	char *child, *parent;
	MINODE *mip, *pmip;
	// (1). get pathname's INODE into memory
	ino = getino(dev, filename);
	if (ino == 0)
	{
		return -1;
	}
	mip = iget(dev, ino);
	// (2). verify it's a FILE (REG or LNK), can not be a DIR; 
	if (S_ISDIR(mip->INODE.i_mode))
	{
		return -1;
	}
	// (3). remove name entry from parent DIR's data block
	parent = dirname(filename);
	child = basename(filename);
	pino = getino(dev,parent);
	pmip = iget(dev, pino);
	rm_child(pmip, child);
	pmip->dirty = 1;
	iput(pmip);
	// (4). decrement INODE's link count by 1
	mip->INODE.i_links_count--;
	// (5). if i_links_count == 0 ==> rm pathname by deallocating its data blocks
	if (mip->INODE.i_links_count > 0)
	{
		mip->dirty = 1;
	}
	else
	{
		mip->refCount++;
		mip->dirty = 1;
		idalloc(dev, mip->ino);
	}
	iput(mip);
}

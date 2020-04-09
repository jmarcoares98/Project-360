// creates a file newFileName which has the SAME inode (number) as that of oldFileName
int link(char* old_file, char* new_file)
{
	int oino, pino;
	char *child, *parent;
	MINODE *omip, *pmip;
	// (1). get the INODE of /a/b/c into memory: mip->minode[ ]
	oino = getino(dev,old_file);
	if (oino == 0)
	{
		return -1;
	}
	omip = iget(dev, oino);
	// (2). check /a/b/c is a REG or LNK file (link to DIR is NOT allowed).
	if (S_ISDIR(omip->INODE.i_mode))
	{
		return -1;
	}
	// (3). creat new_file with the same inode number of old_file
	if (getino(dev,new_file) == 0)
	{
		parent = dirname(new_file);
		child = basename(new_file);
		pino = getino(dev,parent);
		pmip = iget(dev, pino);
		// create entry in new parent DIR with same inode number of old_file
		enter_name(pmip, oino, child);
	}
	else
	{
		return -1;
	}
	// (5). increment the i_links_count of INODE by 1
	omip->INODE.i_links_count++;
	omip->dirty = 1;
	// (6). write INODE back to disk      
	iput(dev,omip);
	iput(dev,pmip);
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
	rm_child(pmip, ino, child);
	pmip->dirty = 1;
	iput(dev,pmip);
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
		idealloc(dev, mip->ino);
	}
	iput(dev, mip);
}

// creates a symbolic link from newFileName to oldFileName
// symlink /a/b/c /x/y/z
void symlink(char *oldName, char *newName)
{
	int ino, new_ino;
	MINODE *mip, *new_mip;
	// ASSUME: oldName has <= 60 chars including the ending NULL byte
	// (1).verify oldNAME exists(either a DIR or a REG file)
	ino = getino(oldName);
	if (ino == 0)
		return -1;
	mip = iget(dev, ino);
	if (!S_ISDIR(mip->INODE.i_mode)) {
		if (!S_ISREG(mip->INODE.i_mode)) {
			printf("ERROR %s does not exist\n", oldName);
			return -1;
		}
	}
	// check newName not yet exist
	
	// (2).creat a FILE / x / y / z
	creat_file(newName);
	// (3).change / x / y / z's type to LNK (0120000)=(1010.....)=0xA...
	new_ino = getino(newName);
	new_mip = iget(dev, new_ino);
	new_mip->INODE.i_mode = 0120000;
	// (4).write the string oldNAME into the i_block[], which has room for 60 chars.
	// (INODE has 24 unused bytes after i_block[]. So, up to 84 bytes for oldNAME)
	new_mip->INODE.i_block[] = oldName;
	// set / x / y / z file size = number of chars in oldName
	new_mip->INODE.i_size = strlen(oldName);
	// (5).mark newFile parent minode dirty
	new_mip->dirty = 1;
	// (6).swrite the INODE of / x / y / z back to disk.
	iput(new_mip);
	iput(mip);
}

// reads the target fileName of a symbolic file and returns the contents
int readlink(char *pathname, char buf)
{
	int ino;
	MINODE* mip;
	// (1).get INODE of pathname into a minode[].
	ino = getino(dev, pathname);
	mip = iget(dev, ino);
	// (2).check INODE is a symbolic LNK file.
	if (mip->INODE.i_mode != 0120000) {
		printf("ERROR %s INODE is not symbolic link file", pathname);
		return -1;
	}
	// (3).copy filename from INODE.i_block[] into buffer
	strcpy(buf, mip->INODE.i_block[]);
	// (3).return its string contents in INODE.i_block[].
	
	// (4).return file size
	return;
}

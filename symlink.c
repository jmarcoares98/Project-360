// creates a symbolic link from newFileName to oldFileName
// symlink /a/b/c /x/y/z
void symlink(char *pathname)
{
	// ASSUME: oldName has <= 60 chars including the ending NULL byte
	// (1).verify oldNAME exists(either a DIR or a REG file)
	int ino;
	MINODE *mip;
	ino = getino(filename);
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

	// (4).write the string oldNAME into the i_block[], which has room for 60 chars.

	// (INODE has 24 unused bytes after i_block[].So, up to 84 bytes for oldNAME)

	// set / x / y / z file size = number of chars in oldName

	// (5).write the INODE of / x / y / z back to disk.

}

// reads the target fileName of a symbolic file and returns the contents
void readlink(char *pathname)
{
	int ino;
	MINODE* mip;
	// (1).get INODE of pathname into a minode[].
	ino = getino(dev, pathname);
	mip = iget(dev, ino);
	// (2).check INODE is a symbolic Link file.

	// (3).copy its string contents in INODE.i_block[].

	// (4).return file size
}

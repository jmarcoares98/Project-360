int access(char* pathname, char mode) // mode ='r', 'w', 'x' 
{
	int ino;
	MINODE* mip;

	// if running is SUPERuser process : return OK;
	if (running->uid == 0)
		return 1;

	// get INODE of pathname into memory;
	ino = getino(running->cwd, pathname);
	mip = iget(dev, ino);

	// if owner     : return OK if rwx --- --- mode bit is on
	if ((mip->INODE.i_mode & 0xF000) == OWNER) {
		if(mode == 'r' && (mip->INODE.i_mode & S_IRUSR))
			return 1;
		if(mode == 'w' && (mip->INODE.i_mode & S_IWUSR))
			return 1;
		if(mode == 'x' && (mip->INODE.i_mode & S_IXUSR))
			return 1;
	}

	// if same group : return OK if --- rwx --- mode bit is on
	else if ((mip->INODE.i_mode & 0xF000) == GROUP) {
		if (mode == 'r' && (mip->INODE.i_mode & S_IRGRP))
			return 1;
		if (mode == 'w' && (mip->INODE.i_mode & S_IWGRP))
			return 1;
		if (mode == 'x' && (mip->INODE.i_mode & S_IXGRP))
			return 1;
	}

	// if other : return OK if --- --- rwx mode bit is on
	else if ((mip->INODE.i_mode & 0xF000) == OTHER) {
		if (mode == 'r' && (mip->INODE.i_mode & S_IROTH))
			return 1;
		if (mode == 'w' && (mip->INODE.i_mode & S_IWOTH))
			return 1;
		if (mode == 'x' && (mip->INODE.i_mode & S_IXOTH))
			return 1;
	}

	// return NO;
	return 0;
}

int maccess(MINODE* mip, char mode) // mode ='r', 'w', 'x' 
{
	// if running is SUPERuser process : return OK;
	if (running->uid == 0)
		return 1;

	// if owner     : return OK if rwx --- --- mode bit is on
	if ((mip->INODE.i_mode & 0xF000) == OWNER) {
		if (mode == 'r' && (mip->INODE.i_mode & S_IRUSR))
			return 1;
		if (mode == 'w' && (mip->INODE.i_mode & S_IWUSR))
			return 1;
		if (mode == 'x' && (mip->INODE.i_mode & S_IXUSR))
			return 1;
	}

	// if same group : return OK if --- rwx --- mode bit is on
	else if ((mip->INODE.i_mode & 0xF000) == GROUP) {
		if (mode == 'r' && (mip->INODE.i_mode & S_IRGRP))
			return 1;
		if (mode == 'w' && (mip->INODE.i_mode & S_IWGRP))
			return 1;
		if (mode == 'x' && (mip->INODE.i_mode & S_IXGRP))
			return 1;
	}

	// if other : return OK if --- --- rwx mode bit is on
	else if ((mip->INODE.i_mode & 0xF000) == OTHER) {
		if (mode == 'r' && (mip->INODE.i_mode & S_IROTH))
			return 1;
		if (mode == 'w' && (mip->INODE.i_mode & S_IWOTH))
			return 1;
		if (mode == 'x' && (mip->INODE.i_mode & S_IXOTH))
			return 1;
	}

	// return NO;
	return 0;
}
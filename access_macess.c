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
	if ((mip->INODE->i_mode & 0xF000) == OWNER) {
		if(mode == 'r' && S_IRUSR(mip->INODE->i_mode))
			return 1;
		if(mode == 'w' && S_IWUSR(mip->INODE->i_mode))
			return 1;
		if(mode == 'x' && S_IXUSR(mip->INODE->i_mode))
			return 1;
	}

	// if same group : return OK if --- rwx --- mode bit is on
	else if ((mip->INODE->i_mode & 0xF000) == GROUP) {
		if (mode == 'r' && S_IRGRP(mip->INODE->i_mode))
			return 1;
		if (mode == 'w' && S_IWGRP(mip->INODE->i_mode))
			return 1;
		if (mode == 'x' && S_IXGRP(mip->INODE->i_mode))
			return 1;
	}

	// if other : return OK if --- --- rwx mode bit is on
	else if ((mip->INODE->i_mode & 0xF000) == OTHER) {
		if (mode == 'r' && S_IROTH(mip->INODE->i_mode))
			return 1;
		if (mode == 'w' && S_IWOTH(mip->INODE->i_mode))
			return 1;
		if (mode == 'x' && S_IXOTH(mip->INODE->i_mode))
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
	if ((mip->INODE->i_mode & 0xF000) == OWNER) {
		if (mode == 'r' && S_IRUSR(mip->INODE->i_mode))
			return 1;
		if (mode == 'w' && S_IWUSR(mip->INODE->i_mode))
			return 1;
		if (mode == 'x' && S_IXUSR(mip->INODE->i_mode))
			return 1;
	}

	// if same group : return OK if --- rwx --- mode bit is on
	else if ((mip->INODE->i_mode & 0xF000) == GROUP) {
		if (mode == 'r' && S_IRGRP(mip->INODE->i_mode))
			return 1;
		if (mode == 'w' && S_IWGRP(mip->INODE->i_mode))
			return 1;
		if (mode == 'x' && S_IXGRP(mip->INODE->i_mode))
			return 1;
	}

	// if other : return OK if --- --- rwx mode bit is on
	else if ((mip->INODE->i_mode & 0xF000) == OTHER) {
		if (mode == 'r' && S_IROTH(mip->INODE->i_mode))
			return 1;
		if (mode == 'w' && S_IWOTH(mip->INODE->i_mode))
			return 1;
		if (mode == 'x' && S_IXOTH(mip->INODE->i_mode))
			return 1;
	}

	// return NO;
	return 0;
}
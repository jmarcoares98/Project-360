// opens a file for read or write
// flags = 0|1|2|3 for READ|WRITE|RDWR|APPEND
int open_file(char* filename, char* flags)
{
	int ino, index, flag = -1, i, dev;
	MINODE* mip;
	INODE* ip;
	OFT* oftp;

	// 1. ask for a pathname and mode to open:
	if (strcmp(flags, "0") == 0)
		flag = 0;
	else if (strcmp(flags, "1") == 0)
		flag = 1;
	else if (strcmp(flags, "2") == 0)
		flag = 2;
	else if (strcmp(flags, "3") == 0)
		flag = 3;
	else {
		printf("INVALID!\n");
		return flag;
	}

	// 2. get pathname's inumber:
	if (filename[0] == '/') 
		dev = root->dev;         // root INODE's dev
	else 
		dev = running->cwd->dev;

	ino = getino(filename);
	if (ino == 0)
	{
		printf("ERROR: No such file!\n");
		return;
	}


	// 3. get its Minode pointer
	mip = iget(dev, ino);
	ip = &mip->INODE;

	// 4. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.
	if (!S_ISREG(ip->i_mode)) {
		printf("ERROR %s is not REG file\n", filename);
		iput(mip);
		return -1;
	}
	// check if file is ALREADY opened with INCOMPATIBLE mode (R is okay- W/RW/APPEND not okay)

	// 5. allocate a FREE OpenFileTable (OFT) and fill in values:
	oftp = (OFT*)malloc(sizeof(OFT));
	oftp->mode = flag;      // mode = 0|1|2|3 for R|W|RW|APPEND 
	oftp->refCount = 1;
	oftp->mptr = mip;  // point at the file's minode[]

	// 6. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:
	switch (flag){
	case 0:
		printf("%s OPEN FOR READ\n", filename);
		oftp->offset = 0;     // R: offset = 0
		break;
	case 1:
		printf("%s OPEN FOR WRITE\n", filename);
		truncate(mip);        // W: truncate file to 0 size
		oftp->offset = 0;
		break;
	case 2:
		printf("%s OPEN FOR RW\n", filename);
		oftp->offset = 0;     // RW: do NOT truncate file
		break;
	case 3:
		printf("%s OPEN FOR APPEND\n", filename);
		oftp->offset = mip->INODE.i_size;  // APPEND mode
		break;
	default:
		printf("IVALID\n");
		return(-1);
	}

	// 7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
	for (i = 0; i < 10; i++)
	{
		if (running->fd[i] == NULL)
			break;

	}
	index = i;
	printf("fd[i] = %d\n", index);

	// Let running->fd[i] point at the OFT entry
	running->fd[index] = oftp;

	// 8. update INODE's time field; R: touch atime, W|RW|APPEND: touch atime and mtime
	if(flag != 0)
		mip->dirty = 1;

	// 9. return index as the file descriptor
	return index;
}

int truncate(MINODE* mip)
{
	int buf[256];
	int buf2[256];
	int bnumber, i, j;
	// 1. release mip->INODE's data blocks;
	// a file may have 12 direct blocks, 256 indirect blocks and 256 * 256
	// double indirect data blocks. release them all.
	//deallocate for direct
	for (i = 0; i < 12; i++)
	{
		if (mip->INODE.i_block[i] != 0)
		{
			bdalloc(mip->dev, mip->INODE.i_block[i]);
		}
	}

	//Deallocate Indirect blocks
	if (mip->INODE.i_block[12] != 0)
	{
		get_block(dev, mip->INODE.i_block[12], (char*)buf);
		for (i = 0; i < 256; i++)
		{
			if (buf[i] != 0) { bdalloc(mip->dev, buf[i]); }
		}
		bdalloc(mip->dev, mip->INODE.i_block[12]);
		if (mip->INODE.i_block[13] != 0)
		{
			memset(buf, 0, 256);
			get_block(mip->dev, mip->INODE.i_block[13], (char*)buf);
			for (i = 0; i < 256; i++)
			{
				if (buf[i])
				{
					get_block(mip->dev, buf[i], (char*)buf2);
					for (j = 0; j < 256; j++)
					{
						if (buf2[j] != 0)  
							bdalloc(mip->dev, buf2[j]); 
					}
					bdalloc(mip->dev, buf[i]);
				}
			}
			bdalloc(mip->dev, mip->INODE.i_block[13]);
		}
	}

	// 2. update INODE's time field
	mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);

	// 3. set INODE's size to 0 and mark Minode[ ] dirty
    //deallocateInodeDataBlocks(dev,mip);
    mip->INODE.i_size = 0;
    mip->dirty = 1;
}

int close_file(char* pathname)
{
	int flag = -1;

	if (strcmp(pathname, "0") == 0)
		flag = 0;
	else if (strcmp(pathname, "1") == 0)
		flag = 1;
	else if (strcmp(pathname, "2") == 0)
		flag = 2;
	else if (strcmp(pathname, "3") == 0)
		flag = 3;
	else {
		printf("INVALID!\n");
		return flag;
	}

	my_close(fd);
	return;
}

// closes a file descriptor
int my_close(int fd)
{
	MINODE *mip;
	OFT *oftp;
	// 1. verify fd is within range.
	if (fd < 0 || fd >= 10)
	{
		printf("ERROR: OUT OF RANGE\n");
		return;
	}

	// 2. verify running->fd[fd] is pointing at a OFT entry
	if (running->fd[fd] == NULL)
	{
		printf("ERROR fd:%d is not OFT\n", fd);
		return -1;
	}

	// 3. The following code segments should be fairly obvious:
	oftp = running->fd[fd];
	running->fd[fd] = 0;
	oftp->refCount--;
	if (oftp->refCount > 0) 
		return 0;

	// last user of this OFT entry ==> dispose of the Minode[]
	iput(oftp->mptr);

	return 0;
}

// sets the offset in the OFT of an opened file descriptor to the byte position either from the file beginning
// (SEEK_SET) or relative to the current position (SEEK_CUR)
int mylseek(char *pathname, char*pathname2)
{
	OFT *oftp;
	int fd = atoi(pathname), position = atoi(pathname2);
	
	// From fd, find the OFT entry.
	oftp = running->fd[fd];
	// change OFT entry's offset to position but make sure NOT to over run either end
	// of the file.
	if (position < 0 || position > oftp->mptr->INODE.i_size)
	{
		printf("ERROR position:%d out of file bounds\n", position);
		return -1;
	}
	int originalPosition = oftp->offset;
	oftp->offset = position;
	// return originalPosition
	return originalPosition;
}

int pfd()
{
	// This function displays the currently opened files as follows:
	//  fd     mode    offset    INODE
	// ----    ----    ------   --------
	//  0     READ    1234   [dev, ino]  
	//  1     WRITE      0   [dev, ino]
	// ---------------------------------
	// to help the user know what files has been opened.

	int i; 
	OFT* ofpt;

	printf("\n  fd   mode   offset   device   inode \n");
	printf("  --   ----   ------   ------   ----- \n");
	for (i = 0; i < 10; i++){
		if (running->fd[i] == NULL)
			break;

		ofpt = running->fd[i];

		if (ofpt->refCount == 0)
			return;

		printf("  %d    ", i); // prints fd

		if (ofpt->mode == 0) // prints mode
			printf("RD");
		else if (ofpt->mode == 1)
			printf("WR");
		else if (ofpt->mode == 2)
			printf("RW");
		else if (ofpt->mode == 3)
			printf("AP");
		else
			printf("--");

		//prints the offset, dev and ino
		printf("    %6d     %2d     %5d\n", ofpt->offset, ofpt->mptr->dev, ofpt->mptr->ino);
	}

	printf("\n");
}

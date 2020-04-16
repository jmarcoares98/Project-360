// opens a file for read or write
// flags = 0|1|2|3 for READ|WRITE|RDWR|APPEND
int open_file(char* filename, int flags)
{
	int ino, index, flag;
	MINODE* mip;
	OFT* oftp;
	// 1. ask for a pathname and mode to open:

	// 2. get pathname's inumber:
	if (filename[0] == '/') 
		dev = root->dev;          // root INODE's dev
	else 
		dev = running->cwd->dev;

	ino = getino(dev, filename);
	if (ino == 0)
	{
		creat_file(filename);
		ino = getino(dev, filename);
	}

	// 3. get its Minode pointer
	mip = iget(dev, ino);

	// 4. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.
	if (!S_ISREG(mip->INODE.i_mode)) {
		printf("ERROR %s is not REG file\n", filename);
		return -1;
	}
	// check if file is ALREADY opened with INCOMPATIBLE mode (R is okay- W/RW/APPEND not okay)

	// 5. allocate a FREE OpenFileTable (OFT) and fill in values:
	oftp = (OFT*)malloc(sizeof(OFT));
	oftp->mode = flags;      // mode = 0|1|2|3 for R|W|RW|APPEND 
	oftp->refCount = 1;
	oftp->mptr = mip;  // point at the file's minode[]

	// 6. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:
	switch (oftp->mode){
	case 0:
		oftp->offset = 0;     // R: offset = 0
		break;
	case 1:
		truncate(mip);        // W: truncate file to 0 size
		oftp->offset = 0;
		break;
	case 2:
		oftp->offset = 0;     // RW: do NOT truncate file
		break;
	case 3:
		oftp->offset = mip->INODE.i_size;  // APPEND mode
		break;
	default:
		printf("invalid mode\n");
		return(-1);
	}

	// 7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
	int i;
	for (i = 0; i < 10; i++)
	{
		if (running->fd[i] = NULL)
			break;
	}
	index = i;
	printf("fd[i] = %d\n", index);

	// Let running->fd[i] point at the OFT entry
	running->fd[index] = oftp;

	// 8. update INODE's time field; R: touch atime, W|RW|APPEND: touch atime and mtime
	mip->dirty = 1;

	// 9. return index as the file descriptor
	return index;
}

int truncate(MINODE* mip)
{
	// 1. release mip->INODE's data blocks;
	// a file may have 12 direct blocks, 256 indirect blocks and 256 * 256
	// double indirect data blocks. release them all.

	// 2. update INODE's time field
	// mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
	// 3. set INODE's size to 0 and mark Minode[ ] dirty
	mip->INODE.i_size = 0;
	mip->dirty = 1;
}

// closes a file descriptor
int close_file(int fd)
{
	MINODE *mip;
	OFT *oftp;
	// 1. verify fd is within range.

	// 2. verify running->fd[fd] is pointing at a OFT entry
	if (running->fd[fd] == 0)
	{
		printf("ERROR fd:%d is not OFT\n", fd);
		return -1;
	}
	// 3. The following code segments should be fairly obvious:
	oftp = running->fd[fd];
	running->fd[fd] = 0;
	oftp->refCount--;
	if (oftp->refCount > 0) return 0;

	// last user of this OFT entry ==> dispose of the Minode[]
	mip = oftp->mptr;
	iput(mip);

	return 0;
}

// sets the offset in the OFT of an opened file descriptor to the byte position either from the file beginning
// (SEEK_SET) or relative to the current position (SEEK_CUR)
int lseek(int fd, int position)
{
	OFT *oftp;
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
}

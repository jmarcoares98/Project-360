// opens a file for read or write
// flags = 0|1|2|3 for READ|WRITE|RDWR|APPEND
int open_file(char *filename, int flags) 
{
	int ino, i;
	MINODE *mip;
	// q. ask for a pathname and mode to open:

	// 2. get pathname's inumber:
	if (pathname[0] == '/') 
		dev = root->dev;          // root INODE's dev
	else                  
		dev = running->cwd->dev;
	ino = getino(pathname);
	if (ino == 0)
	{
		creat(filename);
		ino = getino(filename);
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
	 oftp->mode = mode;      // mode = 0|1|2|3 for R|W|RW|APPEND 
         oftp->refCount = 1;
         oftp->minodePtr = mip;  // point at the file's minode[]

	// 6. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:
	switch(oftp->mode)
	{
		case 0 : 
			oftp->offset = 0;     // R: offset = 0
                  	break;
         	case 1 : 
			truncate(mip);        // W: truncate file to 0 size
                  	oftp->offset = 0;
                  	break;
         	case 2 : 
			oftp->offset = 0;     // RW: do NOT truncate file
                 	break;
         	case 3 : 
			oftp->offset =  mip->INODE.i_size;  // APPEND mode
                  	break;
         	default: 
			printf("invalid mode\n");
                  	return(-1);
	}
	
	// 7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
	
	
	// Let running->fd[i] point at the OFT entry
	fd[i] = &OFT;
	// 8. update INODE's time field; R: touch atime, W|RW|APPEND: touch atime and mtime

	mip->dirty = 1;
	// 9. return index as the file descriptor
	return i;
}

int truncate(MINODE* mip) 
{
	// 1. release mip->INODE's data blocks;
	// a file may have 12 direct blocks, 256 indirect blocks and 256 * 256
	// double indirect data blocks.release them all.

	// 2. update INODE's time field

	// 3. set INODE's size to 0 and mark Minode[ ] dirty
}

// closes a file descriptor
int close_file(int fd) 
{
	MINODE *mip;
	// 1. verify fd is within range.

	// 2. verify running->fd[fd] is pointing at a OFT entry
	
	// 3. The following code segments should be fairly obvious:
	//oftp = running->fd[fd];
	//running->fd[fd] = 0;
	//oftp->refCount--;
	//if (oftp->refCount > 0) return 0;

	// last user of this OFT entry ==> dispose of the Minode[]
	// mip = oftp->inodeptr;
	// iput(mip);

	return 0;
}

// sets the offset in the OFT of an opened file descriptor to the byte position either from the file beginning
// (SEEK_SET) or relative to the current position (SEEK_CUR)
int lseek(int fd, int position) 
{
	// From fd, find the OFT entry.

	// change OFT entry's offset to position but make sure NOT to over run either end
	// of the file.

	// return originalPosition
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


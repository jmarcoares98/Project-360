int open_file() {
	// ask for a pathname and mode to open:

	// get pathname's inumber:

	// if (pathname[0] == '/') dev = root->dev;          // root INODE's dev
	// else                  dev = running->cwd->dev;
	// ino = getino(pathname);

	// get its Minode pointer

	// check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.

	// allocate a FREE OpenFileTable (OFT) and fill in values:

	// Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:

	// find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
	// Let running->fd[i] point at the OFT entry

	// update INODE's time field

	// return i as the file descriptor
}

int truncate(MINODE* mip) {
	// 1. release mip->INODE's data blocks;
	//    a file may have 12 direct blocks, 256 indirect blocks and 256 * 256
	//	  double indirect data blocks.release them all.

	// 2. update INODE's time field

	// 3. set INODE's size to 0 and mark Minode[ ] dirty
}

int close_file(int fd) {
	// verify fd is within range.

	// verify running->fd[fd] is pointing at a OFT entry

	// The following code segments should be fairly obvious:
	//oftp = running->fd[fd];
	//running->fd[fd] = 0;
	//oftp->refCount--;
	//if (oftp->refCount > 0) return 0;

	// last user of this OFT entry ==> dispose of the Minode[]
	// mip = oftp->inodeptr;
	// iput(mip);

	// return 0;
}

int lseek(int fd, int position) {
	// From fd, find the OFT entry.

	// change OFT entry's offset to position but make sure NOT to over run either end
	// of the file.

	// return originalPosition
}

int pfd() {
	// This function displays the currently opened files as follows:
}


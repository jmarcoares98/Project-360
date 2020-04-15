int read_file(char *pathname) 
{
	int fd;
	// 1. Assume that fd is opened for READ.

	// 2. The offset in the OFT points to the current byte position in the file from
	//	  where we wish to read nbytes.

	// 3. To the kernel, a file is just a sequence of contiguous bytes, numbered from
	// 0 to file_size - 1. As the figure shows, the current byte position, offset
	// falls in a LOGICAL block(lbk), which is

	// the byte to start read in that logical block is 
	// start = offset % BLKSIZE

	// and the number of bytes remaining in the logical block is 
	// remain = BLKSIZE - start.

	// At this moment, the file has 
	// avil = file_size - offset

	// 4. myread() 
	// return (myread(fd, buf, nbytes));
}

// behaves EXACTLY the same as the read() system call in Unix / Linux.
// returns the actual number of bytes read
int myread(int fd, char *buf, int nbytes) 
{ 
	int count = 0;
	// offset = OFT.offset;
	// avil = fileSize - offset // number of bytes still available in file.
	char* cq = buf;                // cq points at buf[ ]

	//while (nbytes && avil) {

	//	Compute LOGICAL BLOCK number lbkand startByte in that block from offset;

	//	lbk = oftp->offset / BLKSIZE;
	//	startByte = oftp->offset % BLKSIZE;

	//	// I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT

	//	if (lbk < 12) {                     // lbk is a direct block
	//		blk = mip->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk
	//	}
	//	else if (lbk >= 12 && lbk < 256 + 12) {
	//		//  indirect blocks 
	//	}
	//	else {
	//		//  double indirect blocks
	//	}

	//	/* get the data block into readbuf[BLKSIZE] */
	//	get_block(mip->dev, blk, readbuf);

	//	/* copy from startByte to buf[ ], at most remain bytes in this block */
	//	char* cp = readbuf + startByte;
	//	remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]

	//	while (remain > 0) {
	//		*cq++ = *cp++;             // copy byte from readbuf[] into buf[]
	//		oftp->offset++;           // advance offset 
	//		count++;                  // inc count as number of bytes read
	//		avil--; nbytes--;  remain--;
	//		if (nbytes <= 0 || avil <= 0)
	//			break;
	//	}

	//	// if one data block is not enough, loop back to OUTER while for more ...

	//}

	// printf("myread: read %d char from file descriptor %d\n", count, fd);
	// return count;   // count is the actual number of bytes read
}

int mycat(char* pathname) {
	//char mybuf[1024], dummy = 0;  // a null char at end of mybuf[ ]
	//int n;

	// 1. int fd = open filename for READ;

	// 2. while (n = read(fd, mybuf[1024], 1024)) {
	//	mybuf[n] = 0;             // as a null terminated string
	//	// printf("%s", mybuf);   <=== THIS works but not good
	//	spit out chars from mybuf[] but handle \n properly;
	//}

	// 3. close(fd);
}

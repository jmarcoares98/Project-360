int read_file(char *pathname, char *pathname2) 
{
	int fd = 0, nbytes = 0, read = 0;
	OFT* oftp;
	MINODE* mip;
	INODE* ip;

	// ask for a fd  and  nbytes to read;
	fd = atoi(pathname);
	nbytes = atoi(pathname2);
	char buf[nbytes + 1];

	// verify that fd is indeed opened for RD or RW;
	if (strcmp(pathname, "") == 0) {
		printf("NO FD\n");
		return;
	}

	// verify that there is indeed bytes;
	if (strcmp(pathname2, "") == 0) {
		printf("NO BYTE\n");
		return;
	}

	// return(myread(fd, buf, nbytes));
	read = myread(fd, buf, nbytes);

	if (read < 0) {
		printf("CANT READ FILE\n");
		return;
	}

	return read;
}

// behaves EXACTLY the same as the read() system call in Unix / Linux.
// returns the actual number of bytes read
int myread(int fd, char *buf, int nbytes) 
{ 
	int count = 0, lbk, blk, avil, offset, startByte, *ip, dblk, remain;
	OFT* oftp = running->fd[fd]; 
	MINODE* mip = oftp->mptr;

	offset = oftp->offset;
	avil = mip->INODE.i_size - offset; // number of bytes still available in file.
	char* cq = buf;                // cq points at buf[ ]

	int ibuf[256], buf13[256], dbuf[256], readbuf[BLKSIZE];
	

	while (nbytes && avil) {

		//	Compute LOGICAL BLOCK number lbkand startByte in that block from offset;

		lbk = offset / BLKSIZE;
		startByte = offset % BLKSIZE;

		// I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT

		if (lbk < 12) {                     // lbk is a direct block
			blk = mip->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk
		}

		else if (lbk >= 12 && lbk < 256 + 12) { //  indirect blocks 
			// read INODE.i_block[12] into int ibuf[256];
			get_block(mip->dev, mip->INODE.i_block[12], ibuf);
			blk = ibuf[lbk - 12];;
		}

		else {
			lbk -= (12 + 256); // lbk count from 0 

			// 1. get i_block[13] into int buf13[256];  // buf13[ ] = |D0|D1|D2| ...... |
			get_block(mip->dev, mip->INODE.i_block[13], buf13);

			// 2. dblk = buf13[lbk / 256];
			dblk = buf13[lbk / 256];

			// 3. get dblk into int dbuf[256];          // dbuf[  ] = |256 block numbers|
			ip = buf13[dblk];
			get_block(mip->dev, *ip, dbuf);

			// 4. blk = dbuf[lbk % 256];
			blk = dbuf[lbk % 256];
		}

		/* get the data block into readbuf[BLKSIZE] */
		get_block(mip->dev, blk, readbuf);

		/* copy from startByte to buf[ ], at most remain bytes in this block */
		char* cp = readbuf + startByte;
		remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]

		while (remain > 0) {
			*cq++ = *cp++;             // copy byte from readbuf[] into buf[]
			oftp->offset++;           // advance offset 
			count++;                  // inc count as number of bytes read
			avil--; nbytes--;  
			remain--;
			if (nbytes <= 0 || avil <= 0)
				break;
		}

	//	// if one data block is not enough, loop back to OUTER while for more ...

	}

	 printf("myread: read %d char from file descriptor %d\n", count, fd);
	 return count;   // count is the actual number of bytes read
}

int mycat(char* pathname) {
	char mybuf[BLKSIZE], dummy = 0;  // a null char at end of mybuf[ ]
	int n, fd = 0;

	// 1. int fd = open filename for READ;
	fd = open_file(pathname, 0);

	// 2. while (n = read(fd, mybuf[1024], 1024)) {
	while (n = read(fd, mybuf, BLKSIZE)) {
		mybuf[n] = 0;             // as a null terminated string
		printf("%s", mybuf);
	}
	//	spit out chars from mybuf[] but handle \n properly;
	printf("\n\r");

	// 3. close(fd);
	close_file(fd);
}

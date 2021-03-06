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
	int count = 0, lbk, blk, avil, offset, startByte, dblk, remain, indblk, indoff;
	int* ip;
	OFT* oftp = running->fd[fd]; 
	MINODE* mip = oftp->mptr;

	offset = oftp->offset;
	avil = mip->INODE.i_size - offset; // number of bytes still available in file.
	char* cq = buf;                // cq points at buf[ ]
	char readbuf[BLKSIZE];
	
	while (nbytes && avil) {

		//	Compute LOGICAL BLOCK number lbk and startByte in that block from offset;

		lbk = offset / BLKSIZE;
		startByte = offset % BLKSIZE;

		// I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT

		if (lbk < 12) {                     // lbk is a direct block
			printf("READ DIRECT...\n");
			blk = mip->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk
		}

		else if (lbk >= 12 && lbk < 256 + 12) { //  indirect blocks 
			// read INODE.i_block[12] into int ibuf[256];
			printf("READ INDIRECT..\n");
			get_block(mip->dev, mip->INODE.i_block[12], readbuf);

			// set ip and put into block with the recieved indirect block from i_block[12]
			ip = (int*)readbuf + lbk - 12;
			blk = *ip;
		}

		else {
			printf("READ DOUBLE INDIRECT..\n");
			// get i_block[13] into int buf13[256];  // buf13[ ] = |D0|D1|D2| ...... |
			get_block(mip->dev, mip->INODE.i_block[13], readbuf);

			lbk -= (12 + 256); // lbk count from 0 

			indblk = (lbk) / 256; // double indirect block, blocks are in multiple of 256
			indoff = (lbk) % 256; // double indirect offset

			ip = (int*)readbuf + indblk; // set readbuf into an int pointer with added double indirect blocks

			get_block(mip->dev, *ip, readbuf); // get the ip with the double blocks into readbuf;         

			ip = (int*)readbuf + indoff; // set readbuf with indirect offset and then put into block;
			blk = *ip;
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
	char mybuf[BLKSIZE];  // a null char at end of mybuf[ ]
	int n, i, fd = 0;

	// 1. int fd = open filename for READ;
	fd = open_file(pathname, "0");
	pfd();

	if (fd < 0)
		return;

	// 2. while (n = read(fd, mybuf[1024], 1024)) {
	while (n = myread(fd, mybuf, BLKSIZE)) {
		i = 0;
		mybuf[n] = '\0';             // as a null terminated string
		while (mybuf[i]) {
			putchar(mybuf[i]);
			if (mybuf[i] == '\n')
				putchar('\r');

			i++;
		}
	}

	//	spit out chars from mybuf[] but handle \n properly;
	printf("\n\r");

	// 3. close(fd);
	my_close(fd);
}

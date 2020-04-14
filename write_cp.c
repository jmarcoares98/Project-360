int write_file()
{
	// 1. Preprations:
	// ask for a fdand a text string to write;

	// 2. verify fd is indeed opened for WR or RW or APPEND mode

	// 3. copy the text string into a buf[] and get its length as nbytes.

	// return(mywrite(fd, buf, nbytes));
}

int mywrite(int fd, char *buf, int nbytes)
{
	// while (nbytes > 0) {

	//	compute LOGICAL BLOCK(lbk) and the startByte in that lbk :

	//	lbk = oftp->offset / BLKSIZE;
	//	startByte = oftp->offset % BLKSIZE;

	//	// I only show how to write DIRECT data blocks, you figure out how to 
	//	// write indirect and double-indirect blocks.

	//	if (lbk < 12) {                         // direct block
	//		if (ip->INODE.i_block[lbk] == 0) {   // if no data block yet

	//			mip->INODE.i_block[lbk] = balloc(mip->dev);// MUST ALLOCATE a block
	//		}
	//		blk = mip->INODE.i_block[lbk];      // blk should be a disk block now
	//	}
	//	else if (lbk >= 12 && lbk < 256 + 12) { // INDIRECT blocks 
	//			 // HELP INFO:
	//		if (i_block[12] == 0) {
	//			allocate a block for it;
	//			zero out the block on disk !!!!
	//				// NOTE: you may modify balloc() to zero out the allocated block on disk
	//		}
	//		get i_block[12] into an int ibuf[256];
	//		blk = ibuf[lbk - 12];
	//		if (blk == 0) {
	//			allocate a disk block;
	//			record it in i_block[12];
	//		}
	//		.......
	//	}
	//	else {
	//		// double indirect blocks */
	//	}

	//	/* all cases come to here : write to the data block */
	//	get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]  
	//	char* cp = wbuf + startByte;      // cp points at startByte in wbuf[]
	//	remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

	//	while (remain > 0) {               // write as much as remain allows  
	//		*cp++ = *cq++;              // cq points at buf[ ]
	//		nbytes--; remain--;         // dec counts
	//		oftp->offset++;             // advance offset
	//		if (offset > INODE.i_size)  // especially for RW|APPEND mode
	//			mip->INODE.i_size++;    // inc file size (if offset > fileSize)
	//		if (nbytes <= 0) break;     // if already nbytes, break
	//	}
	//	put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk

	//	// loop back to outer while to write more .... until nbytes are written
	//}

	//mip->dirty = 1;       // mark mip dirty for iput() 
	//printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);
	//return nbytes;
}

int cp_file(char* pathname) {
	// 1. fd = open src for READ;

	// 2. gd = open dst for WR | CREAT;

	// NOTE:In the project, you may have to creat the dst file first, then open it
	// for WR, OR  if open fails due to no file yet, creat itand then open it
	// for WR.

	// 3. while (n = read(fd, buf[], BLKSIZE)) {
	//	write(gd, buf, n);  // notice the n in write()
	// }
}

int mv_file(char* pathname) {
	// 1. verify src exists; get its INODE in ==> you already know its dev
	// 2. check whether src is on the same dev as src

	//	CASE 1: same dev :
	// 3. Hard link dst with src(i.e.same INODE number)
	// 4. unlink src(i.e.rm src name from its parent directory and reduce INODE's
	//		link count by 1).

	//	CASE 2 : not the same dev :
	// 3. cp src to dst
	// 4. unlink src
}
// writes nbytes from buf in user space to an opened file descriptor and returns the actual number of bytes written
int write_file()
{
    char *fds, *str;
	// 1. Preprations:
	// ask for a fd and a text string to write;
    
    scanf("What file directory would you like to write to?\n", &fds);
    scanf("What string would you like to write to it?\n", &str);
    
    int fd = atoi(fds);
    
    if (fd < 0 || fd > 9)
    {
        printf("Invalid file\n");
        return -1;
    }
    // 2. verify fd is indeed opened for WR or RW or APPEND mode

    if(running->fd[fd] == NULL)
    {
        printf("No running fd\n");
        return -1;
    }
    
    if (running->fd[fd]->mode == 0)
    {
        printf("File is in read mode!\n");
        return -1;
    }
    
	// 3. copy the text string into a buf[] and get its length as nbytes.
    
    char new_buf[BLKSIZE];
    
    strcpy(fds, new_buf);
    
    int len = sizeof(new_buf);

	return(mywrite(fd, new_buf, len));
}

int mywrite(int fd, char *buf, int nbytes)
{
	int count = 0, lbk, blk, offset, startByte, remain, indblk, indoff, allocBlk;
	int* tip;
	OFT* oftp = running->fd[fd];
	MINODE* mip = oftp->mptr;
	INODE* ip = &mip->INODE;

	offset = oftp->offset;
	char* cq = buf, * cp;                // cq points at buf[ ]
	char writebuf[BLKSIZE];

    while(nbytes > 0)
    {
		// compute LOGICAL BLOCK (lbk) and the startByte in that lbk:
        lbk = offset/ BLKSIZE;
        startByte = offset % BLKSIZE;

        if (lbk < 12)	//direct block
        {
			printf("WRITE DIRECT...\n");

            if (ip->i_block[lbk] == 0) //if no data block yet
                ip->i_block[lbk] = balloc(dev); // MUST ALLOCATE a BLOCK
            
            blk = ip->i_block[lbk];  // blk should be a disk block now
        }

        else if (lbk >= 12 && lbk < 256+12){ // indirect blocks
			printf("WRITE INDIRECT...\n");
            if (ip->i_block[12] == 0){ //if no data block yet
                ip->i_block[12] = balloc(dev); // allocate a block for it;
				memset(writebuf, 0, BLKSIZE); //zero out the block on disk !!!!
				put_block(dev, ip->i_block[12], writebuf); // put 0 blocks in
            }
            get_block(mip->dev, ip->i_block[12], writebuf);

			// set tip and put into block with the recieved indirect blocks from i_block[12]
			tip = (int*)writebuf + lbk - 12;
			blk = *tip;

            if (blk==0){
				*tip = balloc(mip->dev); // allocate a disk block;
				blk = *tip; // record it in i_block[12];
            }
        }
        else{   // double indirect blocks
			printf("WRITE DOUBLE INDIRECT...\n");

			if (ip->i_block[13] == 0){ //if no data block yet
				ip->i_block[13] = balloc(dev); // MUST ALLOCATE a BLOCK

				get_block(dev, ip->i_block[13], writebuf);
				memset(writebuf, 0, BLKSIZE); //zero out the block on disk !!!!

				put_block(dev, ip->i_block[13], writebuf);
			}

            get_block(dev, ip->i_block[13], writebuf);
			lbk -= (12 + 256); // lbk count from 0 
            indblk = (lbk) / 256; // double indirect block, blocks are in multiple of 256
			indoff = (lbk) % 256; // double indirect offset

			tip = (int*)writebuf + indblk; // add double indirect block into writebuf and set to int pointer
			blk = *tip; // set it to block


			if (blk == 0){ // check if block needs allocation 
				allocBlk = balloc(dev); //allocate block
				tip = &allocBlk;	// set it to integer pointer
				blk = *tip;	// put integer pointer to block

				get_block(dev, blk, writebuf);
				memset(writebuf, 0, BLKSIZE); //zero out the block on disk !!!!

				put_block(dev, blk, writebuf);
			}
			get_block(dev, blk, writebuf);

			tip = (int*)writebuf + indoff; // add the offset into writebuf and set to integer pointer
			blk = *tip; // set it to block

			if (blk == 0){ // check if block needs more allocation
				allocBlk = balloc(dev); //more allocation of block
				tip = &allocBlk; // set it to integer pointer
				blk = *tip; // put integer pointer to block
				// does not need to zero out writebuf
				put_block(dev, blk, writebuf); 
			}
        }        
        //    /* all cases come to here : write to the data block */
        get_block(mip->dev, blk, writebuf);   // read disk block into wbuf[ ]
        cp = writebuf + startByte;      // cp points at startByte in wbuf[]
        remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

        while (remain > 0) {               // write as much as remain allows
            *cq++ = *cp++;              // cq points at buf[ ]
            nbytes--; remain--;         // dec counts
			count++;
            oftp->offset++;             // advance offset
            if (oftp->offset > mip->INODE.i_size)  // especially for RW|APPEND mode
                mip->INODE.i_size++;    // inc file size (if offset > fileSize)
            if (nbytes <= 0) break;     // if already nbytes, break
        }
        put_block(mip->dev, blk, writebuf);   // write wbuf[ ] to disk

		 // loop back to outer while to write more .... until nbytes are written
    }
	mip->dirty = 1;		 // mark mip dirty for iput() 
    printf("wrote %d char into file descriptor fd=%d\n", count, fd);
    return count;
}

int cp_file(char* source, char* dest)
{
	char buf[BLKSIZE];
	int n = 0, fd = 0, gd = 0, i = 0;
	// NOTE:In the project, you may have to creat the dst file first, then open it
	// for WR, OR  if open fails due to no file yet, creat it and then open it
	// for WR.
	check_file(dest);

	// 1. fd = open src for READ;
	fd = open_file(source, "0");

	// 2. gd = open dst for WR | CREAT;
	gd = open_file(dest, "1");

	pfd();

	while (n = myread(fd, buf, BLKSIZE))
    {
		mywrite(gd, buf, n);
    }
	printf("\n\r");
    
   my_close(fd);
   my_close(gd);
}

// checks for cp if file exists
int check_file(char *path)
{
	int ino;
	MINODE* tip;
	char pathname[128];
	strcpy(pathname, path);

	ino = getino(pathname);

	if (ino != 0) {		// the file exists
		printf("FILE EXISTS...\n");

		tip = iget(dev, ino);
		tip->INODE.i_mtime = time(0L);
		tip->dirty = 1;

		iput(tip);
		return;
	}
	else {		//file does not exist and we call creat to create file
		printf("CREATING FILE %s...\n", pathname);
		creat_file(pathname);
	}

	return;
}

//int mv_file(char* source, char* dest)
//{
//    int srcfd = open_file(source, '0');
//	// 1. verify src exists; get its INODE in ==> you already know its dev
//	// 2. check whether src is on the same dev as src
//    if (srcfd == -1)
//    {
//        printf("Wrong answer for file\n");
//        return -1;
//    }
//    
//    MINODE *fmip = running->fd[srcfd]->MINODE;
//	//	CASE 1: same dev :
//	// 3. Hard link dst with src(i.e.same INODE number)
//	// 4. unlink src(i.e.rm src name from its parent directory and reduce INODE's
//	//		link count by 1).
//    if (fmip->dev == fd)
//    {
//        link(source, dest);
//        unlink(source);
//    }
//    else
//    {
//        my_close(srcfd);
//        cp_file(source, dest);
//        unlink(source);
//    }
//	//	CASE 2 : not the same dev :
//	// 3. cp src to dst
//	// 4. unlink src
//}

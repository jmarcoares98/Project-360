// writes nbytes from buf in user space to an opened file descriptor and returns the actual number of bytes written
int write_file()
{
    char *fds, *str;
	// 1. Preprations:
	// ask for a fd and a text string to write;
    
    scanf("What file directory would you like to write to?\n", &fd);
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
    int count = 0, lbk, startByte, blk, dblk, remain, ibuf[256], dbuf[256], offset;
	char* wbuf[BLKSIZE], * cp, * cq = buf;

	OFT* oftp = running->fd[fd];
	MINODE* mip = oftp->mptr;
	offset = oftp->offset;

    while(nbytes > 0)
    {
		// compute LOGICAL BLOCK (lbk) and the startByte in that lbk:
        lbk = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;

		// I only show how to write DIRECT data blocks, you figure out how to 
		// write indirect and double-indirect blocks.
        if (lbk < 12)	//direct block
        {
            if (mip->INODE.i_block[lbk] == 0) //if no data block yet
                mip->INODE.i_block[lbk] = balloc(mip->dev);
            
            blk = mip->INODE.i_block[lbk];
        }
        else if (lbk >= 12 && lbk < 256+12){ // indirect blocks
            if (mip->INODE.i_block[12]==0){
				// allocate a block for it;
				// zero out the block on disk !!!!
                mip->INODE.i_block[12] = balloc(mip->dev);
                memset(ibuf,0,256);
            }
            get_block(mip->dev, mip->INODE.i_block[12], (char*)ibuf);
            blk = ibuf[lbk - 12];
			// NOTE: you may modify balloc() to zero out the allocated block on disk
            if (blk==0){
				// allocate a disk block;
                mip->INODE.i_block[lbk] = balloc(mip->dev);

				// record it in i_block[12];
                ibuf[lbk - 12] = mip->INODE.i_block[lbk];
            }
        }
        else{   // double indirect blocks
            memset(ibuf, 0, 256);
            get_block(mip->dev, mip->INODE.i_block[13], (char*)dbuf);
            lbk -= (12 + 256);
			dblk = dbuf[lbk % 256];
            get_block(mip->dev, dblk, (char*)dbuf);
            blk = dbuf[lbk % 256];
        }
        
        memset(wbuf,0,BLKSIZE);
        //read to buf
        get_block(mip->dev, blk, wbuf);
        cp = wbuf + startByte;
        remain = BLKSIZE - startByte;
        
        //    /* all cases come to here : write to the data block */
        get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]
        char* cp = wbuf + startByte;      // cp points at startByte in wbuf[]
        remain = BLKSIZE - startByte;     // number of BYTEs remain in this block
        char* cq = buf;

        while (remain > 0) {               // write as much as remain allows
            *cp++ = *cq++;              // cq points at buf[ ]
            nbytes--; remain--;         // dec counts
            oftp->offset++;             // advance offset
            if (offset > mip->INODE.i_size)  // especially for RW|APPEND mode
                mip->INODE.i_size++;    // inc file size (if offset > fileSize)
            if (nbytes <= 0) break;     // if already nbytes, break
        }
        put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk

		 // loop back to outer while to write more .... until nbytes are written
    }
	mip->dirty = 1;
    printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);
    return nbytes;
}

int cp_file(char* source, char* dest)
{
	// 1. fd = open src for READ;
    int fs = open_file(source, '0');
	// 2. gd = open dst for WR | CREAT;
    int gd = open_file(dest, '1');
	// NOTE:In the project, you may have to creat the dst file first, then open it
	// for WR, OR  if open fails due to no file yet, creat itand then open it
	// for WR.
    char buf[BLKSIZE];
    int n;

	while (n = read(fs, buf, BLKSIZE))
    {
		mywrite(gd, buf, n);  // notice the n in write()
    }
    
    close_file(fs);
    close_file(gd);
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

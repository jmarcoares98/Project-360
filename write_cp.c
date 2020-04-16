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
    
    char new_buf[];
    
    strcpy(fds, new_buf);
    
    int len = sizeof(new_buf);

	return(mywrite(fd, new_buf, len));
}

int mywrite(int fd, char *buf, int nbytes)
{
    int count = 0;
    
    int lbk, start, blk;
    
    oftp = running->fd[fd];
    mip - oftp->INODE;
    
    while(nbytes > 0)
    {
        lbk = oft->offset / BLKSIZE;
        start = oft->offset % BLKSIZE;
        
        if (lblk < 12)
        {
            if (mip->INODE.i_block[lblk] == 0)
            {
                mip->INODE.i_block[lblk] = balloc(mip->dev);
            }
            blk = mip->INODE.i_block[lblk];
        }
        // indirect blocks
        else if (lblk >= 12 && lblk < 256+12)
        {
            if (mip->INODE.i_block[12]==0)
            {
                mip->INODE.i_block[12] = balloc(mip->dev);
                memset(ibuf,0,256);
            }
            get_block(mip->dev, mip->INODE.i_block[12], (char*)ibuf);
            blk = ibuf[lmlk - 12];
            if (blk==0)
            {
                mip->INODE.i_block[lblk] = balloc(mip->dev);
                ibuf[lmlk - 12] = mip->INODE.i_block[lblk];
            }
        }
        // double indirect blocks
        else
        {
            memset(ibuf, 0, 256);
            get_block(mip->dev, mip->INODE.i_block[13], (char*)dbuf);
            lblk -= (12 + 256);
            get_block(mip->dev, dlbk, (char*dbuf));
            blk = dbuf[lblk % 256];
        }
        
        memset(writeBuf,0,BLKSIZE);
        //read to buf
        get_block(mip->dev, blk, writeBuf);
        cp = writeBuf + start;
        remain = BLKSIZE - start;
        
        if(remain < nbytes)
           {
             strncpy(cp, cq, remain);
             count += remain;
             nbytes -= remain;
             running->fd[fd]->offset += remain;
             //check offset
             if(running->fd[fd]->offset > mip->INODE.i_size)
             {
               mip->INODE.i_size += remain;
             }
             remain -= remain;
           }
           else
           {
             strncpy(cp, cq, nbytes);
             count += nbytes;
             remain -= nbytes;
             running->fd[fd]->offset += nbytes;
             if(running->fd[fd]->offset > mip->INODE.i_size)
             {
               mip->INODE.i_size += nbytes;
             }
             nbytes -= nbytes;
           }
           put_block(mip->dev, blk, writeBuf);
           mip->dirty = 1;
           printf("Wrote %d chars into file.\n", count);
    }
}

	//	/* all cases come to here : write to the data block */
	get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]
	char* cp = wbuf + startByte;      // cp points at startByte in wbuf[]
	remain = BLKSIZE - startByte;     // number of BYTEs remain in this block
    char* cq = buf;

    while (remain > 0) {               // write as much as remain allows
        *cp++ = *cq++;              // cq points at buf[ ]
        nbytes--; remain--;         // dec counts
        oftp->offset++;             // advance offset
        if (offset > INODE.i_size)  // especially for RW|APPEND mode
            mip->INODE.i_size++;    // inc file size (if offset > fileSize)
        if (nbytes <= 0) break;     // if already nbytes, break
    }
    put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk
	//	// loop back to outer while to write more .... until nbytes are written
	//}

	//mip->dirty = 1;       // mark mip dirty for iput() 
	//printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);
	//return nbytes;

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

	while (n = read(fs, buf[], BLKSIZE))
    {
		mywrite(gd, buf, n);  // notice the n in write()
    }
    
    my_close(fs);
    my_close(gd);
}

int mv_file(char* source, char* dest)
{
    int srcfd = open_file(source, '0');
	// 1. verify src exists; get its INODE in ==> you already know its dev
	// 2. check whether src is on the same dev as src
    if (srcfd == -1)
    {
        printf("Wrong answer for file\n");
        return -1;
    }
    
    MINODE *fmip = running->fd[srfd]->MINODE;
	//	CASE 1: same dev :
	// 3. Hard link dst with src(i.e.same INODE number)
	// 4. unlink src(i.e.rm src name from its parent directory and reduce INODE's
	//		link count by 1).
    if (fmip->dev == fd)
    {
        link(source, dest);
        unlink(source);
    }
    else
    {
        my_close(srcfd);
        cp_file(source, dest);
        unlink(source);
    }
	//	CASE 2 : not the same dev :
	// 3. cp src to dst
	// 4. unlink src
}

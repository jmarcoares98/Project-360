int link(MINODE* old_file, MINODE* new_file)
{
    oino = getino(old_file);
    
    if (oino == 0)
    {
        return -1;
    }
    
    omip = iget(dev, oino);
    
    if (S_ISDIR(omip->INODE.i_mode))
    {
        return -1;
    }
    
    if (getino(new_file) == 0)
    {
        parent = dirname(new_file);
        child = basename(new_file);
        pino = getino(parent);
        pmip = iget(dev, pino);
        // create entry in new parent DIR with same inode number of old_file
        enter_name(pmip, oino, child);
    }
    else
    {
        return -1;
    }
    
    omip->INODE.i_links_count++;
    omip->dirty = 1;
    iput(omip);
    iput(pmip);
    
}

int unlink(char* filename)
{
    ino = getino(filename);
    
    if (ino == 0)
    {
        return -1;
    }
    
    mip = iget(dev, ino);
    
    if (S_ISDIR(mip->INODE.i_mode))
    {
        return -1;
    }
    
    parent = dirname(filename);
    child = basename(filename);
    pino = getino(parent);
    pmip = iget(dev, pino);
    rm_child(pmip, ino, child);
    pmip->dirty = 1;
    iput(pmip);
    
    mip->INODE.i_links_count--;
    
    if (mip->INODE.i_links_count > 0)
    {
        iput(mip);
    }
    else
    {
        mip->refCount++;
        mip->dirty = 1;
        iput(mip);
        idealloc(dev, mip->ino);
    }
}


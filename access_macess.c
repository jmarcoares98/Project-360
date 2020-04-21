int access(char* pathname, char mode) // mode ='r', 'w', 'x' 
{
	/*if running is SUPERuser process : return OK;

	get INODE of pathname into memory;
	if owner     : return OK if rwx-- - -- - mode bit is on
		if same group : return OK if -- - rwx-- - mode bit is on
			if other : return OK if -- - -- - rwx mode bit is on

				return NO;*/
}

int maccess(MINODE* mip, char mode) // mode ='r', 'w', 'x' 
{
	// same as access() but work on mip
}
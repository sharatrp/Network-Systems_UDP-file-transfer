struct dirent * directtoryEntry;
DIR *dir = opendir(".");
int cn = 0;
char response[2000];

if(dir != NULL){
	while((directtoryEntry = readdir(dir)) != NULL)
	{
		cn += sprintf(response + cn, "%s\n",directtoryEntry -> d_name);
	}

	closedir(dir);
}
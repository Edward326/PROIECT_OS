#include<stdio.h>
#include"gitStruct.h"
#define gitSaves "localSaves"
#define gitSavesFile "metadata.bin"



void readFile(LocalDir *reff,struct dirent *i){
    reff->directoryName=i->d_name;

    char *path;
    if((path=malloc(strlen(gitSaves)+strlen(reff->directoryName)+strlen(gitSavesFile)+3)==NULL))return;
    strcat(path,gitSaves);strcat(path,"/");strcat(path,reff->directoryName);
    strcat(path,"/");strcat(gitSavesFile);path[strlen(path)]='\0';

    int file=open(path,O_RDONLY);


int sizeString;
read(file,reff->entryCount,sizeof(int));
reff->entry=malloc(sizeof(Entries)*reff->entryCount);
for(int i=0;i<reff->entryCount;i++)
{
    read(file,&sizeString,sizeof(int));
    reff->entry[i].fileName=malloc(sizeString+1);
    read(file,reff->entry[i].fileName,sizeString);   
  reff->entry[i].fileName[sizeString]='\0';

  reff->entry[i].metadata=malloc(sizeof(internalData));
    read(file,&reff->entry[i].metadata->inodeNo,sizeof(ino_t)); 
    read(file,&reff->entry[i].metadata->type,sizeof(mode_t)); 
    read(file,&reff->entry[i].metadata->totalSize,sizeof(off_t)); 
    read(file,&reff->entry[i].metadata->timeLastModiff,sizeof(struct timespec)); 
}
}
void writeFile(int file,LocalDir *newDir){
write(file,newDir->entryCount,sizeof(int));
for(int i=0;i<newDir->entryCount;i++)
{
    write(file,&strlen(newDir->entry[i].fileName),sizeof(int));   
    write(file,newDir->entry[i].fileName,strlen(newDir->entry[i].fileName));
    write(file,&newDir->entry[i].metadata->inodeNo,sizeof(ino_t)); 
    write(file,&newDir->entry[i].metadata->type,sizeof(mode_t)); 
    write(file,&newDir->entry[i].metadata->totalSize,sizeof(off_t)); 
    write(file,&newDir->entry[i].metadata->timeLastModiff,sizeof(struct timespec)); 
}
}
LocalDir **gitLoad(){//nu putem vect deoarece struct stat trebuie sa fie salvat de catre un * 
 struct stat trash;
    if(lstat(gitSaves,&trash)==-1)return NULL;//nu exista localSaves
  
  LocalDir **array=NULL;DIR *dir;dir=opendir(gitSaves);int index=0;
  while(struct dirent *i=readdir(dir);i;i=readdir(dir)){
    array=realloc(array,(++index)*sizeof(LocalDir*));
    array[index-1]=malloc(sizeof(LocalDir));
    readFile(array[index-1],i);
  }
return array;
}

void deleteDir(LocalDir *dirToDelete){
    struct stat trash;
    if(lstat(gitSaves,&trash)==-1)return;

    DIR *dir;dir=opendir(gitSaves);
    for(struct dirent *i=readdir(dir);i || strcmp(i->d_name,dirToDelete->directoryName);i=readdir(dir));
    closedir(dir);

    if(!i)return;//not finded

    char *path;
    if((path=malloc(strlen(gitSaves)+strlen(dirToDelete->directoryName)+2)==NULL))return;
    strcat(path,gitSaves);strcat(path,"/");strcat(path,dirToDelete->directoryName);path[strlen(path)]='\0';

    if(rmdir(path)){free(path);return;}
    free(path);
}
void writeDir(LocalDir *newDir) {
    struct stat trash;
    if (lstat(gitSaves, &trash) == -1) {
        if (mkdir(gitSaves, S_IRWXU | S_IRWXG | S_IRWXO) == -1)return;
    }

    char *path;
    if((path=malloc(strlen(gitSaves)+strlen(newDir->directoryName)+2)==NULL))return;
    strcat(path,gitSaves);strcat(path,"/");strcat(path,newDir->directoryName);path[strlen(path)]='\0';

    if (mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) == -1){free(path);return;}

    if(path=realloc(path,strlen(gitSaves)+strlen(newDir->directoryName)+strlen(gitSavesFile)+3)==NULL)return;
    strcat(path,"/");strcat(path,gitSavesFile);path[strlen(path)]='\0';

    int fileDesc;
    if((fileDesc=open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO))==-1){ free(path);return;}
    writeFile(fileDesc,newDir);
    close(fileDesc);
    free(path);
}

void gitWrite(LocalDir *newDir){
LocalDir **dataBase=gitLoad(),*copy;
    if((*copy=find(dataBase,newDir->directoryName))==NULL){
        writeDir(newDir);
    }else
    {
        deleteDir(newDir);
        writeDir(newDir);
    }
}

void makeLocal(char *dirToSaveName,LocalDir **dirToSave){
    //copiaza metadatele in pt si le incarca in db 
}

int gitinit(char *dirToSaveName,LocalDir **dirToSave){
    struct stat infoDir;
    if(lstat(dirToSaveName,&infoDir)==-1){
       return -1;
    }if(!S_IFDIR(infoDir.st_mode))return -1;
    LocalDir **dataBase=gitLoad();
    if((*dirToSave=find(dataBase,dirToSaveName))==NULL){
        makeLocal(dirToSaveName,dirToSave);
    }
}
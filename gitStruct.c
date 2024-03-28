#include<stdio.h>
#include"gitStruct.h"
#define gitSaves "localSaves"
#define gitSavesFile "metadata.bin"


Entries* newEntryRead(int file){
    Entries *elem=malloc(sizeof(Entries));
    
    int sizeString;
    read(file,&sizeString,sizeof(int));   
    read(file,elem->fileName,sizeString);
    read(file,&elem->metadata->inodeNo,sizeof(ino_t)); 
    read(file,&elem->metadata->type,sizeof(mode_t)); 
    read(file,&elem->metadata->totalSize,sizeof(off_t)); 
    read(file,&elem->metadata->timeLastModiff,sizeof(struct timespec));
    read(file,&elem->filesCount,sizeof(int));

    elem->next=NULL;
    if(elem->filesCount)
    {elem->next=malloc(sizeof(Entries*)*(elem->filesCount));}
    
    for(int i=0;i<elem->filesCount;i++){
     elem->next[i]=newEntryRead(file);
    }
    return elem;
}

void readFile(LocalDir *reff,struct dirent *i){

    char *path;
    path=malloc(strlen(gitSaves)+strlen(i->d_name)+strlen(gitSavesFile)+3);
    strcat(path,gitSaves);strcat(path,"/");strcat(path,i->d_name);
    strcat(path,"/");strcat(gitSavesFile);path[strlen(path)]='\0';

    int file=open(path,O_RDONLY);

reff->directoryName=i->d_name;
read(file,&reff->dirIdent,sizeof(ino_t));
read(file,&reff->entryCount,sizeof(int));
reff->entry=NULL;
if(reff->entry)
{reff->entry=malloc(sizeof(Entries)*(reff->entryCount));}
for(int i=0;i<reff->entryCount;i++)
    reff->entry[i]=*newEntryRead(file);

close(file);
}
//--------------------------------------------------------fct pt incarcarea dir versionate din folderul gitSaves intr un database de dir vrsionate(folosite de gitLoad())






void writeFileRecc(int file,Entries newDir){
     
    int size=strlen(newDir.fileName);
    write(file,&size,sizeof(int));   
    write(file,newDir.fileName,size);
    write(file,&newDir.metadata->inodeNo,sizeof(ino_t)); 
    write(file,&newDir.metadata->type,sizeof(mode_t)); 
    write(file,&newDir.metadata->totalSize,sizeof(off_t)); 
    write(file,&newDir.metadata->timeLastModiff,sizeof(struct timespec));
    write(file,&newDir.filesCount,sizeof(int));

    if(newDir.filesCount)
    {
        for(int i=0;i<newDir.filesCount;i++)
        writeFileRecc(file,*newDir.next[i]);
    }
}

void writeFile(int file,LocalDir *newDir){
write(file,newDir->dirIdent,sizeof(ino_t));
write(file,newDir->entryCount,sizeof(int));
for(int i=0;i<newDir->entryCount;i++)
   writeFileRecc(file,newDir->entry[i]);
}
//--------------------------------------------------------fct pt descarcarea noilor date despre dir versionat newDir in folderul gitSaves






LocalDir **gitLoad(int *size){
 struct stat trash;
    if(lstat(gitSaves,&trash)==-1)return NULL;//nu exista localSaves
  
  LocalDir **array=NULL;DIR *dir;dir=opendir(gitSaves);int index=0;
  struct dirent *i;
  while(i=readdir(dir)){
    array=realloc(array,(++index)*sizeof(LocalDir*));
    array[index-1]=malloc(sizeof(LocalDir));
    readFile(array[index-1],i);
  }
  *size=index;
return array;
}
//--------------------------------------------------------fct pt incarcarea tuturor dir urilor versionate intr un dataBase returneza un array de dir versionate,si specifica cate sunt in size






LocalDir *find(char *dirToFind){
    LocalDir **database;
    int size=0;
if( !(database=gitLoad(&size)))return NULL;

struct stat trash;
char *path=malloc(strlen(gitSaves)+strlen(dirToFind)+1);
strcat(path,gitSaves);strcat(path,"/");strcat(path,dirToFind);path[strlen(path)]='\0';

if(lstat(path,&trash)==-1)return NULL;//dir nu exista in folderul gitSaves
for(int i=0;i<size;i++){
    if(database[i]->dirIdent==trash.st_ino)
    return database[i];
}
//poate are acelasi nume dar iNode differit
return NULL;
}
//--------------------------------------------------------fct pt cautarea unui dir pentru a vedea daca el e versionat(il cauta in database generate de gitLoad())







void deleteDir(LocalDir *dirToDelete){
    struct stat trash;
    if(lstat(gitSaves,&trash)==-1)return;

    DIR *dir;dir=opendir(gitSaves);
    struct dirent *i;
    for(i=readdir(dir);i || strcmp(i->d_name,dirToDelete->directoryName);i=readdir(dir));
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
LocalDir *copy;

    if((copy=find(newDir->directoryName))==NULL){
        writeDir(newDir);
    }else
    {
        deleteDir(newDir);
        writeDir(newDir);
    }
}
//--------------------------------------------------------fct pt descarcarea noilor date in gitSaves(daca nu dir nu e versionat se creeaza prima vers a lui),daca nu se sterge si se inlocuiseste cu cea noua







internalData* newInternalData(char *path){
    struct stat info;
if(lstat(path,&info)==-1)return NULL;
internalData *newElem=malloc(sizeof(internalData));
newElem->inodeNo=info.st_ino;
newElem->type=info.st_mode;
newElem->totalSize=info.st_size;
newElem->timeLastModiff=info.st_mtime;
return newElem;
}

Entries* newFileEntry(char *path,char *filename){

    Entries elem=malloc(sizeof(Entries));
    elem->fileName=filename;
    elem->metadata=newInternalData(path);
    elem->next=NULL;
    elem->filesCount=0;

}

Entries* newEntry(char *path,char *filename){

    
    path=realloc(path,strlen(path)+strlen(filename)+2);
    strcat(path,"/");strcat(path,filename);path[strlen(path)]='\0';
    
    struct stat info;
    if(lstat(path,&info)==-1)return;

    if(S_ISREG(info.st_mode)){
        free(path);
      return newFileEntry(path,filename);
    }
    else
    if(S_ISDIR(info.st_mode)){
        Entries elem=malloc(sizeof(Entries));
       elem->fileName=filename;
       elem->metadata=newInternalData(path);
       elem->next=NULL;
       elem->filesCount=0;

    DIR *dir;
    if(!(dir=opendir(path))){free(path);return NULL;}

    int index=0;
    struct dirent *i;
    while(i=readdir(dir)){
    elem->next=realloc(elem->next,sizeof(Entries*)*(++index));
    elem->next[index-1]=newEntry(path,i->d_name);
    }
    elem->filesCount=index;
    closedir(dir);
    free(path);
    return elem;
    }
}

void loadCurrentDir(char *dirToSaveName,LocalDir *dirToSave){
    DIR *dir;if(!(dir=opendir(dirToSaveName)))return;

    int index=0;
    dirToSave->directoryName=dirToSaveName;
    struct stat inf;
       if(lstat(dirToSaveName,&inf)==-1)return;
    dirToSave->dirIdent=inf.st_ino;
    dirToSave->entry=NULL;

while(i=readdir(dir)){
      dirToSave->entry=realloc(dirToSave->entry,sizeof(Entries)*(++index));
      dirToSave->entry[index-1]=*newEntry(dirToSaveName,i->d_name);
}
dirToSave->entryCount=index;
}

void makeLocal(char *dirToSaveName,LocalDir **dirToSave){
    //copiaza metadatele in pt si le incarca in db
    *dirToSave=malloc(sizeof(LocalDir)); 
    loadCurrentDir(dirToSaveName,*dirToSave);
    gitWrite(*dirToSave);
}
//--------------------------------------------------------fct pt a incarca datele despre toate fisierele din dir local(nu se cauta in gitSaves),aici se vad ultimele modiff





//================================================================funct to be accesed outside the implemenation
int gitinit(char *dirToSaveName,LocalDir **dirToSave){
    struct stat infoDir;
    if(lstat(dirToSaveName,&infoDir)==-1){
       return -1;
    }if(!S_IFDIR(infoDir.st_mode))return -1;

    LocalDir *aux;

    if((*dirToSave=find(dirToSaveName))==NULL){
        makeLocal(dirToSaveName,dirToSave);
        return 1;
    }
    return 0;
}


LocalDir *gitcheck(LocalDir *dirToCheck)
{//to be created
}
int gitcommit(LocalDir *dirToCommit)
{//to be created
}
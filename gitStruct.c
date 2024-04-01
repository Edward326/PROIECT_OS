#include<stdio.h>
#include<string.h>
#include"gitStruct.h"
#include<stdarg.h>

//FCT DE CAUTARE A ELEMENTULUI IN BD E DEZACTIVATA ,fiidnca citirea elementelor i punerea lor in db ,pentru a fii citit ulterior pentru a se gasii daca folderul e versionat,citirea e corupta 
//newEntryRead e bufferata 
void printRec(Entries reff,int spacing){
   
     for(int j=0;j<spacing;j++)printf("\t");
     puts(reff.fileName);
      for(int j=0;j<spacing;j++)printf("\t");
printf("inode: %d\n", reff.metadata->inodeNo);
for(int j=0;j<spacing;j++)printf("\t");
printf("totalsize: %d\n", reff.metadata->totalSize);
for(int j=0;j<spacing;j++)printf("\t");
printf("lastmodiff: %d\n", reff.metadata->timeLastModiff);

     for(int j=0;j<spacing;j++)printf("\t");
    if(S_ISREG(reff.metadata->type))printf("type:file\n");
    else printf("type:dir\n");
     for(int j=0;j<spacing;j++)printf("\t");
    printf("filesContained:%d\n",reff.filesCount);
    if(reff.filesCount){
        for(int i=0;i<reff.filesCount;i++)
        printRec(*reff.next[i],strlen(reff.fileName)+spacing+1);
    }
}

void print(LocalDir *reff){
puts(reff->directoryName);
if(!reff->entryCount){printf("ID:%d\n",reff->dirIdent);printf("empty dir\n\n\n");}
else printf("ID:%d\n\n\n",reff->dirIdent);
for(int i=0;i<reff->entryCount;i++){
printRec(reff->entry[i],0);
printf("\n\n===============\n\n");
}

}
//----------------------------------------------------fct de viz a dir versionat





Entries* newEntryRead(int file){
    
    Entries *elem=malloc(sizeof(Entries));
    
    int sizeString;
    read(file,&sizeString,sizeof(int));//CORRUPTION!!
    elem->fileName=malloc(sizeString);   
    read(file,elem->fileName,sizeString);
    //puts(elem->fileName);

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
    strcpy(path,gitSaves);strcat(path,"/");strcat(path,i->d_name);
    strcat(path,"/");strcat(path,gitSavesFile);path[strlen(path)]='\0';
    
    int file=open(path,O_RDONLY);
    //lseek(file,0,SEEK_SET);

reff->directoryName=i->d_name;
read(file,&reff->dirIdent,sizeof(ino_t));
read(file,&reff->entryCount,sizeof(int));
reff->entry=NULL;
if(reff->entryCount)
{reff->entry=malloc(sizeof(Entries)*(reff->entryCount));
for(int i=0;i<reff->entryCount;i++)
    reff->entry[i]=*newEntryRead(file);
}
    
close(file);
}
//--------------------------------------------------------fct pt incarcarea dir versionate din folderul gitSaves intr un database de dir vrsionate(folosite de gitLoad())





void writeFileRecc(int file,Entries newDir){
     
    int size=strlen(newDir.fileName)+1;

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

write(file,&newDir->dirIdent,sizeof(ino_t));
write(file,&newDir->entryCount,sizeof(int));
for(int i=0;i<newDir->entryCount;i++)
  writeFileRecc(file,newDir->entry[i]);
}
//--------------------------------------------------------fct pt descarcarea noilor date despre dir versionat newDir in folderul gitSaves





LocalDir *find(char *dirToFind){//return NULL;
     struct stat trash,local;
    if(lstat(gitSaves,&trash)==-1)return NULL;//nu exista localSaves
    if(lstat(dirToFind,&local)==-1)return NULL;//nu exista localSaves
    DIR *dir;
    if(!(dir=opendir(gitSaves)))return NULL;

  struct dirent *i;
  char *path=NULL;
while((i=readdir(dir))){
     if(strcmp(i->d_name,".")==0 || strcmp(i->d_name,"..")==0)continue;
    path=malloc(strlen(gitSaves)+strlen(i->d_name)+strlen(gitSavesFile)+3);
    strcpy(path,gitSaves);strcat(path,"/");strcat(path,i->d_name);
    strcat(path,"/");strcat(path,gitSavesFile);path[strlen(path)]='\0';
    int fd=open(path,O_RDONLY);
    read(fd,&trash.st_ino,sizeof(ino_t));close(fd);
    if(trash.st_ino==local.st_ino){
        LocalDir *reff=malloc(sizeof(LocalDir));
        readFile(reff,i);
        free(path);
        closedir(dir);
        return reff;
    } 
    free(path);
  }
  closedir(dir);
return NULL;
}
//--------------------------------------------------------fct pt cautarea unui dir pentru a vedea daca el e versionat(il cauta in database generate de gitLoad())





void deleteDir(LocalDir *dirToDelete){
    struct stat trash;
    if(lstat(gitSaves,&trash)==-1)return;

    DIR *dir;dir=opendir(gitSaves);
    struct dirent *i;
    for(i=readdir(dir);i && strcmp(i->d_name,dirToDelete->directoryName);i=readdir(dir));
    closedir(dir);

    if(!i)return;//not finded

    char *path;
    if((path=malloc(strlen(gitSaves)+strlen(dirToDelete->directoryName)+2))==NULL)return;
    strcpy(path,gitSaves);strcat(path,"/");strcat(path,dirToDelete->directoryName);path[strlen(path)]='\0';

    rmdir(path);
    free(path);
}

void writeDir(LocalDir *newDir) {
    struct stat trash;
    if (lstat(gitSaves, &trash) == -1) {
        if (mkdir(gitSaves, S_IRWXU | S_IRWXG | S_IRWXO) == -1)return;
    }

    char *path;
    if((path=malloc(strlen(gitSaves)+strlen(newDir->directoryName)+2))==NULL)return;
    strcpy(path,gitSaves);strcat(path,"/");strcat(path,newDir->directoryName);path[strlen(path)]='\0';
      
      
    if (mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) == -1){free(path);return;}

if((path=realloc(path,strlen(gitSaves)+strlen(newDir->directoryName)+strlen(gitSavesFile)+3))==NULL){free(path);return;}
    strcat(path,"/");strcat(path,gitSavesFile);path[strlen(path)]='\0';
  //puts(path);

    int fileDesc;
    if((fileDesc=open(path, O_RDWR | O_CREAT | O_TRUNC, 111111111))==-1){ free(path);return;}
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
        deleteDir(copy);
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

 struct timespec ts;
    ts.tv_sec = info.st_mtime;
    ts.tv_nsec = 0;
    newElem->timeLastModiff=ts;

return newElem;
}

Entries* newFileEntry(char *path,char *filename){

    Entries *elem=malloc(sizeof(Entries));
    elem->fileName=malloc(strlen(filename)+1);
    strcpy(elem->fileName,filename);
    strcpy(elem->fileName,filename);
    elem->metadata=newInternalData(path);
    elem->next=NULL;
    elem->filesCount=0;
    return elem;
}

Entries* newEntry(char *pathOriginal,char *filename){
    
    char *path=malloc(strlen(pathOriginal)+strlen(filename)+2);
    strcpy(path,pathOriginal);strcat(path,"/");
    strcat(path,filename);path[strlen(path)]='\0';
    

    struct stat info;
    if(lstat(path,&info)==-1)return NULL;

    if(S_ISREG(info.st_mode)){
      Entries *reff=newFileEntry(path,filename);
      free(path);
      return reff;
    }
    else
    if(S_ISDIR(info.st_mode)){
        Entries *elem=malloc(sizeof(Entries));
       elem->fileName=filename;
       elem->metadata=newInternalData(path);
       elem->next=NULL;
       elem->filesCount=0;

    DIR *dir;
    if(!(dir=opendir(path))){free(path);return NULL;}

    int index=0;
    struct dirent *i;
    while((i=readdir(dir))){
         if(strcmp(i->d_name,".")==0 || strcmp(i->d_name,"..")==0)continue;
    elem->next=realloc(elem->next,sizeof(Entries*)*(++index));
    elem->next[index-1]=newEntry(path,i->d_name);
    }
    elem->filesCount=index;
    closedir(dir);
    free(path);
    return elem;
    }
    return NULL;
    
}

void loadCurrentDir(char *dirToSaveName,LocalDir *dirToSave){
    DIR *dir;if(!(dir=opendir(dirToSaveName)))return;

    int index=0;
    dirToSave->directoryName=dirToSaveName;
    struct stat inf;
       if(lstat(dirToSaveName,&inf)==-1)return;
    dirToSave->dirIdent=inf.st_ino;
    dirToSave->entry=NULL;
struct dirent *i;

while((i=readdir(dir))){
      if(strcmp(i->d_name,".")==0 || strcmp(i->d_name,"..")==0)continue;
      dirToSave->entry=realloc(dirToSave->entry,sizeof(Entries)*(++index));
      dirToSave->entry[index-1]=*newEntry(dirToSaveName,i->d_name);
}
dirToSave->entryCount=index;
closedir(dir);
}

void makeLocal(char *dirToSaveName,LocalDir **dirToSave){
    //copiaza metadatele in pt si le incarca in db
    *dirToSave=malloc(sizeof(LocalDir)); 
    loadCurrentDir(dirToSaveName,*dirToSave);
    //print(*dirToSave);
    gitWrite(*dirToSave);
}
//--------------------------------------------------------fct pt a incarca datele despre toate fisierele din dir local(nu se cauta in gitSaves),aici se vad ultimele modiff





//================================================================funct to be accesed outside the implemenation
int gitinit(char *dirToSaveName,LocalDir **dirToSave){
    struct stat infoDir;
    if(lstat(dirToSaveName,&infoDir)==-1){
       return -1;
    }if(!S_ISDIR(infoDir.st_mode))return -1;
    
    if((*dirToSave=find(dirToSaveName))==NULL){
        makeLocal(dirToSaveName,dirToSave);
        print(*dirToSave);
        return 1;
    }
    print(*dirToSave);
    return 0;
}

/*
int gitcommit(char *dirToSaveName,LocalDir *dirVersionated)
{//to be created
}
*/









//testunit
int main(int argv,char **argc){

LocalDir *base=NULL;
printf("%d",gitinit(argc[1],&base));

return 0;
}

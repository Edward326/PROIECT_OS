#include<stdio.h>
#include<string.h>
#include"gitStruct.h"
#include<stdarg.h>
#define maxDirToVers 10

void printRec(Entries reff,int spacing){

      printf("\n");
     for(int j=0;j<spacing;j++)printf("\t");
    printf("filename: \e[1;35m");
     puts(reff.fileName);
     
     printf("\e[1;37m");
     
      for(int j=0;j<spacing;j++)printf("\t");
printf("inode: \e[1;31m%d\n", reff.metadata->inodeNo);

printf("\e[1;37m");

for(int j=0;j<spacing;j++)printf("\t");
printf("totalsize: \e[1;32m%d\n", reff.metadata->totalSize);

printf("\e[1;37m");

for(int j=0;j<spacing;j++)printf("\t");
printf("lastmodiff: \e[4;37m");
 time_t t = reff.metadata->timeLastModiff.tv_sec;
    struct tm *local_time = localtime(&t);
    if (local_time != NULL) {
        char buffer[80];
        strftime(buffer, sizeof(buffer),"%Y-%m-%d %H:%M:%S", local_time);
        puts(buffer);
    } else {
        printf("Failed to convert timespec to date.\n");
    }

printf("\e[0;37m\e[1;37m");

     for(int j=0;j<spacing;j++)printf("\t");
    if(S_ISREG(reff.metadata->type))printf("type: \e[1;31mFILE\n");
    else printf("type: \e[1;31mDIR\n");
    printf("\e[1;37m");

     for(int j=0;j<spacing;j++)printf("\t");
    printf("filesContained:%d\n",reff.filesCount);
    
    if(reff.filesCount){
        for(int i=0;i<reff.filesCount;i++)
        printRec(*reff.next[i],spacing+2);
    }
}





//=======================================================functs to be accesed outside the implementation
void print(LocalDir *reff){
    printf("\n\n");
    if(!reff){perror("uninitilized DIR,maybe its NULL");return;}
    printf("\e[1;37m");printf("DirVersionated saved with name\e[1;32m ");
puts(reff->directoryName);printf("\e[1;37m");
if(!reff->entryCount){printf("ID:\e[1;31m%d\n",reff->dirIdent);printf("\e[1;33mempty dir\n\n\n");}
else printf("ID:\e[1;31m%d\e[1;37m\n\n\n",reff->dirIdent);
for(int i=0;i<reff->entryCount;i++){
printRec(reff->entry[i],0);
if(i+1!=reff->entryCount)
printf("\e[0;37m\n\n===============\n\n\e[1;37m");
else
printf("\n\n");
}
printf("\e[0;37m");
}
//----------------------------------------------------fct de viz a dir versionat
void freeLibRec(Entries *elem){
free(elem->fileName);elem->fileName=NULL;
free(elem->metadata);elem->metadata=NULL;

if(elem->filesCount){
for(int i=0;i<elem->filesCount;i++){
    freeLibRec(elem->next[i]);
    elem->next[i]=NULL;
}
free(elem->next);elem->next=NULL;
}

}
void freeLib(LocalDir **dir){
    //free((*dir)->directoryName);
    //printf("%s",(*dir)->directoryName);
    (*dir)->directoryName=NULL;
    for(int i=0;i<(*dir)->entryCount;i++){
        freeLibRec(&(*dir)->entry[i]);
    }
 free(*dir);
 *dir=NULL;
}





//=======================================================functs not to be accesed outside the implementation
Entries* newEntryRead(int file){
    
    Entries *elem=malloc(sizeof(Entries));
    elem->metadata=malloc(sizeof(internalData));
    
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

    char *pathFile;
    if((pathFile=malloc(strlen(gitSaves)+strlen(dirToDelete->directoryName)+strlen(gitSavesFile)+3))==NULL){free(pathFile);free(path);return;}
     strcpy(pathFile,gitSaves);strcat(pathFile,"/");strcat(pathFile,dirToDelete->directoryName);strcat(pathFile,"/");
     strcat(pathFile,gitSavesFile);pathFile[strlen(pathFile)]='\0';

    if(remove(pathFile))printf("eroare");
    rmdir(path);
    free(pathFile);
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
        freeLib(&copy);
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
       elem->fileName=malloc(strlen(filename)+1);
    strcpy(elem->fileName,filename);
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
    dirToSave->directoryName=malloc(strlen(dirToSaveName)+1);
    strcpy(dirToSave->directoryName,dirToSaveName);
    
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
int compare(Entries *newVers,Entries *oldVers){
if(strcmp(newVers->fileName,oldVers->fileName)){
        return 1;
}
 if(newVers->filesCount!=oldVers->filesCount)
        return 1;
if(newVers->metadata->totalSize!=oldVers->metadata->totalSize)
        return 1;
if(newVers->metadata->timeLastModiff.tv_sec!=oldVers->metadata->timeLastModiff.tv_sec)
        return 1;
if(newVers->filesCount)
{
    for(int i=0;i<newVers->filesCount;i++)
   return compare(newVers->next[i],oldVers->next[i]);
}

return 0;
}





//================================================================funct to be accesed outside the implemenation
int gitinit(char *dirToSaveName,LocalDir **dirToSave){
    struct stat infoDir;
    if(lstat(dirToSaveName,&infoDir)==-1){
       return -1;
    }if(!S_ISDIR(infoDir.st_mode))return -1;
    
    if((*dirToSave=find(dirToSaveName))==NULL){
        makeLocal(dirToSaveName,dirToSave);
        //print(*dirToSave);
        return 1;
    }
    //print(*dirToSave);
    return 0;
}

int gitcommit(char *dirToSaveName,LocalDir *dirVersionated)
{
    if(!dirVersionated)return 0;

    char *name=malloc(strlen(dirVersionated->directoryName)+1);
    strcpy(name,dirVersionated->directoryName);

    LocalDir *dirNewVers=malloc(sizeof(LocalDir));
    loadCurrentDir(dirToSaveName,dirNewVers);
    
    dirVersionated->directoryName=name;
   
    if(strcmp(dirNewVers->directoryName,dirVersionated->directoryName)){
        gitWrite(dirNewVers);
        return 1;}
        if(dirNewVers->entryCount!=dirVersionated->entryCount){
        gitWrite(dirNewVers);
        return 1;}
        for(int i=0;i<dirNewVers->entryCount;i++){
        if(compare(&dirNewVers->entry[i],&dirVersionated->entry[i])){
           puts(dirVersionated->entry[i].fileName);
        gitWrite(dirNewVers);
        return 1;}
        }
freeLib(&dirNewVers);  
return 0;
}











void versionate(char *argc,int view){
    if(view){
LocalDir *base=NULL;
int verify=gitinit(argc,&base);

printf("\n");

if(verify==-1){printf("dir doesnt exists/path not redirecting to a dir type file\n\n");return;}

if(verify==1){printf("dir versionat\n\n");return;}

else{
printf("dir already versionated,finding possible modifies\n\n");

if(gitcommit(argc,base)){
printf("\n\e[1;31mmodifies found\e[0;37m\ntype y/n to see changes:\t");
char opt;scanf("%c",&opt);printf("\n");
if(opt=='y'){
    printf("\n\n\e[1;31OLD VERS VERS\e[0;37m\n");
    print(base);
    printf("\n\n\e[1;31NEW VERS VERS\e[0;37m\n");
     LocalDir *dirNewVers=malloc(sizeof(LocalDir));
    loadCurrentDir(argc,dirNewVers);
    print(dirNewVers);
    freeLib(&dirNewVers);
}
else
printf("\n\n");
}
else
printf("\e[1;32mdir is clear,nothing to be modified\e[0;37m\n");
}
 printf("\n\n");
}
else
{
    LocalDir *base=NULL;
gitinit(argc,&base);
gitcommit(argc,base);
freeLib(&base);
}
}
int parc(char **argc,char *cargc,int stop){
 for(int j=0;j<stop;j++){
        if(strcmp(argc[j],cargc)==0)return 1;
    }
return 0;
}
//testunit
int main(int argv,char **argc){

if(argv>maxDirToVers+1){printf("too much arg to process");return 0;}
for(int i=1;i<argv;i++){
   if(parc(argc,argc[i],i))continue;//verificam sa nu mai existe acel arg in lista de arg
    versionate(argc[i],1);  //daca nu activeasa fct de versionare  
}

return 0;
}

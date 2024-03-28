#ifndef GITSTRUCT
#define GITSTRUCT

//lib de sistem
#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#define gitSaves "localSaves"
#define gitSavesFile "metadata.bin"

typedef struct{
    ino_t inodeNo;
    mode_t type;
    off_t totalSize;
    struct timespec timeLastModiff;   /* time of last modification */ 
}internalData;

typedef struct Entries{
char *fileName;
internalData *metadata;
struct Entries **next;
int filesCount;
}Entries;

typedef struct{
    char *directoryName;
    ino_t dirIdent;
    Entries *entry;
    int entryCount;
}LocalDir;


int gitinit(char *dirToSaveName,LocalDir **dirToSave);
//-1 nu exista directorul in calea curenta(unde se afla fisierul de unde sunt executate fct astea) sau fis nu e director
//0-daca fisierul e deja versionat 
//1-daca fisierul nu e versionat s-a salvat vers curenta a lui
//fct lucreaza pe dirToSave ,iar daca e cazul 0,1 va retine un pt catre directorul trackuit(la care i s a creat deja o vers/snapshot)
LocalDir *gitcheck(LocalDir *dirToCheck);
//!null =nu s-au gasit modiff
//null =s-au gasit modiff si  noul snapshot e returnat
int gitcommit(LocalDir *dirToCommit);
//1 daca s a salvat un snapshot diferit ,0daca nu s a detectat nicio schimbare
//combinat gitinit(argc[1],&dir);gitcommit(gitcheck(dir));vede daca dir argc[1] exista si e deja versionat daca nu il versioneaza 
#endif
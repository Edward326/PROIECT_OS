#ifndef GITSTRUCT
#define GITSTRUCT

#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<sys/wait.h>
#include<string.h>
#include<stdarg.h>
#define gitSaves "localSaves"
#define gitSavesFile "metadata.bin"

typedef struct{
    ino_t inodeNo;// id INODE
    mode_t type;  //tipu fisierului
    off_t totalSize; //DIMENSIUNEA TOTALA
    struct timespec timeLastModiff;   //(SEC) UKLTIMA DATA A MODIFFICARII FIS/DIR
}internalData;

typedef struct Entries{
char *fileName;                //numele fisierului
internalData *metadata;
struct Entries **next;         //daca e dir contine alte fisiere/dir
int filesCount;                //cate alte elemente contine(daca e dir)
}Entries;

typedef struct{
    char *directoryName;            //numele directorui versionat
    ino_t dirIdent;                 //id INODE ,nu se schimba nciioidata
    Entries *entry;                 //intrarile continute de el
    int entryCount;                 //nr de intrari
}LocalDir;


void print(LocalDir *reff);//prints the vers dir stored in reff 

int gitinit(char *dirToSaveName,LocalDir **dirToSave);
//-1 -nu exista directorul in calea curenta(unde se afla fisierul de unde sunt executate fct astea) sau fis nu e director
//0 -daca fisierul e deja versionat 
//1-daca fisierul nu e versionat s-a salvat vers curenta a lui
//fct lucreaza pe dirToSave ,in dirToSave se puine mereu dir gasit(nou creat/deja versionat)

int gitcommit(char *dirToSaveName,LocalDir *dirVersionated);
//1-daca s a salvat un snapshot diferit(s-au gasit modiff si s-a salvat noua vers automat) 
//0-daca nu s a detectat nicio schimbare

void versionate(char *argc,int typeOfview);
//typeofView 
//0- vers, explicit cu afisarea schimbarilor
//1- vers, fara afisarea schimbarilor,fara nicio afisare
#endif
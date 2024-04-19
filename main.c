#include<stdio.h>
#include<sys/sendfile.h>
#include"gitStruct.h"
#define maxDirToVers 10
#define isolatedDirName "isolatedFiles"

void deleteFile(char *path,char *filename){
    struct stat trash;
    if (lstat(isolatedDirName, &trash) == -1) {
        if (mkdir(isolatedDirName, S_IRWXU | S_IRWXG | S_IRWXO) == -1)return;
    }

char *isPath=malloc(strlen(isolatedDirName)+strlen(filename)+2);
    strcpy(isPath,isolatedDirName);strcat(isPath,"/");
    strcat(isPath,filename);isPath[strlen(isPath)]='\0';

int fileDesc,fileDescOrigin;
 if((fileDesc=open(isPath, O_RDWR | O_CREAT | O_TRUNC,111111111 ))==-1){ free(isPath);return;}
if((fileDescOrigin=open(path, O_RDONLY))==-1){{ free(isPath);close(fileDesc);return;}}

struct stat source_stat;
if(lstat(path,&source_stat)==-1){free(isPath);close(fileDesc);close(fileDescOrigin);return;}
if((sendfile(fileDesc, fileDescOrigin, NULL, source_stat.st_size))==-1){free(isPath);close(fileDesc);close(fileDescOrigin);return;}

free(isPath);close(fileDesc);close(fileDescOrigin);
if(remove(path))return;
}
void verifyEachFile(char *pathOriginal,char *filename){
    
    char *path=malloc(strlen(pathOriginal)+strlen(filename)+2);
    strcpy(path,pathOriginal);strcat(path,"/");
    strcat(path,filename);path[strlen(path)]='\0';
    
    struct stat info;
    if(lstat(path,&info)==-1)return;
    
    if(S_ISDIR(info.st_mode)){
        DIR *dir;
    if(!(dir=opendir(path))){free(path);return;}
    struct dirent *i;
    while((i=readdir(dir))){
         if(strcmp(i->d_name,".")==0 || strcmp(i->d_name,"..")==0)continue;
    verifyEachFile(path,i->d_name);
    }
    closedir(dir);
    }
     if(S_ISREG(info.st_mode)){
    if(info.st_mode==32768){//codul pentru 0 permisiuni u=000 g=000 o=000
        pid_t id=fork();
        if(!id){
            execl("checkIntegrity.sh","./checkIntegrity.sh",path,NULL);
        }
        else
        {int st;
            wait(&st);
            if(WEXITSTATUS(st)==255){//s a terminat cu -1,fis e malitios/virus(practic in binar 1111 1111=255 (adica-2+1)=-1)
            deleteFile(path,filename);}
        }
    }
     }
     free(path);
}
void checkMalitious(char *dirToCheck){
    DIR *dir;if(!(dir=opendir(dirToCheck))) return;
struct dirent *i;
while((i=readdir(dir))){
      if(strcmp(i->d_name,".")==0 || strcmp(i->d_name,"..")==0)continue;
      verifyEachFile(dirToCheck,i->d_name);
}
closedir(dir);
} 
int parc(char **argc,char *cargc,int stop){
 for(int j=0;j<stop;j++){
        if(strcmp(argc[j],cargc)==0)return 1;
    }
return 0;
}
int processOpener(int argv,char **argc){

if(argv>maxDirToVers+1 ||argv<2){printf("too much arg/less arg\n");exit(-1);}//daca sunt prea multe argumente

pid_t idProc;
for(int i=1;i<argv;i++){//daca sunt suff arg mergem la fieacre,daca nu apare inca odata si exista si e dir creeam un proces nou(in care vers dir respectiv),toate procesele astea facandu se in paralel
   
   if(parc(argc,argc[i],i))continue;//verificam sa nu mai existe acel arg in lista de arg
    struct stat infoDir;
    if(lstat(argc[i],&infoDir)==-1)continue;
    if(!S_ISDIR(infoDir.st_mode))continue;//verificam sa exsiste argumentul si sa fie director ca sa putem sa i creeam proces sa l versionam    
    if((idProc=fork())==-1){printf("error on fork\n");exit(-1);}
    
    if(!idProc){
        checkMalitious(argc[i]);
    versionate(argc[i],0);//doar daca suntem in fiu atunci il veriosnam si terminam procesul
    exit(0);//linia X
    }
}
int status;
wait(&status);//la final procesul tata(creator al tuturor celorlalte procese,in care se vers dir) primeste codul de terminare de la fiecare proces al lui,ca mai apoi sa le inchida

if(WIFEXITED(status)){//mereu va fii 0 datorita==>linia X

if(WEXITSTATUS(status))
printf("program terminated abnormally\n");
else
printf("program terminated succesfully\n");
}
else
printf("program terminated abnormally\n");

exit(0);
}










//CALCULAT:in paralel de 8x ori mai rapid
//fct cere dir sa le versioneze
//in fct main vom inititia un proces nou(mainProcess) in care vom apela fct processOpener in care vom versiona fiecare dintre dir primite ca arg in linia de cmd
//mainProcess va returna 255(adica -1) daca sunt mai multe arg decat max,sau fct fork a generat vreo eroare undeva
//mainProcess va returna  0 daca dir s-au versionat(daca dir nu se mai afla in lista,daca exista in calea curenta si daca e dir)
//daca se ret 0 insemana ca pt fiecare dir l am verficat:
//sa nu aiba fisiere malitioase
// daca nu e vers-->il vers
// daca e vers--> il verifica de o versiune mai noua,iar daca gaseste sterge statusul vechi si il incarca pe cel nou in localSaves
int main(int argv,char **argc){
clock_t start=clock();

pid_t pid=fork();
if(!pid)
exit(processOpener(argv,argc));

int status;
wait(&status);

clock_t end=clock();

if(WIFEXITED(status))
printf("\n\n\nmainProcess terminated with code:%d\ntotalExecTime:%f sec\n",WEXITSTATUS(status),((double) (end - start)) / CLOCKS_PER_SEC);
}

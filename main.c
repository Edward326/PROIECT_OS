#include<stdio.h>
#include"gitStruct.h"
#define maxDirToVers 10

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











//fct cere dir sa le versioneze
//in fct main vom inititia un proces nou(mainProcess) in care vom apela fct processOpener in care vom versiona fiecare dintre dir primite ca arg in linia de cmd
//mainProcess va returna 255(adica -1) daca sunt mai multe arg decat max,sau fct fork a generat vreo eroare undeva
//mainProcess va returna  0 daca dir s-au versionat(daca dir nu se mai afla in lista,daca exista in calea curenta si daca e dir)
//daca se ret 0 insemana ca pt fiecare dir l am verficat:
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
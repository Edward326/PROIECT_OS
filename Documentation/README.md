INSTRUCTIUNI DE FOLOSIRE:DUMMY VERSIONATING SYSTEM WITH THREAT FINDING CONTROL//OS

in linia de comanda se apeleaza numele executabilului pentru a se creea un proces,responsabil de a lansa programul de versionare in executie

in linia de comanda se specifica ./numeProgExec(ex in cel mai comun caz) urmat de argumente,care sunt directoarele pe care dorim in a le versiona

programul va returna direct pe ecran:

-->program terminated abnormally:
cand nr de dir pe care vrem sa le vers >maxDirToVers(macro) sau <1(niciunul)
cand din start avem un maxDirToVers mult prea mare,asftel incat s ar creea prea multe procese carora(cu fork),os nu mai are pid ramase sa le asigneze

-->program terminated succefully:
cand !conditia de la (program terminated abnormally)
practic aici se va duce peste fiecare dir in parte si ii creeaza un proces in care se va aplica algoritmul k
procesele in care se aplica alg K se ruleaza in paralel

algoritm K:
se aplica doar daca dir dat in linia de comanda,la care este atunci EXISTA si E fisier de tip DIR si nu s a mai dat inca odata de la tastatura
consta in :
1.curatarea dir de fis malitoase
2.versionarea acestuia(nemaiavand fis maltioase,practic ele daca sunt gasite sunt sterse)


1.curatarea dir de fis malitoase
se parcurge tot arborele de fis din directorul dat,in mod recursiv
daca e fisier il analizeaza daca merge recc pe fiecare intrare a lui
analizarea:
daca fis are file permissions !000(u+g+o) atunci il ignora
daca fis are file permissions 000 ii creeaza un nou proces care va lansa in executie checkIntegrity.sh(fis care verifica conditiile 1/2/3, care daca sunt adverate cel putin unul dintre ele (1||2||3), va scrie in pipe "CORRUPT" daca nu "SUCCES",iar procesul tata care l a creat(proc care se ocupa de alg K pe dir) citeste din pipeul respectiv ,iar daca s a intalnit CORRUPT,fis se muta in directorul prestabilit isolatedFiles si se sterge din dir curent si se contorizeaza nr de fis malitioase
ATENTIE! daca se gaseste ca are file permissions 000 ,fis e posibil sa fie un malware si deci e creeat procesul nepot(proc fiu),iar acesta va merge in paralel cu celelate parcurgeri de fis pe dir curent,iar pipe ul pe care scrie e diferit in fiecare proc nepot,pentru a evita suprascrierea de alte procese care au detectat fis malitioase

2.versionarea acestuia(nemaiavand fis maltioase,practic ele daca sunt gasite sunt sterse)
DIR STANDARD in care se salveza snapshotruile localSaves in care avem fiecare dir cu numele lui iar in inetrior avem data binara de la fiecare (metadata.bin),in care e prezenta structura arborelui de fisiere (se noteza la fiecare detaliile din INode-ul lui)
aici se virifica daca dir e vers,
-->NU,se versioneza
-->DA,se retine vechea versiune(din localSaves) intr o referinta si se face o referinta catre noua versiune(dir curent dat) si se compara cele doura,iar daca se gaseste o diferenta se salveaza noua versiune
capabil sa recunoasca:schimbarea numelui dir principal/dir din dir principal/fis din dir pricipal date modificate in fisier(size adaugat in plus) sau daca size ul e la fel ia ultima data de modiff a fisierului/dir
prin intermediul unui pipe scriemun continut la iesirea std si l redirectam catre capatul de scriere al pipeului(ca la verif de fis malitioase),continutul e daca dir e deja vers si daca s au gasit/nu modiff,s au nu era vers si s a versionat

procesul care aplica alg K va returna in procesul tata numarul de fis malitiose gasite ,iar proc tata care asteapta dupa toate procesle resposabile de versionare ,va insuma de la fieacare proces nr de fis maltioase gasite




exemplu de rulare:
./ex-->ret -1(eroare)
program terminated abnormally

./ex test1 test2-->ret 0(succes)
statusul pe fiecare dir dupa versionarea lui(daca dir exista nu s a mai dat spre versionare in procesul curent si e dir)





program terminated succefully
mainProcess terminated with a total of Malitious Files found: nr de fis malitoase gasite
totalExecTime: %f sec

<span style="color: red;">Red text</span>
ATENTIE:daca fis are permis 000 si nu se intalensc cazul 1,2 se va intalnii mereu cazul 3(BUG->nonascci char detector)

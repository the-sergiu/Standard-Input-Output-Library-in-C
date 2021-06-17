# Organizare

Abordarea generala a temei presupune implementarea generala a functiilor din biblioteca stdio.h. Implementarile C se folosesc de functiile specifice sistemului de operare in cauza, utilizand automat apelurile de sistem integrate. 
Implementarea are ca scop portabilitate, putand imita intr-o buna masura comportamentul bibliotecii stdio.h "nativa" (pentru functiile C implementate).
Tema mi s-a parut utila si revelatiorie, oferind o mai buna intelegere asupra operatiilor pe care le utilizam des, daca nu de fiecare data cand scriem cod.
Nu as putea sa ofer o estimare asupra implementarii mele, eu o consider demna, dar sunt convins ca ar putea exista optimizari ale spatiului/numarului de operatii pe care le-am omis.

## Implementare
Intregul enunt al temei a fost implementat *aproape* la perfectie (pe linux), si fara operatiile de Popen/Pclose pe Windows. O functionalitate extra pe care o are implementarea mea este utilizarea functiei din laboratorul 1 "xwrite" (pe linux), care m-a ajutat la solutionarea testului 19 pentru rand write. As fi putut integra si xread pentru usurarea propriei existente, dar era prea tarziu cand am realizat aceasta posibilitate :). 
Cumva consider ca au existat extraordinar de multe detalii care lipseau din cerinta pentru a-ti da seama de cum trebuie realizata, nespecificandu-se o sursa concreta de documentatie asupra comportamentului fiecarei functii. Desi am putut sa o duc la bun sfarsit, cerinta e extrem de vaga, ceea ce pe de-o parte ne permite libertate in implementare, dar care in acelasi timp ne face sa "pierdem" mult timp pana ne dam seama ce trebuie, din nou, uitandu-ne manual pe teste. Cumva, consider ca o tema bine explicata invalideaza aproape in totalitate nevoia de a te uita pe teste ca sa intelegi "ce se cere". Poate gandesc eu gresit.
Cea mai dificila functie de implemetnat a fost fread, care mi-a oferit multe batai de cap de la inceputul temei, pana la final. Mereu s-a intamplat sa am cate o problema cu cititul si a necesitat suficiente experimente din partea mea pentru o mai buna intelegere a cerintei.
Cred ca cea mai interesanta parte a temei a fost partea de popen si pclose, care au o functionalitate interesanta, cumva neintuitiva initial . 


## Compilare
Se linkeaza biblioteca codul bibliotecii Standard Input/Output, stdio.c (stdio.h - static), obtinand o biblioteca dinamica. Arhiva temei contine un makefile. Alternativ, se poate rula folosind comanda "make".
Prima linie va genera un fisier obiect bazat pe implementarile functiilor din biblioteca stdio. A doua linie va crea o biblioteca dinamica bazata pe acest fisier obiect (numita si biblioteca partajata).

Linux:
```C++
gcc -Wall -fPIC -c so_stdio.c -o so_stdio.o
gcc -Wall -shared so_stdio.o -o libso_stdio.so

OR

make GNUmakefile

```

Windows:
```C++
cl /nologo /W4 /D_CRT_SECURE_NO_WARNINGS /c /Foso_stdio.obj so_stdio.c
cl /nologo /LD /Feso_stdio.dll so_stdio.obj

OR

nmake [Makefile]

```

## Bibliografie
In principiu, nu am folosit un site anume pentru a procura informatiile, fiind mult prea multe cautari ca sa le includ pe toate :). 
Site-ul de SO, GeeksForGeeks, LinuxManualPage si StackOverflow au cam fost sursele principale de documentare.

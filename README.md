# peer2peer
Aplicatie de tipul peer-to-peer pentru partajarea fisierelor. Programul foloseste socket-uri pentru comunicatie, putind fi rulate pe masini diferite astfel: programul client poate fi rulat de catre orice utilizator, iar programul server(tracker-ul) poate fi rulat doar de autor, care este si responsabil cu pornirea/oprirea serviciului. Programul server este capabil sa serveasca simultan mai multe cereri de transfer provenite de la "colegii" sai din retea. Fisierele dorite pot fi cautate conform unor criterii (nume, tip, lungime etc.) in cadrul "retelei".



### 1. Pachete utilizate

- arpa/inet.h
- netinet/in.h
- stdio.h
- sys/types.h
- sys/socket.h
- sys/wait.h
- unistd.h
- stdlib.h
- string.h
- sqlite3.h
- pthread.h
- signal.h
- errno.h
- sqlite3 (folosit doar de tracker/serverul principal)
- pthread (doar pentru tracker si procesul de server aferent fiecarui client)
- utile.h

### 2. Compilare

Pentru compilare:

Fiecare client are nevoie de fisierul "utile.h",la fel si tracker-ul.

Intr-un folder separat ce contine tracker.c si Makefile-ul lui, al trackerului, rulati comanda make care va compila si porni procesul tracker.

Fiecare client are nevoie de un folder separat in care se alfa fisierele server.c si client.c.Motivul este reprezentat de faptul ca fiecare client va downloada/uploada fisiere in folderul din care se pornesc executabilele ./server si ./client
(intr-o situatie de uz real, clientii vor avea foldere diferite,deoarece ar rula pe masini diferite in retea)

Pentru fiecare client, executati comanda make.Makefile-ul compileaza client.c si server.c si apoi deschide procesul server aferent clientului.
Intr-o fereastra noua,puteti deschide acuma procesul client, cu comanda ./client <adresa_ip_tracker> <port_tracker>
Default,trackerul face bind pe portul 2908

**Felicitari!Tocmai v-ati conectat la tracker.**


### 3. Explicatii

Tracker-ul se foloseste de o baza de date in care retine informatii despre fisiere.
Avem nevoie ca in baza de date sa retinem portul peste care procesul server a facut bind.De aceea, procesul server scrie local intr-un fisier 'thisport' portul sau.Procesul client citeste apoi de acolo portul aferent serverlui lui.

Baza de date nu este persistenta.Aceasta este stearsa si reinitializata la fieacare excutie a serverului.

Toate operatiile intre procesul client si tracker se realizeaza prin TCP.Clientul preia input de la tastautura o comanda formata din mai multe cuvinte.Clientul sparge in cuvinte inputul si pune intr-un *arg[] folosind functia ParsareInput.
La arg[0] se va afla comanda. Daca aceasta nu este valida, se inchide procesul.

Comenzile pe care clientul le poate introduce sunt :

**upload filename**

Se verifica existenta fisierului 'filename' in folderul clientului.Daca aceasta nu exista,trackerul primeste eroare.
Se incepe procesul de upload.Clientul trimite la tracker numele fisierului,dimensiunea acestuia, ip-ul si portul pe care procesul lui server a facut bind si o lista de tip unsigned long ce reprezinta hash-ul celor dimensiune/BUFLEN pachete ce ar urma sa fie trimise unui coleg din retea.Clientul citeste binar din fisier cate BUFLEN caractere, apeleaza functia de hash pentru bucata citita de fisier si adauga numarul obtinut in lista.Este important ca lista sa fie trimisa prin TPC deoarece vrem sa asiguram mai incolo integritatea datelor trimise prin UDP de fiecare server mai mic.Acum trackerul insereaza in baza de date 'fisiere' o inregistrare de forma (id,numefisier,dimensiune,ip,port),unde id este nr de fisiere din baza de date, si scrie intr-un fisier lista de hashuri,sub forma "filenameipport.has".

**lista**

Serverul face o operatie SELECT peste tabela FISIERE din baza de date si ii trimite clientului lista de fisiere disponibile(care au fost uploadate pana acum de ceilalti clienti).

**search**:
Aceasta comanda poate fi de mai multe tipuri:

**search size1 size2**

Clientul trimite doua numere ce reprezinta dimensiunea in bytes a fisierelor cautate.Serverul ii raspunde cu toate numele de fisiere ce au dimensiunea cuprinsa intre size1 si size2

**search string**

Clientul trimite un string iar serverul raspunde cu toate inregistrarile care contin stringul respectiv in filename.

**search filename -exact**

Serverul trimite doar numele fisierelor care coincid cu filename


**download** :

**download filename ip port** 

Clientul trimite trackerului numele fisierului si primeste lista de hash-uri.Apoi trimite prin UDP numele fisierului la adresa si ip-ul din input.Acesta este un alt client din retea.Serverul-client are un proces care doar primeste numele fisierului.Apoi porneste un fir de excutie cu care retine adresa clientului si numele fisierului cerut de acesta .

Acum, serverul-client stie cui sa trimita si ce sa trimita,asa ca incepe sa construiasca datagrame.

Deschide si citeste din filename BUFLEN bytes si ii asigneaza un numar de ordine.Trimite,prin UDP, datagrame pana cand termina de citit.Clientul primeste datagramele,le verifica calculeaza hash-urile si le compara cu cele trimise de tracker.Cat timp nu a primit toate pachetele corecte,clientul va instantia un nou download de la server-client.Fiecare datagrama este scrisa pe hard imediat cum este primita,in urma procesului de verificare acestea fiind sterse.La sfarsit,se va reconstrui fisierul dorit din aceste datagrame si se vor sterge toate fisierele inutile din folderul clientului.


### 4. Bibliografie

https://profs.info.uaic.ro/~computernetworks/

https://www.ibm.com/support/knowledgecenter/SSLTBW_2.1.0/com.ibm.zos.v2r1.hala001/xsocudp.htm

https://stackoverflow.com/questions/1829187/udp-sockets-in-c

https://www.cs.hmc.edu/~geoff/classes/hmc.cs070.200101/homework10/hashfuncs.html

https://stackoverflow.com/questions/26387873/udp-server-how-can-do-that-each-thread-will-receive-the-right-clients-packet-a

http://www.cplusplus.com/reference/cstdio/fread/

https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm

https://en.wikipedia.org/wiki/Peer-to-peer

https://en.wikipedia.org/wiki/BitTorrent

https://qbforums.shiki.hu/index.php?topic=1768.0

http://www.anu.edu.au/people/Roger.Clarke/EC/P2POview.html






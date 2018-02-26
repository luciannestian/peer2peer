#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#define BUFLEN 4096
#define IP_SIZE 16
struct datagrama
{
	unsigned char date[BUFLEN];
	int ordine;
	int size;
};

void eroare(char *s);

void CitireTastatura(char *linie)
{
    fgets(linie ,300, stdin);
}

void PreluareFilename(char * filename)
{
	printf("Introdu numele fisierului cautat:\n");
    fgets(filename ,300, stdin);
}

void SendDatagram(struct sockaddr_in unde ,int fdsocket,struct datagrama pachet)
{
	//printf("In SendDatagram : %s \n",pachet.date);
	int slen = sizeof(unde);
 	if (sendto(fdsocket, &pachet, sizeof(pachet), 0, (struct sockaddr *)&unde, slen)==-1)
        eroare("sendto()");
}

struct datagrama ReceiveDatagram(struct sockaddr_in unde ,int fdsocket)
{
	struct datagrama pachet;
	int slen = sizeof(unde);
 	if (recvfrom(fdsocket, &pachet, sizeof(pachet), 0, (struct sockaddr *)&unde, &slen)==-1)
        eroare("receivefrom()");
	//printf("[receive datagram] %s\n",pachet.date);
	return pachet;
}

void SendACK(struct sockaddr_in unde ,int fdsocket,int ack)
{
	int slen = sizeof(unde);
 	if (sendto(fdsocket, &ack, sizeof(int), 0, (struct sockaddr *)&unde, slen)==-1)
        eroare("sendto() ack");
}

int ReceiveACK(struct sockaddr_in unde ,int fdsocket)
{
	int ack;
	int slen = sizeof(unde);
 	if (recvfrom(fdsocket, &ack, sizeof(int), 0, (struct sockaddr *)&unde, &slen)==-1)
        eroare("receivefrom() ack");
	//printf("[receive datagram] %s\n",pachet.date);
	return ack;
}


void Prompt()
{
	printf("Introdu o comanda:\n");
	printf("1.Lista de fisiere disponibile : lista\n");
	printf("2.Upload fisier : upload filename\n");
	printf("3.Search fisier dupa dimensiune: search min_size max_size\n");
	printf("3.Search fisier : search string \n");
	printf("3.Search fisier : search filename -exact \n");
	printf("4.Download fisier : download filename ip port\n");
	fflush(stdout);
}

FILE *DeschidereFisier(char *filename)
{
	FILE * fd;
  	fd = fopen (filename,"rb");	
  	if (fd!=NULL)
  	{
	 	return fd;
  	}
	else //eroare("deschidere fisier");
	return NULL;
}

int ObtainFileSize(FILE * fd)
{
	int fSize=0;
	fseek (fd , 0 , SEEK_END);
  	fSize = ftell (fd);
  	rewind (fd);
	return fSize;
}

int ObtainNrDatagrame(int nr)
{
	if(nr % BUFLEN == 0)
		return nr / BUFLEN ;
	return  nr / BUFLEN + 1;
	//trimitem cate trb
}

int DeleteFile(char *filename)
{
	if (remove(filename) == 0)
      return 1;//printf("Deleted successfully");
   else
      return 0;//printf("Unable to delete the file");
}

int ScriereFisier(FILE *fisier,char *date,int count)
{
	fwrite(date,strlen(date),1,fisier);
	return 1;
	/*if(count == fwrite(date,count,1,fisier))
		return 1;
	else {
		eroare("scris in fisier");
		return 0;
	}*/
}

int CitireFisier(FILE * fisier,char *date)
{
	return fread(date,1,BUFLEN,fisier);// read 10 bytes to our buffer
}

void ConstruiestePachetul(char *date,int nr_ordine,int count,struct datagrama *pachet)
{
	memset(pachet->date,0,BUFLEN);
	memcpy(pachet->date,date,count);
	pachet->ordine = nr_ordine;
	pachet->size = count;
}

unsigned long Hash(unsigned char *str)
{
	unsigned long hash = 5381;
	int c;
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	 return hash;
}
//in server trebuie scos fisier deoarece am deja datele citite
unsigned long * ConstruiesteListaHashuri(int nr_pachete,FILE *fisier)
{
	unsigned long *lista;
	lista = malloc(sizeof(long) * nr_pachete);
	unsigned char buf[BUFLEN];
	for(int i=0;i<nr_pachete;i++)
	{
		CitireFisier(fisier,buf);
		unsigned long hash = Hash(buf);
		//printf("IN CLIENT hash = %lu\n",hash);
		lista[i]=hash;
	}
	return lista;
}
void EliminaSpatiiInput(char input[])
{
   // printf("am intrat |%c| \n",input[strlen(input)-1]);
    for(int i=strlen(input)-1;input[i]==' ';i--)
        input[i]=0;//,printf("am gasit spatiu\n");
}

int ParsareInput(char *input,char *argv[])
{
    char *p;
    int len=0;
    
    p=strtok(input," ");
    //printf("Parsare baetiii : \n")\;
    while(p)
    {
        argv[len] = malloc(sizeof(char) * 25);
        strcpy(argv[len],p);
        
       // printf("%s\n",argv[len]);
        len++;
        p=strtok(NULL," ");
    }   
    argv[len]=NULL;

    argv[len-1][strlen(argv[len-1])-1]=0;
    return len;
}

void ReadTcp(int fd,void *buf,size_t nbytes)
{
	if (read (fd,buf,nbytes) <= 0)
    {
      perror ("Eroare la read() de la client.\n");
    }
}
void WriteTcp(int fd,void *buf,size_t nbytes)
{
	if (write (fd,buf,nbytes) <= 0)
   {
		perror ("[client]Eroare la write() spre server.\n");
    }
}

void FisierEvidentaHash(char *filename,unsigned long *lista,int nr_lista,char *ip,int port)
{
	char filename_has[42];
	memset(filename_has,0,34);
	strcpy(filename_has,filename);
	strcat(filename_has,ip);
	char *str= malloc(4);
	sprintf(str,"%d",port);
	strcat(filename_has,str);
	strcat(filename_has,".has");
	
	remove(filename_has);
	
	FILE * fisier = fopen(filename_has,"wb");

	fwrite(&nr_lista,sizeof(int),1,fisier);
	
	fwrite(lista,sizeof(unsigned long),nr_lista,fisier);
	fclose(fisier);
}

void NumeFisierHashOrd(char *filename,int ordine,char *filename2)
{
	char *filename_o = malloc(30+12);
	char *str = malloc(10);
	sprintf(str,"%d",ordine);
	memset(filename_o,0,42);
	strcat(filename_o,filename);
	strcat(filename_o,str);
	strcat(filename_o,".o");
	strcpy(filename2,filename_o);
}




int ExistaFisier(char *filename){

	if(access(filename,F_OK) != -1)
	{
		return 1;
	}
	return 0;
}

void eroare(char *s)
{
	perror(s);
	exit(1);
}

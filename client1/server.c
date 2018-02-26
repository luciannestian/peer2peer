/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
	Intoarce corect identificatorul din program al thread-ului.
  
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

#include "utile.h"

/* portul folosit */
#define PORT 3002

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	struct sockaddr_in client; //descriptorul intors de accept
    char filename[30];
}thData;

struct sockaddr_in server;	// structura folosita de server
int socket_server;	

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

int main ()
{

  int nr;		//mesajul primit de trimis la client 
	//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
    int i=0;
    
    struct sockaddr_in client;	
  char msg[30];		//mesajul primit de la client 
  char msgrasp[100]=" ";        //mesaj de raspuns pentru client 
  
  /* crearea unui socket */
  if ((socket_server = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }

  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&client, sizeof (client));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (socket_server, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }
    int port = ntohs(server.sin_port);
    remove("thisport");
    FILE * fisier_port = fopen("thisport","wb");
    
    fwrite(&port,sizeof(port),1,fisier_port);
    
    printf("Current port %d \n",port);


    fclose(fisier_port);
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {

      thData * td; //parametru functia executata de thread     
      int msglen;
      int length = sizeof (client);
    
      printf ("[server]Astept la portul %d...\n",PORT);
      fflush (stdout);

    bzero (msg, 30);

    if (recvfrom(socket_server, &msg, sizeof(msg), 0,(struct sockaddr *) &client, &length)==-1)
        eroare("recvfrom()");

   // bzero(td->filename,30);

      /* citirea mesajului primit de la client */
    int idThread; //id-ul threadului
 //   int cl; //descriptorul intors de accept
   // printf("[server]Crapa aicea!?\n");
    td=(struct thData*)malloc(sizeof(struct thData));	
    td->idThread=i++;
    memcpy((void *)&td->client,&client,sizeof(client));
    bzero(td->filename,0);
    strcpy(td->filename,msg);
    // //td->cl=client;

    // memcpy(td->filename,msg,30);
    // printf("[server]Crapa aicea!?\n");
    pthread_create(&th[i], NULL, &treat, td);

	}//while   
    close(socket_server); 
};				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - \n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
		return(NULL);	
  		
};


void raspunde(void *arg)
{
    int nr, i=0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
    printf("[server] serveste clientul %d\n",tdL.idThread);
   // printf("[server] in raspunde, %s\n",tdL.filename);
    char filename[30];
    strcpy(filename,tdL.filename);

    struct datagrama pachet;
    int slen = sizeof(tdL.client);
    char buf[BUFLEN];
    memset(pachet.date,0,BUFLEN);

    
    //printf("Serverul o sa trimita %s\n",tdL.filename);
    //memset(filename,0,30);
  //  memcpy(filename,tdL.filename,30);
  

    FILE * fisier_cerut;
    int deschidere_succes = 0;

    
    fisier_cerut = fopen(filename,"rb");

    if(fisier_cerut != NULL)
    {
        deschidere_succes =1;
    }
    else {
        deschidere_succes=0;
        perror("deschidere fisier server");//,eroare("deschidere fisier server");
    }
    if(deschidere_succes == 1)
    {
        fisier_cerut = DeschidereFisier(filename);//DeschidereFisier(filename);
         //obtinem si trimitem dim fisierului sub forma unui nr de datagrame
        int fSize = ObtainFileSize(fisier_cerut);
        int nr_datagrame = ObtainNrDatagrame(fSize);
        
     //printf("File size %d\nDeci nr pachete necesar %d\n",fSize,ObtainNrDatagrame(fSize));
        if (sendto(socket_server, &nr_datagrame, sizeof(int), 0, (struct sockaddr*)&tdL.client, slen)==-1)
              eroare("sendto()");

        printf("Incepem sa alcatuim %d datagrame \n",nr_datagrame);
        for(int i=0;i<nr_datagrame;i++)
        {
            int size=CitireFisier(fisier_cerut,buf);//fread(buf,1,BUFLEN,fisier_cerut);
            ConstruiestePachetul(buf,i,size,&pachet);
            
            //printf("[server] Pachetul are %d bytes\n",pachet.size);
            //printf("[server] Pachetul are %d nr de ordine \n",pachet.ordine);
            printf("[server] Datele:  \n");
            for(int i = 0; i<pachet.size; i++)
                printf("%u ", pachet.date[i]); // prints a series of bytes
            fflush(stdout);
            SendDatagram(tdL.client,socket_server,pachet);
            // int ack = 0,pasi = 0;
            // while(ack == 0 && pasi <= 5){
               
            //     ack = ReceiveACK(tdL.client,socket_server);
            //     pasi++;        
            // }
            
        }
        printf("Am trimis %s catre clientul cu %d\n",tdL.filename,tdL.idThread);
        fclose(fisier_cerut);
   
    }
    else {
        //n-am deschis ce trebuie, iesim
        printf("Trimit 0\n");
        if (sendto(socket_server, &deschidere_succes, sizeof(int), 0, (struct sockaddr*)&tdL.client, slen)==-1)
              eroare("sendto()");
    }

}

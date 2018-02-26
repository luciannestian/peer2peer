/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include "utile.h"


/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  char buf[512];
  char ip[IP_SIZE]="127.0.0.1";
  int port_server_mic=255;

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }
   int my_port=0;
   
   FILE *fisier_port = fopen("thisport","rb");
    
  fread(&my_port,sizeof(my_port),1,fisier_port);

    // remove("thisport");
    printf("[client] Am preluat %d ca fiind portul serverului meu\n",my_port);
        
  /* citirea mesajului */
  fflush (stdout);
//   read (0, buf, sizeof(buf));
//   nr=atoi(buf);
  //scanf("%d",&nr);
  
//  printf("[client] Am citit %d\n",nr);
    Prompt();
    while(1)
    {

    char *comanda = malloc(255);
    printf("Introduceti comanda : \n");
    fflush(stdin);
    fgets(comanda,255,stdin);
    
    EliminaSpatiiInput(comanda);
    
    char *arg[10];

    int nr_arg=ParsareInput(comanda,arg);
    int nr_comanda = -1;
    
    if(strcmp(arg[0],"lista") == 0)
      nr_comanda = 1;
    else if(strcmp(arg[0],"upload") == 0)
      nr_comanda = 2;
    else if(strcmp(arg[0],"search") == 0)
      nr_comanda = 3;
    else if(strcmp(arg[0],"download") ==0 )
      nr_comanda = 4;


    if (write (sd,&nr_comanda,4) <= 0)
       {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
     }
    if(nr_comanda == 1)
    {
        char *lista = malloc(2000);
        memset(lista,0,2000);
        if (read (sd,lista,2000) <= 0)
        {
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
       }
       printf("%s\n",lista);
       free(lista);
    }
    if(nr_comanda == 2)
    {
      if(nr_arg >= 2)
      {
        printf("[client] Face upload \n");
        fflush(stdout);
        char filename[30];
        memset(filename,0,30);
        strcpy(filename,arg[1]);
        
        FILE *fisier = DeschidereFisier(filename);
        if(fisier == NULL){
          memset(filename,0,30);
          strcpy(filename,"eroare");
        }
        printf("[client] Trimitem fisierul : %s\n",filename);
        if (write (sd,filename,30) <= 0)
       {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
        if(fisier == NULL)
          exit(1);
          /* trimiterea mesajului la server */

        /* citirea raspunsului dat de server 
          (apel blocant pina cind serverul raspunde) */
        unsigned long * lista;
        int nr_pachete = 1;
        lista = malloc(sizeof(long) * nr_pachete);
        
        int file_size = ObtainFileSize(fisier);
        nr_pachete = ObtainNrDatagrame(file_size);
        lista=ConstruiesteListaHashuri(nr_pachete,fisier);
        printf("[client] NR PACHETE%d\n",nr_pachete);
        if (write (sd,&nr_pachete,sizeof(int)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
        printf("[client] filesize e %d\n",file_size);
            if (write (sd,&file_size,sizeof(int)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }


        // printf("[client]Hashuri\n");
        // for(int i=0;i<nr_pachete;i++)
        //     printf("%lu : ",lista[i]);
        //trimitem nr_pachete,ca sa stie serverul cat pentru malloc
        
        //trimitem lista
        if (write (sd,lista,nr_pachete*sizeof(long)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }

       printf("size of lista %lu\n",sizeof(lista));

        if (write (sd,ip,sizeof(ip)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
      
       if (write (sd,&my_port,sizeof(int)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }


        /* inchidem conexiunea, am terminat */
        free(lista);//stergem lista de  hashuri, am trimis-o la serverul M
      }
      else {
          char *filename = malloc(30);
          memset(filename,0,30);
          strcpy(filename,"eroare");
        if (write (sd,filename,30) <= 0)
       {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
      return 1;
      }
    }
    else if(nr_comanda == 3)
    {
      if (write (sd,&nr_arg,sizeof(int)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
      if(nr_arg >= 3)
      {
        if (write (sd,arg[1],30) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
        if (write (sd,arg[1],10) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }

        if (write (sd,arg[2],10) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
        char *raspuns = malloc(2000);
        memset(raspuns,0,2000);
        if (read (sd,raspuns,2000) <= 0)
        {
          perror ("Eroare la read() client.\n");
        }	
        printf("%s\n",raspuns);
        free(raspuns);

      }else if(nr_arg == 2)
      {
        if (write (sd,arg[1],30) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
        char *raspuns = malloc(2000);
        memset(raspuns,0,2000);
        if (read (sd,raspuns,2000) <= 0)
        {
          perror ("Eroare la read() client.\n");
        }	
        printf("%s\n",raspuns);
        free(raspuns);
      }
      else return errno;
  }
  else if(nr_comanda == 4)
  {
    if(nr_arg >=4)
    {
    int PORT = atoi(arg[3]);
    int fileLen;
    
    struct sockaddr_in miniserver; // asta tre sa fie serverul la care ma duc adica serverA
    int socket_server, i, slen=sizeof(miniserver);
    unsigned char buf[BUFLEN];
      if (write (sd,arg[1],30) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
        if (write (sd,arg[2],IP_SIZE) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
      if (write (sd,&PORT,sizeof(int)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
    
     int nr_lista = 0;
      if (read (sd,&nr_lista,4) <= 0)
        {
          perror ("Eroare la read() client.\n");
        }	
      unsigned long *lista = malloc(sizeof(long)*nr_lista);
      if(read(sd, lista, sizeof(long)*nr_lista)<=0)
      {
        perror("eroare read la client\n");
      }
      printf("[client]lista are %d elemente\n",nr_lista);
      

    if ((socket_server=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
      eroare("socket");

    memset((char *) &miniserver, 0, sizeof(miniserver));

  //eu nu trebuie sa ma conectez la my_port !!!


    miniserver.sin_family = AF_INET;
    miniserver.sin_port = htons(PORT);
 
    if (inet_aton(arg[2], &miniserver.sin_addr)==0) {
      fprintf(stderr, "inet_aton() failed\n");
      exit(1);
    }

    //semnal ca sa stie serverul sa ne faca thread separat al nostru unic
    //semnal ca sa stie serverul sa ne faca thread separat al nostru unic

    char filename[30];
    memset(filename,0,30);
    strcpy(filename,arg[1]);
    
    struct datagrama pachet;

    if (sendto(socket_server, &filename, sizeof(filename), 0,(struct sockaddr *) &miniserver, slen) == -1)
        eroare("recv()");
          // a deschis serverlul fisierul dorit?
    int nr_datagrame=0;

    if ( (fileLen = recvfrom (socket_server, &nr_datagrame, sizeof(int) , 0 ,(struct sockaddr*)&miniserver, &slen)) < 0)
        eroare("recv client nr datagrame");

    //remove
    DeleteFile(filename);
    FILE * fisier = fopen(filename,"wb");

    printf("Nr datagrame %d \n",nr_datagrame);
    if(nr_datagrame != 0)
    {
      printf("[client]Incep sa descarc %d datagrame\n",nr_datagrame);
      char * filename_has = malloc(20);
      char *lista_aparitie = malloc(nr_datagrame);
      

      for(int i=0;i<nr_datagrame;i++)
      {
        //verificare
        pachet = ReceiveDatagram(miniserver,socket_server);
        NumeFisierHashOrd(filename,pachet.ordine,filename_has);
        FILE * fisier_has = fopen(filename_has,"wb");
        fwrite(&pachet.size,4,1,fisier_has);
        fwrite(pachet.date,pachet.size,1,fisier_has);
       // printf("%d\n%s",pachet.size,pachet.date);
        lista_aparitie[pachet.ordine]=1;
        fclose(fisier_has);

        //fwrite(pachet.date,pachet.size,1,fisier);
      }

      //sleep(1);

      for(int i=0;i<nr_datagrame-1;i++)
      {
          NumeFisierHashOrd(filename,i,filename_has);
          if(HashFromFile(filename_has,lista[i]) != 1)
            {
              lista_aparitie[i]=0;
              remove(filename_has);
            }
      } 
    
    while(!ListaVida(lista_aparitie,nr_datagrame))
    {
      if (sendto(socket_server, &filename, sizeof(filename), 0,(struct sockaddr *) &miniserver, slen) == -1)
        eroare("recv()");
          // a deschis serverlul fisierul dorit?

      if ( (fileLen = recvfrom (socket_server, &nr_datagrame, sizeof(int) , 0 ,(struct sockaddr*)&miniserver, &slen)) < 0)
        eroare("recv client nr datagrame");

      for(int i=0;i<nr_datagrame;i++)
      {
        //verificare
        pachet = ReceiveDatagram(miniserver,socket_server);
        if(lista_aparitie[pachet.ordine] == 0)
        {
          NumeFisierHashOrd(filename,pachet.ordine,filename_has);
          FILE * fisier_has = fopen(filename_has,"wb");
          fwrite(&pachet.size,4,1,fisier_has);
          fwrite(pachet.date,pachet.size,1,fisier_has);
          // printf("%d\n%s",pachet.size,pachet.date);
          lista_aparitie[pachet.ordine]=1;
          fclose(fisier_has);
        }
      }

      for(int i=0;i<nr_datagrame;i++)
      {
          NumeFisierHashOrd(filename,i,filename_has);
          if(HashFromFile(filename_has,lista[i]) != 1)
            {
              lista_aparitie[i]=0;
              remove(filename_has);
            }
      } 


    }





      printf("%d\n",ListaVida(lista_aparitie,nr_datagrame));
      char buf[BUFLEN];
      for(int i=0;i<nr_datagrame;i++)
      {
          memset(buf,0,BUFLEN);
          NumeFisierHashOrd(filename,i,filename_has);
          FILE * fisier_has = fopen(filename_has,"rb");
          int size = 0;
          fread(&size,sizeof(int),1,fisier_has);
          fread(buf,BUFLEN,1,fisier_has);
	fclose(fisier_has);
          fwrite(buf,size,1,fisier);
      }
        for(int i=0;i<nr_datagrame;i++)
      {
          memset(buf,0,BUFLEN);
          NumeFisierHashOrd(filename,i,filename_has);
          remove(filename_has);  
      }



      fclose(fisier);
    // printf("Am terminat de descarcat %s\n",filename);

     
      printf("Am terminat de descarcat %s\n",filename);

    }
    else {
      fclose(fisier);
      printf("Fisierul nu a fost gasit pe server!Bad file name!\n");
      DeleteFile(filename);
      fflush(stdout); 
      return 0;
      //TO DO mai departe de aicea
    }

    }
    else {
        char * eroare = "eroare";
         if (write (sd,eroare,30) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
        printf("Prea putin argumente la download \n");
        return 0;
    }
  }
    else if(nr_comanda == -1)
    {
      close(sd);
      return 1;
    }
  }

  close (sd);
}

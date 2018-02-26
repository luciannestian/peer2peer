
#include "utile.h"

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
int id_fisier = 0;
sqlite3 *db;
char *zErrMsg = 0;
int rc;


static int callback(void *NotUsed, int argc, char **argv, char **azColName) {

   int i;
   
   for(i = 0; i<argc; i++) {
      printf("%s ", argv[i] ? argv[i] : "NULL");
      strcat((char *)NotUsed,argv[i]);
      strcat((char *)NotUsed," ");
   }
   printf("\n");
   strcat((char *)NotUsed,"\n");
   return 0;
}

void InsertUploadFile(sqlite3 *db,char *zErrMsg,int rc,int id,char *filename,int size,char *ip,int port )
{
    char sql[255];
    memset(sql,0,255);
     /* Create SQL statement */
          // sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
          // "VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
          // "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
          // "VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "     \
          // "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
          // "VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
          // "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
          // "VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";
    strcat(sql , "INSERT INTO FISIERE(ID,FILENAME,SIZE,IP,PORT) VALUES(");

    char *str=malloc(25);
    sprintf(str, "%d", id);
    strcat(sql,str);
    strcat(sql,",'");

    strcat(sql,filename);
    strcat(sql,"',"); 
    
    memset(str,0,25);
    sprintf(str, "%d", size);
    strcat(sql,str);
    
    strcat(sql,",");


    strcat(sql,"'");
    strcat(sql,ip);
    strcat(sql,"',");
    
    memset(str,0,25);
    sprintf(str, "%d", port);
    strcat(sql,str);
    strcat(sql," );");
    printf("Comanda este %s \n",sql);
   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
   
   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "Upload cu succes\n");
   }
}



void ListaFisiere(char *data)
{
	char *sql ;
	/* Create SQL statement */
	sql = "SELECT FILENAME,SIZE,IP,PORT from FISIERE";

	/* Execute SQL statement */
  
	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);

	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	} else {
	//	fprintf(stdout, "Operation done successfully\n");
	}
}

void FindDimensiune(char *lower,char *upper,char *data)
{
  	char *sql = malloc(300);
    memset(sql,0,300);
	/* Create SQL statement */
	strcpy(sql,"SELECT FILENAME,SIZE,IP,PORT from FISIERE where size >= ");
  strcat(sql,lower);
  strcat(sql," and size <= ");
  strcat(sql,upper);

	/* Execute SQL statement */
  
	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);

	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	} else {
	//	fprintf(stdout, "Operation done successfully\n");
	}
}

void InterogareDownload(char *filename,char *ip,char *port,char *data)
{
  	char *sql = malloc(300);
    memset(sql,0,300);
	/* Create SQL statement */
	strcpy(sql,"SELECT SIZE from FISIERE where ip like '");
  strcat(sql,ip);
  strcat(sql,"' and port like '");
  strcat(sql,port);
  strcat(sql,"' and filename like '");
  strcat(sql,filename);
  strcat(sql,"'");
  printf("Interogarea: %s\n",sql);

	/* Execute SQL statement */
  
	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);

	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	} else {
	//	fprintf(stdout, "Operation done successfully\n");
	}
}

void FindString(char *string,char *data)
{
  	char *sql = malloc(300);
    memset(sql,0,300);
	/* Create SQL statement */
	strcpy(sql,"SELECT FILENAME,SIZE,IP,PORT from FISIERE where filename like '%");

  strcat(sql,string);
  strcat(sql,"%'");
 // printf("[server] Comanda dorita este %s\n",sql);
	/* Execute SQL statement */
  
	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);

	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	} else {
	//	fprintf(stdout, "Operation done successfully\n");
	}
}

void FindNume(char *string,char *data)
{
  	char *sql = malloc(300);
    memset(sql,0,300);
	/* Create SQL statement */
	strcpy(sql,"SELECT FILENAME,SIZE,IP,PORT from FISIERE where filename like '");

  strcat(sql,string);
  strcat(sql,"'");
  
  printf("[server] Comanda dorita este %s\n",sql);
	/* Execute SQL statement */
  
	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);

	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	} else {
	//	fprintf(stdout, "Operation done successfully\n");
	}
}



int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;

  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
  //deschidere baza date
  remove("baza.db"); //nu ne intereseaza ca baza sa ramana pe mai multe rulari... 
  
  char *sqlfisiere,*sqlhashuri;

  /* Open database */
   rc = sqlite3_open("baza.db", &db);

  if( rc ) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    return(0);
  }
  else {
    fprintf(stdout, "Opened database successfully\n");
  }
  sqlfisiere = "CREATE TABLE FISIERE("  \
         "ID INT PRIMARY KEY     NOT NULL," \
         "FILENAME           TEXT    NOT NULL," \
         "SIZE            INT     NOT NULL," \
         "IP        CHAR(16)," \
         "PORT         CHAR(2) );";

 /* Execute SQL statement */
   rc = sqlite3_exec(db, sqlfisiere, callback, 0, &zErrMsg);
   
  if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "Table created successfully\n");
   }

   sqlhashuri = "CREATE TABLE HASHURI("  \
         "ID INT PRIMARY KEY     NOT NULL," \
         "HASH        INT," \
         "ORDINE         INT );";
  
 /* Execute SQL statement */
   rc = sqlite3_exec(db, sqlhashuri, callback, 0, &zErrMsg);
   
  if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "Table created successfully\n");
   }


  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
 
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

	//client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	int idThread; //id-ul threadului
	int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;

	pthread_create(&th[i], NULL, &treat, td);	      
				
	}//while    
};				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
		return(NULL);	
};


void raspunde(void *arg)
{
	struct thData tdL;
  
	tdL= *((struct thData*)arg);
  int ok =0;
  while(ok==0)
  {
    int nr_comanda = -1;

    if (read (tdL.cl,&nr_comanda,sizeof(int)) <= 0)
        {
          printf("[Thread %d]\n",tdL.idThread);
          perror ("Eroare la read() de la client.\n");
        }	  
    printf("[server] Comanda este %d\n",nr_comanda);
    if(nr_comanda == -1){
      //printf("Comanda nerecunoscuta.Intrerupem conexiunea");
      fflush(stdout);
      ok=1;
      return;
    }
    if(nr_comanda == 1)
    {
       // Find();
        char *raspuns = malloc(2000);
        memset(raspuns,0,2000);
        sprintf(raspuns,"filename  size  ip   port\n");
        ListaFisiere(raspuns);

        printf("[server] trimitem %s",raspuns);
        
        if (write (tdL.cl,raspuns,2000) <= 0)
        {
          printf("[Thread %d]\n",tdL.idThread);
          perror ("Eroare la write() de la client.\n");
        }	
        free(raspuns);

    }
    else if (nr_comanda == 2)
    {
      int nr_pachete = 0;
      int size_fisier=0;
      char filename[30];
      memset(filename,0,30);

      ReadTcp(tdL.cl,filename,30);

      if(strcmp(filename,"eroare") == 0 )
        {
          printf("[server] intru in if,trebuie sa dau return\n");
          return ;
        }
      printf("[serverul]CONTINUA!\n");
      if (read (tdL.cl, &nr_pachete,sizeof(int)) <= 0)
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
      }
      printf("[sever] NR PACHETE%d\n",nr_pachete);
      if (read (tdL.cl, &size_fisier,sizeof(int)) <= 0)
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
      }
      printf("[server] FILE SIZE %d \n",size_fisier);
      // lista = malloc(sizeof(long) * nr_pachete + 100);
      unsigned long *lista = malloc(sizeof(unsigned long) * nr_pachete);

      if (read (tdL.cl, lista,sizeof(long)*nr_pachete) <= 0)
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
      }
     
      printf("[server]Hashuri\n");
      printf("[sever] lista hashuri\n");

      for(int i=0;i<nr_pachete;i++)
        printf("%lu : ",lista[i]);
      printf("\n\n");

      char ip_client[IP_SIZE];
      int port_client;

      if (read (tdL.cl, ip_client,sizeof(ip_client)) <= 0)
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
      }
      printf("Am primit ip-ul %s\n",ip_client);

      if (read (tdL.cl, &port_client,sizeof(int)) <= 0)
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
      }

      printf("Am primit portul %d\n",port_client);
      //inseram in baza de date ,tabela fisiere

      InsertUploadFile(db,zErrMsg,rc,id_fisier,filename,size_fisier,ip_client,port_client);
      id_fisier++;

      printf("Am inserat in baza de date\n");
      
      FisierEvidentaHash(filename,lista,nr_pachete,ip_client,port_client);

      free(lista);
      fflush(stdout);
    }

    else if(nr_comanda == 3)
    {
        //find
        int nr_arg = 0;
        if (read (tdL.cl, &nr_arg,sizeof(int)) <= 0)
        {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
        }
        if(nr_arg >= 3)
      {
        char *filename = malloc(30);
        if (read (tdL.cl, filename,30) <= 0)
        {
          printf("[Thread %d]\n",tdL.idThread);
          perror ("Eroare la read() de la client.\n");
        }
        char *size1=malloc(10);
        if (read (tdL.cl, size1,10) <= 0)
        {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
        }

        char *size2=malloc(10);
        if (read (tdL.cl, size2,10) <= 0)
        {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
        }
        if(strcmp(size2,"-exact")!=0)
        {
          char *raspuns = malloc(2000);
          memset(raspuns,0,2000);
          FindDimensiune(size1,size2,raspuns);
          if (write (tdL.cl,raspuns,2000) <= 0)
          {
            printf("[Thread %d]\n",tdL.idThread);
            perror ("Eroare la write() de la client.\n");
          }	
          free(raspuns);
        }
        else {
          printf("[server] sunt aici\n");
          char *raspuns = malloc(2000);
          memset(raspuns,0,2000);
          FindNume(filename,raspuns);
          if (write (tdL.cl,raspuns,2000) <= 0)
          {
            printf("[Thread %d]\n",tdL.idThread);
            perror ("Eroare la write() de la client.\n");
          }	
          printf("trimt\n%s\n",raspuns);
          free(raspuns);
        }
      }
        else if(nr_arg == 2)
        {
          char *filename = malloc(30);
          if (read (tdL.cl, filename,30) <= 0)
          {
            printf("[Thread %d]\n",tdL.idThread);
            perror ("Eroare la read() de la client.\n");
          }
          char *raspuns = malloc(2000);
          memset(raspuns,0,2000);
          FindString(filename,raspuns);

          if (write (tdL.cl,raspuns,2000) <= 0)
          {
            printf("[Thread %d]\n",tdL.idThread);
            perror ("Eroare la write() de la client.\n");
          }	

          free(raspuns);

        }
    }
    else if(nr_comanda == 4)
    {
        //download
        char *ip = malloc(16);
        int port =0;
        char *filename = malloc(30);
        if (read (tdL.cl, filename,30) <= 0)
          {
            printf("[Thread %d]\n",tdL.idThread);
            perror ("Eroare la read() de la client.\n");
          }
        if(strcmp(filename,"eroare")==0)
          {
            printf("eroare");
            break;
          }
        if (read (tdL.cl, ip,16) <= 0)
          {
            printf("[Thread %d]\n",tdL.idThread);
            perror ("Eroare la read() de la client.\n");
          }
        if (read (tdL.cl, &port,4) <= 0)
          {
            printf("[Thread %d]\n",tdL.idThread);
            perror ("Eroare la read() de la client.\n");
          }

        	char filename_has[42];
          memset(filename_has,0,34);
          strcpy(filename_has,filename);
          strcat(filename_has,ip);
          char *str= malloc(4);
          sprintf(str,"%d",port);
          strcat(filename_has,str);
          strcat(filename_has,".has");

          int exista_has = ExistaFisier(filename_has);

          if(exista_has == 1)
          {
              FILE *fisier_has = fopen(filename_has,"rb");
              int nr_lista = 0;
              fread(&nr_lista,4,1,fisier_has);
              printf("Nr lista %d\n",nr_lista);
              unsigned long *lista = malloc(sizeof(long)*nr_lista);
              fread(lista,sizeof(long),nr_lista,fisier_has);
              fclose(fisier_has);
              if (write (tdL.cl,&nr_lista,4) <= 0)
              {
                printf("[Thread %d]\n",tdL.idThread);
                perror ("Eroare la write() de la client.\n");
              }
              if (write (tdL.cl,lista,sizeof(long)*nr_lista) <= 0)
              {
                printf("[Thread %d]\n",tdL.idThread);
                perror ("Eroare la write() de la client.\n");
              }	
            	
            
          }
          else {
            printf("Eroare,nu avem acest fisier pe server\n");
            break;
          }

    }
  }
}
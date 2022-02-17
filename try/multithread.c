#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "libpq-fe.h"


void *test_driver(void *vargp)
{

  const char *conninfo;
  

  // conninfo = "host=10.150.1.126,10.150.0.218 port=5433 dbname=yugabyte user=yugabyte password=yugabyte  load_balance=true topology_keys=gcp.us-west1.us-west1-d,gcp.us-west1.us-west1-e ";  // load_balance=true topology_keys=gcp.us-west1.us-west1-c
  conninfo = "host=10.150.0.218 port=5433 dbname=yugabyte user=yugabyte password=yugabyte load_balance=true";  // load_balance=true topology_keys=gcp.us-west1.us-west1-c
  PGconn     *conn;
  PGresult   *res;
  
  int         nFields;
  int         i, j;
  conn = PQconnectdb(conninfo);
 
  if (PQstatus(conn) != CONNECTION_OK)
  {
  
      fprintf(stderr, "Connection to database failed: %s",
        PQerrorMessage(conn));
      
        
      PQfinish(conn);
         pthread_exit(&i);
    return NULL ;
   }

  res = PQexec(conn, "SELECT  * from yb_servers() ; ");
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
      fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
      PQclear(res);
      PQfinish(conn);
         pthread_exit(&i);
    return NULL ;
  }


  nFields = PQnfields(res);
  for (i = 0; i < PQntuples(res); i++)
  {
      printf("Query returned: ");
      for (j = 0; j < nFields; j++)
        printf("%s ", PQgetvalue(res, i, j));
      printf("\n");
  }
  PQclear(res);
  
  sleep(60*3) ;
  PQfinish(conn);
  pthread_exit(&i);
  return NULL ;

}

int main(void){
  // Declare variable for thread's ID:
  pthread_t thread_ids[10];
  int itr ;
  for(itr =0 ; itr < 10 ; itr++ )
  {
    int j = 1;
    pthread_create(thread_ids+itr, NULL, test_driver, &j);
  }
    
  int* ptr;
  for(itr =0 ; itr < 10 ;itr++ )
  pthread_join( *(thread_ids+itr) , (void**)&ptr);
}

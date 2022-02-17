#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "libpq-fe.h"
// Global variable:
int i = 2;

void* foo(void* p){
  // Print value received as argument:
  printf("Value recevied as argument in starting routine: ");
  printf("%i\n", * (int*)p);

  // Return reference to global variable:
  pthread_exit(&i);
}

void test_driver()
{

    const char *conninfo;  
    // conninfo = "host=10.150.1.126,10.150.0.218 port=5433 dbname=yugabyte user=yugabyte password=yugabyte  load_balance=true topology_keys=gcp.us-west1.us-west1-d,gcp.us-west1.us-west1-e ";  // load_balance=true topology_keys=gcp.us-west1.us-west1-c
    conninfo = "host=10.150.0.218 port=5433 dbname=yugabyte user=yugabyte password=yugabyte load_balance=true topology_keys=gcp.us-west1.us-west1-d ";  // load_balance=true topology_keys=gcp.us-west1.us-west1-c
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
  return ;
   }




  res = PQexec(conn, "SELECT  * from yb_servers() ; ");
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
      fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
      PQclear(res);
      PQfinish(conn);
    return ; 
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

 

  sleep(5) ;
  conn = PQconnectdb(conninfo) ; 
 PQfinish(conn);
 

}

int main(void){
test_driver();

}

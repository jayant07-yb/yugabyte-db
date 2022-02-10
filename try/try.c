#include <stdio.h>
#include <stdlib.h>
#include "libpq-fe.h"

int
main(int argc, char **argv)
{
  const char *conninfo;
  PGconn     *conn;
  PGresult   *res;
  int         nFields;
  int         i, j;
  
   
  /* connection string */
 // conninfo = "host=10.150.1.126,10.150.0.218 port=5433 dbname=yugabyte user=yugabyte password=yugabyte  load_balance=true topology_keys=gcp.us-west1.us-west1-d,gcp.us-west1.us-west1-e ";  // load_balance=true topology_keys=gcp.us-west1.us-west1-c
  conninfo = "host=10.150.0.218 port=5433 dbname=yugabyte user=yugabyte password=yugabyte load_balance=true  topology_keys=gcp.us-west1.us-west1-d,gcp.us-west1.us-west1-a,gcp.us-west1.us-west1-c ";  // load_balance=true topology_keys=gcp.us-west1.us-west1-c
  conn = PQconnectdb(conninfo);

 
  /* Check to see that the backend connection was successfully made */
  if (PQstatus(conn) != CONNECTION_OK)
  {
  
      fprintf(stderr, "Connection to database failed: %s",
        PQerrorMessage(conn));
      
        
      PQfinish(conn);
      exit(1) ;
   }



  /* Query the row */
  res = PQexec(conn, "SELECT  * from yb_servers() ; ");
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
      fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
      PQclear(res);
      PQfinish(conn);
      exit(1);
  }

  /* print out the rows */
  nFields = PQnfields(res);
  for (i = 0; i < PQntuples(res); i++)
  {
      printf("Query returned: ");
      for (j = 0; j < nFields; j++)
        printf("%s ", PQgetvalue(res, i, j));
      printf("\n");
  }
  PQclear(res);


  /* close the connection to the database and cleanup */
  
  PQfinish(conn);

  return 0;
}

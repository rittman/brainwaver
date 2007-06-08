#include<stdio.h>
#include<stdlib.h>
#include<R.h>
#include <stddef.h>
#include <math.h>



/* Internal function to return the ij th element of a matrix

int ij(int i, int j, int nrow)
{
  return (nrow*j+i);
}
*/

/* $$$$ */  
/* Rkfun */
/* $$$$ */ 

void Rkfun(int *pnnod, int *conn, double *k)
{
  int i, j;
  int nnod=*pnnod;
  for(i = 0; i < nnod; i++)  /* for each vertex */
  {
    k[i] = 0.0;
    for(j = 0; j < nnod; j++)
    {
      if(conn[ij(i,j,nnod)] == 1) /* if j is neighbour of i */
        k[i] += 1.0;
    }
  }
}

/* $$$$$ */  
/* Rcpfun */
/* $$$$$ */  

void Rcpfun(int *pnnod, int *conn, double *cp)
{
  int nnod=*pnnod;
  double maxcp;
  long i, j, m, count1, count2;
  
  for(i = 0; i < nnod; i++)  /* for each vertex */
  {
    count1 = 0; /* number of vertices connected to i */
    count2 = 0; /* number of edges between neighbour vertices of i */
    
    for(j = 0; j < nnod; j++)
    {
      if(conn[ij(i,j,nnod)] == 1) /* if j is neighbour of i */
      {  
        count1++;
	//printf("i=%i, j=%i, nsommet1=%i \n",i,j,count1);
        for(m = 0; m < j; m++)
        {
          if((conn[ij(i,m,nnod)] == 1) && (conn[ij(j,m,nnod)] == 1))  /* both j and m are neighbours of i and are connected by and edge */
            count2++;
	  //printf("m=%i, nsommet2=%i \n",m,count2);
        }
      }
    }
    
    if(count1 < 2) /* there is a node with less than 2 neighbours (Cp is 0/0) */
      cp[i] = -1.0;

    else
    {
      maxcp = (double)count1 * (count1 - 1) / 2.0;  /* maximum possible number of edges between neighbours of i */
      cp[i] = (double)count2 / maxcp; /* proportion observed */
    }
    //printf("i=%i, cp=%e \n",i,cp[i]);
  }
}

/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */ 
/* Rlpfun (Dijkstra's algorithm, based on version of Luis Goddyn) */
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */ 

void Rlpfun(int *pnnod, int *conn, double *dist, double *Lp)
{
  int nnod=*pnnod;
  double minl, lp, meanp, dist2;
  long i, j, v, nedges, count;
  int T[nnod]; /* contains flags on found shortest paths (of the nodes already included in tree T) */
  double l[nnod];    /* contains distance (label) of shortest known path */
  int p[nnod];  /* contains the predecessors (pointers) of each node */
  
  lp = 0.0;
  
  for(i = 0; i < nnod; i++)  /* each vertex is considered the root of the tree */
  {
    meanp = 0.0;
    count = 0;
    
    /* initialise T and d */
    
    for(j = 0; j < nnod; j++)  
    {
      if(j == i)  /* distance and inclusion of initial vertex in tree */
      {
        T[j] = 1; 
        l[j] = 0;
        p[j] = -1;
      }
      else  /* initially we don't now shortest path to any vertex */
      {
        T[j] = 0;
        l[j] = 1e5;
        p[j] = -2;
      }  
    }
    
    v = i;  /* analysed node is initially i */
    
    do
    {
      /* update distances (labels) of arcs containing v and a node not included in T */
    
      for(j = 0; j < nnod; j++)
      {
        if((T[j] == 0) && (conn[ij(v,j,nnod)] == 1))  /* j is node not in T and has arc with v */
        {
          if(l[v] + dist[ij(v,j,nnod)] < l[j])  /* we should update label of j */
          {
            l[j] = l[v] + dist[ij(v,j,nnod)];
            p[j] = v;  /* set pointer of j */
          }
        }
      }
     
      /* look for new v */
    
      minl = 1e5;
      for(j = 0; j < nnod; j++)  
      {
        if((T[j] == 0) && (l[j] < minl))
        {
          minl = l[j];
          v = j;
        }
      }
    
      if(minl < 1e5)  /* there is a new vertex to include in T? */
        T[v] = 1;
    }  
    while(minl < 1e5);  /* while there is a new vertex v included in T */  
    
    /* we check if all vertices were reachable from i, if not we cannot calculate lp */
    
    /* calculate average minimum distance (in number of nodes) between vertex i and the rest
    
    for(j = 0; j < nnod; j++)
    {
      if(j != i)
      {
        nedges = 1;
        v = p[j];
        while(v != i)
        {
          v = p[v];
          nedges++;
        }
        meanp += (double)nedges;
      }
    }
    lp += meanp / (double) (nnod - 1); */
    
    /* calculate average minimum distance (euclidean) between vertex i and the rest */
    
    for(j = 0; j < nnod; j++)
    {
      dist2 = 0.0;
      if((j != i) && (T[j] != 0))
      {
        count++;
        v = p[j];
        dist2 += dist[ij(j,v,nnod)];
        while(v != i)
        {
          dist2 += dist[ij(v,p[v],nnod)];
          v = p[v];
        }
        meanp += dist2;
      }
    }
    if(count > 0)
      Lp[i] = meanp / (double) (count);
    else
      Lp[i] = -1.0;
  }
  
}

/* Simulation of a regular graph */

void Rregsim(int *pnnod, int *pnedge, int *conn)
{
  int i, j, m, incr, n;
  int nnod=*pnnod;
  int nedge=*pnedge;

  for(j = 0; j < (nnod-1); j++)  /* fill matrix conn with zeros */
    {
      for(m = j+1; m < nnod; m++)
	{
	  conn[ij(j,m,nnod)] = 0;
	  conn[ij(m,j,nnod)] = 0;
	}
    }
  j = 0;
  incr = 1;
  do
    {
      m = j;
      n = j + incr;
      if(m >= nnod)
	m = m % nnod;
      if(n >= nnod)
	n = n % nnod;

      if(conn[ij(m,n,nnod)] == 0)
	{  
	  conn[ij(m,n,nnod)] = 1;
	  conn[ij(n,m,nnod)] = 1;
	  j++;
	}
      else
	incr++;


    }
  while(j < nedge);

}


/* Simulation of a random graph */



void Rrandsim(int *pnnod, int *pnedge, int *conn)
{

  int p, j, flag, row, col, m; 
  int nnod=*pnnod;
  int nedge=*pnedge;

 for(j = 0; j < (nnod-1); j++)  /* fill conn matrix with zeros */
    {
      for(m = j+1; m < nnod; m++)
      {
        conn[ij(j,m,nnod)] = 0;
        conn[ij(m,j,nnod)] = 0;
      }
    }
      
    for(p = 0; p < nedge; p++)  /* assigning count random edges */
    {
      flag = 0;
      do
      {
        row = (short int) ((double)rand() / RAND_MAX * (double)nnod);
        col = (short int) ((double)rand() / RAND_MAX * (double)nnod);
	
	if((row != col) && (conn[ij(row,col,nnod)] == 0))
        {
          flag = 1;
          conn[ij(row,col,nnod)] = 1;
          conn[ij(col,row,nnod)] = 1;
        }
      }
      while(flag == 0);
    }
}




/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */ 
/* Refficiencyfun for efficiency (Dijkstra's algorithm, based on version of Luis Goddyn) */
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */ 

void Refficiencyfun(int *pnnod, int *conn, double *dist, double *Lp, double *eff, double *matlp)
{
  int nnod=*pnnod;
  double minl, lp, meanp, dist2,tmpeff;
  long i, j, v, nedges, count;
  int T[nnod]; /* contains flags on found shortest paths (of the nodes already included in tree T) */
  double l[nnod];    /* contains distance (label) of shortest known path */
  int p[nnod];  /* contains the predecessors (pointers) of each node */
  
  lp = 0.0;

  tmpeff = 0.0;  

  for(i = 0; i < nnod; i++)  /* each vertex is considered the root of the tree */
  {
    meanp = 0.0;
    count = 0;
    
    /* initialise T and d */
    
    for(j = 0; j < nnod; j++)  
    {
      if(j == i)  /* distance and inclusion of initial vertex in tree */
      {
        T[j] = 1; 
        l[j] = 0;
        p[j] = -1;
      }
      else  /* initially we don't now shortest path to any vertex */
      {
        T[j] = 0;
        l[j] = 1e5;
        p[j] = -2;
      }  
    }
    
    v = i;  /* analysed node is initially i */
    
    do
    {
      /* update distances (labels) of arcs containing v and a node not included in T */
    
      for(j = 0; j < nnod; j++)
      {
        if((T[j] == 0) && (conn[ij(v,j,nnod)] == 1))  /* j is node not in T and has arc with v */
        {
          if(l[v] + dist[ij(v,j,nnod)] < l[j])  /* we should update label of j */
          {
            l[j] = l[v] + dist[ij(v,j,nnod)];
            p[j] = v;  /* set pointer of j */
          }
        }
      }
     
      /* look for new v */
    
      minl = 1e5;
      for(j = 0; j < nnod; j++)  
      {
        if((T[j] == 0) && (l[j] < minl))
        {
          minl = l[j];
          v = j;
        }
      }
    
      if(minl < 1e5)  /* there is a new vertex to include in T? */
        T[v] = 1;
    }  
    while(minl < 1e5);  /* while there is a new vertex v included in T */  
    
    /* we check if all vertices were reachable from i, if not we cannot calculate lp */
     

    for(j=0; j<nnod; j++){
      matlp[nnod*j+i]=l[j];
    } 

    for(j=0; j<nnod; j++){
      
      if((l[j]!=1e5) && (j!=i)){
	tmpeff+= (double) 1/l[j];
	//printf("tmpeff %e \n",tmpeff);
	//printf("value %e \n",l[j]);
      }
    } 
   
    /* calculate average minimum distance (in number of nodes) between vertex i and the rest
    
    for(j = 0; j < nnod; j++)
    {
      if(j != i)
      {
        nedges = 1;
        v = p[j];
        while(v != i)
        {
          v = p[v];
          nedges++;
        }
        meanp += (double)nedges;
      }
    }
    lp += meanp / (double) (nnod - 1); */
    
    /* calculate average minimum distance (euclidean) between vertex i and the rest */
    
    for(j = 0; j < nnod; j++)
    {
      dist2 = 0.0;
      if((j != i) && (T[j] != 0))
      {
        count++;
        v = p[j];
        dist2 += dist[ij(j,v,nnod)];
        while(v != i)
        {
          dist2 += dist[ij(v,p[v],nnod)];
          v = p[v];
        }
        meanp += dist2;
      }
    }
    if(count > 0)
      Lp[i] = meanp / (double) (count);
    else
      Lp[i] = -1.0;
  }
  
 tmpeff=tmpeff/(nnod*(nnod-1));
 //printf("tmpeff %e  \n",tmpeff);
      *eff = tmpeff;
      //printf("eff %e \n",eff);


}











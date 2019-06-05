/*
 *  Copyright 2019, Chee Bin Hoh, All right reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define DEMANDSIZE_PER_NODE  500

struct demand
{
    char   geohash6[7];
    int    day;
    int    hh;
    int    mm;
    double value;
};

typedef struct demand Demand;

struct demandnode
{
    struct demandnode *next;
    struct demandnode *prev;
    long               cnt;
    struct demand     *d[1];
};

typedef struct demandnode DemandNode;

struct demandintime
{
    DemandNode * day[365];
    DemandNode * hour[365 * 24];
    DemandNode * mininterval[365 * 24 * 4]; 
};


// functions...
DemandNode *
newDemandNode( size_t num );

Demand *
scanDemand( char * cptr, Demand * dptr );

int
main( int argc, char * argv[] )
{ 
    struct demandintime dInTime = { { 0 }, { 0 }, { 0 } };

    Demand * base  = NULL;
    Demand * dptr  = NULL;
    long     nrDemand = 0; 
    int      batchsize = 1000;
    int      n;
    int      ret;
    int      hh;
    int      mm;
    char     buf[BUFSIZ];
    FILE *   file = stdin;

    argc -= optind;
    argv += optind;   

    if ( argc-- > 0 )
    {
        file = fopen( *argv++, "r" );
        if ( file == NULL) 
        {
            fprintf( stderr, "open file error: %s\n", *argv );
            exit( 1 );
        }
    }

    do
    {
        while ( fgets( buf, sizeof buf, file ) != NULL )
        {
            if ( NULL == base )
            {
               base = malloc( batchsize * ( sizeof ( Demand ) ) );
                dptr = base;
            }
            else if ( ( nrDemand % batchsize ) == 0 )
            {
                dptr = realloc( base, ( nrDemand + batchsize ) * sizeof ( Demand )  );
                if ( dptr  == NULL )
                {
                    free( base );
                }

                base = dptr;              
            }

            if ( dptr == NULL )
            {
                fprintf( stderr, "no memory\n" );
                exit( 1 );
            }

	    if ( scanDemand( buf, &( dptr[nrDemand] ) ) )
            {
                nrDemand++;
            }
        } // while still got next line

       
        if ( argc-- > 0 )
        {
           file = fopen( *argv++, "r" );
           if ( file == NULL) 
           {
               fprintf( stderr, "open file error: %s\n", *argv );
               exit( 1 );
           }
        }
        else
        {
           break;  
        }
    } while ( 1 );

    // demand in time
    for ( n = 0; n < nrDemand; n++ )
    {
        int d = dptr[n].day - 1;
        DemandNode *nPtr = dInTime.day[d];

        if ( NULL == nPtr 
             || nPtr->cnt >= DEMANDSIZE_PER_NODE )
        {
             DemandNode *newNode;

             newNode = newDemandNode( DEMANDSIZE_PER_NODE );
             if ( NULL == newNode )
             {
                 fprintf( stderr, "no memory for new demand node of size = %d\n", DEMANDSIZE_PER_NODE );
                 exit( 1 );
             }

             newNode->next = nPtr;
             if ( NULL != nPtr )
             {
                 nPtr->prev = newNode;
             }

             dInTime.day[d] = newNode;         

             nPtr = dInTime.day[d];
        }

        nPtr->d[nPtr->cnt++] = &( dptr[n] );
    }

    /*
    for ( n = 0; n < 365; n++ )
    {
        DemandNode *nPtr;
    
        for ( nPtr = dInTime.day[n]; NULL != nPtr; nPtr = nPtr->next )
        {
            int i;

            for ( i = 0; i < nPtr->cnt; i++ )
            {
                printf( "%s\t%d\t%02d:%02d\t%lf\n",
                        nPtr->d[i]->geohash6, nPtr->d[i]->day, nPtr->d[i]->hh, nPtr->d[i]->mm, nPtr->d[i]->value );
	    }
        }
    }
     */

    // yup, I do not release memory as OS will claim it, there is no point to do so. :)

    return 0;
}

Demand *
scanDemand( char * cptr, Demand * dptr )
{
    int    c;
    int    inMm        = 0;
    int    index       = 0;
    int    geohash6Len = 0;
    Demand d           = { "\0", 0, 0, 0.0 };

    while ( ( c = *cptr++ ) != '\0'
            && c != '\n' )
    {
        if ( ',' == c )
        {
            if ( 0 == index )
            {
                d.geohash6[geohash6Len] = '\0';
            }
            else if ( 2 == index )
            {
                d.value = atof( cptr );
                break;
            }

            index++;
        }
        else
        {
            switch ( index )
            {
                case 0:
                    if ( geohash6Len < 6 )
                    {
                        d.geohash6[geohash6Len++] = c;
                    }
                    else
                    {
                        return NULL;
                    }

                    break;

                case 1:
                    if ( !isdigit( c ) )
                    {
                        return NULL;                      
                    }

                    d.day = d.day * 10 + ( c - '0' );
                    break;

                case 2:
                    if ( c == ':' )
                    {
                        inMm = 1;
                    }
                    else 
                    {
                        if ( !isdigit( c ) )
                        {
                            return NULL;                      
                        }

                        if ( inMm )
                        {
                            d.mm = d.mm * 10 + ( c - '0' );
                        }
                        else
                        {
                            d.hh = d.hh * 10 + ( c - '0' );
                        }
                    }

                    break;
            } // switch        
        } // else not yet reach ','
    } // while not end of line

    *dptr = d;

    return dptr;
}


DemandNode *
newDemandNode( size_t num )
{
    DemandNode *newNode;

    newNode = malloc( sizeof( DemandNode ) + ( ( num - 1 ) * ( sizeof( Demand * ) ) ) );
    if ( NULL != newNode )
    {
        newNode->next = NULL;
        newNode->prev = NULL;
        newNode->cnt = 0;
    }

    return newNode;
}

/*
 *  Copyright 2019, Chee Bin Hoh, All right reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <assert.h>

#define DEMANDSIZE_PER_NODE  500
#define DAYS_IN_YEAR         365
#define HOURS_IN_YEAR        ( DAYS_IN_YEAR * 24 )
#define MININTERVALS_IN_YEAR  ( HOURS_IN_YEAR * 4 )

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
    Demand            *d[1];
};

typedef struct demandnode DemandNode;

struct demandintime
{
    DemandNode * day[DAYS_IN_YEAR];
    DemandNode * hour[HOURS_IN_YEAR];
    DemandNode * mininterval[MININTERVALS_IN_YEAR]; 
};

typedef struct demandintime DemandInTime;

// functions...
Demand *
scanDemand( char * cptr, Demand * dptr );

DemandNode *
newDemandNode( size_t num );

DemandInTime *
newDemandInTime( void );

void
deleteDemandNode( DemandNode * list );

void
deleteDemandInTime( DemandInTime * dit ); 

DemandNode *
processDemandNode( DemandNode * list, Demand * dptr, int nrDemand );

void
processDemandInTime( DemandInTime * dit, Demand * dptr, int nrDemand );

void
printDebugDemandInTime( DemandInTime * dit );

void
printDebugDemandNode( DemandNode * list );

int
main( int argc, char * argv[] )
{ 
    DemandInTime *dit = NULL;
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
    dit = newDemandInTime();
    if ( NULL == dit )
    {
        fprintf( stderr, "error in new demand in time\n" );
        exit( 1 );
    }
 
    processDemandInTime( dit, dptr, nrDemand );

    printDebugDemandInTime( dit );
 
    deleteDemandInTime( dit );

    // yup, I do not release memory as OS will claim it, there is no point to do so. :)

    return 0;
}


void
deleteDemandNode( DemandNode * list )
{
    DemandNode *tmp;
    DemandNode *item = list;

    while ( NULL != item )
    {
        tmp = item;
        item = item->next;
 
        free( tmp );
    }
}

void
deleteDemandInTime( DemandInTime * dit )
{
    int i;

    for ( i = 0; i < DAYS_IN_YEAR; i++ )
    {
        deleteDemandNode( dit->day[i] );  
    }
 
    for ( i = 0; i < HOURS_IN_YEAR; i++ )
    {
        deleteDemandNode( dit->hour[i] );  
    }
 
    for ( i = 0; i < MININTERVALS_IN_YEAR; i++ )
    {
        deleteDemandNode( dit->mininterval[i] );  
    }
 
    free( dit );   
}


void
printDebugDemandNode( DemandNode * item )
{
    int i;

    while ( NULL != item )
    {
        for ( i = 0; i < item->cnt; i++ )
        {
            printf( "%s,%02d,%02d:%02d,%lf\n", 
                    item->d[i]->geohash6, 
                    item->d[i]->day,
                    item->d[i]->hh, 
                    item->d[i]->mm, 
                    item->d[i]->value );
        }

        item = item->next;
    }
}

void
printDebugDemandInTime( DemandInTime * dit )
{
    int dayIndex;
    int hourIndex;

    for ( dayIndex = 0; dayIndex < DAYS_IN_YEAR; dayIndex++ )
    {
        if ( NULL != dit->day[dayIndex] )
        { 
            // printf( "---- day = %d\n", dayIndex + 1 );
            // printDebugDemandNode( dit->day[dayIndex] );    
 
            for ( hourIndex = dayIndex * 24; hourIndex < dayIndex * 24 + 24; hourIndex++ )
            {
                if ( NULL != dit->hour[hourIndex] )
                {
                    // printf( "-------- hour = %d\n", ( hourIndex % 24 ) + 1 );
 
                    printDebugDemandNode( dit->hour[hourIndex] );
                }
            }
        }
    }
}


DemandInTime *
newDemandInTime( void )
{
    DemandInTime *dit;
    int           i;

    dit = malloc( sizeof( *dit ) );
    if ( NULL != dit )
    {
        for ( i = 0; i < DAYS_IN_YEAR; i++ )
        {   
            dit->day[i] = NULL;
        } 

        for ( i = 0; i < HOURS_IN_YEAR; i++ )
        {
            dit->hour[i] = NULL;
        } 

        for ( i = 0; i < MININTERVALS_IN_YEAR; i++ )
        {
            dit->mininterval[i] = NULL;
        } 
    }

    return dit;
}


DemandNode *
processDemandNode( DemandNode * list, Demand * dptr, int nrDemand )
{
    int         n;   
    DemandNode *newNode;

    for ( n = 0; n < nrDemand; n++ )
    {
        if ( NULL == list
             || list->cnt >= DEMANDSIZE_PER_NODE )
        {
            newNode = newDemandNode( DEMANDSIZE_PER_NODE );
            if ( NULL == newNode )
            {
	        fprintf( stderr, "no memory for new demand node of size = %d\n", DEMANDSIZE_PER_NODE );
	        exit( 1 );
            }

            newNode->next = list;
            if ( NULL != list )
            {
	        list->prev = newNode;
            }

            list = newNode;
        }  

        list->d[list->cnt++] = &( dptr[n] );
    }

    return list;
}


void
processDemandInTime( DemandInTime * dit, Demand * dptr, int nrDemand )
{
    int n;
    int dayIndex;
    int hourIndex;
    int minIntervalIndex;

    for ( n = 0; n < nrDemand; n++ )
    {
        dayIndex  = dptr[n].day - 1;
        hourIndex = dayIndex * 24 + dptr[n].hh - 1;
        minIntervalIndex = hourIndex * 4 + ( dptr[n].mm / 15 ) - 1;

        assert( dayIndex < DAYS_IN_YEAR );
        assert( hourIndex < HOURS_IN_YEAR );
        assert( minIntervalIndex < MININTERVALS_IN_YEAR );

        dit->day[dayIndex] = processDemandNode( dit->day[dayIndex], &( dptr[n] ), 1 );
        dit->hour[hourIndex] = processDemandNode( dit->hour[hourIndex], &( dptr[n] ), 1 );
        dit->mininterval[minIntervalIndex] = processDemandNode( dit->mininterval[minIntervalIndex], &( dptr[n] ), 1 );
    }
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

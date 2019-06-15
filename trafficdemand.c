/*
 * Copyright 2019, Chee Bin Hoh, All right reserved.
 *
 * This program is built for programming challenge, https://www.aiforsea.com/traffic-management
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <assert.h>


enum 
{
    NUM_DEMAND_PER_NODE  = 500,
    HOURS_IN_DAY         = 24,
    DAYS_IN_YEAR         = 365,
    HOURS_IN_YEAR        = DAYS_IN_YEAR * HOURS_IN_DAY,  
    MININTERVALS_IN_YEAR = HOURS_IN_YEAR * 4,
    HASH_MULTIPLIER      = 37,
    NUM_HASH_SIZE        = 5000
};


typedef struct demand Demand;

struct demand
{
    char   geohash6[7];
    int    day;
    int    hh;
    int    mm;
    double value;
};


typedef struct demandnode DemandNode;

struct demandnode
{
    DemandNode *next;
    DemandNode *prev;
    long        cnt;
    Demand     *d[1]; //varying size, allocated by malloc
};


typedef struct demandintime DemandInTime;

struct demandintime
{
    DemandNode * day[DAYS_IN_YEAR];
    DemandNode * hour[HOURS_IN_YEAR];
    DemandNode * mininterval[MININTERVALS_IN_YEAR]; 
};


typedef struct demandingeohash6 DemandInGeohash6; 

struct demandingeohash6
{
    DemandNode       * d;
    DemandInGeohash6 * next;
    char               geohash6[7];
};


Demand *
scanDemand( char * cptr, Demand * dptr );


/* Start of DemandInTime API 
 * 
 * DemandInTime is ADT that allows us to store and organize demands by day and hour 
 * and min interval.
 */

DemandInTime *
newDemandInTime( void );

void
deleteDemandInTime( DemandInTime * dit ); 

void
processDemandInTime( DemandInTime * dit, Demand * dptr, long nrDemand );

void
processDemandNodeInTime( DemandInTime * dit, DemandNode * list );

void
printDebugDemandInTime( DemandInTime * dit );

/* End of DemandInTime API */


/* Start of DemandNode API 
 */

DemandNode *
newDemandNode( void );

void
deleteDemandNode( DemandNode * list );

DemandNode *
processDemandNode( DemandNode * list, Demand * dptr, long nrDemand );

void
printDebugDemandNode( DemandNode * list );

/* End of DemandNode API */


/* Start of DemandInGeohash6 API 
 *
 * DemandInGeohash6 is ADT that allows us to store and organize demands by geohash6 value. 
 *
 * The ADT is a hash struture that each entry referring to a list of object DemandInGeohash6
 * that its geohash6 values are hashed into the same hash key. Each DemandInGeohash6 object stores 
 * a list of demands of the same geohash6 and link to next DemandInGeohash6 that has different 
 * geohash6 value but hashed into the same key in the high level DemandInGeohash6 hash structure.
 */

long
getHashValueOfString( char * s );

DemandInGeohash6 * *
newDemandInGeohash6( void );

void
deleteDemandInGeohash6( DemandInGeohash6 * * digh6 );

DemandInGeohash6 *
insertGeohash6( DemandInGeohash6 * * digh6, char * geohash6, int createIfNotExist  );

DemandInGeohash6 *
insertDemandInGeohash6( DemandInGeohash6 * * digh6, Demand * d, int createIfNotExist );

void
processDemandInGeohash6( DemandInGeohash6 * * digh6, Demand * d, long nrDemand, int createIfNotExist );

void
printDebugDemandInGeohash6( DemandInGeohash6 * * digh6 );

/* End of DemandInGeohash6 API */ 


int
main( int argc, char * argv[] )
{
    char   * geohash6 = NULL;
    Demand * base  = NULL;
    Demand * dptr  = NULL;
    long     nrDemand = 0; 
    int      n;
    int      ret;
    int      hh;
    int      mm;
    int      opt;
    char     buf[BUFSIZ];
    FILE *   file = stdin;

    while ( ( opt = getopt( argc, argv, "g:" ) ) != -1 )
    {
        switch ( opt )
        {
            case 'g':
                geohash6 = optarg;
                break;

            default:
                fprintf( stderr, "invalid option (%c) and argument\n", opt );
                exit( 1 );
        }
    }

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
               base = malloc( NUM_DEMAND_PER_NODE  * ( sizeof ( base[0] ) ) );
               dptr = base;
            }
            else if ( ( nrDemand % NUM_DEMAND_PER_NODE ) == 0 )
            {
                dptr = realloc( base, ( nrDemand + NUM_DEMAND_PER_NODE ) * sizeof ( base[0] )  );
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


    {
        DemandInGeohash6 * * glist = NULL;
        int                  hasFilterGeohash6 = 0;


        hasFilterGeohash6 = ( NULL != geohash6 );

        glist = newDemandInGeohash6();
        
        if ( hasFilterGeohash6 )
        {
            char *tok = NULL;

            tok = strtok( geohash6, "," );
            while ( NULL != tok )
            {
                insertGeohash6( glist, tok, 1 );
   
                tok = strtok( NULL, "," );
            }
        }

        processDemandInGeohash6( glist, dptr, nrDemand, ! hasFilterGeohash6 );

       // printDebugDemandInGeohash6( glist );

        deleteDemandInGeohash6( glist );
    }

    // yup, I do not release memory as OS will claim it, there is no point to do so. :)

    return 0;
}


/* Start of DemandNode API */

DemandNode *
newDemandNode( void )
{
    DemandNode * newNode;

    /* reduce by 1 because DemandNode already contains 1 element for the array
     */
    newNode = malloc( sizeof( DemandNode ) + ( ( NUM_DEMAND_PER_NODE - 1 ) * ( sizeof( Demand * ) ) ) );
    if ( NULL == newNode )
    {
        fprintf( stderr, "failed to allocate memory for more DemandNode\n" );
        exit( 1 );
    }

    newNode->next = NULL;
    newNode->prev = NULL;
    newNode->cnt = 0;

    return newNode;
}


void
deleteDemandNode( DemandNode * item )
{
    DemandNode * tmp;

    while ( NULL != item )
    {
        tmp = item;
        item = item->next;
 
        free( tmp );
    }

    free( item );
}


DemandNode *
processDemandNode( DemandNode * list, Demand * dptr, long nrDemand )
{
    long         i;   
    DemandNode * newNode;

    for ( i = 0; i < nrDemand; i++ )
    {
        if ( NULL == list
             || list->cnt >= NUM_DEMAND_PER_NODE )
        {
            newNode = newDemandNode();

            newNode->next = list;
            if ( NULL != list )
            {
	        list->prev = newNode;
            }

            list = newNode;
        }  

        list->d[list->cnt++] = &( dptr[i] );
    }

    return list;
}


void
printDebugDemandNode( DemandNode * item )
{
    int i;

    while ( NULL != item )
    {
        for ( i = 0; i < item->cnt; i++ )
        {
            printf( "%s,%02d,%02d:%02d,%.18lf\n", 
                    item->d[i]->geohash6, 
                    item->d[i]->day,
                    item->d[i]->hh, 
                    item->d[i]->mm, 
                    item->d[i]->value );
        }

        item = item->next;
    }
}

/* End of DemandNode API */


/* Start of DemandInGeohash6 API */

long
getHashValueOfString( char * s )
{
    long result;
    int  c;

    result = 0;
    while ( ( c = *s++ )  != '\0' )
    {
        result = result * HASH_MULTIPLIER + c;   
    }

    return result % NUM_HASH_SIZE;
}


DemandInGeohash6 * *
newDemandInGeohash6( void )
{
    DemandInGeohash6 * * digh6 = NULL;
    int                  i;

    digh6 = malloc( NUM_HASH_SIZE * sizeof( DemandInGeohash6 * ) );
    if ( NULL == digh6 )
    {
        fprintf( stderr, "failed to allocte memory for more DemandInGeohash6 *\n" );
        exit( 1 );
    }

    for ( i = 0; i < NUM_HASH_SIZE; i++ )
    {
        digh6[i] = NULL;
    } 

    return digh6;
}


void
deleteDemandInGeohash6( DemandInGeohash6 * * digh6 )
{
    int                i;
    DemandInGeohash6 * hashItem;  
    DemandInGeohash6 * next;

    for ( i = 0; i < NUM_HASH_SIZE; i++ )
    {
        for ( hashItem = digh6[i]; NULL != hashItem; hashItem = next )
        {
            next = hashItem->next;
            
            deleteDemandNode( hashItem->d );
            free( hashItem );   
        }
    }

    free( digh6 );
}


DemandInGeohash6 *
insertGeohash6( DemandInGeohash6 * * digh6, char * geohash6, int createIfNotExist  )
{
    long               hashkey;
    DemandInGeohash6 * hashItem = NULL;
 
    hashkey = getHashValueOfString( geohash6 ); 

    assert( hashkey >= 0 && hashkey < NUM_HASH_SIZE );

    for ( hashItem = digh6[hashkey]; hashItem != NULL; hashItem = hashItem->next )
    {   
        if ( strcmp( hashItem->geohash6, geohash6 ) == 0 )
        {   
            break;
        }
    }
    
    if ( NULL == hashItem 
         && createIfNotExist )
    {
        hashItem = malloc( sizeof( * hashItem ) );
        if ( NULL == hashItem )
        {   
            fprintf( stderr, "failed to allocate memory for more DemandInGeohash6\n" );
            exit( 1 );
        }
        
        strncpy( hashItem->geohash6, geohash6, sizeof( hashItem->geohash6 ) );
        hashItem->d = NULL;
        hashItem->next = digh6[hashkey];
        digh6[hashkey] = hashItem;
    }

    return hashItem;
}


DemandInGeohash6 *
insertDemandInGeohash6( DemandInGeohash6 * * digh6, Demand * d, int createIfNotExist )
{
    DemandInGeohash6 * hashItem;

    
    hashItem = insertGeohash6( digh6, d->geohash6, createIfNotExist );

    if ( NULL != hashItem )
    {
        hashItem->d = processDemandNode( hashItem->d, d, 1 );
    }

    return hashItem;
}


void
processDemandInGeohash6( DemandInGeohash6 * * digh6, Demand * d, long nrDemand, int createIfNotExist )
{
    while ( nrDemand-- > 0 )
    {
        insertDemandInGeohash6( digh6, d++, createIfNotExist );
    }
}


void
printDebugDemandInGeohash6( DemandInGeohash6 * * digh6 )
{
    int                i;
    DemandInGeohash6 * hashItem;

    for ( i = 0; i < NUM_HASH_SIZE; i++ )
    {
        for ( hashItem = digh6[i]; NULL != hashItem; hashItem = hashItem->next )
        {
            DemandInTime * dit;
            DemandNode   * list;

            dit = newDemandInTime();
            
            for ( list = hashItem->d; NULL != list; list = list->next )
            {
                processDemandNodeInTime( dit, list );
            }

            printDebugDemandInTime( dit );

	    deleteDemandInTime( dit );
        }
    }
}

/* End of DemandInGeohash6 API */


/* Start of DemandInTime API */

DemandInTime *
newDemandInTime( void )
{
    DemandInTime * dit;
    int            i;

    dit = malloc( sizeof( *dit ) );
    if ( NULL == dit )
    {
        fprintf( stderr, "failed to allocate more memory for DemandInTime\n" );
        exit( 1 );
    }

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

    return dit;
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
printDebugDemandInTime( DemandInTime * dit )
{
    int dayIndex;
    int hourIndex;
    int minIntervalIndex;

    for ( dayIndex = 0; dayIndex < DAYS_IN_YEAR; dayIndex++ )
    {
        if ( NULL != dit->day[dayIndex] )
        { 
            for ( hourIndex = dayIndex * 24; hourIndex < dayIndex * 24 + 24; hourIndex++ )
            {
                if ( NULL != dit->hour[hourIndex] )
                {
                    for ( minIntervalIndex = hourIndex * 4; minIntervalIndex < hourIndex * 4 + 4; minIntervalIndex++ )
                    {
                         if ( NULL != dit->mininterval[minIntervalIndex] )
                         {
                             printDebugDemandNode( dit->mininterval[minIntervalIndex] );
                         }
                    }          
                }
            }
        }
    }
}


void
processDemandInTime( DemandInTime * dit, Demand * dptr, long nrDemand )
{
    long i;
    int  dayIndex;
    int  hourIndex;
    int  minIntervalIndex;

    for ( i = 0; i < nrDemand; i++ )
    {
        // day is started at 1, hour at 0 and min interval at 0 from import data

        dayIndex  = dptr[i].day - 1;  
        hourIndex = dayIndex * 24 + dptr[i].hh;
        minIntervalIndex = hourIndex * 4 + ( dptr[i].mm / 15 );

        assert( dayIndex < DAYS_IN_YEAR );
        assert( hourIndex < HOURS_IN_YEAR );
        assert( minIntervalIndex < MININTERVALS_IN_YEAR );

        dit->day[dayIndex] = processDemandNode( dit->day[dayIndex], &( dptr[i] ), 1 );
        dit->hour[hourIndex] = processDemandNode( dit->hour[hourIndex], &( dptr[i] ), 1 );
        dit->mininterval[minIntervalIndex] = processDemandNode( dit->mininterval[minIntervalIndex], &( dptr[i] ), 1 );
    }
}


void
processDemandNodeInTime( DemandInTime * dit, DemandNode * list )
{
    int i;

    for ( i = 0; i < list->cnt; i++ )
    {
        processDemandInTime( dit, list->d[i], 1 );
    }
}

/* End of DemandInTime API */


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




/*
 *  Copyright 2019, Chee Bin Hoh, All right reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

struct demand
{
    char   geohash6[7];
    int    day;
    int    hh;
    int    mm;
    double value;
};

typedef struct demand Demand;

Demand *
scanDemand( char * cptr, Demand * dptr );

int
main( int argc, char * argv[] )
{
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

    printf( "geohash6,day,timestamp,demand\n" );

    for ( n = 0; n <nrDemand; n++ )
    {
        printf( "%-6s,%02d,%02d:%02d,%1.16lf\n", dptr[n].geohash6, dptr[n].day, dptr[n].hh, dptr[n].mm, dptr[n].value );
    }

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


//
// $Id$
//

//
// Sphinx Hamming Distance
//
// Linux
// gcc -fPIC -shared -o hamdist.so hamdist.c
// CREATE FUNCTION hamdist RETURNS INT SONAME 'hamdist.so';
// CREATE FUNCTION hamdist_mv RETURNS INT SONAME 'hamdist.so';
//
// Windows
// cl /MTd /LD udfexample.c
// CREATE FUNCTION hamdist RETURNS INT SONAME 'hamdist.dll';
// CREATE FUNCTION hamdist_mv RETURNS INT SONAME 'hamdist.dll';
//

#include "sphinxudf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

DLLEXPORT int hamdist_ver ()
{
        return SPH_UDF_VERSION;
}

long long hamdist_fn(sphinx_uint64_t x, sphinx_uint64_t y)
{
        long long dist = 0;
        sphinx_uint64_t val = x ^ y; // XOR
 
        // Count the number of set bits
        while(val)
        {
            ++dist; 
            val &= val - 1;
        }
 
        return dist;
}

long long parse_fn(SPH_UDF_ARGS * args, int argn) {
    char * ptr = args->arg_values[argn];
    if (args->arg_types[argn]==SPH_UDF_TYPE_UINT32) {
        return *((long *)ptr);
    }
    if (args->arg_types[argn]==SPH_UDF_TYPE_INT64) {
        return *((sphinx_uint64_t *)ptr);
    }
    return 0;
}


DLLEXPORT int hamdist_init ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_message )
{
        // check argument count
        if ( args->arg_count != 2 )
        {
                snprintf ( error_message, SPH_UDF_ERROR_LEN, "HAMDIST() takes either 2 arguments" );
                return 1;
        }

        // check argument type
        if ( args->arg_types[0]!=SPH_UDF_TYPE_UINT32 && args->arg_types[1]!=SPH_UDF_TYPE_UINT32
            && args->arg_types[0]!=SPH_UDF_TYPE_INT64 && args->arg_types[1]!=SPH_UDF_TYPE_INT64)
        {
                snprintf ( error_message, SPH_UDF_ERROR_LEN, "HAMDIST() requires argument to be uint or int64" );
                return 1;
        }

        return 0;
}

DLLEXPORT void hamdist_deinit ( SPH_UDF_INIT * init )
{
}

DLLEXPORT sphinx_uint64_t hamdist ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
        return hamdist_fn(parse_fn(args, 0), parse_fn(args, 1));
}

// hamdist_mv

DLLEXPORT int hamdist_mv_init ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_message )
{
        // check argument count
        if ( args->arg_count != 2 )
        {
                snprintf ( error_message, SPH_UDF_ERROR_LEN, "HAMDIST_MV() takes either 2 arguments" );
                return 1;
        }

        // check argument type
        if ( args->arg_types[0]!=SPH_UDF_TYPE_UINT32SET && args->arg_types[0]!=SPH_UDF_TYPE_UINT64SET )
        {
                snprintf ( error_message, SPH_UDF_ERROR_LEN, "HAMDIST_MV() requires 1 MVA argument" );
                return 1;
        }
        
        /*if ( args->arg_types[1]!=SPH_UDF_TYPE_UINT32 && args->arg_types[1]!=SPH_UDF_TYPE_INT64 )
        {
                snprintf ( error_message, SPH_UDF_ERROR_LEN, "HAMDIST_MV() requires argument 2 to be uint" );
                return 1;
        }
         */
        
        return 0;
}

DLLEXPORT sphinx_uint64_t hamdist_mv ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{

        unsigned int * mva = (unsigned int *) args->arg_values[0];
        
        int i, n;
        long long limit = -1, bf = 0;
        
        if ( !mva )
        {
            return -1;
        }

        sphinx_uint64_t arg1 = parse_fn(args, 1);

        if ( args->arg_types[0]==SPH_UDF_TYPE_UINT64SET )
        {
                // handle mva64
                n = *mva++ / 2;
                for ( i=0; i<n; i++ )
                {

                        bf = hamdist_fn((((sphinx_uint64_t)mva[1]) << 32) + (sphinx_uint64_t)mva[0], arg1);
                        
                        if (limit == -1 || bf < limit) 
                        {
                            limit = bf;
                        }

                        mva += 2; 
                }
        } else
        {
                // handle mva32
                n = *mva++;
                for ( i=0; i<n; i++ )
                {
                        bf = hamdist_fn(*mva++, parse_fn(args, arg1));

                        if (limit == -1 || bf < limit) 
                        {
                            limit = bf;
                        }
                }
        }
        
        return limit;
}

//
// $Id$
//

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
// CREATE FUNCTION hamdist RETURNS BIGINT SONAME 'hamdist.dll';
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

unsigned hamdist_fn(unsigned x, unsigned y)
{
  	unsigned dist = 0, val = x ^ y; // XOR
 
  	// Count the number of set bits
  	while(val)
  	{
    		++dist; 
    		val &= val - 1;
  	}
 
  	return dist;
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

        // allocate and init counter storage
        init->func_data = (void*) malloc ( sizeof(int) );
        if ( !init->func_data )
        {
                snprintf ( error_message, SPH_UDF_ERROR_LEN, "malloc() failed" );
                return 1;
        }
        *(int*)init->func_data = 1;

        return 0;
}

DLLEXPORT void hamdist_deinit ( SPH_UDF_INIT * init )
{
        // deallocate storage
        if ( init->func_data )
        {
                free ( init->func_data );
                init->func_data = NULL;
        }
}

DLLEXPORT sphinx_uint64_t hamdist ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
		return hamdist_fn(*args->arg_values[0], *args->arg_values[1]);
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
        
        if ( args->arg_types[1]!=SPH_UDF_TYPE_UINT32 && args->arg_types[1]!=SPH_UDF_TYPE_INT64 )
		{
                snprintf ( error_message, SPH_UDF_ERROR_LEN, "HAMDIST_MV() requires argument 2 to be uint" );
                return 1;
        }
        
        return 0;
}


DLLEXPORT sphinx_uint64_t hamdist_mv ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
	sphinx_uint64_t * mva = (sphinx_uint64_t *) args->arg_values[0];
	int i, n;
	sphinx_uint64_t limit = -1, bf = 0;
		
	if ( args->arg_types[0]==SPH_UDF_TYPE_UINT64SET )
        {
                // handle mva64
                n = *mva++ / 2;
                for ( i=0; i<n; i++ )
                {
                        bf = hamdist_fn((((sphinx_uint64_t)mva[1]) << 32) + mva[0], *args->arg_values[1]);
                        
                        if (limit == -1 || bf < limit) 
                        {
				bf = limit;
			}
						
			mva += 2; 
                }
        } else
        {
                // handle mva32
                n = *mva++;
                for ( i=0; i<n; i++ )
                {
			bf = hamdist_fn(*mva++, *args->arg_values[1]);
					
			if (limit == -1 || bf < limit) 
			{
				bf = limit;
			}
		}
        }
        
        return bf;
}

//
// $Id$
//

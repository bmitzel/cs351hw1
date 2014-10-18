#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * TODO: Implement a Vector struct to replace the array of strings
 * See happybearsoftware.com/implementing-a-dynamic-array.html
 */
	
#define FALSE			0
#define TRUE			1
#define DEBUG_CHILDREN	FALSE
#define DEBUG_SEARCH	FALSE

const int MAX_STRINGS = 1000;
const int MAX_CHARS = 100;

int main( int argc, char* argv[] )
{
	/* validate the command line  */
	if ( 4 != argc )
	{
		fprintf( stderr, "Usage: %s <filename> <key> <number>\n", argv[ 0 ] );
		exit( 1 );
	}
	
	char* filename = argv[ 1 ];
	char* key = argv[ 2 ];
	
	/* validate the input number */
	long number = strtol( argv[ 3 ], NULL, 0 );
	if ( 1 > number || ERANGE == errno )
	{
		fprintf( stderr, "Error: Number out of range.\nExiting.\n", number );
		exit( 1 );
	}
	
	/* open the input file for reading */
	FILE* fp = fopen( filename, "r" );
	if ( NULL == fp )
	{
		fprintf( stderr, "Error: Unable to open %s.\nExiting.\n", filename );
		exit( 1 );
	}
		
	/* load the strings from the input file into an array */
	int size = 0;
	char strings[ MAX_STRINGS ][ MAX_CHARS ];
	while ( size < MAX_STRINGS && fscanf( fp, "%s", strings[ size ] ) != EOF )
	{
		++size;
	}
	fclose( fp );
	
	/* fork n child processes */
	pid_t pid;
	int pids[ number ];
	int child = 0;
	for ( int i = 0; i < number; i++ )
	{
		pid = fork();
		if ( 0 > pid ) /* error occurred */
		{
			fprintf( stderr, "Error: Fork failed.\nExiting.\n" );
			exit( -1 );
		}
		else if ( 0 == pid ) /* child process */
		{
			break; /* children stop forking */
		}
		else /* parent process */
		{
#if DEBUG_CHILDREN
			printf( "Spawning pid %d\n", pid );
#endif
			pids[ child++ ] = pid; /* count the children from 0 to number - 1 */
		}
	}
	
	/* search the array for the input key in parallel */
	if ( 0 == pid ) /* child process */
	{
#if DEBUG_SEARCH
		printf( "Debug: In child #%d process...\n", child );
#endif

		/* count how many strings each child will search */
		int count;
		if ( size < number ) /* each child searches 1 string */
		{
			count = 1;
		}
		else
		{
			count = size / number;
		}
		
		/* compute where in the strings array to start and stop the search */
		int divisor = count;
		if ( child == number - 1 ) /* add any leftover strings to the last child */
		{
			count += size % number;
		}
		int begin = child * divisor;
		int end = begin + count;
#if DEBUG_SEARCH
		printf( "Debug: Size = %d, Number = %d, Count = %d, Divisor = %d, Begin = %d, End = %d\n",
				size, number, count, divisor, begin, end );
#endif

		/* search the strings for the key */
		for ( int i = begin; i < end; i++ )
		{
#if DEBUG_SEARCH
			printf( "Debug: Searching %s...\n", strings[ i ] );
#endif
			if ( NULL != strstr( strings[ i ], key ) )
			{
#if DEBUG_SEARCH
				printf( "Debug: Found %s\n", key );
#endif
				exit( 0 ); /* key found */
			}
		}
		exit( 1 ); /* key not found */
	}
	else /* parent process */
	{
		for ( int i = 0; i < number; i++ )
		{
			int exit_status;
			if ( wait( &exit_status ) < 1 ) /* error occurred */
			{
				perror( "wait" );
				exit( -1 );
			}
			if ( 0 == WEXITSTATUS( exit_status ) ) /* key found */
			{
				/* kill off all the children and exit the program */
				/* TODO: how to not try killing children that have already exited? */
				for ( int i = 0; i < number; i++ )
				{
#if DEBUG_CHILDREN
					printf( "Killing pid %d\n", pids[ i ] );
#endif
					kill( pids[ i ], SIGKILL );
				}
				exit( 0 );
			}
		}
		
		printf( "No string found.\n" );
	}
	
	return 0;
}

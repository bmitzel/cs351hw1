#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main( int argc, char* argv[] )
{
	/* open urls.txt for reading */
	FILE* fp = fopen( "urls.txt", "r" );
	if ( fp == NULL )
	{
		fprintf( stderr, "Unable to open urls.txt. Exiting.\n" );
		exit( 1 );
	}
	
	/* read urls into an array */
	int size = 0;
	char urls[ 100 ][ 100 ];
	while ( fscanf( fp, "%s", urls[ size ] ) != EOF )
	{
		++size;
	}
	
	/* fork ( n = size ) child processes */
	int i;
	pid_t pid;
	for ( i = 0; i < size; i++ )
	{
		pid = fork();
		if ( pid < 0 ) /* error occurred */
		{
			fprintf( stderr, "Fork failed. Exiting.\n" );
			exit( -1 );
		}
		else if ( pid == 0 ) /* child process */
		{
			break; /* children stop forking */
		}
	}
	
	/* download urls in parallel */
	if ( pid == 0 ) /* child process */
	{
		execlp( "/usr/bin/wget", "wget", urls[ i ], NULL );
	}
	else /* parent process */
	{
		for ( i = 0; i < size; i++ )
		{
			wait( NULL );
		}
	}
	
	fclose( fp );
	
	return 0;
}

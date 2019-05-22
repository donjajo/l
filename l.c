#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

char *get_ago( time_t *t ) {
    struct tm *last_modified;
    last_modified = localtime( t );
    char *ago = malloc( sizeof( char ) * 20 );

    time_t current_time = time( NULL );
    time_t file_time = mktime( last_modified );
    int offset = current_time - file_time;
    

    if( offset < 59 )
        sprintf( ago, "%d secs ago", offset );
    else if( offset > 59 && offset < 3600 ) {
        int min = offset / 60;
        sprintf( ago, "%d min ago", min );
    } 
    else if( offset > 3600 && offset < 86400 ) {
        int hour  = offset / 3600;
        sprintf( ago, "%d %s ago", hour, hour > 1 ? "hours" : "hour" );
    }
    else if( offset > 86400 ) {
        int days = offset / 86400;
        if( days < 31 ) 
            sprintf( ago, "%d %s ago", days, days > 1 ? "days" : "day" );
        else if( days > 31 && days < 365 ) {
            int month = days / 31;
            sprintf( ago, "%d %s ago", month, month > 1 ? "months" : "month" );
        }
        else {
            int years = days / 365;
            sprintf( ago, "%d %s ago", years, years > 1 ? "years" : "year" );
        }
    }

    return ago;
}

void list_files( char *dir, _Bool recurse, _Bool show_all ) {
    DIR *d;
    struct dirent *dir_info;
    struct stat f_stat;
    char *ago;

    if( ( d = opendir( dir ) ) == NULL ) {
        perror( "Dir error" );
        exit(1);
    }

    while( ( dir_info = readdir( d ) ) != NULL ) {
        char *filename = ( char* ) malloc( PATH_MAX +1 );
        strcpy( filename, dir );
        if( filename[ strlen( filename )-1 ] != '/' )
            strcat( filename, "/" );
        strcat( filename, dir_info->d_name );

        if( stat( filename, &f_stat ) < 0 ) 
            perror( filename );
        if( S_ISDIR( f_stat.st_mode ) && recurse && ( strcmp( dir_info->d_name, "." )  && strcmp( dir_info->d_name, ".." ) ) ) {
           list_files( filename, recurse, show_all );
        } 
        else {
            if( !show_all && dir_info->d_name[0] == '.' ) 
                continue;
            ago = get_ago( &f_stat.st_mtime );

            if( S_ISLNK( f_stat.st_mode ) ) 
                printf( "l" );
            else if( S_ISDIR( f_stat.st_mode ) ) 
                printf( "d" );
            else 
                printf( "-" );

            printf( "%c%c%c", 
                f_stat.st_mode & S_IRUSR ? 'r' : '-',
                f_stat.st_mode & S_IWUSR ? 'w' : '-',
                f_stat.st_mode & S_IXUSR ? 'x' : '-'
            );
            printf( "%c%c%c", 
                f_stat.st_mode & S_IRGRP ? 'r' : '-',
                f_stat.st_mode & S_IWGRP ? 'w' : '-',
                f_stat.st_mode & S_IXGRP ? 'x' : '-'
            );
            printf( "%c%c%c\t\t", 
                f_stat.st_mode & S_IROTH ? 'r' : '-',
                f_stat.st_mode & S_IWOTH ? 'w' : '-',
                f_stat.st_mode & S_IXOTH ? 'x' : '-'
            );
            printf( "%10d\t\t", f_stat.st_size );
            printf( "%-15s\t\t", ago );
            printf("%s", dir_info->d_name );
            
            printf( "\n" );
            free( ago );
        }
        free( filename );
    }

    closedir( d );
}

void main( int argc, char * argv[] ) {
    _Bool recurse = 0, show_all = 0;
    char arg;

    while( ( arg = getopt( argc, argv, "ra" ) ) != EOF ) {
        switch( arg ) {
            case 'r':
                recurse = 1;
                break;
            case 'a':
                show_all = 1;
                break;
            case '?':
                fprintf( stderr, "Invalid option %c", optopt );
        }
    }

    argv += optind;
    argc -= optind;

    if( argc == 0 ) 
        list_files( "./", recurse, show_all );
    else {
        while( argc-- ) {
            char *path_name = *argv++;
            list_files( path_name, recurse, show_all );
        }
    }
}
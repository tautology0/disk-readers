#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "d64.h"

char perm_string[4];
char infile[256];
int hidden,longfiles;

char *parse_permissions( permission )
int permission;
{
   strcpy( perm_string, "---" );
   /* first owner */
   if ( (permission & 4) != 0 ) perm_string[0]='r';
   if ( (permission & 2) != 0 ) perm_string[1]='w';
   if ( (permission & 1) != 0 ) perm_string[2]='x';

   return perm_string;
}

int syntax( void )
{
   printf( "Usage: a2ls -i <infile> [-alh]\n" );
}

int parse_cli( argc, argv )
int argc;
char **argv;
{
   int i;
   while ( 1 )
   {
      i=getopt( argc, argv, "i:alh?" );
      if (i == -1 ) /* no more parameters */
         break;

      switch (i)
      {
         case 'i' :
            strncpy(infile,optarg,256);
            break;

         case 'a' :
            hidden=1;
            break;
       
         case 'l' :
            longfiles=1;
            break;

         case 'h' :
         case '?' :
            syntax();
            exit (0);

         default :
            printf ("Illegal option specified: %c\n",i);
            syntax();
            exit (1);
      }
   }
   if ( strlen( infile ) == 0 )
   {
      printf( "Input file not specified\n" );
      syntax();
      exit (1);
   }

   if ( optind < argc )
   {
      printf( "Extra parameters supplied\n" );
      syntax();
      exit (1);
   }

   return optind;
}

int main( argc, argv )
int argc;
char **argv;
{
   unsigned char *dblock;
   int parameters;
   int success,i;
   FILE *inhandle;
   directory_type *directory;
   char permissions[12];

   dblock=(char *) calloc( 1, 260 );
   directory=(directory_type *) calloc( 144, sizeof( directory_type ) );

   parameters=parse_cli(argc,argv);

   inhandle=fopen( infile, "r" );
   if ( inhandle == 0 )
   {
      printf( "Failed to open file %s\n", argv[1] );
      return 1;
   }

   success=a2_list_directory( directory, inhandle );
   if ( success == -1 )
   {
      printf( "Failed to read directory\n" );
      fclose( inhandle );
      return 1;
   }

   for ( i=0; i <= success; i++ )
   {
      if ( directory[i].flags != 0 )
      {
         /* break up the access rights */ 
         unsigned char type=directory[i].flags & 0x7f;
         if ( type == 0 )
            strcpy(permissions,"t");
         else if ( type == 1  )
            strcpy(permissions,"i");
         else if ( type == 2  )
            strcpy(permissions,"a");
         else if ( type == 4  )
            strcpy(permissions,"b");
         else if ( type == 8  )
            strcpy(permissions,"S");
         else if ( type == 0x10  )
            strcpy(permissions,"r");
         else if ( type == 0x20  )
            strcpy(permissions,"A");
         else if ( type == 0x40  )
            strcpy(permissions,"B");
         else
            strcpy(permissions,"-");

         strcat(permissions,parse_permissions(directory[i].owner_permissions));
         strcat(permissions,parse_permissions(directory[i].group_permissions));
         strcat(permissions,parse_permissions(directory[i].world_permissions));

         if ( longfiles == 1 )
         {
            printf( "%s   0 %-9d%-9d%8d %s\n", permissions, directory[i].owner,\
            directory[i].group, directory[i].size*254, directory[i].name);
         }
         else
         {
            printf( "%s\n", directory[i].name );
         }
      }
   }

   return 0;
}

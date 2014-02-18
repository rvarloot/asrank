#include <iostream>
#include <set>
#include <vector>
#include <string>
#include "data.h"
#include "io.h"
#include "inference.h"

using namespace std;

/*
 * asrank [--ixp ixpFile] [--rel relationshipFile] [--clique cliqueFile] file1 [file2 ...]
 *
 * --ixp ixpFile
 *   ixpFile contains a list of AS numbers corresponding to Internet Exchange Points.
 *   Two AS numbers can be separated by a blank or newline character.
 *   Multiple files may be given (each must be precede by --ixp).
 *   The '#' character comments the rest of the line it is on.
 *         
 * --rel relationshipFile
 *   relationshipFile contains a list of AS relationships, using CAIDA format.
 *   Each relationship must be on a separate line.
 *   Multiple files may be given (each must be precede by --rel).
 *   The '#' character comments the rest of the line it is on.
 *     
 * --clique cliqueFile
 *   cliqueFile contains a list of AS numbers corresponding to Tier 1 providers.
 *   Two AS numbers can be separated by a blank or newline character.
 *   Only one file may be given (in case multiple files are given, the last one is used).
 *   The '#' character comments the rest of the line it is on.
 *           
 * file1 file2 ...
 *   These files contain AS paths.
 *   The format is one AS path per line, with each AS separated by a space (no prefix).
 *   At least one file must be provided.
 *   The '#' character comments the rest of the line it is on.
 *
 */

int main( int argc, char** argv )
{
    ios_base::sync_with_stdio( false ); // Theoretically speeds up I/O operations but requires never using stdin/stdout/stderr
    
    ////////////////
    // Parse argv //
    ////////////////

    string cliqueFile;
    vector< string > dataFiles, ixpFiles, relFiles;

    int i;
    for ( i = 1; i < argc; i++ )
    {
        string arg( argv[i] );
        if ( arg == "--ixp" )
            ixpFiles.push_back( argv[++i] );
        else if ( arg == "--clique" )
            cliqueFile = argv[++i];
        else if ( arg == "--rel" )
            relFiles.push_back( argv[++i] );
        else
            dataFiles.push_back( arg );
    }

    if ( dataFiles.empty() )
    {
        cerr << "Usage : asrank [--ixp ixpFile] [--rel relationshipFile] [--clique cliqueFile] file1 [file 2 ...]." << endl;
        return 1;
    }

    cerr << "ixp :";
    for ( unsigned int i = 0; i < ixpFiles.size(); ++i )
        cerr << " " << ixpFiles[i];
    cerr << endl << "clique : " << cliqueFile;
    cerr << endl << "relationships :";
    for ( unsigned int i = 0; i < relFiles.size(); ++i )
        cerr << " " << relFiles[i];
    cerr << endl << "data :";
    for ( unsigned int i = 0; i < dataFiles.size(); ++i )
        cerr << " " << dataFiles[i];
    cerr << endl;

    //////////////////////////////////
    // Parse files and infer clique //
    //////////////////////////////////

    set< AS > ixp = ixpFiles.empty() ? set< AS >() : loadASSet( ixpFiles );
    set< AS > clique = cliqueFile.empty() ? computeClique( dataFiles, ixp ) : loadASSet( cliqueFile );

    Data data( dataFiles, relFiles, ixp, clique );

    /////////////////////
    // Begin Inference //
    /////////////////////

    addUpstreamProviderLinks( data );
    findClientStubsSeenFromPartialVP( data );
    addLinksToSmallerProviders( data );
    breakTiesWhenNoProvider( data );
    setCliqueStubLinksAsP2C( data, clique );
    breakRemainingTies( data );
    completeWithP2PLinks( data ); 

    //////////////////////
    // End of Inference //
    //////////////////////

    printGraph( data, clique );

    return 0;
}


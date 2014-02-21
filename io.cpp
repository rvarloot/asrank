/*
 * This file must be used under the terms of the CeCILL.
 * This source file is licensed as described in the file COPYING, which
 * you should have received as part of this distribution.  The terms
 * are also available at
 *   http://www.cecill.info/licences/Licence_CeCILL_V2.1-en.txt
*/

#include "io.h"
#include <iostream>
#include <fstream>
#include <sstream>

///////////////////////////////////
// Input format detailed in io.h //
///////////////////////////////////

const char eof = char_traits< char >::eof();

// Loads AS numbers from one file
set< AS > loadASSet( const string& file )
{
    ifstream fs( file.c_str() );
    set< AS > asSet;
    char peek;
    AS as;

    while ( ( peek = fs.peek() ) != eof )
    {
        switch ( peek )
        {
            case ' ':
            case '\n':
                fs.ignore();
                break;
            case '#':
                fs.ignore( 1024, '\n' );
                break;
            default:
                fs >> as;
                asSet.insert( as );
                break;
        }
    }

    return asSet;
}

// Loads AS numbers from files
set< AS > loadASSet( const vector< string >& files )
{
    set< AS > asSet;
    for ( unsigned int i = 0; i < files.size(); ++i )
    {
        set< AS > add = loadASSet( files[i] );
        asSet.insert( add.begin(), add.end() );
    }

    return asSet;
}

// Reads relationships listed in relFiles and places them in data
// Does not chack ASs exist (should be called in Data initialization only)
void loadRelationships( const vector< string >& relFiles, Data& data )
{
    ifstream fs;
    AS as1, as2;
    char peek;
    int t;

    for ( unsigned int i = 0; i < relFiles.size(); ++i )
    {
        fs.open( relFiles[i].c_str() );

        while ( ( peek = fs.peek() ) != eof )
        {
            switch ( peek )
            {
                case ' ':
                case '\n':
                    fs.ignore();
                    break;
                case '#':
                    fs.ignore( 1024, '\n' );
                    break;
                default:
                    fs >> as1;
                    fs.ignore();
                    fs >> as2;
                    fs.ignore();
                    fs >> t;
                    data.setRelationship( as1, as2, static_cast< TypeOfRelationship >( t ) );
                    break;
            }
        }

        fs.close();
    }
} 

// Extracts an AS path from a string stream
// 'is' is expected to contain the AS numbers seperated by spaces
void extractPath( istringstream& is, Data& data, const set< AS > ixp , const set< AS >& clique )
{
    vector< AS > asPath;
    AS as;

    while ( is >> as )
        if ( ixp.count( as ) == 0 )
            if ( asPath.size() == 0 || asPath.back() != as )
                asPath.push_back( as );

    if ( asPath.empty() )
        return;

    if ( asPath.back() != as )
        asPath.push_back( as ); // IXP at end of path

    unsigned int size = asPath.size();

    unsigned int c = 0;
    set< AS > visitedAS;
    for ( unsigned int i = 0; i < size; ++i )
    {
        visitedAS.insert( asPath[i] );
        if ( clique.count( asPath[i] ) != c % 2 )
            ++c;
    }

    if ( c > 2 || visitedAS.size() != size || size < 2 )
        return; // Loops or non-consecutive clique AS in path

    // Accept path
    ASData& data0 = data[asPath[0]];
    data0.visibilityAsVP.insert( asPath[size-1] );
    data0[asPath[1]];
    data[asPath[1]][asPath[0]];

    if ( size == 2 )
        return;

    unsigned int i = 0;
    while ( true )
    {
        ++i;
        const AS& x = asPath[i-1], y = asPath[i], z = asPath[i+1];
    
        /*
         * Explicit declarations, non-optimal
         *
         * data[y][z];
         * data[z][y];
         * data[y][x].transit = true;
         * data[y][z].transit = true;
         * data[x][y][z].count++;
         * data[z][y][x].count++;
         * data[z][y][x].upstream = true;
         * data[y].transitPairs.insert( make_pair( x, z ) );
         * if ( i == size - 2 )
         * {
         *     data[z][y][x].endOfPath = true;
         *     if ( size == 3 )
         *         data[x][y][z].twoEdgePath = true;
         * }
         *
         */

        // Code runs faster
        TripletData& dZYX = data[z][y][x];
        TripletData& dXYZ = data[x][y][z];
        ++dZYX.count;
        ++dXYZ.count;

        if ( !dZYX.upstream ) // Triplet already seen
        {
            dZYX.upstream = true;
            ASData& dY = data[y];
            dY[x].transit = true;
            dY[z].transit = true;
            dY.transitPairs.insert( make_pair( x, z ) );
        }

        if ( i == size - 2 )
        {
            dZYX.endOfPath = true;
            dXYZ.twoEdgePath |= ( size == 3 );
            break;
        }
    }
}

// Loads paths from pathFiles into data
void loadPaths( const vector< string >& pathFiles, Data& data, const set< AS > ixp , const set< AS >& clique )
{
    ifstream fs;

    data.clear();

    for ( unsigned int i = 0; i < pathFiles.size(); ++i )
    {
        fs.open( pathFiles[i].c_str() );
        
        string line;

        while ( getline( fs, line ) )
        {
            if ( !line.empty() && line.find( '#' ) == string::npos )
            {
                istringstream is( line );
                extractPath( is, data, ixp, clique );
            }
        }

        fs.close();
    }
}

// Output infered relationships
void printGraph( const Data& data, const set< AS >& clique )
{
    cout << "# " << data.size() << " visible AS" << endl;
    cout << "# Clique :";
    for ( set< AS >::const_iterator it = clique.begin(); it != clique.end(); ++it )
        cout << ' ' << *it;
    cout << endl;

    for ( Data::const_iterator it = data.begin(); it != data.end(); ++it )
        for ( ASData::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt )
            if ( it->first < jt->first )
                cout << it->first << '|' << jt->first << '|' << static_cast<int>( jt->second.relationship ) << endl;
}


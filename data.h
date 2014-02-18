#ifndef DATA_H
#define DATA_H

#include <map>
#include <set>
#include <vector>
#include <string>

using namespace std;

typedef unsigned int AS;
enum TypeOfRelationship { P2P = 0, P2C = -1, C2P = 1, S2S = 2, UNKNOWN = 3 }; // cf io.h

/*
 * Data --> Overall data structure
 *
 *      asByRank (vector of AS)
 *
 * Data[x] --> AS data
 *
 *      inClique (boolean)
 *      rank (integer)
 *      transitDegree (integer) [transit degree as defined by CAIDA]
 *      customerCone (set of AS)
 *      providerCone (set of AS)
 *      visibilityAsVP (set of AS) [all AS for which the VP announces a route]
 *      transitPairs (set of AS*AS ) [pairs y z such that y:x:z is in a path]
 *
 * Data[x][y] --> Link data
 *      
 *      transit (boolean) [their exists an AS z such that z:x:y is in a path]
 *      relationship (TypeOfRelationship)
 *
 * Data[x][y][z] --> Triplet data
 *      
 *      upstream (bool) [z:y:x was seen in a path (false if only x:y:z was seen)]
 *      endOfPath (bool) [a path finished with z:y:x]
 *      twoEdgePath (bool) [the exact path x:y:z was seen]
 *      count (integer) [number of paths the triplet was in]
 */

struct TripletData
{
    TripletData();
    bool upstream;
    bool endOfPath;
    bool twoEdgePath;
    unsigned short int count;
};

struct LinkData : map< AS, TripletData >
{
    LinkData();
    bool transit;
    TypeOfRelationship relationship;
};

struct ASData : map< AS, LinkData >
{
    ASData();
    set< AS > customerCone;
    set< AS > providerCone;
    set< AS > visibilityAsVP;
    set< pair< AS, AS > > transitPairs;
    unsigned int transitDegree;
    unsigned int rank;
    bool inClique;
};

struct Data : map< AS, ASData >
{
    Data( const vector< string >& dataFiles, const vector< string >& relFile, const set< AS >& ixp, const set< AS >& clique );
    bool setRelationship( AS a, AS b, TypeOfRelationship t );

    vector< AS > asByRank;
};

#endif


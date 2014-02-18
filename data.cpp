#include "data.h"
#include "io.h"
#include <algorithm>

// 0-initialization of data structures
TripletData::TripletData() : upstream( false ), endOfPath( false ), twoEdgePath( false ), count( 0 ) {}
LinkData::LinkData() : transit( false ), relationship( UNKNOWN ) {}
ASData::ASData() : rank( 0 ), inClique( false ) {}

// Helper function
// Called once, at initialization of data
// Sets clique ASs in data and creates the corresponding peering links
// Initializes ASData::inClique
inline void setClique( Data& data, const set< AS >& clique )
{
    for ( set< AS >::iterator it = clique.begin(); it != clique.end(); ++it )
    {
        data[*it].inClique = true;
        
        for ( set< AS >::iterator jt = clique.begin(); jt != it; ++jt )
            data.setRelationship( *it, *jt, P2P );
    }
}

// Helper function
// Required for count_if in function computeTransitDegrees
bool transitPredicate( const pair< AS, LinkData >& l ) { return l.second.transit; }

// Helper function
// Called once, at initialization of data
// Computes the transit degrees of all the ASs in data
// Initializes ASData::transitDegree
inline void computeTransitDegrees( Data& data )
{
    for ( Data::iterator it = data.begin(); it != data.end(); ++it )
        it->second.transitDegree = count_if( it->second.begin(), it->second.end(), transitPredicate );
}

// Helper functor
// Required for sort in function computeASRanks
struct Comparator
{
    Comparator ( const Data& data ) : d( data ) {}
    const Data& d;

    bool operator()( AS a, AS b ) const // a < b ?
    {
        const ASData& dA = d.at( a );
        const ASData& dB = d.at( b );

        if ( dA.inClique != dB.inClique )
            return dA.inClique;

        if ( dA.transitDegree != dB.transitDegree )
            return dA.transitDegree > dB.transitDegree;

        if ( dA.size() != dB.size() )
            return dA.size() > dB.size();

        return a < b;
    }
};

// Helper function
// Called once, at initialization of data
// Computes AS ranking and fills in asByRank
// Initializes Data::asByRank and ASData::rank
inline void computeASRanks( Data& data )
{
    Comparator comp( data );

    for ( Data::const_iterator it = data.begin(); it != data.end(); ++it )
        data.asByRank.push_back( it->first );
    
    sort( data.asByRank.begin(), data.asByRank.end(), comp );

    for ( unsigned int i = 0; i < data.asByRank.size(); ++i )
        data[data.asByRank[i]].rank = i+1;
}

// Data constructor
// Loads paths into Data, then intializes all required fields
// All arguments can be empty except dataFiles, which should contain at least one file name
Data::Data( const vector< string >& dataFiles, const vector< string >& relFile, const set< AS >& ixp, const set< AS >& clique )
{
    loadPaths ( dataFiles, *this, ixp, clique );
    
    if ( !relFile.empty() )
        loadRelationships( relFile, *this );

    setClique( *this, clique );
    computeTransitDegrees( *this );
    computeASRanks( *this );

    for ( iterator it = begin(); it != end(); ++it )
    {
        it->second.customerCone.insert( it->first );
        it->second.providerCone.insert( it->first );
    }
}

// Sets relationship value
// Should always be called when setting a relationship
// When called after Data initialization, both a and b should already exist in Data
// Updates provider/customer cones
// Returns false if assignment not possible (already assigned or would create a loop)
bool Data::setRelationship( AS a, AS b, TypeOfRelationship t )
{
    Data& data = *this;

    if ( data[a][b].relationship != UNKNOWN )
        return false;

    if ( t != P2C && t != C2P )
    {
        data[a][b].relationship = t;
        data[b][a].relationship = t;
    }
    else
    {
        if ( t == C2P ) // Switch a and b so that a is the provider
        {
            AS tmp = a;
            a = b;
            b = tmp;
        }

        if ( data[a].providerCone.count( b ) != 0 )
            return false;

        data[a][b].relationship = P2C;
        data[b][a].relationship = C2P;

        for ( set< AS >::iterator it = data[a].providerCone.begin(); it != data[a].providerCone.end(); ++it )
            data[*it].customerCone.insert( data[b].customerCone.begin(), data[b].customerCone.end() );

        for ( set< AS >::iterator it = data[b].customerCone.begin(); it != data[b].customerCone.end(); ++it )
            data[*it].providerCone.insert( data[a].providerCone.begin(), data[a].providerCone.end() );
    }

    return true;
}


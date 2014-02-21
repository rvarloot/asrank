/*
 * This file must be used under the terms of the CeCILL.
 * This source file is licensed as described in the file COPYING, which
 * you should have received as part of this distribution.  The terms
 * are also available at
 *   http://www.cecill.info/licences/Licence_CeCILL_V2.1-en.txt
*/

#include "inference.h"
#include <algorithm>

// Computes a clique of central AS
// First finds the biggest clique amongst the 10 AS of largest transit degree
// Then adds ASs such that the whole remains a clique
// Computes AS transit degrees by constructing a temporary Data (almost doubles run time)
// Possible improvement: use known relationships to exclude ASs with providers
set< AS > computeClique( const vector< string >& dataFiles, const set< AS >& ixp )
{
    Data data( dataFiles, vector< string >(), ixp, set< AS >() );

    set< AS > clique;
    const vector< AS >& asByRank = data.asByRank;

    for ( unsigned short int s = 0; s < 1<<10; ++s ) // Subsets of [1..10] are represented as 10 bit integers
    {
        set< AS > candidateSet;
        bool valid = true;

        for ( unsigned char e = 0; e < 10; ++e )
            if ( (s>>e) % 2 )
                candidateSet.insert( asByRank[e] );
        
        if ( candidateSet.size() <= clique.size() )
            continue;

        for ( set< AS >::iterator it = candidateSet.begin(); it != candidateSet.end() && valid; ++it )
            for ( set< AS >::iterator jt = candidateSet.begin(); jt != it && valid; ++jt )
                if ( data.at( *it ).count( *jt ) == 0 )
                    valid = false;

        if ( valid )
            clique = candidateSet;
    }

    for ( unsigned int i = 10; i < asByRank.size(); ++i )
    {
        bool add = true;
        for ( set< AS >::iterator it = clique.begin(); it != clique.end() && add; ++it )
            if ( data.at( asByRank[i] ).count( *it ) == 0 )
                add = false;

        if ( add )
            clique.insert( asByRank[i] );
    }

    return clique;
}

// Helper function
// Top-down inference when assigning non-gradient complient links
// p2cCandidates contains the non-grandient complient links to assign
// The links in p2cCandidates are not all set first; some may be rejected due to intermediate assignments
void topDown( Data& data, set< pair< AS, AS > >& p2cCandidates )
{
    while ( !p2cCandidates.empty() )
    {
        set< pair< AS, AS > >::iterator edge = p2cCandidates.begin();
        const AS x = edge->first;
        const AS y = edge->second;
        p2cCandidates.erase( edge );

        if ( data.setRelationship( x, y, P2C ) )
        {
            const unsigned int rY = data[y].rank;
            LinkData& linkData = data[x][y];

            for ( LinkData::iterator it = linkData.begin(); it != linkData.end(); ++it )
            {
                const AS z = it->first;
                ASData& dZ = data[z];
                if ( rY < dZ.rank && dZ[y][x].upstream )
                    p2cCandidates.insert( make_pair( y, z ) );                  
            }
        }
    }
}

// Infers relationship where x>y?z, x-y?z or x?y-z (in the last case, only if the triplet is seen at least 3 times)
void addUpstreamProviderLinks( Data& data )
{
    for ( unsigned int i = 0; i < data.asByRank.size(); ++i )
    {
        const AS z = data.asByRank[i];
        ASData& dZ = data[z];

        if ( dZ.inClique )
            continue;

        for ( map< AS, LinkData >::iterator it = dZ.begin(); it != dZ.end(); ++it )
        {
            const AS y = it->first;
            LinkData& link = it->second;

            if ( data[y].rank > dZ.rank || dZ[y].relationship != UNKNOWN )
                continue;

            for ( map< AS, TripletData >::iterator jt = link.begin(); jt != link.end(); ++jt )
            {
                const AS x = jt->first;
                TripletData& triplet = jt->second;
                TypeOfRelationship t = data[x][y].relationship;

                if ( ( t == P2C && triplet.upstream ) || ( t == P2P && ( triplet.upstream || triplet.count > 2 ) ) ) // Why 2 ?
                {
                    data.setRelationship( y, z, P2C );
                    break;
                }
            }
        }           
    }
}

// Infer P2C links to stub ASs as 2 hops of a partial VP (one that does not give us a full view)
void findClientStubsSeenFromPartialVP( Data& data )
{
    for ( Data::iterator it = data.begin(); it != data.end(); ++it )
        if ( it->second.visibilityAsVP.size() * 50 < data.size() ) // visibility < 2%
            for ( ASData::iterator jt = it->second.begin(); jt != it->second.end(); ++jt )
                for ( LinkData::iterator kt = jt->second.begin(); kt != jt->second.end(); ++kt )
                    if ( kt->second.twoEdgePath && data[kt->first].transitDegree == 0 )
                        data.setRelationship( jt->first, kt->first, P2C );
}

// Helper structure
// Required in function addLinksToSmallerProviders
struct Triplet
{
    AS z;
    AS y;
    AS x;
};

// Finds providers with a lesser transit degree, requiring that they announce at least one prefix
void addLinksToSmallerProviders( Data& data )
{
    multimap< unsigned int, Triplet > candidates; // Serves as a priority queue

    for ( Data::iterator zt = data.begin(); zt != data.end(); ++zt )
    {
        for ( ASData::iterator yt = zt->second.begin(); yt != zt->second.end(); ++yt )
        {
            ASData& dY = data[yt->first];
            if ( zt->second.rank > dY.rank || yt->second.relationship != UNKNOWN )
                continue;

            for ( LinkData::iterator xt = yt->second.begin(); xt != yt->second.end(); ++xt )
            {
                if ( !xt->second.endOfPath || dY[xt->first].relationship != C2P )
                    continue;

                const Triplet t = { zt->first, yt->first, xt->first };

                candidates.insert( make_pair( xt->second.count, t ) );
            }
        }
    }

    while ( !candidates.empty() )
    {
        multimap< unsigned int, Triplet >::iterator cI = --candidates.end();
        const unsigned int priority = cI->first;
        const Triplet t = cI->second;
        candidates.erase( cI );

        if ( priority > 2 ) // Why 2 ?
        {
            if ( data.setRelationship( t.y, t.z, P2C ) ) // Propagation
            {
                LinkData& linkYZ = data[t.y][t.z];
                set< pair< AS, AS > > nextInLine;

                for ( LinkData::iterator it = linkYZ.begin(); it != linkYZ.end(); ++it )
                {
                    LinkData& linkIZ = data[it->first][t.z];
                    if ( linkIZ.relationship == UNKNOWN ) // Can now be oriented
                    {
                        const Triplet tI = { it->first, t.z, t.y };

                        if ( data[it->first].rank > data[t.z].rank ) // Top-down
                            nextInLine.insert( make_pair( t.y, t.z ) );
                        else if ( linkIZ[t.y].endOfPath ) // SmallerProvider
                            candidates.insert( make_pair( linkIZ[t.y].count, tI ) );
                    }
                }

                topDown( data, nextInLine );
            }
        }
    }
}

// Helper functor
// Required as set comparison in breakTiesWhenNoProvider
struct rankCompare
{
    rankCompare( const Data& data ) : d( data ) {}

    bool operator() ( AS a, AS b ) { return d.at( a ).rank < d.at( b ).rank; }

    const Data& d;
};

// Orients triplets x?y?z when y has no provider
void breakTiesWhenNoProvider( Data& data )
{
    rankCompare compare( data );
    for ( unsigned int i = 0; i < data.asByRank.size(); ++i )
    {
        const AS x = data.asByRank[i];
        ASData& dX = data[x];

        if ( dX.providerCone.size() == 1
            && !dX.inClique
            && dX.transitDegree >= 10 ) // Wy 10 ?
        {
            set< AS, rankCompare > neighbors( compare );
            for ( ASData::iterator it = dX.begin(); it != dX.end(); ++it )
                if ( data[it->first][x].size() != 0 && dX[it->first].relationship == UNKNOWN )
                    neighbors.insert( it->first );

            while ( !neighbors.empty() )
            {
                const AS y = *neighbors.begin();
                neighbors.erase( y );
                LinkData& dXY = dX[y];

                data.setRelationship( x, y, P2P );

                set< pair< AS, AS > > nextInLine;
                for ( LinkData::iterator it = dXY.begin(); it != dXY.end(); ++it )
                    nextInLine.insert( make_pair( y, it->first ) );

                topDown( data, nextInLine );
            }
        }
    }
}

// Infer clique-stub P2C relationships (implies no triplet x-y?z was seen with x, y in clique and z a stub) 
// Passing clique as an argument avoids having to loop over all ASs to find clique ASs
// This second argument could be removed; performance impact is negligeable
void setCliqueStubLinksAsP2C( Data& data, const set< AS >& clique )
{
    for ( set< AS >::const_iterator ct = clique.begin(); ct != clique.end(); ++ct )
    {
        ASData& dC = data[*ct];
        for ( ASData::iterator st = dC.begin(); st != dC.end(); ++st )
            if ( data[st->first].transitDegree == 0 )
                data.setRelationship( *ct, st->first, P2C );
    }
}

// Try and resolve triplets x?y?z, otherwise they will be infered as x-y-z
void breakRemainingTies( Data& data )
{
    for ( unsigned int i = 0; i < data.size(); ++i )
    {
        const AS y = data.asByRank[i];
        ASData& dY = data[y];

        if ( dY.transitDegree == 0 )
            continue;

        set< pair< AS, AS > > candidates;
        set< AS > upstream;
        set< AS > downstream;

        for ( set< pair< AS, AS > >::iterator it = dY.transitPairs.begin(); it != dY.transitPairs.end(); ++it )
        {
            LinkData& dXY = data[it->first][y];

            bool skip = false;
            for ( LinkData::iterator jt = dXY.begin(); jt != dXY.end() && !skip; ++jt )
                if ( dY.providerCone.count( jt->first ) != 0 )
                    skip = true;

            if ( skip )
                continue;

            candidates.insert( *it );
            upstream.insert( it->first );
            downstream.insert( it->second );
        }

        for ( set< pair< AS, AS > >::iterator it = candidates.begin(); it != candidates.end(); ++it )
            if ( upstream.count( it->second ) != 0 || downstream.count( it->first ) != 0 )
                candidates.erase( it-- );

        set< pair< AS, AS > > nextInLine; 
        for ( set< pair< AS, AS > >::iterator it = candidates.begin(); it != candidates.end(); ++it )
        {
            const AS z = it->second;
            if ( dY.rank < data[z].rank )
                nextInLine.insert( make_pair( y, z ) );
        }

        topDown( data, nextInLine );
    }
}

// Set all unoriented edges to P2P
void completeWithP2PLinks( Data& data )
{
    for ( Data::iterator it = data.begin(); it != data.end(); ++it )
        for ( ASData::iterator jt = it->second.begin(); jt != it->second.end(); ++jt )
            data.setRelationship( it->first, jt->first, P2P );
}


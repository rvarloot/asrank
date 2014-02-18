#ifndef INFERENCE_H
#define INFERENCE_H

#include <set>
#include <vector>
#include <string>
#include "data.h"

set< AS > computeClique( const vector< string >& dataFiles, const set< AS >& ixp );
void addUpstreamProviderLinks( Data& data );
void findClientStubsSeenFromPartialVP( Data& data );
void addLinksToSmallerProviders( Data& data );
void breakTiesWhenNoProvider( Data& data );
void setCliqueStubLinksAsP2C( Data& data, const set< AS >& clique );
void breakRemainingTies( Data& data );
void completeWithP2PLinks( Data& data ); 

#endif


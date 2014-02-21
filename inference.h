/*
 * This file must be used under the terms of the CeCILL.
 * This source file is licensed as described in the file COPYING, which
 * you should have received as part of this distribution.  The terms
 * are also available at
 *   http://www.cecill.info/licences/Licence_CeCILL_V2.1-en.txt
*/

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


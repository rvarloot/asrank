/*
 * This file must be used under the terms of the CeCILL.
 * This source file is licensed as described in the file COPYING, which
 * you should have received as part of this distribution.  The terms
 * are also available at
 *   http://www.cecill.info/licences/Licence_CeCILL_V2.1-en.txt
*/

#ifndef IO_H
#define IO_H

#include <set>
#include <vector>
#include <string>
#include "data.h"

/*
 * The character # comments the end of lines.
 *
 * //////////////
 * // AS lists //
 * //////////////
 *
 * ASs can be seperated by spaces or newlines.
 *
 * ///////////////////
 * // Relationships //
 * ///////////////////
 *
 * Relationships between ASs are written on seperate lines.
 * The format is the following:
 *
 * a|b|r
 *
 * a and b are the AS numbers
 * r is the relationship:
 *  -1 is a is a provider of b
 *   0 if a and b are peers
 *   1 if a is a customer of b
 *   2 if a and b are siblings
 *   3 if no relationship has been infered 
 *
 * ////////////////
 * // Path Files //
 * ////////////////
 *
 * One AS path per line, composed of AS numbered seperated by spaces.
 *
 */

set< AS > loadASSet( const string& file );
set< AS > loadASSet( const vector< string >& files );
void loadRelationships( const vector< string >& relFiles, Data& data );
void loadPaths( const vector< string >& pathFiles, Data& data, const set< AS > ixp , const set< AS >& clique );
void printGraph( const Data& data, const set< AS >& clique );

#endif


/*
 *  Copyright (C) 2008 Greg Benison
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef _HAVE_RULE_H
#define _HAVE_RULE_H



/*

Applying rules
---------------


dictionary + rule --> graph

* rule operates on all possible pairs (i, j) in 'dictionary'


Example rules
-------------

1) 'differ by one letter'
   let S[n] == sorted list of words of length [n]
   for all n:
      i <- 0
      while i < length:
        j <- i + 1
        while (compare (S[n][i], S[n][j]) == true):
           link (i, j)
           ++j
        i <- j

2) 'deletion of one letter'

*/

#endif /* not  _HAVE_RULE_H */

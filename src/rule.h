
/* FIXME */

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

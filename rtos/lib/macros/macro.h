#ifndef MACRO_H
#define MACRO_H

#define _cat2(A, B) A##B
#define cat2(A, B) _cat2(A, B)
#define cat3(A, B, C) cat2(A, cat2(B, C))
#define join2(A, B) cat3(A, _, B)
#define join3(A,B,C) cat3(A, _, join2(B,C))

#endif /* ifndef MACRO_H */

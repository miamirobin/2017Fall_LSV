#ifndef __ECO_MINISAT__
#define __ECO_MINISAT__

// ABC headers
extern "C" {
#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
}
#include "sat/bsat2/Solver.h"
#include <iostream>



namespace Hank {
//using:
using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::ostream;
using Minisat::Lit;
using Minisat::Var;
using Minisat::vec;
using Minisat::Solver;
using Minisat::mkLit;
using Minisat::lbool;
/*
using Minisat::l_False;
using Minisat::l_True;
using Minisat::l_Undef;
*/
typedef vec<Lit> VecLit;
typedef vec<vec<Lit> > VecVecLit;
//typedef Minisat::lbool mlbool;

//vec<Lit>
template<class T> bool isSorted( const vec<T>& );
bool isAllPositive( const VecLit& );
bool isPositiveAndSorted( const VecLit& );
Vec_Int_t * mkVecLit( const VecLit& );
void mkVecLit( Vec_Int_t *, VecLit );
void getSmallestAndBiggest( const VecLit&, Var&, Var& );
void sort( VecLit& );
void minus( const VecLit&, const VecLit&, VecLit& );





/*
void _print( Vec_Int_t* v, string period = "," , bool changeLine = true ) {
   static const unsigned maxLength = 90; 
   if( Vec_IntSize(v) < maxLength ) {
      cout << "(";
      int entry, i;
      Vec_IntForEachEntry( v, entry, i ) {
         cout << entry;
         if( i != Vec_IntSize(v) - 1 ) cout << period;
      }
      cout << ")";
   } else cout << "(length" << Vec_IntSize(v) << ">" << maxLength << ")";
   if( changeLine ) cout << endl;
}
void print( string t1, Vec_Int_t * v, string period, string t2, bool changeLine = true ) {
   cout << t1;
   _print ( v , period, false );
   cout << t2;
   if( changeLine ) cout << endl;
}
bool isSorted( Vec_Int_t * v ) {
   for ( int i = 1; i < Vec_IntSize(v); ++i ) if( Vec_IntEntry( v, i-1 ) > Vec_IntEntry( v, i ) ) return false;
   return true;
}
bool isPositive( Vec_Int_t * v ) {
   for ( int i = 0; i < Vec_IntSize(v); ++i ) if( Vec_IntEntry( v, i ) % 2 == 1 ) return false;
   return true;
}
bool isNegative( Vec_Int_t * v ) {
   for ( int i = 0; i < Vec_IntSize(v); ++i ) if( Vec_IntEntry( v, i ) % 2 == 0 ) return false;
      return true;
   }
   bool isPositiveAndSorted( Vec_Int_t * v ) { return ( isPositive( v ) && isSorted( v ) ); }
};

*/






};
#endif
/* 
                                 _oo8oo_
                                o8888888o
                                88" . "88
                                (| -_- |)
                                0\  =  /0
                              ___/'==='\___
                            .' \\|     |// '.
                           / \\|||  :  |||// \
                          / _||||| -:- |||||_ \
                         |   | \\\  -  /// |   |
                         | \_|  ''\---/''  |_/ |
                         \  .-\__  '-'  __/-.  /
                       ___'. .'  /--.--\  '. .'___
                    ."" '<  '.___\_<|>_/___.'  >' "".
                   | | :  `- \`.:`\ _ /`:.`/ -`  : | |
                   \  \ `-.   \_ __\ /__ _/   .-` /  /
               =====`-.____`.___ \_____/ ___.`____.-`=====
                                 `=---=`
 
 
      ~~~~~~~Powered by https://github.com/ottomao/bugfreejs~~~~~~~
*/

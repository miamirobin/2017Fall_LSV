#include "eco/ecoMinisat.h"
#include "vector"
namespace Hank {





bool isAllPositive( const vec<Lit>& v ) {
   for( int i = 0; i < v.size(); ++i ) if( sign( v[i] ) ) return false;
   return true;
}

template<class T> bool isSorted( const vec<T>& v ) {
   for( int i = 1; i < v.size(); ++i ) if( v[i-1] >= v[i] ) return false;
   return true;
}                        

bool isPositiveAndSorted( const vec<Lit>& v ) {
   return ( isAllPositive(v) && isSorted(v) );
}


//Vec_Int_t * mkVecLit( VecLit& v ) {
Vec_Int_t * mkVecLit( const VecLit& v ) {
   Vec_Int_t * ret = Vec_IntStart( v.size() );
   for( int i = 0; i < v.size(); ++i ) Vec_IntWriteEntry( ret, i, v[i].x );
   return ret;
}

void mkVecLit( Vec_Int_t * vin, VecLit& vout  ) {
   vout.clear();
   vout.growTo( Vec_IntSize( vin ) );
   for( int i = 0; i < vout.size(); ++i ) {
      Lit lit;   
      lit.x = Vec_IntEntry( vin, i );
      vout[i] = lit;
   }
}

void getSmallestAndBiggest( const VecLit& vect, Var& smallestVar, Var& biggestVar ) {
   // https://stackoverflow.com/questions/13544476/how-to-find-max-and-min-in-array-using-minimum-comparisons
   if( vect.empty() ) {
      smallestVar = 0;
      biggestVar = 0;
      return;
   }
   assert( isAllPositive(vect) );
   smallestVar = INT_MAX - 2;
   biggestVar = 0;
   for(int i = 0; i < vect.size(); i++ ) {
      const Var v = var( vect[i] );
      //const int bias = sizeof(Var)-1;
      const Var minDiff = v - smallestVar;
      const Var maxDiff = v - biggestVar;
      if( minDiff < 0 ) smallestVar += minDiff;
      if( maxDiff > 0 ) biggestVar += maxDiff;
   }
   //cout << "vect" << vect << " = " << "(" << smallestVar << "~" << biggestVar << ")" << flush;
}

void sort( VecLit& v ) {
   assert( isAllPositive(v) );
   const int originalSize = v.size();
   Var smallestVar, biggestVar;
   getSmallestAndBiggest( v, smallestVar, biggestVar );
   vec<bool> have( biggestVar - smallestVar + 1, false );
   for( int i = 0; i < v.size(); ++i ) have[ var(v[i]) - smallestVar ] = true;
   v.clear();
   for( int i = 0; i < have.size(); ++i ) if( have[i] ) v.push( mkLit(i+smallestVar) );
   assert( v.size() == originalSize );
}

void minus( const VecLit& lhs, const VecLit& rhs, VecLit& ret ) {
	assert( isPositiveAndSorted( lhs ) && isPositiveAndSorted( rhs ) );
	ret.clear();
	for( int lli = 0, rri = 0; lli < lhs.size(); ) {
		Var llv = var( lhs[lli] );
		Var rrv;
		if( rri == rhs.size() ) rrv = INT_MAX;
		else rrv = var( rhs[rri] );
		if( llv == rrv ) {
			++lli;
			++rri;
		} else if( llv < rrv ) {
			ret.push( mkLit(llv) );
			++lli;
		} else {
			cerr << "Warning: Minus var = " << rrv << " in rhs but not in lhs." << endl; 
			++rri;
		}
	}
}









};

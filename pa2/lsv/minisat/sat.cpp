/****************************************************************************
  FileName     [ sat.cpp ]
  PackageName  [ minisat ]
  Synopsis     [ functions for using minisat ]
  Author       [ music960633 ]
****************************************************************************/

#include "sat.h"

#include <iostream>
#include <cassert>

void SatSolver::addUnitCNF(Var var, bool val) {
  solver_->addClause(mkLit(var, !val));
}

void SatSolver::addEqCNF(Var var0, Var var1, bool inv) {
  Lit lit0 = mkLit(var0, false);
  Lit lit1 = mkLit(var1, inv);
  solver_->addClause( lit0, ~lit1);
  solver_->addClause(~lit0,  lit1);
}

void SatSolver::addAndCNF(Var out, Var in0, bool inv0, Var in1, bool inv1) {
  Lit outLit = mkLit(out, false);
  Lit in0Lit = mkLit(in0, inv0);
  Lit in1Lit = mkLit(in1, inv1);
  // out = in0 & in1
  // clause (in0 + ~out)
  solver_->addClause(in0Lit, ~outLit);
  // clause (in1 + ~out)
  solver_->addClause(in1Lit, ~outLit);
  // clause (~in0 + ~in1 + out)
  solver_->addClause(~in0Lit, ~in1Lit, outLit);
}

void SatSolver::addXorCNF(Var out, Var in0, bool inv0, Var in1, bool inv1) {
  Lit outLit = mkLit(out, false);
  Lit in0Lit = mkLit(in0, inv0);
  Lit in1Lit = mkLit(in1, inv1);
  // out = in0 ^ in1
  // clause (in0 + in1 + ~out)
  solver_->addClause( in0Lit,  in1Lit, ~outLit);
  // clause (in0 + ~in1 + out)
  solver_->addClause( in0Lit, ~in1Lit,  outLit);
  // clause (~in0 + in1 + out)
  solver_->addClause(~in0Lit,  in1Lit,  outLit);
  // clause (~in0 + ~in1 + ~out)
  solver_->addClause(~in0Lit, ~in1Lit, ~outLit);
}

/****************************************************************************
  FileName     [ sat.h ]
  PackageName  [ minisat ]
  Synopsis     [ functions for using minisat ]
  Author       [ music960633 ]
****************************************************************************/

#ifndef MINISAT_SAT_H_
#define MINISAT_SAT_H_

#include "Solver.h"

#include <iostream>
#include <cassert>

using Minisat::Solver;
using Minisat::Var;
using Minisat::Lit;
using Minisat::mkLit;
using Minisat::vec;
using Minisat::lbool;/*
using Minisat::l_True;
using Minisat::l_False;
using Minisat::l_Undef;
*/
/**************************************
  Class SatSolver
 **************************************/
class SatSolver {
 public:
  SatSolver(): solver_(NULL) {}
  ~SatSolver() {
    if(solver_ != NULL) delete solver_;
  }

  // initialize solver
  void init() {
    if(solver_ != NULL) delete solver_;
    solver_ = new Solver;
    solver_->newVar();
  }

  int nVars() const { return solver_->nVars(); }

  void setRndFreq(double freq) { solver_->random_var_freq = freq; }

  void addClause(Lit lit0) { solver_->addClause(lit0); }
  void addClause(Lit lit0, Lit lit1) { solver_->addClause(lit0, lit1); }
  void addClause(Lit lit0, Lit lit1, Lit lit2) {
    solver_->addClause(lit0, lit1, lit2);
  }
  void addClause(const vec<Lit>& clause) { solver_->addClause(clause); }

  void simplify() { solver_->simplify(); }

  // functions for adding clauses
  Var newVar() { return solver_->newVar(); }
  void addUnitCNF(Var var, bool inv);
  void addEqCNF(Var var0, Var var1, bool inv);
  void addAndCNF(Var out, Var in0, bool inv0, Var in1, bool inv1);
  void addXorCNF(Var out, Var in0, bool inv0, Var in1, bool inv1);

  // functions for solving
  bool solve() { return solver_->solve(); }
  bool solve(Var var, bool val) { return solver_->solve(mkLit(var, !val)); }
  bool solve(const vec<Lit>& assump) {
    return solver_->solve(assump);
  }/*
  lbool solveLimited(int64_t budget) {
    vec<Lit> assump;
    return solveLimited(assump, budget);
  }
  lbool solveLimited(Var var, bool val, int64_t budget) {
    vec<Lit> assump;
    assump.push(mkLit(var, !val));
    return solveLimited(assump, budget);
  }
  lbool solveLimited(const vec<Lit>& assump, int64_t budget) {
    solver_->setConfBudget(budget);
    return solver_->solveLimited(assump);
  }*/
  unsigned getVal(Var var) const {
    if(solver_->modelValue(var) == Minisat::l_True ) return 1;
    assert(solver_->modelValue(var) == Minisat::l_False);
    return 0;
  }

 private:
  Solver* solver_;
};

#endif  // MINISAT_SAT_H_

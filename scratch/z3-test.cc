#include <z3++.h>

#include <vector>

using namespace z3;

/**
 * Demonstration of how Z3 can be used to prove validity of
 * De Morgan's Duality Law: {e not(x and y) <-> (not x) or ( not y) }
 */
void demorgan() {
  std::cout << "de-Morgan example\n";
    
  context c;

  expr x = c.bool_const("x");
  expr y = c.bool_const("y");
  expr conjecture = (!(x && y)) == (!x || !y);
    
  solver s(c);
  // adding the negation of the conjecture as a constraint.
  // HEBI add constraints & formula
  s.add(!conjecture);
  std::cout << s << "\n";
  std::cout << "111"  << "\n";
  std::cout << s.to_smt2() << "\n";
  std::cout << "222"  << "\n";
  switch (s.check()) {
  case unsat:   std::cout << "de-Morgan is valid\n"; break;
  case sat:     std::cout << "de-Morgan is not valid\n"; break;
  case unknown: std::cout << "unknown\n"; break;
  }
}


/**
   \brief Find a model for <tt>x >= 1 and y < x + 3</tt>.
*/
void find_model_example1() {
  std::cout << "find_model_example1\n";
  context c;
  // HEBI define const variables
  expr x = c.int_const("x");
  expr y = c.int_const("y");
  // HEBI create server
  solver s(c);

  // HEBI add constraints
  s.add(x >= 1);
  s.add(y < x + 3);
  // HEBI check satisfiability
  std::cout << s.check() << "\n";

  // HEBI get model
  model m = s.get_model();
  std::cout << m << "\n";
  // traversing the model
  for (unsigned i = 0; i < m.size(); i++) {
    func_decl v = m[i];
    // this problem contains only constants
    assert(v.arity() == 0);
    // HEBI model extraction
    std::cout << v.name() << " = " << m.get_const_interp(v) << "\n";
  }
  // we can evaluate expressions in the model.
  // HEBI evaluate
  std::cout << "x + y + 1 = " << m.eval(x + y + 1) << "\n";
}

/**
   \brief Prove <tt>x = y implies g(x) = g(y)</tt>, and
   disprove <tt>x = y implies g(g(x)) = g(y)</tt>.
   This function demonstrates how to create uninterpreted types and
   functions.
*/
void prove_example1() {
  std::cout << "prove_example1\n";
    
  context c;
  expr x      = c.int_const("x");
  expr y      = c.int_const("y");
  sort I      = c.int_sort();
  func_decl g = function("g", I, I);
    
  solver s(c);
  // Create imply assertion
  expr conjecture1 = implies(x == y, g(x) == g(y));
  std::cout << "conjecture 1\n" << conjecture1 << "\n";
  s.add(!conjecture1);
  if (s.check() == unsat) 
    std::cout << "proved" << "\n";
  else
    std::cout << "failed to prove" << "\n";

  s.reset(); // HEBI remove all assertions from solver s

  expr conjecture2 = implies(x == y, g(g(x)) == g(y));
  std::cout << "conjecture 2\n" << conjecture2 << "\n";
  s.add(!conjecture2);
  if (s.check() == unsat) {
    std::cout << "proved" << "\n";
  }
  else {
    std::cout << "failed to prove" << "\n";
    // HEBI ONLY if fail to prove, we can get the model?
    // HEBI And the model is a counter example
    model m = s.get_model();
    std::cout << "counterexample:\n" << m << "\n";
    std::cout << "g(g(x)) = " << m.eval(g(g(x))) << "\n";
    std::cout << "g(y)    = " << m.eval(g(y)) << "\n";
  }
}

/**
   \brief Simple function that tries to prove the given conjecture using the following steps:
   1- create a solver
   2- assert the negation of the conjecture
   3- checks if the result is unsat.
*/
void prove(expr conjecture) {
  context & c = conjecture.ctx();
  solver s(c);
  s.add(!conjecture);
  std::cout << "conjecture:\n" << conjecture << "\n";
  if (s.check() == unsat) {
    std::cout << "proved" << "\n";
  }
  else {
    std::cout << "failed to prove" << "\n";
    std::cout << "counterexample:\n" << s.get_model() << "\n";
  }
}

/**
   \brief Test ite-term (if-then-else terms).
*/
void ite_example() {
  std::cout << "if-then-else example\n";
  context c;
  // HEBI instead of creating a constant, aka variable,
  // HEBI we can create a value, aka constant ...
  expr f    = c.bool_val(false);
  expr one  = c.int_val(1);
  expr zero = c.int_val(0);
  // HEBI ite, and it seems to call C API
  expr ite  = to_expr(c, Z3_mk_ite(c, f, one, zero));

  std::cout << "term: " << ite << "\n";
}

void ite_example2() {
  std::cout << "if-then-else example2\n";
  context c;
  expr b = c.bool_const("b");
  expr x = c.int_const("x");
  expr y = c.int_const("y");
  // HEBI this actually create ite on-the-fly
  std::cout << (ite(b, x, y) > 0) << "\n";
}

/**
   \brief Unsat core example
*/
void unsat_core_example1() {
  std::cout << "unsat core example1\n";
  context c;
  // We use answer literals to track assertions.
  // An answer literal is essentially a fresh Boolean marker
  // that is used to track an assertion.
  // For example, if we want to track assertion F, we 
  // create a fresh Boolean variable p and assert (p => F)
  // Then we provide p as an argument for the check method.
  expr p1 = c.bool_const("p1");
  expr p2 = c.bool_const("p2");
  expr p3 = c.bool_const("p3");
  expr x  = c.int_const("x");
  expr y  = c.int_const("y");
  solver s(c);
  s.add(implies(p1, x > 10));
  s.add(implies(p1, y > x));
  s.add(implies(p2, y < 5));
  s.add(implies(p3, y > 0));
  expr assumptions[3] = { p1, p2, p3 };
  // HEBI check three assumptions?
  // I guess it is: assume p1,p2,p3, what's the result
  std::cout << s.check(3, assumptions) << "\n";
  // HEBI the "core"
  expr_vector core = s.unsat_core();
  std::cout << core << "\n";
  std::cout << "size: " << core.size() << "\n";
  // HEBI the core should mean: what cause the trouble
  for (unsigned i = 0; i < core.size(); i++) {
    std::cout << core[i] << "\n";
  }
  // Trying again without p2
  expr assumptions2[2] = { p1, p3 };
  std::cout << s.check(2, assumptions2) << "\n";
}

/**
   \brief Unsat core example 2
*/
void unsat_core_example2() {
  std::cout << "unsat core example 2\n";
  context c;
  // The answer literal mechanism, described in the previous example,
  // tracks assertions. An assertion can be a complicated
  // formula containing containing the conjunction of many subformulas.
  expr p1 = c.bool_const("p1");
  expr x  = c.int_const("x");
  expr y  = c.int_const("y");
  solver s(c);
  expr F  = x > 10 && y > x && y < 5 && y > 0;
  s.add(implies(p1, F));
  expr assumptions[1] = { p1 };
  std::cout << s.check(1, assumptions) << "\n";
  expr_vector core = s.unsat_core();
  std::cout << core << "\n";
  std::cout << "size: " << core.size() << "\n";
  for (unsigned i = 0; i < core.size(); i++) {
    std::cout << core[i] << "\n";
  }
  // The core is not very informative, since p1 is tracking the formula F
  // that is a conjunction of subformulas.
  // Now, we use the following piece of code to break this conjunction
  // HEBI break long conjunction, so that we get a larger size of unsat core
  // into individual subformulas. First, we flat the conjunctions by
  // HEBI using the method simplify.
  std::vector<expr> qs; // auxiliary vector used to store new answer literals.
  assert(F.is_app()); // I'm assuming F is an application.
  // HEBI why only support "and" operator?
  if (F.decl().decl_kind() == Z3_OP_AND) {
    // F is a conjunction
    std::cout << "F num. args (before simplify): " << F.num_args() << "\n";
    F = F.simplify();
    std::cout << "F num. args (after simplify):  " << F.num_args() << "\n";
    for (unsigned i = 0; i < F.num_args(); i++) {
      std::cout << "Creating answer literal q" << i << " for " << F.arg(i) << "\n";
      std::stringstream qname; qname << "q" << i;
      expr qi = c.bool_const(qname.str().c_str()); // create a new answer literal
      s.add(implies(qi, F.arg(i)));
      qs.push_back(qi);
    }
  }
  // The solver s already contains p1 => F
  // To disable F, we add (not p1) as an additional assumption
  qs.push_back(!p1);
  std::cout << s.check(qs.size(), &qs[0]) << "\n";
  expr_vector core2 = s.unsat_core();
  std::cout << core2 << "\n";
  std::cout << "size: " << core2.size() << "\n";
  for (unsigned i = 0; i < core2.size(); i++) {
    std::cout << core2[i] << "\n";
  }
}

// Interesting
void sudoku_example() {
  std::cout << "sudoku example\n";

  context c;

  // 9x9 matrix of integer variables 
  expr_vector x(c);
  for (unsigned i = 0; i < 9; ++i)
    for (unsigned j = 0; j < 9; ++j) {
      std::stringstream x_name;

      x_name << "x_" << i << '_' << j;
      x.push_back(c.int_const(x_name.str().c_str()));
    }

  solver s(c);

  // each cell contains a value in {1, ..., 9}
  for (unsigned i = 0; i < 9; ++i)
    for (unsigned j = 0; j < 9; ++j) {
      s.add(x[i * 9 + j] >= 1 && x[i * 9 + j] <= 9);
    }

  // each row contains a digit at most once
  for (unsigned i = 0; i < 9; ++i) {
    expr_vector t(c);
    for (unsigned j = 0; j < 9; ++j)
      t.push_back(x[i * 9 + j]);
    s.add(distinct(t));
  }

  // each column contains a digit at most once
  for (unsigned j = 0; j < 9; ++j) {
    expr_vector t(c);
    for (unsigned i = 0; i < 9; ++i)
      t.push_back(x[i * 9 + j]);
    s.add(distinct(t));
  }

  // each 3x3 square contains a digit at most once
  for (unsigned i0 = 0; i0 < 3; i0++) {
    for (unsigned j0 = 0; j0 < 3; j0++) {
      expr_vector t(c);
      for (unsigned i = 0; i < 3; i++)
        for (unsigned j = 0; j < 3; j++)
          t.push_back(x[(i0 * 3 + i) * 9 + j0 * 3 + j]);
      s.add(distinct(t));
    }
  }

  // sudoku instance, we use '0' for empty cells
  int instance[9][9] = {{0,0,0,0,9,4,0,3,0},
                        {0,0,0,5,1,0,0,0,7},
                        {0,8,9,0,0,0,0,4,0},
                        {0,0,0,0,0,0,2,0,8},
                        {0,6,0,2,0,1,0,5,0},
                        {1,0,2,0,0,0,0,0,0},
                        {0,7,0,0,0,0,5,2,0},
                        {9,0,0,0,6,5,0,0,0},
                        {0,4,0,9,7,0,0,0,0}};

  for (unsigned i = 0; i < 9; i++)
    for (unsigned j = 0; j < 9; j++)
      if (instance[i][j] != 0)
        s.add(x[i * 9 + j] == instance[i][j]);

  std::cout << s.check() << std::endl;
  std::cout << s << std::endl;

  model m = s.get_model();
  for (unsigned i = 0; i < 9; ++i) {
    for (unsigned j = 0; j < 9; ++j)
      std::cout << m.eval(x[i * 9 + j]);
    std::cout << '\n';
  }
}



int main() {
  // demorgan();
  // find_model_example1();
  // prove_example1();
  // ite_example();
  // ite_example2();
  // unsat_core_example1();
  // unsat_core_example2();
  sudoku_example();
}

// Minion https://github.com/minion/minion
// SPDX-License-Identifier: MPL-2.0





#ifndef CONSTRAINT_POW_H
#define CONSTRAINT_POW_H

#include "../constraint_checkassign.h"
#include <math.h>

// This constraint is half-way to being changed from using
// LRINT to roundup and rounddown. Still don't quite have my head around
// this +0.5 business. Or at least I'm not convinced that it's OK.
// at least now it passes test_nightingale_pow.minion.

#ifndef LRINT
#define LRINT(x) static_cast<SysInt>(x + 0.5)
#endif

/// var1 ^ var2 = var3
template <typename VarRef1, typename VarRef2, typename VarRef3>
struct PowConstraint : public AbstractConstraint {
  virtual string constraintName() {
    return "pow";
  }

  VarRef1 var1;
  VarRef2 var2;
  VarRef3 var3;

  CONSTRAINT_ARG_LIST3(var1, var2, var3);

  PowConstraint(VarRef1 _var1, VarRef2 _var2, VarRef3 _var3)
      : var1(_var1), var2(_var2), var3(_var3) {
    CHECK((var1.initialMin() >= 0 && var2.initialMin() >= 0 && var3.initialMin() >= 0),
          "The 'pow' constraint only supports non-negative numbers at present.");
    CHECK(var2.initialMin() != 0, "The 'pow' constraint (x^y = z) does not "
                                     "allow y to contain 0, to avoid the case "
                                     "0^0.");


    // Take this check out for now; it seems to be too restrictive. The
    // propagator
    // uses doubles anyway so integer overflow is not the problem.
    // BigInt pow=1;
    // for(SysInt i=0; i<var2.initialMax(); i++) {
    //    pow=pow*var1.initialMax();
    //    CHECKSIZE(pow, "Magnitude of domain bounds is too large in 'pow'
    //    constraint.");
    //}
  }

  virtual SysInt dynamicTriggerCount() {
    return 6;
  }

  void setupTriggers() {
    moveTriggerInt(var1, 3, LowerBound);
    moveTriggerInt(var2, 4, LowerBound);
    moveTriggerInt(var3, 5, LowerBound);
    moveTriggerInt(var1, 0, UpperBound);
    moveTriggerInt(var2, 1, UpperBound);
    moveTriggerInt(var3, 2, UpperBound);
  }

  inline SysInt roundup(double x) {
    // remember all numbers are non-negative in here, so
    // how are we going to hit the lower limit for ints?
    if(x < std::numeric_limits<SysInt>::min()) {
      return std::numeric_limits<SysInt>::min();
    } else {
      return static_cast<SysInt>(x); // Actually this should round up!
    }
  }

  inline SysInt rounddown(double x) {
    if(x > std::numeric_limits<SysInt>::max()) {
      return std::numeric_limits<SysInt>::max();
    } else {
      return checked_cast<SysInt>(x);
    }
  }

  double my_pow(DomainInt x, DomainInt y) {
    return pow(checked_cast<double>(x), checked_cast<double>(y));
  }

  double my_y(DomainInt x, DomainInt z) {
    return log(checked_cast<double>(z)) / log(checked_cast<double>(x));
  }

  double my_x(DomainInt y, DomainInt z) {
    return exp(log(checked_cast<double>(z)) / checked_cast<double>(y));
  }

  virtual void propagateDynInt(SysInt flag, DomainDelta) {
    PROP_INFO_ADDONE(Pow);
    switch(checked_cast<SysInt>(flag)) {
    case 3: {
      // var3 >= min(var1) ^ min(var2)
      var3.setMin(LRINT(my_pow(var1.min(), var2.min())));
      DomainInt var1_min = var1.min();
      if(var1_min > 1)
        // var2 <= log base max(var3) of min(var1)
        var2.setMax(LRINT(my_y(var1_min, var3.max())));
      break;
    }
    case 4:
      // var3>= min(var1) ^ min(var2)
      var3.setMin(LRINT(my_pow(var1.min(), var2.min())));
      var1.setMax(LRINT(my_x(var2.min(), var3.max())));
      break;

    case 5: {
      var1.setMin(LRINT(my_x(var2.max(), var3.min())));
      DomainInt var1_max = var1.max();
      if(var1_max > 1)
        var2.setMin(LRINT(my_y(var1_max, var3.min())));
      break;
    }
    case 0: {
      var3.setMax(
          rounddown(my_pow(var1.max(),
                           var2.max()))); // wraparound was occurring here, so use rounddown
      DomainInt var1_max = var1.max();
      if(var1_max > 1)
        var2.setMin(LRINT(my_y(var1_max, var3.min())));
      break;
    }
    case 1:
      var3.setMax(rounddown(my_pow(var1.max(), var2.max()))); // wraparound here.
      var1.setMin(LRINT(my_x(var2.max(), var3.min())));
      break;

    case 2: {
      var1.setMax(LRINT(my_x(var2.min(), var3.max())));
      DomainInt var1_min = var1.min();
      if(var1_min > 1)
        var2.setMax(LRINT(my_y(var1_min, var3.max())));
      break;
    }
    }
  }

  virtual void fullPropagate() {
    setupTriggers();
    for(int i = 0; i < 6; ++i)
      propagateDynInt(i, DomainDelta::empty());
  }

  virtual BOOL checkAssignment(DomainInt* v, SysInt vSize) {
    D_ASSERT(vSize == 3);
    return my_pow(v[0], v[1]) == v[2];
  }

  virtual vector<AnyVarRef> getVars() {
    vector<AnyVarRef> v;
    v.push_back(var1);
    v.push_back(var2);
    v.push_back(var3);
    return v;
  }

  virtual bool getSatisfyingAssignment(box<pair<SysInt, DomainInt>>& assignment) {
    for(DomainInt v1 = var1.min(); v1 <= var1.max(); ++v1) {
      if(var1.inDomain(v1)) {
        for(DomainInt v2 = var2.min(); v2 <= var2.max(); ++v2) {
          if(var2.inDomain(v2) && var3.inDomain(my_pow(v1, v2))) // implicit conversion here causes
          // a warning -- perh use roundup or
          // rounddown
          {
            assignment.push_back(make_pair(0, v1));
            assignment.push_back(make_pair(1, v2));
            assignment.push_back(make_pair(2, my_pow(v1, v2)));
            return true;
          }
        }
      }
    }
    return false;
  }

  // Function to make it reifiable in the lousiest way.
  virtual AbstractConstraint* reverseConstraint() {
    return forwardCheckNegation(this);
  }
};
#endif

// Minion https://github.com/minion/minion
// SPDX-License-Identifier: MPL-2.0





#ifndef CONSTRAINT_DYNAMIC_UNARY_NOTINSET_H
#define CONSTRAINT_DYNAMIC_UNARY_NOTINSET_H

#include "dynamic_inset.h"

// Checks if a variable is not in a fixed set.
template <typename Var>
struct WatchNotInSetConstraint : public AbstractConstraint {
  virtual string constraintName() {
    return "w-notinset";
  }

  CONSTRAINT_ARG_LIST2(var, vals);

  Var var;

  vector<DomainInt> vals;

  template <typename T>
  WatchNotInSetConstraint(const Var& _var, const T& _vals)
      : var(_var), vals(_vals.begin(), _vals.end()) {
    stable_sort(vals.begin(), vals.end());
  }

  virtual SysInt dynamicTriggerCount() {
    return 2;
  }

  virtual void fullPropagate() {
    if(var.isBound()) {
      moveTriggerInt(var, 0, DomainChanged);
      propagateDynInt(0, DomainDelta::empty());
    } else {
      for(SysInt i = 0; i < (SysInt)vals.size(); ++i)
        var.removeFromDomain(vals[i]);
    }
  }

  virtual void propagateDynInt(SysInt dt, DomainDelta) {
    PROP_INFO_ADDONE(WatchInSet);
    // If we are in here, we have a bounds variable.
    D_ASSERT(var.isBound());
    // lower loop
    SysInt lowerIndex = 0;

    while(lowerIndex < (SysInt)vals.size() && vals[lowerIndex] <= var.min()) {
      var.setMin(vals[lowerIndex] + 1);
      lowerIndex++;
    }

    SysInt upperIndex = (SysInt)vals.size() - 1;

    while(upperIndex >= 0 && vals[upperIndex] >= var.max()) {
      var.setMax(vals[upperIndex] - 1);
      upperIndex--;
    }
  }

  virtual BOOL checkAssignment(DomainInt* v, SysInt vSize) {
    D_ASSERT(vSize == 1);
    return !binary_search(vals.begin(), vals.end(), v[0]);
  }

  virtual vector<AnyVarRef> getVars() {
    vector<AnyVarRef> vars;
    vars.reserve(1);
    vars.push_back(var);
    return vars;
  }

  virtual bool getSatisfyingAssignment(box<pair<SysInt, DomainInt>>& assignment) {
    /// TODO: Make faster
    for(DomainInt i = var.min(); i <= var.max(); ++i) {
      if(var.inDomain(i) && !binary_search(vals.begin(), vals.end(), i)) {
        assignment.push_back(make_pair(0, i));
        return true;
      }
    }
    return false;
  }

  virtual AbstractConstraint* reverseConstraint() {
    return new WatchInSetConstraint<Var>(var, vals);
  }
};

// From dynamic_inset.h
template <typename Var>
AbstractConstraint* WatchInSetConstraint<Var>::reverseConstraint() {
  return new WatchNotInSetConstraint<Var>(var, vals);
}

template <typename VarArray1>
AbstractConstraint* BuildCT_WATCHED_NOT_INSET(const VarArray1& _varArray_1,
                                              const ConstraintBlob& b) {
  return new WatchNotInSetConstraint<typename VarArray1::value_type>(_varArray_1[0],
                                                                     b.constants[0]);
}

/* JSON
  { "type": "constraint",
    "name": "w-notinset",
    "internal_name": "CT_WATCHED_NOT_INSET",
    "args": [ "read_var", "read_constant_list" ]
  }
*/
#endif

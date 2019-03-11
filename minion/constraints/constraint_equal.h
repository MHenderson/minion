/*
 * Minion http://minion.sourceforge.net
 * Copyright (C) 2006-09
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

/** @help constraints;eq Description
Constrain two variables to take equal values.
*/

/** @help constraints;eq Example
eq(x0,x1)
*/

/** @help constraints;eq Notes
Achieves bounds consistency.
*/

/** @help constraints;eq Reference
help constraints minuseq
*/

/** @help constraints;minuseq Description
Constraint

   minuseq(x,y)

ensures that x=-y.
*/

/** @help constraints;minuseq Reference
help constraints eq
*/

/** @help constraints;diseq Description
Constrain two variables to take different values.
*/

/** @help constraints;diseq Notes
Achieves arc consistency.
*/

/** @help constraints;diseq Example
diseq(v0,v1)
*/

// This will become always true sooner or later.

/// (var1 = var2) = var3

#ifndef CONSTRAINT_EQUAL_H
#define CONSTRAINT_EQUAL_H

// New version written by PN with bound triggers.
// Also stronger in eq case: copies bounds across rather than just propagating
// on assignment.
template <typename EqualVarRef1, typename EqualVarRef2, typename BoolVarRef, bool negated = false>
struct ReifiedEqualConstraint : public AbstractConstraint {
  virtual string constraintName() {
    return "__reify_eq";
  }

  EqualVarRef1 var1;
  EqualVarRef2 var2;
  BoolVarRef var3;

  DomainInt true_value() const {
    if(negated)
      return 0;
    else
      return 1;
  }

  DomainInt false_value() const {
    if(negated)
      return 1;
    else
      return 0;
  }

  virtual AbstractConstraint* reverseConstraint() {
    return new ReifiedEqualConstraint<EqualVarRef1, EqualVarRef2, BoolVarRef, !negated>(var1, var2,
                                                                                        var3);
  }

  virtual string fullOutputName() {

    vector<Mapper> v = var2.getMapperStack();
    if(!v.empty() && v.back() == Mapper(MAP_NEG)) {
      if(negated) {
        FATAL_REPORTABLE_ERROR();
      }
      return ConOutput::print_con("__reify_minuseq", var1, var2.popOneMapper(), var3);
    } else {

      return ConOutput::print_con(negated ? "__reify_diseq" : "__reify_eq", var1, var2, var3);
    }
  }

  ReifiedEqualConstraint(EqualVarRef1 _var1, EqualVarRef2 _var2, BoolVarRef _var3)
      : var1(_var1), var2(_var2), var3(_var3) {
    CHECK(var3.initialMin() >= 0 && var3.initialMax() <= 1,
          "reify only works on Boolean variables");
    // CHECK(var3.initialMin() < 0, "Reification variables must have domain
    // within {0,1}");
    // CHECK(var3.initialMax() > 1, "Reification variables must have domain
    // within {0,1}");
  }

  virtual SysInt dynamicTriggerCount() {
    return 5;
  }

  void trigger_setup() {
    moveTriggerInt(var1, 0, LowerBound);
    moveTriggerInt(var1, 1, UpperBound);
    moveTriggerInt(var2, 2, LowerBound);
    moveTriggerInt(var2, 3, UpperBound);
    moveTriggerInt(var3, 4, Assigned);
  }

  // rewrite the following two functions.
  virtual void fullPropagate() {
    trigger_setup();

    D_ASSERT(var3.min() >= 0);
    D_ASSERT(var3.max() <= 1);
    if(var3.isAssigned()) {
      if(var3.assignedValue() == true_value())
        eqprop();
      else {
        if(var1.isAssigned()) {
          diseqvar1assigned();
        }
        if(var2.isAssigned()) {
          diseqvar2assigned();
        }
      }
    } else { // r not assigned.
      check();
    }
  }

  virtual void propagateDynInt(SysInt i, DomainDelta) {
    PROP_INFO_ADDONE(ReifyEqual);
    switch(checked_cast<SysInt>(i)) {
    case 0:
      // var1 lower bound has moved
      if(var3.isAssigned()) {
        if(var3.assignedValue() == true_value()) {
          var2.setMin(var1.min());
        } else { // not equal.
          diseq();
        }
      } else {
        check();
      }
      break;

    case 1:
      // var1 upper bound has moved.
      if(var3.isAssigned()) {
        if(var3.assignedValue() == true_value()) {
          var2.setMax(var1.max());
        } else { // not equal.
          diseq();
        }
      } else {
        check();
      }
      break;

    case 2:
      // var2 lower bound has moved.
      if(var3.isAssigned()) {
        if(var3.assignedValue() == true_value()) {
          var1.setMin(var2.min());
        } else {
          diseq();
        }
      } else {
        check();
      }
      break;

    case 3:
      // var2 upper bound has moved.
      if(var3.isAssigned()) {
        if(var3.assignedValue() == true_value()) {
          var1.setMax(var2.max());
        } else {
          diseq();
        }
      } else {
        check();
      }
      break;

    case 4:
      DomainInt assigned_val = var3.assignedValue();
      if(assigned_val == true_value()) {
        eqprop();
      } else if(assigned_val == false_value()) {
        diseq();
      } else {
        CHECK(0, "Fatal Error in reify_eq");
      }
      break;
    }
  }

  inline void eqprop() {
    var1.setMin(var2.min());
    var1.setMax(var2.max());
    var2.setMin(var1.min());
    var2.setMax(var1.max());
  }

  inline void check() { // var1 or var2 has changed, so check
    if(var1.max() < var2.min() || var1.min() > var2.max()) { // not equal
      var3.assign(false_value());
    }
    if(var1.isAssigned() && var2.isAssigned() &&
       var1.assignedValue() == var2.assignedValue()) { // equal
      var3.assign(true_value());
    }
  }

  inline void diseqvar1assigned() {
    DomainInt remove_val = var1.assignedValue();
    if(var2.isBound()) {
      if(var2.min() == remove_val)
        var2.setMin(remove_val + 1);
      if(var2.max() == remove_val)
        var2.setMax(remove_val - 1);
    } else {
      var2.removeFromDomain(remove_val);
    }
  }

  inline void diseqvar2assigned() {
    DomainInt remove_val = var2.assignedValue();
    if(var1.isBound()) {
      if(var1.min() == remove_val)
        var1.setMin(remove_val + 1);
      if(var1.max() == remove_val)
        var1.setMax(remove_val - 1);
    } else {
      var1.removeFromDomain(remove_val);
    }
  }

  inline void diseq() {
    if(var1.isAssigned()) {
      diseqvar1assigned();
    } else if(var2.isAssigned()) {
      diseqvar2assigned();
    }
  }

  virtual bool getSatisfyingAssignment(box<pair<SysInt, DomainInt>>& assignment) {
    bool hasFalse = var3.inDomain(false_value());
    bool hasTrue = var3.inDomain(true_value());
    // D_ASSERT(hasFalse || hasTrue); No longer true
    if(hasFalse) {
      if(var1.min() != var2.max()) {
        assignment.push_back(make_pair(0, var1.min()));
        assignment.push_back(make_pair(1, var2.max()));
        assignment.push_back(make_pair(2, false_value()));
        return true;
      }

      if(var1.max() != var2.min()) {
        assignment.push_back(make_pair(0, var1.max()));
        assignment.push_back(make_pair(1, var2.min()));
        assignment.push_back(make_pair(2, false_value()));
        return true;
      }

      D_ASSERT(var1.isAssigned() && var2.isAssigned());
      D_ASSERT(var1.assignedValue() == var2.assignedValue());
      if(hasTrue) {
        assignment.push_back(make_pair(0, var1.assignedValue()));
        assignment.push_back(make_pair(1, var2.assignedValue()));
        assignment.push_back(make_pair(2, true_value()));
        return true;
      }
    }
    if(hasTrue) {
      DomainInt dom_min = max(var1.min(), var2.min());
      DomainInt dom_max = min(var1.max(), var2.max());
      for(DomainInt i = dom_min; i <= dom_max; ++i) {
        if(var1.inDomain(i) && var2.inDomain(i)) {
          assignment.push_back(make_pair(0, i));
          assignment.push_back(make_pair(1, i));
          assignment.push_back(make_pair(2, true_value()));
          return true;
        }
      }
    }
    return false;
  }

  virtual BOOL checkAssignment(DomainInt* v, SysInt v_size) {
    D_ASSERT(v_size == 3);
    D_ASSERT(v[2] == 0 || v[2] == 1);
    return (v[0] == v[1]) == (v[2] == true_value());
  }

  virtual vector<AnyVarRef> getVars() {
    vector<AnyVarRef> vars;
    vars.reserve(3);
    vars.push_back(var1);
    vars.push_back(var2);
    vars.push_back(var3);
    return vars;
  }
};

template <typename VarRef1, typename VarRef2>
struct NeqConstraintBinary : public AbstractConstraint {
  virtual string constraintName() {
    return "diseq";
  }

  VarRef1 var1;
  VarRef2 var2;

  CONSTRAINT_ARG_LIST2(var1, var2);

  NeqConstraintBinary(const VarRef1& _var1, const VarRef2& _var2) : var1(_var1), var2(_var2) {}

  virtual SysInt dynamicTriggerCount() {
    return 6;
  }

  virtual void propagateDynInt(SysInt prop_val, DomainDelta) {
    PROP_INFO_ADDONE(BinaryNeq);
    if(prop_val == 1) {
      DomainInt remove_val = var1.assignedValue();
      if(var2.isBound()) {
        if(var2.min() == remove_val)
          var2.setMin(remove_val + 1);
        if(var2.max() == remove_val)
          var2.setMax(remove_val - 1);
      } else {
        var2.removeFromDomain(remove_val);
      }
    } else if(prop_val == 3) { // ub moved var1
      if(var2.isAssigned() && var2.assignedValue() == var1.max())
        var1.setMax(var1.max() - 1);
      if(var1.isAssigned()) {
        var1assigned();
      }
    } else if(prop_val == 4) { // lb moved var1
      if(var2.isAssigned() && var2.assignedValue() == var1.min())
        var1.setMin(var1.min() + 1);
      if(var1.isAssigned()) {
        var1assigned();
      }
    } else if(prop_val == 5) { // ub moved var2
      if(var1.isAssigned() && var1.assignedValue() == var2.max())
        var2.setMax(var2.max() - 1);
      if(var2.isAssigned()) {
        var2assigned();
      }
    } else if(prop_val == 0) { // lb moved var2
      if(var1.isAssigned() && var1.assignedValue() == var2.min())
        var2.setMin(var2.min() + 1);
      if(var2.isAssigned()) {
        var2assigned();
      }
    } else {
      D_ASSERT(prop_val == 2);
      DomainInt remove_val = var2.assignedValue();
      if(var1.isBound()) {
        if(var1.min() == remove_val)
          var1.setMin(remove_val + 1);
        if(var1.max() == remove_val)
          var1.setMax(remove_val - 1);
      } else {
        var1.removeFromDomain(remove_val);
      }
    }
  }

  inline void var1assigned() {
    DomainInt remove_val = var1.assignedValue();
    if(var2.isBound()) {
      if(var2.min() == remove_val)
        var2.setMin(remove_val + 1);
      if(var2.max() == remove_val)
        var2.setMax(remove_val - 1);
    } else {
      var2.removeFromDomain(remove_val);
    }
  }

  inline void var2assigned() {
    DomainInt remove_val = var2.assignedValue();
    if(var1.isBound()) {
      if(var1.min() == remove_val)
        var1.setMin(remove_val + 1);
      if(var1.max() == remove_val)
        var1.setMax(remove_val - 1);
    } else {
      var1.removeFromDomain(remove_val);
    }
  }

  void trigger_setup() {
    if(var1.isBound()) {
      moveTriggerInt(var1, 3, UpperBound);
      moveTriggerInt(var1, 4, LowerBound);
    } else {
      moveTriggerInt(var1, 1, Assigned);
    }

    if(var2.isBound()) {
      moveTriggerInt(var2, 5, UpperBound);
      moveTriggerInt(var2, 0, LowerBound);
    } else {
      moveTriggerInt(var2, 2, Assigned);
    }
  }

  virtual void fullPropagate() {
    trigger_setup();

    if(var1.isAssigned()) {
      DomainInt remove_val = var1.assignedValue();
      if(var2.isBound()) {
        if(var2.min() == remove_val)
          var2.setMin(remove_val + 1);
        if(var2.max() == remove_val)
          var2.setMax(remove_val - 1);
      } else {
        var2.removeFromDomain(remove_val);
      }
    }
    if(var2.isAssigned()) {
      DomainInt remove_val = var2.assignedValue();
      if(var1.isBound()) {
        if(var1.min() == remove_val)
          var1.setMin(remove_val + 1);
        if(var1.max() == remove_val)
          var1.setMax(remove_val - 1);
      } else {
        var1.removeFromDomain(remove_val);
      }
    }
  }

  virtual BOOL checkAssignment(DomainInt* v, SysInt v_size) {
    D_ASSERT(v_size == 2);
    if(v[0] == v[1])
      return false;
    return true;
  }

  virtual bool getSatisfyingAssignment(box<pair<SysInt, DomainInt>>& assignment) {
    D_ASSERT(var1.min() <= var1.max());
    D_ASSERT(var2.min() <= var2.max());
    if(var1.min() != var2.max()) {
      assignment.push_back(make_pair(0, var1.min()));
      assignment.push_back(make_pair(1, var2.max()));
      return true;
    }

    if(var1.max() != var2.min()) {
      assignment.push_back(make_pair(0, var1.max()));
      assignment.push_back(make_pair(1, var2.min()));
      return true;
    }

    D_ASSERT(var1.isAssigned() && var2.isAssigned());
    D_ASSERT(var1.assignedValue() == var2.assignedValue());
    return false;
  }

  virtual vector<AnyVarRef> getVars() {
    vector<AnyVarRef> vars(2);
    vars[0] = var1;
    vars[1] = var2;
    return vars;
  }

  virtual AbstractConstraint* reverseConstraint();
};

template <typename EqualVarRef1, typename EqualVarRef2>
struct EqualConstraint : public AbstractConstraint {
  virtual string constraintName() {
    return "eq";
  }

  virtual string fullOutputName() {
    vector<Mapper> v = var2.getMapperStack();
    if(!v.empty() && v.back() == Mapper(MAP_NEG)) {
      return ConOutput::print_con("minuseq", var1, var2.popOneMapper());
    } else {
      return ConOutput::print_con("eq", var1, var2);
    }
  }

  EqualVarRef1 var1;
  EqualVarRef2 var2;
  EqualConstraint(EqualVarRef1 _var1, EqualVarRef2 _var2) : var1(_var1), var2(_var2) {}

  virtual SysInt dynamicTriggerCount() {
    return 4;
  }

  void trigger_setup() {
    moveTriggerInt(var1, 0, UpperBound);
    moveTriggerInt(var1, 1, LowerBound);
    moveTriggerInt(var2, 2, UpperBound);
    moveTriggerInt(var2, 3, LowerBound);
  }

  virtual void fullPropagate() {
    trigger_setup();
    for(int i = 0; i < 4; ++i)
      propagateDynInt(i, DomainDelta::empty());
  }

  virtual void propagateDynInt(SysInt i, DomainDelta) {
    PROP_INFO_ADDONE(Equal);
    switch(checked_cast<SysInt>(i)) {
    case 0: var2.setMax(var1.max()); return;
    case 1: var2.setMin(var1.min()); return;
    case 2: var1.setMax(var2.max()); return;
    case 3: var1.setMin(var2.min()); return;
    }
  }

  virtual BOOL checkAssignment(DomainInt* v, SysInt v_size) {
    D_ASSERT(v_size == 2);
    return (v[0] == v[1]);
  }

  virtual vector<AnyVarRef> getVars() {
    vector<AnyVarRef> vars;
    vars.reserve(2);
    vars.push_back(var1);
    vars.push_back(var2);
    return vars;
  }

  virtual bool getSatisfyingAssignment(box<pair<SysInt, DomainInt>>& assignment) {
    DomainInt min_val = max(var1.min(), var2.min());
    DomainInt max_val = min(var1.max(), var2.max());

    for(DomainInt i = min_val; i <= max_val; ++i) {
      if(var1.inDomain(i) && var2.inDomain(i)) {
        assignment.push_back(make_pair(0, i));
        assignment.push_back(make_pair(1, i));
        return true;
      }
    }
    return false;
  }

  virtual AbstractConstraint* reverseConstraint() {
    return new NeqConstraintBinary<EqualVarRef1, EqualVarRef2>(var1, var2);
  }
};

template <typename VarRef1, typename VarRef2>
AbstractConstraint* NeqConstraintBinary<VarRef1, VarRef2>::reverseConstraint() {
  return new EqualConstraint<VarRef1, VarRef2>(var1, var2);
}

template <typename EqualVarRef1, typename EqualVarRef2>
AbstractConstraint* EqualCon(EqualVarRef1 var1, EqualVarRef2 var2) {
  return new EqualConstraint<EqualVarRef1, EqualVarRef2>(var1, var2);
}

template <typename EqualVarRef1, typename EqualVarRef2>
AbstractConstraint* EqualMinusCon(EqualVarRef1 var1, EqualVarRef2 var2) {
  return new EqualConstraint<EqualVarRef1, VarNeg<EqualVarRef2>>(var1, VarNegRef(var2));
}

template <typename Var1, typename Var2>
AbstractConstraint* NeqConBinary(const Var1& var1, const Var2& var2) {
  return new NeqConstraintBinary<Var1, Var2>(var1, var2);
}

template <typename T1, typename T2>
AbstractConstraint* BuildCT_DISEQ(const T1& t1, const T2& t2, ConstraintBlob& b) {
  return NeqConBinary(t1[0], t2[0]);
}

/* JSON
{ "type": "constraint",
  "name": "diseq",
  "internal_name": "CT_DISEQ",
  "args": [ "read_var", "read_var" ]
}
*/

template <typename T1, typename T2>
AbstractConstraint* BuildCT_EQ(const T1& t1, const T2& t2, ConstraintBlob&) {
  return EqualCon(t1[0], t2[0]);
}

/* JSON
{ "type": "constraint",
  "name": "eq",
  "internal_name": "CT_EQ",
  "args": [ "read_var", "read_var" ]
}
*/

template <typename T1, typename T2>
AbstractConstraint* BuildCT_MINUSEQ(const T1& t1, const T2& t2, ConstraintBlob&) {
  return EqualMinusCon(t1[0], t2[0]);
}

/* JSON
{ "type": "constraint",
  "name": "minuseq",
  "internal_name": "CT_MINUSEQ",
  "args": [ "read_var", "read_var" ]
}
*/

template <typename VarRef1, typename BoolVarRef>
AbstractConstraint* BuildCT_DISEQ_REIFY(const vector<VarRef1>& var1, const vector<VarRef1>& var2,
                                        const vector<BoolVarRef> var3, ConstraintBlob&) {
  return new ReifiedEqualConstraint<VarRef1, VarRef1, BoolVarRef, true>(var1[0], var2[0], var3[0]);
}

template <typename VarRef1, typename VarRef2, typename BoolVarRef>
AbstractConstraint* BuildCT_DISEQ_REIFY(const vector<VarRef1>& var1, const vector<VarRef2>& var2,
                                        const vector<BoolVarRef> var3, ConstraintBlob&) {
  return new ReifiedEqualConstraint<AnyVarRef, AnyVarRef, BoolVarRef, true>(
      AnyVarRef(var1[0]), AnyVarRef(var2[0]), var3[0]);
}

/* JSON
{ "type": "constraint",
  "name": "__reify_diseq",
  "internal_name": "CT_DISEQ_REIFY",
  "args": [ "read_var", "read_var", "read_var" ]
}
*/

template <typename EqualVarRef1, typename BoolVarRef>
AbstractConstraint* BuildCT_EQ_REIFY(const vector<EqualVarRef1>& var1,
                                     const vector<EqualVarRef1>& var2,
                                     const vector<BoolVarRef> var3, ConstraintBlob&) {
  return new ReifiedEqualConstraint<EqualVarRef1, EqualVarRef1, BoolVarRef>(var1[0], var2[0],
                                                                            var3[0]);
}

template <typename EqualVarRef1, typename EqualVarRef2, typename BoolVarRef>
AbstractConstraint* BuildCT_EQ_REIFY(const vector<EqualVarRef1>& var1,
                                     const vector<EqualVarRef2>& var2,
                                     const vector<BoolVarRef> var3, ConstraintBlob&) {
  return new ReifiedEqualConstraint<AnyVarRef, AnyVarRef, BoolVarRef>(AnyVarRef(var1[0]),
                                                                      AnyVarRef(var2[0]), var3[0]);
}

/* JSON
{ "type": "constraint",
  "name": "__reify_eq",
  "internal_name": "CT_EQ_REIFY",
  "args": [ "read_var", "read_var", "read_var" ]
}
*/

template <typename EqualVarRef1, typename BoolVarRef>
AbstractConstraint* BuildCT_MINUSEQ_REIFY(const vector<EqualVarRef1>& var1,
                                          const vector<EqualVarRef1>& var2,
                                          const vector<BoolVarRef> var3, ConstraintBlob&) {
  return new ReifiedEqualConstraint<EqualVarRef1, VarNeg<EqualVarRef1>, BoolVarRef>(
      var1[0], VarNegRef(var2[0]), var3[0]);
}

template <typename EqualVarRef1, typename EqualVarRef2, typename BoolVarRef>
AbstractConstraint* BuildCT_MINUSEQ_REIFY(const vector<EqualVarRef1>& var1,
                                          const vector<EqualVarRef2>& var2,
                                          const vector<BoolVarRef> var3, ConstraintBlob&) {
  return new ReifiedEqualConstraint<AnyVarRef, AnyVarRef, BoolVarRef>(
      AnyVarRef(var1[0]), AnyVarRef(VarNegRef(var2[0])), var3[0]);
}

/* JSON
{ "type": "constraint",
  "name": "__reify_minuseq",
  "internal_name": "CT_MINUSEQ_REIFY",
  "args": [ "read_var", "read_var", "read_var" ]
}
*/

#endif

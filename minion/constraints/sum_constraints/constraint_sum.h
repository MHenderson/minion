// Minion https://github.com/minion/minion
// SPDX-License-Identifier: MPL-2.0

// This files branches between the 3 implementations of sum:

// constraint_fullsum.h  : Standard implementation.
// constraint_lightsum.h : Only for very short arrays by not storing any state.
// constraint_sum_bool.h : Only for arrays of booleans, summed to a constant.

#ifndef CONSTRAINT_SUM_QPWO
#define CONSTRAINT_SUM_QPWO

#include "constraint_fullsum.h"
#include "constraint_lightsum.h"
#include "constraint_sum_bool.h"

template <typename VarArray, typename VarSum>
AbstractConstraint* greaterEqualSumConstraint(const vector<VarArray>& _varArray,
                                              const VarSum _varSum) {
  if(_varArray.size() == 2) {
    std::array<VarArray, 2> v_array;
    for(SysInt i = 0; i < 2; ++i)
      v_array[i] = _varArray[i];
    return LightGreaterEqualSumCon(v_array, _varSum);
  } else {
    return (new LessEqualSumConstraint<vector<typename NegType<VarArray>::type>,
                                       typename NegType<VarSum>::type>(VarNegRef(_varArray),
                                                                       VarNegRef(_varSum)));
  }
}

template <typename VarArray, typename VarSum>
AbstractConstraint* lessEqualSumConstraint(const vector<VarArray>& _varArray,
                                           const VarSum _varSum) {
  if(_varArray.size() == 2) {
    std::array<VarArray, 2> v_array;
    for(SysInt i = 0; i < 2; ++i)
      v_array[i] = _varArray[i];
    return LightLessEqualSumCon(v_array, _varSum);
  } else {
    return new LessEqualSumConstraint<vector<VarArray>, VarSum>(_varArray, _varSum);
  }
}

template <typename VarArray, typename VarSum>
AbstractConstraint* BuildCT_LEQSUM(const VarArray& _varArray, const vector<VarSum>& _varSum,
                                   ConstraintBlob&) {
  if(_varArray.size() == 2) {
    std::array<typename VarArray::value_type, 2> v_array;
    for(SysInt i = 0; i < 2; ++i)
      v_array[i] = _varArray[i];
    return LightLessEqualSumCon(v_array, _varSum[0]);
  } else {
    return new LessEqualSumConstraint<VarArray, VarSum>(_varArray, _varSum[0]);
  }
}

inline AbstractConstraint* BuildCT_LEQSUM(const vector<BoolVarRef>& varArray,
                                          const vector<ConstantVar>& varSum, ConstraintBlob&) {
  SysInt t2(checked_cast<SysInt>(varSum[0].assignedValue()));
  return BoolLessEqualSumCon(varArray, t2);
}

/* JSON
{ "type": "constraint",
  "name": "sumleq",
  "internal_name": "CT_LEQSUM",
  "args": [ "read_list", "read_var" ]
}
*/

template <typename VarArray, typename VarSum>
AbstractConstraint* BuildCT_GEQSUM(const vector<VarArray>& _varArray,
                                   const vector<VarSum>& _varSum, ConstraintBlob&) {
  if(_varArray.size() == 2) {
    std::array<VarArray, 2> v_array;
    for(SysInt i = 0; i < 2; ++i)
      v_array[i] = _varArray[i];
    return LightGreaterEqualSumCon(v_array, _varSum[0]);
  } else {
    return (new LessEqualSumConstraint<vector<typename NegType<VarArray>::type>,
                                       typename NegType<VarSum>::type>(VarNegRef(_varArray),
                                                                       VarNegRef(_varSum[0])));
  }
}

inline AbstractConstraint* BuildCT_GEQSUM(const vector<BoolVarRef>& varArray,
                                          const vector<ConstantVar>& varSum, ConstraintBlob&) {
  SysInt t2(checked_cast<SysInt>(varSum[0].assignedValue()));
  return BoolGreaterEqualSumCon(varArray, t2);
}

/* JSON
{ "type": "constraint",
  "name": "sumgeq",
  "internal_name": "CT_GEQSUM",
  "args": [ "read_list", "read_var" ]
}
*/

// Don't pass in the vectors by reference, as we might need to copy them.
template <typename T1, typename T2>
AbstractConstraint* BuildCT_WEIGHTGEQSUM(vector<T1> vec, const vector<T2>& t2, ConstraintBlob& b) {
  vector<DomainInt> scale = b.constants[0];
  // Preprocess to remove any multiplications by 0, both for efficency
  // and correctness
  if(scale.size() != vec.size()) {
    FAIL_EXIT("In a weighted sum constraint, the vector of weights must have "
              "the same length as the vector of variables.");
  }
  for(UnsignedSysInt i = 0; i < scale.size(); ++i) {
    if(scale[i] == 0) {
      scale.erase(scale.begin() + i);
      vec.erase(vec.begin() + i);
      --i; // So we don't miss an element.
    }
  }

  BOOL multipliersSize_one = true;
  for(UnsignedSysInt i = 0; i < scale.size(); ++i) {
    if(scale[i] != 1 && scale[i] != -1) {
      multipliersSize_one = false;
      i = scale.size();
    }
  }

  if(multipliersSize_one) {
    vector<SwitchNeg<T1>> mult_vars(vec.size());
    for(UnsignedSysInt i = 0; i < vec.size(); ++i)
      mult_vars[i] = SwitchNeg<T1>(vec[i], scale[i]);
    return BuildCT_GEQSUM(mult_vars, t2, b);
  } else {
    vector<MultiplyVar<T1>> mult_vars(vec.size());
    for(UnsignedSysInt i = 0; i < vec.size(); ++i)
      mult_vars[i] = MultiplyVar<T1>(vec[i], scale[i]);
    return BuildCT_GEQSUM(mult_vars, t2, b);
  }
}

/* JSON
{ "type": "constraint",
  "name": "weightedsumgeq",
  "internal_name": "CT_WEIGHTGEQSUM",
  "args": [ "read_constant_list", "read_list", "read_var" ]
}
*/

template <typename T1, typename T2>
AbstractConstraint* BuildCT_WEIGHTLEQSUM(vector<T1> vec, const vector<T2>& t2, ConstraintBlob& b) {
  vector<DomainInt> scale = b.constants[0];
  // Preprocess to remove any multiplications by 0, both for efficency
  // and correctness
  if(scale.size() != vec.size()) {
    FAIL_EXIT("In a weighted sum constraint, the vector of weights must have "
              "the same length to the vector of variables.");
  }
  for(UnsignedSysInt i = 0; i < scale.size(); ++i) {
    if(scale[i] == 0) {
      scale.erase(scale.begin() + i);
      vec.erase(vec.begin() + i);
      --i; // So we don't miss an element.
    }
  }

  BOOL multipliersSize_one = true;
  for(UnsignedSysInt i = 0; i < scale.size(); ++i) {
    if(scale[i] != 1 && scale[i] != -1) {
      multipliersSize_one = false;
      i = scale.size();
    }
  }

  if(multipliersSize_one) {
    vector<SwitchNeg<T1>> mult_vars(vec.size());
    for(UnsignedSysInt i = 0; i < vec.size(); ++i)
      mult_vars[i] = SwitchNeg<T1>(vec[i], scale[i]);
    return BuildCT_LEQSUM(mult_vars, t2, b);
  } else {
    vector<MultiplyVar<T1>> mult_vars(vec.size());
    for(UnsignedSysInt i = 0; i < vec.size(); ++i)
      mult_vars[i] = MultiplyVar<T1>(vec[i], scale[i]);
    return BuildCT_LEQSUM(mult_vars, t2, b);
  }
}

/* JSON
{ "type": "constraint",
  "name": "weightedsumleq",
  "internal_name": "CT_WEIGHTLEQSUM",
  "args": [ "read_constant_list", "read_list", "read_var" ]
}
*/

#endif

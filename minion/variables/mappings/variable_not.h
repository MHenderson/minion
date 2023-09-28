// Minion https://github.com/minion/minion
// SPDX-License-Identifier: MPL-2.0

#include "../../triggering/constraint_abstract.h"

// This is a temporary fix to get around the fact that 'VarNot' is defined in
// some windows header.
// Long term, probably need a better solution.
#ifdef _WIN32
#define VarNot Minion_VarNot
#endif

/**
 * @brief Nots a variable reference.
 *
 * Takes a variable, and returns a new 'psuedo-variable', which is the same as
 * the not of the
 * original. This new variable takes up no extra space of any kind after
 * compilation
 * is performed
 */
template <typename VarRef>
struct VarNot {
  static const BOOL isBool = true;
  static const BoundType isBoundConst = VarRef::isBoundConst;
  VarRef data;

  AnyVarRef popOneMapper() const {
    return data;
  }

  BOOL isBound() const {
    return data.isBound();
  }

  VarNot(const VarRef& _data) : data(_data) {
    // D_ASSERT(data.initialMin() == 0);
    // D_ASSERT(data.initialMax() == 1);
  }

  VarNot() : data() {}

  VarNot(const VarNot& b) : data(b.data) {}

  void operator=(const VarNot& val) {
    data = val.data;
  }

  // There is a good reason this is like this. It is because the 'neg' of an
  // BOOL var
  // might be used in arithmetic. This is an extension to all of the integers
  // which
  // swaps 0 and 1.
  DomainInt swap(DomainInt i) const {
    return -i + 1;
  }

  BOOL isAssigned() const {
    return data.isAssigned();
  }

  DomainInt assignedValue() const {
    return swap(data.assignedValue());
  }

  BOOL isAssignedValue(DomainInt i) const {
    return data.isAssigned() && swap(data.assignedValue()) == i;
  }

  BOOL inDomain(DomainInt b) const {
    return data.inDomain(swap(b));
  }

  BOOL inDomain_noBoundCheck(DomainInt b) const {
    return data.inDomain(swap(b));
  }

  DomainInt domSize() const {
    return data.domSize();
  }

  DomainInt max() const {
    return swap(data.min());
  }

  DomainInt min() const {
    return swap(data.max());
  }

  DomainInt initialMax() const {
    return swap(data.initialMin());
  }

  DomainInt initialMin() const {
    return swap(data.initialMax());
  }

  void setMax(DomainInt i) {
    data.setMin(swap(i));
  }

  void setMin(DomainInt i) {
    data.setMax(swap(i));
  }

  void uncheckedAssign(DomainInt b) {
    data.uncheckedAssign(swap(b));
  }

  void assign(DomainInt b) {
    data.assign(swap(b));
  }

  void removeFromDomain(DomainInt b) {
    data.removeFromDomain(swap(b));
  }

  friend std::ostream& operator<<(std::ostream& o, const VarNot& n) {
    return o << "Not " << n.data;
  }

  DomainInt getDomainChange(DomainDelta d) {
    return data.getDomainChange(d);
  }

  void addDynamicTrigger(Trig_ConRef t, TrigType type, DomainInt pos = NoDomainValue,
                         TrigOp op = TO_Default) {
    switch(type) {
    case UpperBound: data.addDynamicTrigger(t, LowerBound, pos, op); break;
    case LowerBound: data.addDynamicTrigger(t, UpperBound, pos, op); break;
    case Assigned:
    case DomainChanged: data.addDynamicTrigger(t, type, pos, op); break;
    case DomainRemoval: data.addDynamicTrigger(t, DomainRemoval, -(pos - 1), op); break;
    default: D_FATAL_ERROR("Broken dynamic trigger");
    }
  }

  vector<AbstractConstraint*>* getConstraints() {
    return data.getConstraints();
  }

  void addConstraint(AbstractConstraint* c) {
    data.addConstraint(c);
  }

  DomainInt getBaseVal(DomainInt v) const {
    return data.getBaseVal(swap(v));
  }

  Var getBaseVar() const {
    return data.getBaseVar();
  }

  vector<Mapper> getMapperStack() const {
    vector<Mapper> v = data.getMapperStack();
    v.push_back(Mapper(MAP_NOT));
    return v;
  }

#ifdef WDEG
  DomainInt getBaseWdeg() {
    return data.getBaseWdeg();
  }

  void incWdeg() {
    data.incWdeg();
  }
#endif
};

template <typename T>
struct NotType {
  typedef VarNot<T> type;
};

// not(not(T)) = T
template <typename T>
struct NotType<VarNot<T>> {
  typedef T type;
};

template <typename T>
struct NotType<vector<T>> {
  typedef vector<VarNot<T>> type;
};

template <typename T, std::size_t i>
struct NotType<std::array<T, i>> {
  typedef std::array<VarNot<T>, i> type;
};

template <typename VRef>
typename NotType<VRef>::type VarNotRef(const VRef& var_ref) {
  return VarNot<VRef>(var_ref);
}

template <typename VRef>
VRef VarNotRef(const VarNot<VRef>& var_ref) {
  return var_ref.data;
}

template <typename VarRef>
vector<VarNot<VarRef>> VarNotRef(const vector<VarRef>& varArray) {
  vector<VarNot<VarRef>> Not_array;
  Not_array.reserve(varArray.size());
  for(UnsignedSysInt i = 0; i < varArray.size(); ++i)
    Not_array.push_back(VarNotRef(varArray[i]));
  return Not_array;
}

template <typename VarRef, std::size_t i>
std::array<VarNot<VarRef>, i> VarNotRef(const std::array<VarRef, i>& varArray) {
  std::array<VarNot<VarRef>, i> Not_array;
  for(UnsignedSysInt l = 0; l < i; ++l)
    Not_array[l] = VarNotRef(varArray[l]);
  return Not_array;
}

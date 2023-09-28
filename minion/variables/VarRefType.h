// Minion https://github.com/minion/minion
// SPDX-License-Identifier: MPL-2.0

#include "../triggering/constraint_abstract.h"

#ifndef VARREFTYPE_H
#define VARREFTYPE_H

// The follow two types are designed to allow turning a variable type which
// must be fed to a container, into a stand-alone class which is ready to be
// used as a variable.
template <typename InternalRefType>
struct VarRefType {
  static const BOOL isBool = InternalRefType::isBool;
  static const BoundType isBoundConst = InternalRefType::isBoundConst;

  InternalRefType data;

  BOOL isBound() const {
    return data.isBound();
  }

  AnyVarRef popOneMapper() const {
    FATAL_REPORTABLE_ERROR();
  }

  VarRefType(const InternalRefType& _data) : data(_data) {}

  void operator=(const VarRefType& val) {
    data = val.data;
  }

  VarRefType() : data() {}

  VarRefType(const VarRefType& b) : data(b.data) {}

  BOOL isAssigned() const {
    return GET_CONTAINER().isAssigned(data);
  }

  DomainInt assignedValue() const {
    return GET_CONTAINER().getAssignedValue(data);
  }

  BOOL isAssignedValue(DomainInt i) const {
    return GET_CONTAINER().isAssigned(data) && GET_CONTAINER().getAssignedValue(data) == i;
  }

  BOOL inDomain(DomainInt b) const {
    return GET_CONTAINER().inDomain(data, b);
  }

  BOOL inDomain_noBoundCheck(DomainInt b) const {
    return GET_CONTAINER().inDomain_noBoundCheck(data, b);
  }

  DomainInt domSize() const {
    return GET_CONTAINER().getDomSize(data);
  }

  DomainInt max() const {
    return GET_CONTAINER().getMax(data);
  }

  DomainInt min() const {
    return GET_CONTAINER().getMin(data);
  }

  DomainInt initialMax() const {
    return GET_CONTAINER().initialMax(data);
  }

  DomainInt initialMin() const {
    return GET_CONTAINER().initialMin(data);
  }

  void setMax(DomainInt i) {
    GET_CONTAINER().setMax(data, i);
  }

  void setMin(DomainInt i) {
    GET_CONTAINER().setMin(data, i);
  }

  void uncheckedAssign(DomainInt b) {
    GET_CONTAINER().uncheckedAssign(data, b);
  }

  void assign(DomainInt b) {
    GET_CONTAINER().assign(data, b);
  }

  void removeFromDomain(DomainInt b) {
    GET_CONTAINER().removeFromDomain(data, b);
  }

  vector<AbstractConstraint*>* getConstraints() {
    return GET_CONTAINER().getConstraints(data);
  }

  void addConstraint(AbstractConstraint* c) {
    GET_CONTAINER().addConstraint(data, c);
  }

  DomainInt getBaseVal(DomainInt v) const {
    return GET_CONTAINER().getBaseVal(data, v);
  }

  vector<Mapper> getMapperStack() const {
    return vector<Mapper>();
  }

  Var getBaseVar() const {
    return GET_CONTAINER().getBaseVar(data);
  }

#ifdef WDEG
  DomainInt getBaseWdeg() {
    return GET_CONTAINER().getBaseWdeg(data);
  }

  void incWdeg() {
    GET_CONTAINER().incWdeg(data);
  }
#endif

  friend std::ostream& operator<<(std::ostream& o, const VarRefType& v) {
    return o << InternalRefType::name() << v.data.varNum;
  }

  DomainInt getDomainChange(DomainDelta d) {
    return d.XXX_getDomain_diff();
  }

  void addDynamicTrigger(Trig_ConRef t, TrigType type, DomainInt pos = NoDomainValue,
                         TrigOp op = TO_Default) {
    GET_CONTAINER().addDynamicTrigger(data, t, type, pos, op);
  }
};

template <typename GetContainer, typename InternalRefType>
struct QuickVarRefType {
  static const BOOL isBool = InternalRefType::isBool;
  static const BoundType isBoundConst = InternalRefType::isBoundConst;
  InternalRefType data;

  AnyVarRef popOneMapper() const {
    FATAL_REPORTABLE_ERROR();
  }

  BOOL isBound() const {
    return data.isBound();
  }

  QuickVarRefType(const InternalRefType& _data) : data(_data) {}

  QuickVarRefType() : data() {}

  QuickVarRefType(const QuickVarRefType& b) : data(b.data) {}

  void operator=(const QuickVarRefType& val) {
    data = val.data;
  }

  BOOL isAssigned() const {
    return data.isAssigned();
  }

  DomainInt assignedValue() const {
    return data.assignedValue();
  }

  BOOL isAssignedValue(DomainInt i) const {
    return data.isAssigned() && data.assignedValue() == i;
  }
  BOOL inDomain(DomainInt b) const {
    return data.inDomain(b);
  }

  BOOL inDomain_noBoundCheck(DomainInt b) const {
    return data.inDomain_noBoundCheck(b);
  }

  DomainInt domSize() const {
    return data.domSize();
  }

  DomainInt max() const {
    return data.max();
  }

  DomainInt min() const {
    return data.min();
  }

  DomainInt initialMax() const {
    return data.initialMax();
  }

  DomainInt initialMin() const {
    return data.initialMin();
  }

  void setMax(DomainInt i) {
    GET_CONTAINER().setMax(data, i);
  }

  void setMin(DomainInt i) {
    GET_CONTAINER().setMin(data, i);
  }

  void uncheckedAssign(DomainInt b) {
    GET_CONTAINER().uncheckedAssign(data, b);
  }

  void assign(DomainInt b) {
    GET_CONTAINER().assign(data, b);
  }

  void removeFromDomain(DomainInt b) {
    GET_CONTAINER().removeFromDomain(data, b);
  }

  vector<AbstractConstraint*>* getConstraints() {
    return GET_CONTAINER().getConstraints(data);
  }

  void addConstraint(AbstractConstraint* c) {
    GET_CONTAINER().addConstraint(data, c);
  }

  DomainInt getBaseVal(DomainInt v) const {
    return data.getBaseVal(v);
  }

  Var getBaseVar() const {
    return data.getBaseVar();
  }

  vector<Mapper> getMapperStack() const {
    return vector<Mapper>();
  }

#ifdef WDEG
  DomainInt getBaseWdeg() {
    return GET_CONTAINER().getBaseWdeg(data);
  }

  void incWdeg() {
    GET_CONTAINER().incWdeg(data);
  }
#endif

  friend std::ostream& operator<<(std::ostream& o, const QuickVarRefType& b) {
    return o << "Bool:" << b.data;
  }

  DomainInt getDomainChange(DomainDelta d) {
    return d.XXX_getDomain_diff();
  }

  void addDynamicTrigger(Trig_ConRef t, TrigType type, DomainInt pos = NoDomainValue,
                         TrigOp op = TO_Default) {
    GET_CONTAINER().addDynamicTrigger(data, t, type, pos, op);
  }
};

#endif

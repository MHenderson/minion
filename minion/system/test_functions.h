// Minion https://github.com/minion/minion
// SPDX-License-Identifier: MPL-2.0

#ifndef TEST_FUNCTIONS_H
#define TEST_FUNCTIONS_H

#include <sstream>
#include <string>

#include <vector>

template <typename T>
std::string getBaseVarName(const T& t);

template <typename Var>
void add_varDomTo_json(const Var& v, std::ostream& s) {
  s << '"' << getBaseVarName(v) << "\":";
  if(v.isAssigned() || v.isBound()) {
    s << "[ [" << v.min() << "," << v.max() << "] ]";
  } else {
    s << "[ ";
    DomainInt lower_range = v.min();
    bool in_range = true;
    for(DomainInt i = v.min() + 1; i < v.max(); ++i) {
      if(in_range) {
        if(!v.inDomain(i)) {
          s << "[" << lower_range << "," << i - 1 << "],";
          in_range = false;
        }
      } else {
        if(v.inDomain(i)) {
          lower_range = i;
          in_range = true;
        }
      }
    }

    if(in_range) {
      s << "[" << lower_range << "," << v.max() << "] ]";
    } else {
      s << "[" << v.max() << "," << v.max() << "] ]";
    }
  }
}

template <typename T>
raw_json getDom_as_json(const vector<T>& vec) {
  std::ostringstream s;
  s << "{";
  if(!vec.empty()) {
    add_varDomTo_json(vec[0], s);
    for(UnsignedSysInt i = 1; i < vec.size(); ++i) {
      s << ", ";
      add_varDomTo_json(vec[i], s);
    }
  }
  s << "}";
  return raw_json(s.str());
}

template <typename Var>
string getDom_as_string(const Var& v) {
  ostringstream s;
  if(v.isAssigned()) {
    s << v.assignedValue();
  } else {
    if(v.isBound()) {
      s << "[" << v.min() << "," << v.max() << "]";
    } else {
      s << "{" << v.min();
      for(DomainInt i = v.min() + 1; i <= v.max(); ++i)
        if(v.inDomain(i))
          s << "," << i;
      s << "}";
    }
  }
  return s.str();
}

template <typename T>
string getDom_as_string(const vector<T>& vec) {
  string output("<");
  if(!vec.empty()) {
    output += getDom_as_string(vec[0]);
    for(UnsignedSysInt i = 1; i < vec.size(); ++i) {
      output += ",";
      output += getDom_as_string(vec[i]);
    }
  }
  output += ">";
  return output;
}

// Count number of literals present in an array of variables.
template <typename Vars>
DomainInt litCount(Vars& v) {
  DomainInt lits = 0;
  for(SysInt i = 0; i < (SysInt)v.size(); ++i) {
    if(v[i].isBound()) {
      lits += v[i].max() - v[i].min() + 1;
    } else {
      for(DomainInt j = v[i].min(); j <= v[i].max(); ++j)
        if(v[i].inDomain(j))
          lits++;
    }
  }
  return lits;
}

#endif

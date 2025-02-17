#ifndef TRIES_MINLIB_VFD
#define TRIES_MINLIB_VFD

#include "minlib/minlib.hpp"

template <typename T>
struct TrieObj {
  T val;
  TrieObj* ptr;
};

namespace TrieBuilder {
template <typename T>
struct earlyTrieObj {
  T val;
  int offset;
};

// Find how many values there are for index 'depth' between tuples
// start_pos and end_pos.
template <typename Tuples>
int get_distinctValues(const Tuples& tuples, int start_pos, int end_pos, int depth) {
  int currentVal = tuples[start_pos][depth];
  int foundValues = 1;
  for(int i = start_pos; i < end_pos; ++i) {
    if(currentVal != tuples[i][depth]) {
      currentVal = tuples[i][depth];
      foundValues++;
    }
  }
  return foundValues;
}

template <typename Value, typename Tuples>
void buildEarlyTrie(std::vector<earlyTrieObj<Value>>& initialTrie, const Tuples& tuples,
                      int start_pos, int end_pos, int depth) {
  const bool last_stage = (depth == (int)tuples[0].size() - 1);
  assert(depth <= (int)tuples[0].size() - 1);
  assert(start_pos <= end_pos);
  int values = get_distinctValues(tuples, start_pos, end_pos, depth);

  int start_section = initialTrie.size();
  // Make space for this list of values.
  // '+1' is for end marker.
  initialTrie.resize(initialTrie.size() + values + 1);

  int currentVal = tuples[start_pos][depth];
  int currentStart = start_pos;
  int numOfVal = 0;

  for(int i = start_pos; i < end_pos; ++i) {
    if(currentVal != tuples[i][depth]) {
      initialTrie[start_section + numOfVal].val = currentVal;
      if(last_stage) {
        initialTrie[start_section + numOfVal].offset = -1;
      } else {
        initialTrie[start_section + numOfVal].offset = initialTrie.size();
        buildEarlyTrie(initialTrie, tuples, currentStart, i, depth + 1);
      }
      currentVal = tuples[i][depth];
      currentStart = i;
      numOfVal++;
    }
  }

  // Also have to cover last stretch of values.
  initialTrie[start_section + numOfVal].val = currentVal;
  if(last_stage)
    initialTrie[start_section + numOfVal].offset = -1;
  else {
    initialTrie[start_section + numOfVal].offset = initialTrie.size();
    buildEarlyTrie(initialTrie, tuples, currentStart, end_pos, depth + 1);
  }

  assert(numOfVal + 1 == values);
  initialTrie[start_section + values].val = std::numeric_limits<Value>::max();
  initialTrie[start_section + values].offset = -1;
}

template <typename Value>
TrieObj<Value>* buildFinalTrie(const std::vector<earlyTrieObj<Value>>& earlyTrie) {
  TrieObj<Value>* trie = new TrieObj<Value>[earlyTrie.size()];
  for(size_t i = 0; i < earlyTrie.size(); ++i) {
    trie[i].val = earlyTrie[i].val;
    D_ASSERT(earlyTrie[i].offset >= -1);
    if(earlyTrie[i].offset == -1)
      trie[i].ptr = 0;
    else
      trie[i].ptr = trie + earlyTrie[i].offset;
  }

  return trie;
}

} // end namespace TrieBuilder

template <typename Tuples>
TrieObj<int>* buildTrie(const Tuples& tuples) {
  if(tuples.empty()) {
    return 0;
  } else if(tuples[0].empty()) {
    TrieObj<int>* t = new TrieObj<int>;
    t->val = std::numeric_limits<int>::max();
    t->ptr = 0;
    return t;
  } else {
    typedef int Value;
    std::vector<TrieBuilder::earlyTrieObj<Value>> earlyTrie;
    TrieBuilder::buildEarlyTrie<Value>(earlyTrie, tuples, 0, tuples.size(), 0);
    return buildFinalTrie(earlyTrie);
  }
}

template <typename Value>
void randomiseTrie(TrieObj<Value>* to) {
  if(to == NULL)
    return;

  TrieObj<Value>* end = to;
  while(end->val != std::numeric_limits<Value>::max()) {
    if(end->ptr)
      randomiseTrie(end->ptr);
    end++;
  }
  std::shuffle(to, end, GET_GLOBAL(global_random_gen));
}

template <typename Value>
std::vector<std::vector<Value>> unrollTrie(TrieObj<Value>* to) {
  std::vector<std::vector<Value>> final_tuples;
  for(; to->val != std::numeric_limits<Value>::max(); to++) {
    std::vector<std::vector<Value>> loop_tuples;
    if(to->ptr == 0)
      loop_tuples.resize(1);
    else
      loop_tuples = unrollTrie(to->ptr);
    for(auto it = loop_tuples.begin(); it != loop_tuples.end(); ++it)
      it->insert(it->begin(), to->val);
    final_tuples.insert(final_tuples.end(), loop_tuples.begin(), loop_tuples.end());
  }
  return final_tuples;
}

#endif

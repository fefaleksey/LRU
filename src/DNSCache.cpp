#include "DNSCache.h"

namespace lru {

DNSCache::DNSCache(size_t maxSize) : maxSize(maxSize), sortedUsages(maxSize) {
  map.reserve(maxSize);
}

DNSCache &DNSCache::Instance(size_t maxSize) {
  if (maxSize == 0) {
    // actually we can check it in resolve/update methods and return
    // empty string, but I think it doesn't matter
    throw std::runtime_error("maxSize must be positive");
  }

  static DNSCache inst(maxSize);
  return inst;
}

std::string DNSCache::resolve(const std::string &name) {
  absl::MutexLock lock(&mutex);

  auto it = map.find(name);
  if (it == map.end()) {
    return "";
  }

  sortedUsages.moveToFront(it->second.nodePtr);
  return it->second.value;
}

void DNSCache::update(const std::string &name, const std::string &ip) {
  absl::MutexLock lock(&mutex);

  auto it = map.find(name);
  if (it != map.end()) {
    it->second.value = ip;
    sortedUsages.moveToFront(it->second.nodePtr);
    return;
  }

  if (map.size() == maxSize) {
    map.erase(sortedUsages.getTail()->key);
  }

  UsagesList::Node *insertedNodePtr = sortedUsages.insert(name);
  map[name] = {.value = ip, .nodePtr = insertedNodePtr};
}

void DNSCache::dump() {
  std::cout << "-------------------------\n";
  sortedUsages.dump();

  for (auto &[k, v] : map) {
    std::cout << k << " " << v.value << std::endl;
  }
}

DNSCache::UsagesList::UsagesList(size_t maxSize) {
  nodesStorage.resize(maxSize);
  head = &nodesStorage.front();
  tail = head;
  for (size_t i = 0; i < nodesStorage.size() - 1; i++) {
    Node *current = &nodesStorage[i];
    Node *next = &nodesStorage[i + 1];
    current->next = next;
    next->prev = current;
  }
  Node *first = &nodesStorage.front();
  Node *last = &nodesStorage.back();
  first->prev = last;
  last->next = first;
}

DNSCache::UsagesList::Node *DNSCache::UsagesList::insert(
    const std::string &key) {
  head = head->prev;
  head->key = key;
  if (tail == head) {
    tail = tail->prev;
  }
  return head;
}

void DNSCache::UsagesList::moveToFront(Node *node) {
  if (node == head) {
    return;
  }

  if (node == tail) {
    tail = tail->prev;
  }

  node->next->prev = node->prev;
  node->prev->next = node->next;

  node->next = head;
  node->prev = head->prev;

  head->prev->next = node;
  head->prev = node;

  head = node;
}

void DNSCache::UsagesList::dump() {
  auto ptr = head;
  while (ptr != tail) {
    std::cout << ptr->key << " ";
    ptr = ptr->next;
  }
  std::cout << ptr->key << std::endl;
}

}  // namespace lru

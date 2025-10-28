#pragma once

#include <absl/container/flat_hash_map.h>

#include <mutex>
#include <vector>

namespace lru {

class DNSCache {
 public:
  DNSCache(const DNSCache &) = delete;
  DNSCache(DNSCache &&) = delete;
  DNSCache &operator=(const DNSCache &) = delete;
  DNSCache &&operator=(DNSCache &&) = delete;

  static DNSCache &Instance(size_t maxSize = 0);

  std::string resolve(const std::string &name);

  void update(const std::string &name, const std::string &ip);

  void dump();

 private:
  explicit DNSCache(size_t maxSize);

  class UsagesList {
   public:
    struct Node {
      std::string key;
      Node *prev;
      Node *next;
    };

    UsagesList(const UsagesList &) = delete;
    UsagesList(UsagesList &&) = delete;
    UsagesList &operator=(const UsagesList &) = delete;
    UsagesList &&operator=(UsagesList &&) = delete;

    UsagesList(size_t maxSize);

    Node *insert(const std::string &key);

    void moveToFront(Node *node);

    Node *getTail() { return tail; }

    void dump();

   private:
    std::vector<Node> nodesStorage;
    Node *head;
    Node *tail;
  };

  struct HashMapValue {
    std::string value;
    UsagesList::Node *nodePtr;
  };

  size_t maxSize;
  UsagesList sortedUsages;
  absl::flat_hash_map<std::string, HashMapValue> map;
  absl::Mutex mutex;
};
}  // namespace lru

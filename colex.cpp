#include "./InfInt.h"
#include "./json.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <valarray>
#include <vector>
template <typename T> void append(std::vector<T> a, std::vector<T> &b) {
  b.insert(b.end(), a.begin(), a.end());
}
namespace nlohmann {
template <> struct adl_serializer<InfInt> {
  static void to_json(json &j, const InfInt &value) {
    j = value.toString();
    // calls the "to_json" method in T's namespace
  }

  static void from_json(const json &j, InfInt &value) {
    // same thing, but with the "from_json" method
    value = j.get<std::string>();
  }
};
} // namespace nlohmann
void sync();
std::mutex cacheLock;
std::map<InfInt, std::vector<InfInt>> cache;
std::vector<std::shared_ptr<std::thread>> threads;
inline std::vector<InfInt> hailstone(InfInt i) {
  if (i == 0)
    return {};
  if (i < 0)
    return {};
  std::vector<InfInt> v;
  v.push_back(i);
  if (InfInt(1) == i)
    return v;
  auto j = i;
  {
    std::scoped_lock l(cacheLock);
    if (cache.contains(j))
      return cache[j];
  };
  i = (i % 2 != 0) ? (InfInt(3) * i + 1) : (i / 2);
  // hailstone(i)
  append(hailstone(i), v);
  std::scoped_lock l(cacheLock);
  cache[j] = v;
  sync();
  return v;
}
std::map<InfInt, bool> isPrimeCache;
bool isPrime(InfInt n) {
  {
    std::scoped_lock l(cacheLock);
    if (isPrimeCache.contains(n))
      return isPrimeCache[n];
  }
  bool answer = true;
  if (n < 2) {
    answer = false;
    return answer;
  }
  if (n >= 2) {
    for (InfInt i = 2; i < n; i++) {
      if (n % i == 0) {
        answer = false;
        return answer;
      }
    }
    std::scoped_lock l(cacheLock);
    isPrimeCache[n] = answer;
    sync();
    return answer;
  }
}
InfInt nthPrime(InfInt n) {
  InfInt i, counter = 0;
  for (i = 2; counter < n; i++) {
    if (isPrime(i)) {
      counter++;
    }
  }
  return i - 1;
}
std::map<InfInt, std::map<InfInt, InfInt>> factorCache;
std::map<InfInt, InfInt> _primeFactors(InfInt n) {
  if (n < 1)
    return {};
  {
    std::scoped_lock l(cacheLock);
    if (factorCache.contains(n))
      return factorCache[n];
  }
  std::map<InfInt, InfInt> x;
  while (n % 2 == 0) {
    x[2]++;
    n = n / 2;
  }
  for (InfInt i = 3; i <= n.intSqrt(); i = i + 2) {
    if (isPrime(i)) {
      x[i] = x[i];
      while (n % i == 0) {
        x[i]++;
        n = n / i;
      }
    }
  }
  if (n > 2)
    x[n]++;
  std::scoped_lock l(cacheLock);
  factorCache[n] = x;
  return x;
}
template <typename M, typename V> void MapToVec(const M &m, V &v) {
  for (typename M::const_iterator it = m.begin(); it != m.end(); ++it) {
    v.push_back(*it);
  }
}
InfInt stitch(std::vector<InfInt> factors) {
  InfInt i = 1;
  for (int j = 0; j < factors.size(); j++) {
    for (InfInt k = 0; k < factors[j]; k++)
      i *= InfInt(nthPrime(j));
  }
  return i;
}
std::vector<InfInt> primeFactors(InfInt n) {
  auto p = _primeFactors(n);
  std::vector<std::pair<InfInt, InfInt>> pp;
  MapToVec(p, pp);
  std::sort(
      pp.begin(), pp.end(),
      [&](std::pair<InfInt, InfInt> a, std::pair<InfInt, InfInt> b) -> bool {
        return a.first < b.first;
      });
  std::vector<InfInt> ppp;
  for (auto pppp : pp)
    ppp.push_back(pppp.second);
  return ppp;
}
std::vector<InfInt> pack1(std::vector<InfInt> src) {
  std::vector<InfInt> res;
  InfInt i = 1;
  for (auto s : src) {
    InfInt n = 10000;
    n *= s % 100;
    if (i * s > n) {
      res.push_back(i);
      i = n / s;
      if (i < 1)
        i = 1;
    }
    i *= s;
  }
  res.push_back(i);
  return res;
}
void sync() {
  nlohmann::json j = cache;
  std::ofstream a("cache.json");
  a << j;
  j = isPrimeCache;
  std::ofstream b("pcache.json");
  b << j;
  j = factorCache;
  std::ofstream c("fcache.json");
  c << j;
}
void primeCache() {
  std::ifstream a("cache.json");
  nlohmann::json ja;
  a >> ja;
  cache = ja;
  std::ifstream b("pcache.json");
  b >> ja;
  isPrimeCache = ja;
  std::ifstream c("fcache.json");
  c >> ja;
  factorCache = ja;
  for (InfInt i = 0; i < InfInt(1024); i++) {
    auto fn = [=]() {
      hailstone(i);
      nthPrime(i);
      primeFactors(i);
    };
    if (i > InfInt(256)) {
      threads.push_back(std::make_shared<std::thread>(fn));
    } else {
      fn();
    }
  }
}
template <typename T> T mat(std::vector<T> vec, InfInt i) {
  return vec[(i % vec.size()).toInt()];
}
bool is_vowel(char c) {
  std::string v = "aeiou";
  return v.find(std::tolower(c)) != std::string::npos;
}

bool is_consonant(char c) { return std::isalpha(c) && !is_vowel(c); }
std::vector<std::string> l2() {
  std::vector<std::string> ml{". "};
  std::string l = "abcdefgijlmnopstuvwz";
  // std::string l = "abcdefghijklmnopqrstuvwxyz";
  std::string v, c;
  for (char ch : l) {
    if (is_vowel(ch))
      v += ch;
    if (is_consonant(ch))
      c += ch;
  }
  for (char vv : v)
    for (char cc : c)
      ml.push_back(std::string{cc} + vv);
  return ml;
}
std::string lan(InfInt i) {
  auto ml = l2();
  std::string res;
  while (i != 0) {
    res = mat(ml, i) + res;
    i = i / ml.size();
  }
  return res;
}
std::vector<std::string> Split(const std::string &str, int splitLength) {
  int NumSubstrings = str.length() / splitLength;
  std::vector<std::string> ret;

  for (auto i = 0; i < NumSubstrings; i++) {
    ret.push_back(str.substr(i * splitLength, splitLength));
  }

  // If there are leftover characters, create a shorter item at the end.
  if (str.length() % splitLength != 0) {
    ret.push_back(str.substr(splitLength * NumSubstrings));
  }

  return ret;
}
template <typename T> int getIndex(std::vector<T> v, T K) {
  auto it = std::find(v.begin(), v.end(), K);

  // If element was found
  if (it != v.end()) {

    // calculating the index
    // of K
    int index = it - v.begin();
    return index;
  }
  return -1;
}
InfInt unlan(std::string x) {
  InfInt res = 0;
  auto ml = l2();
  auto s = Split(x, 2);
  for (auto ss : s) {
    res *= ml.size();
    res += getIndex(ml, ss);
  };
  return res;
}
void xprint(std::vector<InfInt> x) {
  std::cout << "-----------------" << std::endl;
  for (auto y : x)
    std::cout << lan(y) << " ";
  std::cout << std::endl;
}
int dbgMenu(InfInt i) {
  std::string s;
  std::cout << "What? ";
  std::cin >> s;
  std::cout << std::endl;
  if (s == "hailstone") {
    auto v = hailstone(i);
    for (auto u : v)
      std::cout << u << " (" << lan(u) << ")" << std::endl;
    xprint(v);
  }
  if (s == "packed-hailstone") {
    auto v = pack1(hailstone(i));
    for (auto u : v)
      std::cout << u << " (" << lan(u) << ")" << std::endl;
    xprint(v);
  }
  if (s == "nthprime") {
    auto n = nthPrime(i);
    std::cout << n << " (" << lan(n) << ")" << std::endl;
  }
  if (s == "encode") {
    std::cout << lan(i) << std::endl;
  }
  if (s == "iter-encode") {
    for (InfInt j = 0; j < i; j++) {
      std::cout << lan(j) << " <- " << j << std::endl;
    }
  }
  if (s == "sanity") {
    std::cout << (i == unlan(lan(i))) << ";" << lan(i) << ";" << unlan(lan(i))
              << std::endl;
  }
  if (s != "exit") {
    return dbgMenu(i);
  }
  return 0;
}
inline int m2() {
  InfInt i;
  std::string type;
  std::cout << "What input do you want? ";
  std::cin >> type;
  if (type == "exit")
    return 0;
  if (type == "number")
    std::cin >> i;
  if (type != "number") {
    std::string x;
    std::cin >> x;
    i = unlan(x);
  }
  int j = dbgMenu(i);
  if (j != 0)
    return j;
  return m2();
}
int main() {
  primeCache();
  int m = m2();
  for (auto t : threads)
    if (t->joinable())
      t->join();
  return m;
}
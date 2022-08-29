/// ============================================================================
/// Copyright (C) 2022 Pavol Federl (pfederl@ucalgary.ca)
/// All Rights Reserved. Do not distribute this file.
/// ============================================================================
///
/// You must modify this file and then submit it for grading to D2L.
///
/// You can delete all contents of this file and start from scratch if
/// you wish, as long as you implement the detect_primes() function as
/// defined in "detectPrimes.h".

#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include<thread>
// C++ barrier class (from lecture notes).
// -----------------------------------------------------------------------------
// You do not have to use it in your implementation. If you don't use it, you
// may delete it.
class simple_barrier {
  std::mutex m_;
  std::condition_variable cv_;
  int n_remaining_, count_;
  bool coin_;

  public:
  simple_barrier(int count = 1) { init(count); }
  void init(int count)
  {
    count_ = count;
    n_remaining_ = count_;
    coin_ = false;
  }
  bool wait()
  {
    if (count_ == 1) return true;
    std::unique_lock<std::mutex> lk(m_);
    if (n_remaining_ == 1) {
      coin_ = ! coin_;
      n_remaining_ = count_;
      cv_.notify_all();
      return true;
    }
    auto old_coin = coin_;
    n_remaining_--;
    cv_.wait(lk, [&]() { return old_coin != coin_; });
    return false;
  }
};


// returns true if n is prime, otherwise returns false
// -----------------------------------------------------------------------------
// to get full credit for this assignment, you will need to adjust or even
// re-write the code in this function to make it multithreaded.
static bool is_prime(int64_t n)
{
  // handle trivial cases
  
  if (n < 2) return false;
  if (n <= 3) return true; // 2 and 3 are primes
  if (n % 2 == 0) return false; // handle multiples of 2
  if (n % 3 == 0) return false; // handle multiples of 3
  // try to divide n by every number 5 .. sqrt(n)
  int64_t i = 5;
  int64_t max = sqrt(n);
  while (i <= max) {
    if (n % i == 0) return false;
    if (n % (i + 2) == 0) return false;
    i += 6;
  }
  // didn't find any divisors, so it must be a prime
  return true;
}

// This function takes a list of numbers in nums[] and returns only numbers that
// are primes.
//
// The parameter n_threads indicates how many threads should be created to speed
// up the computation.
// -----------------------------------------------------------------------------
// You will most likely need to re-implement this function entirely.
// Note that the current implementation ignores n_threads. Your multithreaded
// implementation must use this parameter.

std::vector<int64_t> result;
std::atomic<bool> finished = false;
simple_barrier barrier = simple_barrier();
std::atomic<bool> isprime = false;
std::atomic<int> j = 0;
std::atomic<int64_t> num =0;
// std::atomic<bool> flag_parallel_section_completed = false;

void thread_Function(int id, int n_threads,std::vector<int64_t> nums){
  
  while(1){ 
    if(barrier.wait()!=0){
      if(isprime) result.push_back(num);
      while(1){
        isprime = true;
        if(j>=(int64_t)nums.size()){
          finished= true;
          break;
        }else{
          num = nums.at(j++);
          if(num<1024 || num % 2 == 0 || num % 3 == 0){
            if(is_prime(num)) result.push_back(num);  
            continue;
          }else break;
        } 
      }
    }
    barrier.wait();
    int64_t i = 5 + 6*id;
    int64_t max = sqrt((double)num);

    if(finished) break;

    while (i <= max ) {
      if(!isprime) break;  
      if (num % i == 0) isprime = false;
      if (num % (i + 2) == 0) isprime = false;
      i += 6*n_threads; 
    }
  }

  return;   
}



std::vector<int64_t>
detect_primes(const std::vector<int64_t> & nums, int n_threads)
{

  std::thread thread_pool[n_threads];
  
  barrier.init(n_threads);
  for(int i = 0; i < n_threads; i++){
    thread_pool[i] = std::thread(thread_Function, i, n_threads, nums);
  }

  for (int i = 0; i < n_threads; i++) {
    if (thread_pool[i].joinable()) thread_pool[i].join();
  }

  return result;
}

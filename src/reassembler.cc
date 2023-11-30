#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  /*
    1. send instant data
      if next_index in data index range (first_index <= next_index and first_index + data.size() > next_index))
      maintain next_index
      maybe need to remove some pending data (covered by data index range)
      if next_index is already in pending data, send it
        at most, after move pending data, can send one pending data, bcs next missing idx can't be covered by current data
    2. store data
      if first_index > next_index, store it, otherwise no need to store it, all data can be sent
  */
  // Your code here.
  (void)first_index;
  (void)data;
  (void)is_last_substring;
  (void)output;
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return {};
}

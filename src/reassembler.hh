#pragma once

#include "byte_stream.hh"

#include <map>
#include <string>

class Reassembler
{
  using index = uint64_t;
  using Ranges = map<index, index>;
  using RangeIt = Ranges::iterator;
  Ranges missing_ranges; // [missing_start, missing_end]
  map<index, string> pending_data; // [index, data]
  index next_index = 0; // next index that should be sent
  index bytes_pending_ = 0;

  bool try_send_data(string & data, index first_index, Writer& output);

  /*
    potential actions
      1. delete Range (range is before next_index)
      2. leave part of range, and insert pending data
      3. modify range(part of range before next_index)
      4. insert last range (data is greater than last existing range)
  */

  void handle_range(RangeIt &it, string & data, index first_index);

  

public:
  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;
};

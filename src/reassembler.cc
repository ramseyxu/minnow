#include "reassembler.hh"

using namespace std;

bool Reassembler::try_send_data(string & data, index first_index, Writer& output)
{
  index last_index = first_index + data.size() - 1;
  if (first_index <= next_index and last_index >= next_index) {
    index start_pos = next_index - first_index;
    output.push(data.substr(start_pos));
    index sent_bytes = data.size() - start_pos;
    next_index += sent_bytes;
    bytes_pending_ -= sent_bytes;
    return true;
  }
  return false;
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  if (try_send_data(data, first_index, output)) {
    // try send pending data
    for (auto it = pending_data.begin(); it != pending_data.end(); ) {
      index r = it->first + it->second.size() - 1;
      if (r < next_index)
        // covered by current data, erase
        it = pending_data.erase(it);
      else {
        // try send one string, the other can't be valid for sending
        if (try_send_data(it->second, it->first, output))
          pending_data.erase(it);
        break;
      }
    }

    // maintain missing ranges
    for (auto it = missing_ranges.begin(); it != missing_ranges.end(); ) {
      if (it->second < next_index)
        // already sent, erase
        it = missing_ranges.erase(it);
      else {
        if (it->first < next_index) {
          // partial sent, modify
          index r = it->second;
          missing_ranges.erase(it);
          missing_ranges[next_index] = r;
        }
        break;
      }
    }
  } else {
    // maintain missing ranges
    auto it = missing_ranges.lower_bound(first_index);
    if (it != missing_ranges.begin()) {
      auto pre_it = prev(it);
      if (pre_it->second >= first_index)
        it = pre_it;
    }

    while ()
  }
  if (is_last_substring)
    output.close();
}

uint64_t Reassembler::bytes_pending() const
{
  return bytes_pending_;
}

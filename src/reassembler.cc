#include "reassembler.hh"
#include "cstdio"
#include <cstdarg>
#include <cassert>
#include <cstdint>

//#define DEBUG__

void debug_print(const char* format, ...) {
#ifdef DEBUG__
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif
  (void)format;
}

using namespace std;

Reassembler::Reassembler() : missing_ranges(Ranges()), pending_data(map<index, string>()) {
  missing_ranges[0] = UINT64_MAX;
}

bool Reassembler::try_send_data(string & data, index first_index, Writer& output)
{
  index last_index = first_index + data.size() - 1;
  if (first_index <= next_index and last_index >= next_index) {
    index start_pos = next_index - first_index;
    output.push(data.substr(start_pos, data.size() - start_pos));
    index sent_bytes = data.size() - start_pos;
    next_index += sent_bytes;
    assert(next_index == output.bytes_pushed());
    if (last_byte != 0 and next_index == last_byte)
      output.close();
    return true;
  }
  return false;
}

void Reassembler::add_pending_data(string data, index l)
{
  bytes_pending_ += data.size();
  debug_print("bytes_pending_ + %llu =  %llu\n", data.size(), bytes_pending_);
  pending_data[l] = std::move(data);
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  auto buffer_cap = output.available_capacity();
  auto max_data_size = buffer_cap + output.bytes_pushed() - first_index;
  if (max_data_size < data.size())
    data.resize(max_data_size);

  if (is_last_substring)
    last_byte = first_index + data.size();

  // full datagram has been sent
  if (first_index + data.size() <= next_index) {
    if (is_last_substring)
      output.close();
    return;
  }

  if (data.size() == 0)
    return;

  if (try_send_data(data, first_index, output)) {
    // sent some data, check expired pending_data
    for (auto it = pending_data.begin(); it != pending_data.end(); ) {
      index r = it->first + it->second.size() - 1;
      if (r < next_index) {
        // covered by current data, erase
        bytes_pending_ -= it->second.size();
        debug_print("bytes_pending_ - %llu = %llu\n", it->second.size(), bytes_pending_);
        it = pending_data.erase(it);
      }
      else {
        if (try_send_data(it->second, it->first, output)) {
          bytes_pending_ -= it->second.size();
          debug_print("bytes_pending_ - %llu = %llu\n", it->second.size(), bytes_pending_);
          it = pending_data.erase(it);
        } else
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
    //in this branch, first_index will be greater than next_index
    auto it = missing_ranges.begin();

    // check every missing_range, if can be filled by data
    while (it != missing_ranges.end()) {
      auto l = it->first;
      auto r = it->second;
      if (r < first_index) {
        it = next(it);
        continue;
      }

      auto len = data.size();
      auto last_index = first_index + data.size() - 1;
      // missing range is fully greater than data range
      if (l > last_index)
        break;
      // missing range is fully covered by data
      if (l >= first_index and r <= last_index) {
        len = r - l + 1;
        it = missing_ranges.erase(it);
        pending_data[l] = data.substr(l - first_index, len);
        bytes_pending_ += len;
        debug_print("bytes_pending_ + %llu =  %llu\n", len, bytes_pending_);
        continue;
      }
      // now, data can only cover part of missing range
      if (first_index > l)
        missing_ranges[l] = first_index - 1;
      if (last_index < r)
        missing_ranges[last_index + 1] = r;
      if (first_index < l) {
        len = data.size() - (l - first_index);
        add_pending_data(data.substr(l - first_index, len), l);
      }
      if (last_index > r) {
        len = data.size() - (r - last_index);
        add_pending_data(data.substr(0, len), first_index);
      }
      if (l <= first_index and last_index <= r)
        add_pending_data(std::move(data), first_index);
      it = missing_ranges.erase(it);
    }
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return bytes_pending_;
}

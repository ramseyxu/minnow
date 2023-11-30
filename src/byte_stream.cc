#include <stdexcept>

#include "byte_stream.hh"
#include "cassert"
#include "cstdio"
#include <cstdarg>

#define DEBUG__

void debug_print(const char* format, ...) {
#ifdef DEBUG__
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif
}

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), buffer(vector<char> (capacity * 2)) {}

bool ByteStream::buffer_empty() const {
    return buffer_size_ == 0;
}

uint64_t ByteStream::remaining_capacity() const {
    return capacity_ - buffer_size_;
}

uint64_t ByteStream::copy_to_buffer(const string &data) {
    if (remaining_capacity() == 0)
        return 0;
    size_t copy_len = min(data.size(), (size_t)remaining_capacity());
    if (tail + copy_len > buffer.size()) {
      copy(buffer.begin() + head, buffer.begin() + tail, buffer.begin());
      tail -= head;
      head = 0;
    }
    copy(data.begin(), data.begin() + copy_len, buffer.begin() + tail);
    tail += copy_len;
    buffer_size_ += copy_len;
    bytes_written_ += copy_len;

    return copy_len;
}

string_view ByteStream::copy_from_buffer(uint64_t len) const {
    if (buffer_empty())
        return "";
    size_t copy_len = min(len, buffer_size_);
    return string_view(buffer.data() + head, copy_len);
}

uint64_t ByteStream::pop_out(uint64_t len) {
    if (buffer_empty())
        return 0;
    size_t pop_len = min(len, buffer_size_);
    head += pop_len;
    buffer_size_ -= pop_len;
    bytes_read_ += pop_len;
    return pop_len;
}

void Writer::push( string data )
{
  copy_to_buffer(data);
}

void Writer::close()
{
  input_ended_ = true;
}

void Writer::set_error()
{
  error_ = true;
}

bool Writer::is_closed() const
{
  return input_ended_;
}

uint64_t Writer::available_capacity() const
{
  return remaining_capacity();
}

uint64_t Writer::bytes_pushed() const
{
  return bytes_written_;
}

string_view Reader::peek() const
{
  return copy_from_buffer(buffer_size_);
}

bool Reader::is_finished() const
{
  return input_ended_ && buffer_empty();
}

bool Reader::has_error() const
{
  return error_;
}

void Reader::pop( uint64_t len )
{
  pop_out(len);
}

uint64_t Reader::bytes_buffered() const
{
  return buffer_size_;
}

uint64_t Reader::bytes_popped() const
{
  return bytes_read_;
}

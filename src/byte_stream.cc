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

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), buffer(deque<string>()) {}

bool ByteStream::buffer_empty() const {
    return buffer_size_ == 0;
}

uint64_t ByteStream::remaining_capacity() const {
    return capacity_ - buffer_size_;
}

uint64_t ByteStream::copy_to_buffer(string data) {
    if (remaining_capacity() == 0 || data.size() == 0)
        return 0;
    if (data.size() > remaining_capacity())
        data = data.substr(0, remaining_capacity());
    bytes_written_ += data.size();
    buffer_size_ += data.size();
    buffer.push_back(move(data)); // TODO: std::move?
    return buffer.back().size();
}

uint64_t ByteStream::pop_out(uint64_t len) {
    if (buffer_empty())
        return 0;
    size_t pop_len = min(len, buffer_size_);
    size_t unpopped_len = pop_len;
    while (unpopped_len > 0) {
        if (buffer.front().size() > unpopped_len) {
            buffer.front() = buffer.front().substr(unpopped_len);
            unpopped_len = 0;
        } else {
            unpopped_len -= buffer.front().size();
            buffer.pop_front();
        }
    }
    bytes_read_ += pop_len;
    buffer_size_ -= pop_len;
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
  return buffer_empty() ? string_view() : buffer.front();
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

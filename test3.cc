#include <queue>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <iostream>

#include "cassert"
#include "cstdio"
#include <cstdarg>

using namespace std;

class Reader;
class Writer;

class ByteStream
{
public:
  uint64_t capacity_;

  deque<string> buffer;
  string_view front_string_view;

  uint64_t buffer_size_ = 0, bytes_written_ = 0, bytes_read_ = 0;

  uint64_t push(string data);

  string_view peek() const; // Peek at the next bytes in the buffer

  bool input_ended_{};  //!< Flag indicating that the stream input has ended.

  bool error_{};  //!< Flag indicating that the stream suffered an error.

  bool buffer_empty() const;

  uint64_t remaining_capacity() const;

  uint64_t pop_out(uint64_t len);

  explicit ByteStream( uint64_t capacity );

  // Helper functions (provided) to access the ByteStream's Reader and Writer interfaces
  Reader& reader();
  const Reader& reader() const;
  Writer& writer();
  const Writer& writer() const;
};

string_view ByteStream::peek() const
{
  cout<<"peek"<<endl;
  cout<<front_string_view<<endl;
  return front_string_view;
}

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), buffer(deque<string>()),
front_string_view("") {}

bool ByteStream::buffer_empty() const {
    return buffer_size_ == 0;
}

uint64_t ByteStream::remaining_capacity() const {
    return capacity_ - buffer_size_;
}

uint64_t ByteStream::push(string data) {
    if (remaining_capacity() == 0 || data.size() == 0)
        return 0;
    if (data.size() > remaining_capacity())
        data = data.substr(0, remaining_capacity());
    bytes_written_ += data.size();
    buffer_size_ += data.size();
    buffer.push_back(std::move(data));
    if (front_string_view.size() == 0) {
        front_string_view = buffer.front();
        cout<<"push"<<endl;
        cout<<front_string_view<<endl;
    }
    return buffer.back().size();
}

uint64_t ByteStream::pop_out(uint64_t len) {
    if (buffer_empty())
        return 0;
    size_t pop_len = min(len, buffer_size_);
    size_t unpopped_len = pop_len;
    while (unpopped_len > 0) {
        if (front_string_view.size() > unpopped_len) {
          front_string_view.remove_prefix(unpopped_len);
          unpopped_len = 0;
          cout<<"pop"<<len<<endl;
          cout<<front_string_view<<endl;
        } else {
          unpopped_len -= front_string_view.size();
          buffer.pop_front();
          front_string_view = buffer.empty() ? "" : buffer.front();
          cout<<"pop "<<len<<endl;
          cout<<front_string_view<<endl;
        }
    }
    bytes_read_ += pop_len;
    buffer_size_ -= pop_len;
    return pop_len;
}

void read( ByteStream & reader, uint64_t len, std::string& out )
{
  out.clear();

//   while ( reader.bytes_buffered() and out.size() < len ) {
//     auto view = reader.peek();

//     if ( view.empty() ) {
//       throw std::runtime_error( "Reader::peek() returned empty string_view" );
//     }

//     view = view.substr( 0, len - out.size() ); // Don't return more bytes than desired.
//     out += view;
//     reader.pop( view.size() );
//   }
    auto view = reader.peek();
    out += view;
    reader.pop_out(view.size());
    view = reader.peek();
    out += view;
}

int main() {
    ByteStream bs(2);
    /*
      test.execute( Push { "cat" } );
      test.execute( BytesPushed { 2 } );
      test.execute( Pop { 1 } );
      test.execute( Push { "tac" } );

      test.execute( IsClosed { false } );
      test.execute( BufferEmpty { false } );
      test.execute( IsFinished { false } );
      test.execute( BytesPopped { 1 } );
      test.execute( BytesPushed { 3 } );
      test.execute( AvailableCapacity { 0 } );
      test.execute( BytesBuffered { 2 } );
      test.execute( Peek { "at" } );
    */
    bs.push("cat");
    bs.pop_out(1);
    bs.push("tac");
    string out;
    read(bs, 2, out);
    cout<<out<<endl;
}
#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  if (message.SYN) {
    isn_ = message.seqno;
    message.seqno = message.seqno + 1; // move to first_index
  }
  uint64_t first_index = message.seqno.unwrap(isn_.value(), inbound_stream.bytes_pushed() + 1);
  reassembler.insert(first_index, message.payload.release(), message.FIN, inbound_stream);
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  TCPReceiverMessage message {};
  if (isn_.has_value())
    message.ackno = Wrap32::wrap(inbound_stream.bytes_pushed() + 1, isn_.value());
  auto buffer_sz = inbound_stream.available_capacity();
  message.window_size = buffer_sz > UINT16_MAX ? UINT16_MAX : buffer_sz;
  return message;
}

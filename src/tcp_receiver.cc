#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  if (message.SYN) {
    isn_ = message.seqno;
  }

  if (!isn_.has_value()) {
    return;
  }

  uint64_t abs_seqno = message.seqno.unwrap(isn_.value(), inbound_stream.bytes_pushed() + 1);
  uint64_t first_index = message.SYN ? 0 : abs_seqno - 1;
  reassembler.insert(first_index, message.payload.release(), message.FIN, inbound_stream);
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  TCPReceiverMessage message {};
  if (isn_.has_value()) {
    auto abs_seqon = inbound_stream.bytes_pushed() + 1 + inbound_stream.is_closed();
    message.ackno = Wrap32::wrap(abs_seqon, isn_.value());
  }
  auto buffer_sz = inbound_stream.available_capacity();
  message.window_size = buffer_sz > UINT16_MAX ? UINT16_MAX : buffer_sz;
  return message;
}

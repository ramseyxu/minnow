#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  if (message.SYN) {
    ISN = message.seqno;
    message.seqno = message.seqno + 1; // move to first_index
  }
  uint64_t first_index = message.seqno.unwrap(ISN, inbound_stream.bytes_pushed() + 1);
  reassembler.insert(first_index, message.payload.release(), message.FIN, inbound_stream);
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  TCPReceiverMessage message;
  if (inbound_stream.bytes_pushed() != 0)
    message.ackno = Wrap32::wrap(inbound_stream.bytes_pushed() + 1, ISN);
  message.window_size = inbound_stream.available_capacity();
  return message;
}

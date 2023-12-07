#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <deque>

class Timer {
  uint64_t current_time_ms_;
  uint64_t expired_time_ms_;
  bool is_running_;

  uint64_t current_RTO_ms_;
  uint64_t initial_RTO_ms_;

public:
  Timer(uint64_t initial_RTO_ms);

  void start();
  void stop();
  bool is_running();

  bool expired();

  void reset_RTO();
  void doublt_RTO();

  void tick(uint64_t ms_since_last_tick);
};

class TCPSender
{
  Wrap32 isn_;

  Timer timer_;

  uint64_t window_size_;

  deque<TCPSenderMessage> pre_sending_queue_;
  deque<TCPSenderMessage> outstanding_messages_;

  uint64_t sequence_numbers_in_flight_;

  uint64_t consecutive_retransmissions_;

  uint64_t next_seq_no_;

  bool need_retransmission_;

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};

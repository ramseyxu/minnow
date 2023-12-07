#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>
#include <string>
#include <cassert>

using namespace std;

Timer::Timer(uint64_t initial_RTO_ms)
  : current_time_ms_(0),
  expired_time_ms_(0),
  is_running_(false),
  current_RTO_ms_(initial_RTO_ms),
  initial_RTO_ms_(initial_RTO_ms)
{
}

Timer::start()
{
  assert(!is_running_)
  is_running_ = true;
  expired_time_ms_ = current_time_ms_ + current_RTO_ms_;
}

Timer::stop()
{
  assert(is_running_)
  is_running_ = false;
}

Timer::is_running()
{
  return is_running_;
}

Timer::expired()
{
  return is_running_ && current_time_ms_ >= expired_time_ms_;
}

Timer::tick(uint64_t ms_since_last_tick)
{
  current_time_ms_ += ms_since_last_tick;
}

Timer::reset_RTO()
{
  current_RTO_ms_ = initial_RTO_ms_;
}

Timer::doublt_RTO()
{
  current_RTO_ms_ *= 2;
}

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ),
  timer_(Timer(initial_RTO_ms)),
  window_size_(1),
  pre_sending_queue_(queue<TCPSenderMessage>()),
  outstanding_messages_(queue<TCPSenderMessage>()),
  sequence_numbers_in_flight_(0),
  consecutive_retransmissions_(0),
  next_seq_no_(0)
{
}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return sequence_numbers_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return consecutive_retransmissions_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  /*
    1. check if need retransmission, if so, send the first message in outstanding messages
    2. if pre_sending_queue is empty, return empty optional
    3. send the first message in pre_sending_queue
  */
  if (need_retransmission_) {
    assert(!outstanding_messages_.empty());
    need_retransmission_ = false;
    if (!time_.is_running()) {
      time_.start();
    }
    return outstanding_messages_.front();
  }

  if (pre_sending_queue_.empty()) {
    return {};
  }

  if (!time_.is_running()) {
    time_.start();
  }
  TCPSenderMessage message = pre_sending_queue_.front();
  pre_sending_queue_.pop_front();
  outstanding_messages_.push_back(message);
  sequence_numbers_in_flight_ += message.length_in_sequence_space();
  return message;
}

void TCPSender::push( Reader& outbound_stream )
{
  /*
    1. check how window size, how many free buffer I can push
    2. while there are free buffer and there are data in outbound stream
      2.1 make a TCPSenderMessage and store in a queue
      2.2 remeber check SYN or FIN
      2.3 pop data from outbound stream
      2.4 update next_seq_no_
  */

  if (outbound_stream.bytes_buffered() == 0) {
    return;
  }

  uint64_t free_buffer_size = window_size_ > 0 ?
    window_size_ - sequence_numbers_in_flight_ :
    1 - sequence_numbers_in_flight_;

  while (free_buffer_size > 0 && !outbound_stream.is_finished()) {
    bool is_syn = next_seq_no_ == 0;
    uint64_t payload_size = min(free_buffer_size - is_syn, TCPConfig::MAX_PAYLOAD_SIZE);
    auto avaliable_data = outbound_stream.peek();
    payload_size = min(payload_size, avaliable_data.size());
    string payload(avaliable_data.begin(), avaliable_data.begin() + payload_size);
    outbound_stream.pop(payload_size);

    // maybe adding is_fin will overflow the window size, we maybe need to handle it
    bool is_fin = outbound_stream.is_finished();

    TCPSenderMessage message(
      Wrap32::wrap(next_seq_no_, isn_),
      is_syn,
      Buffer(payload),
      is_fin
    );
    auto msg_size = message.length_in_sequence_space();
    pre_sending_queue_.push_back(message);
    free_buffer_size -= msg_size;
    next_seq_no_ += msg_size;
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // send an empty message with right seqno
  return TCPSenderMessage(
    Wrap32::wrap(next_seq_no_, isn_),
    false,
    Buffer(),
    false
  );
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  /*
    1. pop all acked messages, if no outstanding messages, stop timer
    2. reset consecutive_retrans if at least one message is acked
    3. if there are still outstanding messages, restart timer
  */
  window_size_ = msg.window_size;
  if (!msg.ackno.has_value()) {
    return;
  }

  bool is_acked = false;

  auto ackno = msg.ackno.value().unwrap(isn_, next_seq_no_);

  while (!outstanding_messages_.empty()) {
    auto message = outstanding_messages_.front();
    auto seqno = message.seqno.unwrap(isn_, next_seq_no_);
    if (seqno + message.length_in_sequence_space() <= ackno) {
      outstanding_messages_.pop_front();
      sequence_numbers_in_flight_ -= message.length_in_sequence_space();
      if (outstanding_messages_.empty()) {
        timer_.stop();
      }
      is_acked = true;
    } else {
      break;
    }
  }

  if (is_acked) {
    timer_.reset_RTO();
    consecutive_retransmissions_ = 0;
  }

  if (!outstanding_messages_.empty()) {
    timer_.stop();
    timer_.start();
  }
}

void TCPSender::tick(size_t ms_since_last_tick )
{
  /*
    1. update the current time
    2. if retransmission timer expired
      2.1 send first message in outstanding messages
      2.2 if window size is zero, double RTO, ++consecutive_retransmissions
      2.3 restart timer
  */
  timer_.tick(ms_since_last_tick);
  if (timer_.expired()) {
    if (!outstanding_messages_.empty()) {
      need_retransmission_ = true; // looks like after tick, maybe_send will be called
      if (window_size_ == 0) {
        current_RTO_ms_ *= 2;
        consecutive_retransmissions_++;
      }
      timer_.stop();
      timer_.start();
    }
  }
}

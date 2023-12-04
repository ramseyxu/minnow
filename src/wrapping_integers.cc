#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { uint32_t(n - 1 + zero_point.raw_value_) };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t abs_seq1 = uint64_t(raw_value_ - zero_point.raw_value_ + 1) + checkpoint % (1ul << 32);
  uint64_t abs_seq2 = abs_seq1 + (1ul << 32);
  if (abs_seq1 >= checkpoint) {
    abs_seq2 = abs_seq1;
    abs_seq1 -= (1ul << 32);
  }
  if (abs_seq2 - checkpoint < checkpoint - abs_seq1)
    return abs_seq2;
  return abs_seq1;
}

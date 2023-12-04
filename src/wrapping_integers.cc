#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { uint32_t(n + zero_point.raw_value_) };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t abs_seq1 = uint32_t(raw_value_ - zero_point.raw_value_) + (checkpoint / (1LL << 32)) * (1LL << 32);
  auto abs = [](uint64_t x, uint64_t y) { return x > y ? x - y : y - x; };
  uint64_t abs_seq2 = abs_seq1 < checkpoint ? abs_seq1 + (1LL << 32) : abs_seq1 - (1LL << 32);
  return abs(abs_seq1, checkpoint) < abs(abs_seq2, checkpoint) ? abs_seq1 : abs_seq2;
}

export module utils;

export template <typename T> float select1m1(T value, const T one, const T minusOne)
{
  if (value == one) return 1.0F;
  if (value == minusOne) return -1.0F;
  return 0.0F;
}

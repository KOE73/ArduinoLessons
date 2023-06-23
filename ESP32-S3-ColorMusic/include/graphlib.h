#pragma once

template <class X, class M, class N, class O, class Q>
X map_Generic(X x, M in_min, N in_max, O out_min, Q out_max)
{
  O deltaIn = in_max - in_min;
  if (deltaIn == 0)
    return x + out_min;

  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
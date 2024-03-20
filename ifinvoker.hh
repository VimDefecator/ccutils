#pragma once

#include <functional>

template<class Fn>
class IfInvoker
{
public:
  IfInvoker(Fn&& fn)
    : m_fn(std::forward<Fn>(fn))
  {
  }

  template<class... Args>
  IfInvoker &operator()(bool cond, Args&&... args)
  {
    if(cond)
      std::invoke(m_fn, std::forward<Args>(args)...);

    return *this;
  }

private:
  Fn m_fn;
};

template<class Fn>
IfInvoker<Fn> invokeIf(Fn&& fn)
{
  return IfInvoker<Fn>(std::forward<Fn>(fn));
}

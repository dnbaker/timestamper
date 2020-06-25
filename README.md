# Timestamper: track runtimes

Timestamper maintains a list of time points with labels, then emits them in sorted order at conclusion listing the amount of time spent on each task.

#  Demo
```c+++
#include <memory>
#include <string>

int main() {
    timestamp::TimeStamper ts("time things");
    ts.add_event("Allocate vector of strings");
    static constexpr unsigned long long nelem = 1000000;
    auto strs = std::make_unique<std::string[]>(nelem);
    auto strb = strs.get(), stre = strs.get() + nelem;
    ts.add_event("Set strings to be integer encoding");
    for(unsigned i = 0; i < 1000000; ++i) {
        strs[i] = std::to_string(i);
    }
    ts.add_event("Sort by lexicographic order");
    std::sort(strb, stre);
    ts.add_event("Sort by reverse lexicographic order");
    std::sort(strb, stre, [](const auto &x, const auto &y) {return x > y;});
    ts.add_event("Destroy strings");
    {
        auto newstrs = std::move(strs);
    }
    ts.add_event("Stack unwinding");
}

```

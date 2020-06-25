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

Output might be something like:

```
##Total: 653.164ms
#EventID	EventName	Rank	Total	Fraction
3	Sort by lexicographic order	1	434.267ms	%66.4866710351
4	Sort by reverse lexicographic order	2	113.212ms	%17.3328597412
2	Set strings to be integer encoding	3	88.383ms	%13.5315173525
1	Allocate vector of strings	4	12.461ms	%1.90779038649
5	Destroy strings	5	4.835ms	%0.740242879277
0	time things	6	0.006ms	%0.00091860543447
6	Stack unwinding	7	0ms	%0
```
# Motivation

On OSX, `-pg`-based profiling does not work, and standard profilers typically have function-level runtime analysis rather than task-level. This tools solves both problems.

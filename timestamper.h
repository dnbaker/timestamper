#ifndef TIMESTAMP_H__
#define TIMESTAMP_H__
#include <cstdint> // std::uint32_t
#include <cstdio>  // std::fprintf
#include <cstdlib> // std::size_t

#include <algorithm> // std::sort
#include <chrono>    // std::chrono
#include <numeric>
#include <string>
#include <vector>

namespace timestamp {
using std::uint32_t;
using std::size_t;
using hrc = std::chrono::high_resolution_clock;

template<typename Clock>
static inline double timediff2ms(std::chrono::time_point<Clock> start, std::chrono::time_point<Clock> stop) {
    if(stop < start) std::swap(stop, start);
    return std::chrono::duration<double, std::milli>(stop - start).count();
}

struct TimeStamper {
    using value_type = std::chrono::time_point<std::chrono::high_resolution_clock>;
    using v_t = value_type;
    struct Event {
        std::string label;
        v_t time;
        Event(std::string l, v_t t): label(l), time(t) {}
    };
    static auto now() {return hrc::now();}
    std::vector<Event> events;
    bool emit_on_close_ = false, emit_as_tsv_;
    TimeStamper(std::string msg, bool emit_as_tsv=true, bool emit_on_close=true):
        events({Event{msg, now()}}), emit_on_close_(emit_on_close), emit_as_tsv_(emit_as_tsv)
    {}
    TimeStamper() {}
    void restart(std::string label) {
        events = {Event{label, now()}};
    }
    void add_event(std::string label) {
        events.emplace_back(label, now());
    }
    ~TimeStamper() {
        if(emit_on_close_) {
            emit();
        }
    }
    std::vector<std::pair<std::string, double>> to_intervals() const {
        auto t = now();
        std::vector<std::pair<std::string, double>> ret(events.size());
        for(size_t i = 0; i < events.size(); ++i) {
            auto nt = i == events.size() - 1 ? t: events[i + 1].time;
            ret[i] = {events[i].label, timediff2ms(events[i].time, nt)};
        }
        return ret;
    }
    void emit() const {
        auto ivls = to_intervals();
        std::vector<double> fractions(ivls.size());
        auto total_time = std::accumulate(ivls.begin(), ivls.end(), 0., [](auto x, const auto &y) {return x + y.second;});
        auto prod = 100. / total_time;
        for(size_t i = 0; i < ivls.size(); ++i) fractions[i] = prod * ivls[i].second;
        std::vector<unsigned> idx(ivls.size());
        std::iota(idx.data(), idx.data() + ivls.size(), 0);
        std::sort(idx.begin(), idx.end(), [&](auto x, auto y) {return ivls[x].second > ivls[y].second;});
        if(emit_as_tsv_) {
            std::fprintf(stderr, "##Total: %gms\n#EventID\tEventName\tRank\tTotal\tFraction\n", total_time);
            for(unsigned i = 0; i < ivls.size(); ++i) {
                auto eid = idx[i];
                auto t = ivls[eid].second;
                std::fprintf(stderr, "%d\t%s\t%d\t%0.12gms\t%%%0.12g\n", eid, events[eid].label.data(), i + 1, t, t * prod);
            }
        } else {
            for(const auto &ivl: ivls) {
                std::fprintf(stderr, "Event '%s' took %gms, %%%g of total %gms\n", ivl.first.data(), ivl.second, ivl.second * prod, total_time);
            }
            for(size_t i = 0; i < ivls.size(); ++i) {
                std::fprintf(stderr, "%d/%s is %zu{st/th/nd} most expensive %u with %%%g of total time\n",
                             idx[i], events[idx[i]].label.data(), i + 1, idx[i], ivls[idx[i]].second * prod);
            }
        }
    }
};

} // timestamp

#ifdef TIMESTAMP_MAIN
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

#endif

#endif /*TIMESTAMP_H__ */

#ifndef TIMESTAMPER_H__
#define TIMESTAMPER_H__
#include <chrono>
#include <cstdint>
#include <vector>
#include <iostream>
#include <numeric>
#include <algorithm>

namespace timestamp {
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
    static auto now() {return std::chrono::high_resolution_clock::now();}
    std::vector<Event> events;
    bool emit_on_close_ = false;
    TimeStamper(std::string msg, bool emit_on_close=true): events({Event{msg, now()}}), emit_on_close_(emit_on_close) {}
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
    static constexpr const char *int2suf(std::ptrdiff_t i) {
        switch(i) {
        case 1: return "st";
        case 2: return "nd";
        case 3: return "rd";
        case 0: case 4: case 5: case 6: case 7: case 8: case 9:
        case 10: case 11: case 12: case 13: case 14: case 15:
        return "th";
        default: {
            switch(i % 10) {
                case 0: case 4: case 5: case 6: case 7: case 8: case 9: return "th";
                case 1: return "st"; case 2: return "nd"; case 3: return "rd";
                default: __builtin_unreachable();
            }
        }
        }
    }
    void emit() const {
        auto ivls = to_intervals();
        auto total_time = std::accumulate(ivls.begin(), ivls.end(), 0., [](auto x, const auto &y) {return x + y.second;});
        auto prod = 100. / total_time;
        std::fprintf(stderr, "#Event\tTime (ms)\t%% of total (%gms)\n", total_time);
        for(const auto &ivl: ivls) {
            std::fprintf(stderr, "%s\t%g\t%%%0.10g\n", ivl.first.data(), ivl.second, ivl.second * prod);
        }
        std::vector<unsigned> idx(ivls.size());
        std::iota(idx.data(), idx.data() + ivls.size(), 0);
        std::sort(idx.begin(), idx.end(), [&](auto x, auto y) {return ivls[x].second > ivls[y].second;});
        
        std::fprintf(stderr, "##Event\tTime (ms)\t%% of total (%gms)\n", total_time);
        for(size_t i = 0; i < ivls.size(); ++i) {
            std::fprintf(stderr, "%d/%s is %zu%s most expensive %u with %%%0.12g of total time\n",
                         idx[i], events[idx[i]].label.data(), i + 1, int2suf(i + 1), idx[i], ivls[idx[i]].second * prod);
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

#endif /*TIMESTAMPER_H__ */

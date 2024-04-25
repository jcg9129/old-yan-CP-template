/*
本文件比较一些数据结构维护静态区间最值，面对特殊区间查询的效率比较

在 atcoder C++ 23 (gcc 12.2) 语言条件下测试
cmd:        g++-12 -std=gnu++2b -O2 -DONLINE_JUDGE -DATCODER -Wall -Wextra -mtune=native -march=native -fconstexpr-depth=2147483647 -fconstexpr-loop-limit=2147483647 -fconstexpr-ops-limit=2147483647 -I/opt/ac-library -I/opt/boost/gcc/include -L/opt/boost/gcc/lib-lgmpxx -lgmp -I/usr/include/eigen3
CPU:        Intel(R) Xeon(R) Platinum 8375C CPU @ 2.90GHz

在 atcoder C++ 23 (clang 16.0.6) 语言条件下测试
cmd:        clang++ -std=c++2b -Wall -Wextra -O2 -DONLINE_JUDGE -DATCODER -mtune=native -march=native -fconstexpr-depth=2147483647 -fconstexpr-steps=2147483647 -I/opt/boost/clang/include -L/opt/boost/clang/lib -I/opt/ac-library -I/usr/include/eigen3 -fuse-ld=lld
CPU:        Intel(R) Xeon(R) Platinum 8375C CPU @ 2.90GHz

在 luogu C++ 20 (gcc 13.2.0) 语言条件下测试
cmd:        g++ -x c++ -std=C++2a -fPIC -DONLINE_JUDGE -Wall -fno-asm -lm -march=native
CPU:        Intel(R) Xeon(R) Platinum 8369HC CPU @ 3.30GHz
*/
#include <cassert>
#include <chrono>
#include <iostream>
#include <random>

#include "DS/CatTree.h"
#include "DS/MaskRMQ.h"
#include "DS/STTable.h"
#include "DS/SegTree.h"
#include "DS/SqrtTree.h"
#include "DS/ZkwTree.h"

using std::cout;

namespace Bench {
    struct Result {
        std::string table_name;
        size_t value;
        size_t repeat;
        long long build_time;
        long long query_time;
        long long time_for_1e10_query() const {
            return query_time * 1000000000 / repeat;
        }
    };
    void handle(Result res) {
        static Result ans{res};
        if (res.value * ans.repeat != ans.value * res.repeat) {
            cout << "wrong ans!\n";
            cout << res.table_name << '\n';
            cout << "ans.value = " << ans.value << '\n';
            cout << "ans.repeat = " << ans.repeat << '\n';
            cout << "res.value = " << res.value << '\n';
            cout << "res.repeat = " << res.repeat << '\n';
        }
        auto show_width = [](std::string x, int w) {
            cout << x << std::string(w - x.size(), ' ');
        };
        auto show_ratio = [](long long t1, long long t2) {
            if (t1) {
                int x = round(100.0 * t2 / t1);
                if (x < 10000) cout << ' ';
                if (x < 1000) cout << ' ';
                if (x < 100) cout << ' ';
                if (x < 10) cout << ' ';
                cout << x;
                cout << " %";
            } else if (!t2)
                cout << "  100 %";
            else
                cout << "    inf";
        };
        show_width(std::to_string(res.build_time), 8);
        show_ratio(ans.build_time, res.build_time);
        cout << "\t\t";
        show_width(std::to_string(res.query_time) + "(" + std::to_string(res.repeat) + ")", 16);
        cout << "\t";
        show_width(std::to_string(res.time_for_1e10_query()), 8);
        show_ratio(ans.time_for_1e10_query(), res.time_for_1e10_query());
        cout << "\t\t" << res.table_name << '\t' << res.value << '\n';
    }
    template <typename Table>
    std::mt19937_64 &get_rng() {
        static std::mt19937_64 s_rng;
        return s_rng;
    }
    namespace RandomRange {
        template <size_t Id, typename Table, size_t Repeat, size_t BuildSize, size_t QuerySize>
        Result bench(std::string table_name) {
            uint32_t buffer1[BuildSize], buffer2[QuerySize][2];
            for (size_t j = 0; j != BuildSize; j++) buffer1[j] = get_rng<Table>()();
            for (size_t j = 0; j != QuerySize; j++) {
                buffer2[j][0] = get_rng<Table>()() % BuildSize, buffer2[j][1] = get_rng<Table>()() % BuildSize;
                if (buffer2[j][0] > buffer2[j][1]) std::swap(buffer2[j][0], buffer2[j][1]);
            }
            auto t0 = std::chrono::high_resolution_clock::now();
            Table S(buffer1, buffer1 + BuildSize);
            auto t1 = std::chrono::high_resolution_clock::now();
            volatile size_t value = 0;
            for (size_t i = 0; i != Repeat; i++) {
                size_t sum{};
                for (size_t j = 0; j != QuerySize; j++) {
                    size_t left = buffer2[j][0], right = buffer2[j][1];
                    if constexpr (Id == 0)
                        sum += S.query(left, right);
                    else if constexpr (Id == 1)
                        sum += S.maximum(left, right);
                }
                value += sum;
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            return {table_name, value, Repeat * QuerySize, std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count(), std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()};
        }
        template <size_t Id, typename Table, size_t Repeat, size_t BuildSize, size_t QuerySize>
        void run(std::string table_name) {
            auto res = bench<Id, Table, Repeat, BuildSize, QuerySize>(table_name);
            handle(res);
        }
    }
}

#define RUN(...) __VA_ARGS__(#__VA_ARGS__)
struct Node {
    using value_type = uint32_t;
    static uint32_t op(uint32_t x, uint32_t y) { return x > y ? x : y; }
    static bool comp(uint32_t x, uint32_t y) { return x < y; }
    uint32_t m_val;
    const uint32_t &get() const { return m_val; }
    void set(uint32_t val) { m_val = val; }
};
static constexpr size_t N = 1 << 20, M = N * 20, Q = 1 << 10;
int main() {
    cout << "build time\t\tquery time\t\ttime for 1e10 query\n";
    RUN(Bench::RandomRange::run<0, OY::STMaxTable<uint32_t>, 1 << 15, N, Q>);
    RUN(Bench::RandomRange::run<0, OY::CatMaxTable<uint32_t>, 1 << 15, N, Q>);
    RUN(Bench::RandomRange::run<0, OY::ZKW::Tree<Node>, 1 << 10, N, Q>);
    RUN(Bench::RandomRange::run<0, OY::Seg::Tree<Node, OY::Seg::Ignore, true, uint32_t, N * 2>, 1 << 9, N, Q>);
    RUN(Bench::RandomRange::run<0, OY::MaskRMQMaxValueTable<uint32_t>, 1 << 14, N, Q>);
    using SqrtMaxTable_random = OY::SqrtMaxTable<uint32_t, OY::Sqrt::RandomController<>>;
    RUN(Bench::RandomRange::run<0, SqrtMaxTable_random, 1 << 15, N, Q>);
    using SqrtMaxTable_nonrandom = OY::SqrtMaxTable<uint32_t, OY::Sqrt::NonRandomController<>>;
    RUN(Bench::RandomRange::run<0, SqrtMaxTable_nonrandom, 1 << 15, N, Q>);
}
/*
atcoder g++
build time		query time		time for 1e10 query
50        100 %		70(33554432)    	2086      100 %		Bench::RandomRange::run<0, OY::STMaxTable<uint32_t>, 1 << 15, N, Q>	144113270836199424
60        120 %		59(33554432)    	1758       84 %		Bench::RandomRange::run<0, OY::CatMaxTable<uint32_t>, 1 << 15, N, Q>	144113270836199424
4           8 %		121(1048576)    	115394   5532 %		Bench::RandomRange::run<0, OY::ZKW::Tree<Node>, 1 << 10, N, Q>	4503539713631232
11         22 %		172(524288)     	328063  15727 %		Bench::RandomRange::run<0, OY::Seg::Tree<Node, OY::Seg::Ignore, true, uint32_t, N * 2>, 1 << 9, N, Q>	2251769856815616
15         30 %		175(16777216)   	10430     500 %		Bench::RandomRange::run<0, OY::MaskRMQMaxValueTable<uint32_t>, 1 << 14, N, Q>	72056635418099712
5          10 %		76(33554432)    	2264      109 %		Bench::RandomRange::run<0, SqrtMaxTable_random, 1 << 15, N, Q>	144113270836199424
6          12 %		88(33554432)    	2622      126 %		Bench::RandomRange::run<0, SqrtMaxTable_nonrandom, 1 << 15, N, Q>	144113270836199424

*/
/*
atcoder clang++
build time		query time		time for 1e10 query
38        100 %		62(33554432)    	1847      100 %		Bench::RandomRange::run<0, OY::STMaxTable<uint32_t>, 1 << 15, N, Q>	144113270836199424
50        132 %		67(33554432)    	1996      108 %		Bench::RandomRange::run<0, OY::CatMaxTable<uint32_t>, 1 << 15, N, Q>	144113270836199424
3           8 %		114(1048576)    	108718   5886 %		Bench::RandomRange::run<0, OY::ZKW::Tree<Node>, 1 << 10, N, Q>	4503539713631232
13         34 %		179(524288)     	341415  18485 %		Bench::RandomRange::run<0, OY::Seg::Tree<Node, OY::Seg::Ignore, true, uint32_t, N * 2>, 1 << 9, N, Q>	2251769856815616
15         39 %		179(16777216)   	10669     578 %		Bench::RandomRange::run<0, OY::MaskRMQMaxValueTable<uint32_t>, 1 << 14, N, Q>	72056635418099712
6          16 %		95(33554432)    	2831      153 %		Bench::RandomRange::run<0, SqrtMaxTable_random, 1 << 15, N, Q>	144113270836199424
5          13 %		115(33554432)   	3427      186 %		Bench::RandomRange::run<0, SqrtMaxTable_nonrandom, 1 << 15, N, Q>	144113270836199424

*/
/*
luogu g++
build time		query time		time for 1e10 query
44        100 %		66(33554432)    	1966      100 %		Bench::RandomRange::run<0, OY::STMaxTable<uint32_t>, 1 << 15, N, Q>	144113270836199424
55        125 %		60(33554432)    	1788       91 %		Bench::RandomRange::run<0, OY::CatMaxTable<uint32_t>, 1 << 15, N, Q>	144113270836199424
4           9 %		108(1048576)    	102996   5239 %		Bench::RandomRange::run<0, OY::ZKW::Tree<Node>, 1 << 10, N, Q>	4503539713631232
10         23 %		209(524288)     	398635  20276 %		Bench::RandomRange::run<0, OY::Seg::Tree<Node, OY::Seg::Ignore, true, uint32_t, N * 2>, 1 << 9, N, Q>	2251769856815616
14         32 %		141(16777216)   	8404      427 %		Bench::RandomRange::run<0, OY::MaskRMQMaxValueTable<uint32_t>, 1 << 14, N, Q>	72056635418099712
8          18 %		78(33554432)    	2324      118 %		Bench::RandomRange::run<0, SqrtMaxTable_random, 1 << 15, N, Q>	144113270836199424
8          18 %		92(33554432)    	2741      139 %		Bench::RandomRange::run<0, SqrtMaxTable_nonrandom, 1 << 15, N, Q>	144113270836199424

*/
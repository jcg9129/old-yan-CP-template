#include "DS/BIT.h"
#include "DS/LinkBucket.h"
#include "DS/WTree.h"
#include "DS/KindCounter.h"
#include "IO/FastIO.h"

/*
[P1972 [SDOI2009] HH的项链](https://www.luogu.com.cn/problem/P1972)
*/
/**
 * 将每个元素的上次出现的位置列出来
 * 区间查询种类数，就是查询上次出现在区间左侧的元素数量
 * 可以使用小波树封装成的区间种类查询器
 *
 * 此外，还有个离线处理贡献的经典做法
 */

uint32_t val[1000001], ans[1000000], last[1000001];
void solve_bit() {
    uint32_t n;
    cin >> n;
    for (uint32_t i = 1; i <= n; i++) cin >> val[i];
    struct Query {
        uint32_t id, left;
    };
    uint32_t m;
    cin >> m;
    OY::LBC::LinkBucket<Query> qs(n + 1, m);
    for (uint32_t i = 0; i < m; i++) {
        uint32_t l, r;
        cin >> l >> r;
        qs[r].push_front(Query{i, l});
    }

    OY::WTree::Tree<uint32_t> S(n + 1);
    // OY::BIT::Tree<uint32_t, false, 1 << 20> S(n + 1);
    for (uint32_t r = 1; r <= n; r++) {
        uint32_t x = val[r];
        if (last[x]) S.add(last[x], -1);
        S.add(last[x] = r, 1);
        auto it = qs[r].begin(), end = qs[r].end();
        if (it != end) {
            auto sum_r = S.presum(r);
            do {
                auto [i, l] = *it;
                ans[i] = sum_r - S.presum(l - 1);
            } while (++it != end);
        }
    }
    for (uint32_t i = 0; i < m; i++) cout << ans[i] << endl;
}

void solve_wavelet() {
    uint32_t n;
    cin >> n;
    OY::ArrayKindCounter<1000000> S(n, [](auto...) {
        uint32_t x;
        cin >> x;
        return x;
    });
    uint32_t m;
    cin >> m;
    while (m--) {
        uint32_t l, r;
        cin >> l >> r;
        cout << S.query(l - 1, r - 1) << endl;
    }
}

int main() {
    solve_bit();
    // solve_wavelet();
}
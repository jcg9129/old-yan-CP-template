#include "DS/FHQTreap.h"
#include "IO/FastIO.h"

/*
[P3391 【模板】文艺平衡树](https://www.luogu.com.cn/problem/P3391)
*/
template <typename Node>
struct NodeWrap {
    using key_type = uint32_t;
    key_type m_key;
    bool m_reverse;
    void reverse() { m_reverse = !m_reverse; }
    void set(const key_type &key) { m_key = key; }
    const key_type &get() const { return m_key; }
    void pushdown(Node *lchild, Node *rchild) {
        if (m_reverse) {
            std::swap(((Node *)this)->m_lchild, ((Node *)this)->m_rchild);
            if (!lchild->is_null()) lchild->reverse();
            if (!rchild->is_null()) rchild->reverse();
            m_reverse = false;
        }
    }
};

uint32_t buf[100000];
int main() {
    uint32_t n, m;
    cin >> n >> m;
    for (uint32_t i = 0; i < n; i++) buf[i] = i + 1;
    auto S = OY::FHQ::Multiset<NodeWrap, 100001>::from_sorted(buf, buf + n);
    using node = decltype(S)::node;
    for (auto i = 0; i < m; i++) {
        uint32_t l, r;
        cin >> l >> r;
        S.do_for_subtree(l - 1, r - 1, [&](node *p) { p->reverse(); });
    }
    S.do_for_each([](node *p) { cout << p->get() << ' '; });
}
#ifndef SVKFW_DBOPS_H
#define SVKFW_DBOPS_H

#include <unordered_set>
#include <unordered_map>


namespace Simple {
    //  ============  Utilities  ============  \\

    template <class T>
    using un_set = std::unordered_set<T>;
    template <class T>
    using un_mulset = std::unordered_multiset<T>;
    template <class T1, class T2>
    using un_map = std::unordered_map<T1, T2>;


    //  ============  Inner operations  ============  \\

// Used assuming that t1.size() <= t2.size()
    template <class T>
    void _UNION_MIN_OF_TWO(const un_mulset<T> &t1, const un_mulset<T> &t2, un_mulset<T> &_tmp) {
        un_set<T> _tmp2{t2};

        for (auto& elem : t1) {
            for (int i = t1.count(elem); i < _tmp2.count(elem); ++i)
                _tmp2.erase(elem);
        }
        _tmp.insert(_tmp2.begin(), _tmp2.end());
    }

// Used assuming that t1.size() <= t2.size()
    template <class T>
    void _UNION_MAX_OF_TWO(const un_mulset<T> &t1, const un_mulset<T> &t2, un_mulset<T> &_tmp) {
        un_set<T> _tmp2{t2};

        for (auto& elem : t1) {
            for (int i = _tmp2.count(elem); i < t1.count(elem); ++i)
                _tmp2.insert(elem);
        }
        _tmp.insert(_tmp2.begin(), _tmp2.end());
    }

    template <class T>
    void _LEFT_XOR(const un_set<T> &t1, const un_set<T> &t2, un_set<T> &_tmp) {
        for (auto& elem : t1)
            if (t2.find(elem) == t2.end())
                _tmp.insert(elem);
    }

// Used assuming that t1.size() <= t2.size()
    template <class T>
    void _XOR(const un_set<T> &t1, const un_set<T> &t2, un_set<T> &_tmp) {
        _tmp.insert(t2.begin(), t2.end());
        for (auto& elem : t1)
            (t2.find(elem) == t2.end()) ? _tmp.insert(elem).first : _tmp.erase(elem);
    }

// Used assuming that t1.size() <= t2.size()
    template <class T>
    void _INNER_JOIN(const un_set<T> &t1, const un_set<T> &t2, un_set<T> &_tmp) {
        for (auto& elem : t1)
            if (t2.find(elem) != t2.end())
                _tmp.insert(elem);
    }


    //  ============  Operations  ============  \\

    template <class T>
    un_set<T> UNION(const un_set<T> &t1, const un_set<T> &t2) {
        un_set<T> _tmp{t1};

        _tmp.insert(t2.begin(), t2.end());
        return _tmp;
    }

    template <class T>
    un_mulset<T> UNION(const un_mulset<T> &t1, const un_mulset<T> &t2) {
        un_set<T> _tmp{t1};    // SET. ONLY DISTINCT VALUES

        _tmp.insert(t2.begin(), t2.end());
        return { _tmp.begin(), _tmp.end() };
    }

    template <class T>
    un_mulset<T> UNION_ALL(const un_mulset<T> &t1, const un_mulset<T> &t2) {
        un_mulset<T> _tmp{t1};    // MULTISET. WITH DUPLICATES

        _tmp.insert(t2.begin(), t2.end());
        return { _tmp };
    }

    template <class T>
    un_mulset<T> UNION_MIN_OF_TWO(const un_mulset<T> &t1, const un_mulset<T> &t2) {
        un_mulset<T> _tmp;

        if (t1.size() <= t2.size()) {
            _UNION_MIN_OF_TWO(t1, t2, _tmp);
        }
        else {
            _UNION_MIN_OF_TWO(t2, t1, _tmp);
        }
        return { _tmp };
    }

    template <class T>
    un_mulset<T> UNION_MAX_OF_TWO(const un_mulset<T> &t1, const un_mulset<T> &t2) {
        un_mulset<T> _tmp;
        un_set<T> _tmp2;

        if (t1.size() <= t2.size()) {
            _UNION_MAX_OF_TWO(t1, t2, _tmp);
        }
        else {
            _UNION_MAX_OF_TWO(t2, t1, _tmp);
        }
        return { _tmp };
    }


    template <class T>
    un_set<T> INNER_JOIN(const un_set<T> &t1, const un_set<T> &t2) {
        un_set<T> _tmp;

        if (t1.size() <= t2.size()) {
            _INNER_JOIN(t1, t2, _tmp);
        }
        else {
            _INNER_JOIN(t2, t1, _tmp);
        }

        return _tmp;
    }


    template <class T>
    un_set<T> LEFT_XOR(const un_set<T> &t1, const un_set<T> &t2) {
        un_set<T> _tmp;
        _LEFT_XOR(t1, t2, _tmp);
        return _tmp;
    }

    template <class T>
    un_set<T> RIGHT_XOR(const un_set<T> &t1, const un_set<T> &t2) {
        un_set<T> _tmp;
        _LEFT_XOR(t2, t1, _tmp);
        return _tmp;
    }

    template <class T>
    un_set<T> XOR(const un_set<T> &t1, const un_set<T> &t2) {
        un_set<T> _tmp;
        if (t1.size() <= t2.size()) {
            _XOR(t1, t2, _tmp);
        }
        else {
            _XOR(t2, t1, _tmp);
        }
        return _tmp;
    }
};

#endif
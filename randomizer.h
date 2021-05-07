#pragma once
#include "sortings.h"

namespace random {
inline uint get_rd();

template<typename T, typename = requires_int<T>>
class rand_num;

class rand_bool;

template<typename T, typename = requires_int<T>>
std::vector<T> random_vector(std::size_t sz_min, std::size_t sz_max,
                             T v_min = std::numeric_limits<T>::min() / 2, T v_max = std::numeric_limits<T>::max() / 2);

template<typename T, typename = requires_int<T>>
std::vector<T> ordered_vector(std::size_t sz_min, std::size_t sz_max, order,
                              T v_min = std::numeric_limits<T>::min() / 2, T v_max = std::numeric_limits<T>::max() / 2);

template<typename T, typename = requires_int<T>>
std::vector<T> somehow_ordered_vector(std::size_t sz_min, std::size_t sz_max,
                              T v_min = std::numeric_limits<T>::min() / 2, T v_max = std::numeric_limits<T>::max() / 2);

template<typename T, typename = requires_int<T>>
std::vector<T> ordered_vectors_composition(std::size_t sz_min, std::size_t sz_max,
                                       T v_min = std::numeric_limits<T>::min() / 2, T v_max = std::numeric_limits<T>::max() / 2);

template<typename T, typename = requires_int<T>>
std::vector<T> any_vectors_composition(std::size_t sz_min, std::size_t sz_max,
                                   T v_min = std::numeric_limits<T>::min() / 2, T v_max = std::numeric_limits<T>::max() / 2);

template<typename T, typename = requires_int<T>>
void test_vector(std::function<std::vector<T> (std::size_t, std::size_t, T, T)> vector_generator,
                 std::size_t sz_min, std::size_t sz_max, bool show_elems = false,
                 T v_min = std::numeric_limits<T>::min() / 2, T v_max = std::numeric_limits<T>::max() / 2);
}//namespace random

uint random::get_rd() {
    static std::random_device rd;
    return rd();
}

template<typename T, typename>
class random::rand_num {
    std::mt19937 generator{random::get_rd()};
    std::uniform_int_distribution<T> rand;

public:
    rand_num(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
        : rand(min, max)
    {}

    void reset() {
        rand.reset();
    }

    void set_interval(T min, T max) {
        rand.param(typename std::uniform_int_distribution<T>::param_type(min, max));
    }

    void set_min(T min) {
        set_interval(min, rand.b());
    }

    void set_max(T max) {
        set_interval(rand.a(), max);
    }

    void change_min(T offset) {
        set_min(rand.a() + offset);
    }

    void change_max(T offset) {
        set_max(rand.b() + offset);
    }

    T interval() const {
        return rand.b() - rand.a();
    }

    T operator()() {
        return rand(generator);
    }

    ~rand_num() = default;
};

class random::rand_bool {
    std::mt19937 generator{random::get_rd()};
    std::bernoulli_distribution rand{};

public:
    bool operator() () {
        return rand(generator);
    }
};

template<typename T, typename>
std::vector<T> random::random_vector(std::size_t sz_min, std::size_t sz_max, T v_min, T v_max) {
    std::vector<T> v(random::rand_num<std::size_t>(sz_min, sz_max)());

    random::rand_num<T> val(v_min, v_max);
    for (auto i = v.begin(); i < v.end(); ++i) {
        *i = val();
    }

    return v;
}

template<typename T, typename>
std::vector<T> random::ordered_vector(std::size_t sz_min, std::size_t sz_max, order o, T v_min, T v_max) {
    std::vector<T> v(random::rand_num<std::size_t>(sz_min, sz_max)());
    if (!v.size()) {
        return v;
    }

    T max_change;
    T interval = v_max - v_min;

    max_change = interval / v.size();
    if (!max_change) {
        max_change = 1;
    }

    random::rand_num<T> diff_gen(0, max_change);
    T val = random::rand_num<T>(v_min, v_max)();

    std::function<void (T)> change;
    if (o == order::increasing) {
        change = [v_max, &val](T diff) {
            if (v_max - diff >= val) {
                val += diff;
            }
            else {
                val = v_max;
            }
        };
    }
    else {
        change = [v_min, &val](T diff) {
            if (v_min + diff <= val) {
                val -= diff;
            }
            else {
                val = v_min;
            }
        };
    }

    for (auto i = v.begin(); i != v.end(); ++i) {
        *i = val;
        change(diff_gen());
    }

    return v;
}

template<typename T, typename>
std::vector<T> random::somehow_ordered_vector(std::size_t sz_min, std::size_t sz_max, T v_min, T v_max) {
    order o;
    if (random::rand_bool()()) {
        o = order::increasing;
    }
    else {
        o = order::decreasing;
    }
    return random::ordered_vector(sz_min, sz_max, o, v_min, v_max);
}

template<typename T, typename>
std::vector<T> random::ordered_vectors_composition(std::size_t sz_min, std::size_t sz_max, T v_min, T v_max) {
    std::size_t main_vec_sz = random::rand_num<std::size_t>(sz_min, sz_max)();
    std::vector<T> v;
    v.reserve(main_vec_sz);

    std::size_t subvec_size_max = main_vec_sz / 10;
    if (subvec_size_max < 5) {
        subvec_size_max = 5;
    }
    random::rand_num<std::size_t> subvec_size_gen(5, subvec_size_max);

    std::size_t cur_subvec_size;
    std::vector<T> tmp;

    do {
        cur_subvec_size = subvec_size_gen();
        if (v.size() + cur_subvec_size > main_vec_sz) {
            cur_subvec_size = main_vec_sz - v.size();
        }
        if (!cur_subvec_size) {
            break;
        }

        tmp = random::somehow_ordered_vector<T>(cur_subvec_size, cur_subvec_size, v_min, v_max);
        v.insert(v.end(), tmp.begin(), tmp.end());
    } while (true);

    return v;
}

template<typename T, typename>
std::vector<T> random::any_vectors_composition(std::size_t sz_min, std::size_t sz_max, T v_min, T v_max) {
    std::size_t main_vec_sz = random::rand_num<std::size_t>(sz_min, sz_max)();
    std::vector<T> v;
    v.reserve(main_vec_sz);

    std::size_t subvec_size_max = main_vec_sz / 10;
    if (subvec_size_max < 5) {
        subvec_size_max = 5;
    }
    random::rand_num<std::size_t> subvec_size_gen(5, subvec_size_max);

    std::size_t cur_subvec_size;
    std::vector<T> tmp;
    random::rand_bool bool_gen;

    do {
        cur_subvec_size = subvec_size_gen();
        if (v.size() + cur_subvec_size > main_vec_sz) {
            cur_subvec_size = main_vec_sz - v.size();
        }
        if (!cur_subvec_size) {
            break;
        }

        if (bool_gen()) {
            tmp = random::somehow_ordered_vector<T>(cur_subvec_size, cur_subvec_size, v_min, v_max);
        }
        else {
            tmp = random::random_vector<T>(cur_subvec_size, cur_subvec_size, v_min, v_max);
        }
        v.insert(v.end(), tmp.begin(), tmp.end());
    } while (true);
    return v;
}

template<typename T, typename>
void random::test_vector(std::function<std::vector<T> (std::size_t, std::size_t, T, T)> vector_generator,
                                std::size_t sz_min, std::size_t sz_max, bool show_elems, T v_min, T v_max) {
    std::vector<T> v = vector_generator(sz_min, sz_max, v_min, v_max);
    std::unordered_map<T, std::size_t> check_map = func::elems(v);

    if (show_elems) {
        std::cout << "elements:\n";
        func::show<T>(v);
        std::cout << '\n';
    }

    std::cout << std::boolalpha << "is sorted by default: " << func::is_sorted(v) << '\n';

    std::cout << sort::timsort(v) << " microseconds per element\n";

    std::cout << "same elements: " << func::cmp_vec(check_map, v) << '\n';
    std::cout << "is sorted: " << func::is_sorted(v) << '\n';

    if (show_elems) {
        std::cout << "elements:\n";
        func::show<T>(v);
        std::cout << '\n';
    }
}

#pragma once
#include "common.h"

namespace func {
template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
void swap(T&, T&);

inline std::size_t get_minrun(std::size_t);

template<typename T>
void show(const std::vector<T>&);

template<typename T, typename = requires_int<T>>
bool is_sorted(const std::vector<T>&);

template<typename T, typename = requires_int<T>>
bool is_sorted(qiter<T> begin, qiter<T> end);

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
void reverse(qiter<T>, qiter<T>);

template<typename T,
         template<typename> typename Iter,
         typename = requires_int<T>,
         typename = requires_iter<T, Iter>>
validated_iter<T, Iter> basic_binary_search(Iter<T> begin, Iter<T> end, T key, last_occurence last_elem);

template<typename T,
         template<typename> typename Iter,
         typename = requires_int<T>,
         typename = requires_iter<T, Iter>>
Iter<T> spec_binary_search(Iter<T> begin, Iter<T> end, T key, last_occurence last_elem);

template<typename T>
std::unordered_map<T, std::size_t> elems(const std::vector<T>&);

template<typename T>
bool cmp_vec(std::unordered_map<T, std::size_t>&, const std::vector<T>&);

template<typename T, typename = requires_int<T>>
bool invariants_violated(const std::vector<massive<T>>&);

template<typename T, typename = requires_int<T>>
std::size_t size_of(const std::vector<massive<T>>&, std::size_t);

template<bool Left_To_Right,
         typename T,
         typename = requires_int<T>>
constexpr std::function<bool (T, T)> get_cmp_function();

template<bool Left_To_Right,
         typename T,
         template<typename> typename Iter,
         typename = requires_int<T>,
         typename = requires_iter<T, Iter>>
constexpr void set_merging_functions(iter_getter<T, Iter> &begin, iter_getter<T, Iter> &end, std::function<bool (T, T)> &cmp);

inline void advance_offset(std::size_t&);

template<typename T,
         template<typename> typename Iter,
         typename = requires_int<T>,
         typename = requires_iter<T, Iter>>
void insert_range(Iter<T> &insert_pos, Iter<T> &range_begin, Iter<T> range_end);
}//namespace func

std::size_t func::get_minrun(std::size_t sz) {
    std::size_t has_one = 0;
    while (sz > 63) {
        has_one |= sz & 1u;
        sz >>= 1;
    }
    return sz + has_one;
}

template<typename T, typename>
void func::swap(T &x, T &y) {
    if (&x != &y) {
        y ^= (x ^= y);
        x ^= y;
    }
}

template<typename T>
void func::show(const std::vector<T> &v) {
    for (auto i = v.cbegin(); i != v.cend(); ++i) {
        std::cout << *i << ' ';
    }
    std::cout << '\n';
}

template<typename T, typename>
bool func::is_sorted(const std::vector<T> &v) {
    if (v.empty()) {
        return true;
    }

    for (auto i = v.cbegin() + 1; i < v.cend(); ++i) {
        if (i[0] < i[-1]) {
            return false;
        }
    }
    return true;
}

template<typename T, typename>
bool func::is_sorted(qiter<T> begin, qiter<T> end) {
    if (begin == end) {
        return true;
    }
    ++begin;
    while (begin  != end) {
        if (*begin < *std::prev(begin)) {
            return false;
        }
        ++begin;
    }
    return true;
}

template<typename T, typename>
void func::reverse(qiter<T> begin, qiter<T> end) {
    qiter<T> i = begin;
    qreviter<T> ri(end);
    while (std::next(ri).base() > i) {//next можно юзать, так как подаются массивы размером 2 и более
        func::swap(*i, *ri);
        ++i;
        ++ri;
    }
}

template<typename T,
         template<typename> typename Iter,
         typename,
         typename>
validated_iter<T, Iter> func::basic_binary_search(Iter<T> begin, Iter<T> end, T key, last_occurence last_elem) {
    using vi = validated_iter<T, Iter>;
    if (begin == end) {//check
        return vi{end, false};
    }

    using It = Iter<T>;
    constexpr bool left_to_right = std::is_same_v<It, qiter<T>>;

    if constexpr (left_to_right) {//гарантия валидности итераторов prev(begin) и end после конца поиска
        if (*begin > key) {
            return vi{begin, false};
        }
        if (*std::prev(end) < key) {
            return vi{end, false};
        }
    }
    else {
        if (*begin < key) {
            return vi{begin, false};
        }
        if (*std::prev(end) > key) {
            return vi{end, false};
        }
    }

    std::function<bool (T, T)> move_begin_condition;
    if constexpr (left_to_right) {
        move_begin_condition = [](T left, T right) {
            return left < right;
        };
    }//iterator
    else {
        move_begin_condition = [](T left, T right) {
            return left > right;
        };
    }//reverse iterator

    It mid;
    while (begin < end) {
        mid = begin + std::distance(begin, end) / 2;

        if (*mid == key) {
            if (last_elem == last_occurence::right) {
                begin = std::next(mid);
            }
            else {
                end = mid;
            }
        }
        else if (move_begin_condition(*mid, key)) {
            begin = std::next(mid);
        }
        else {
            end = mid;
        }
    }

    return vi{begin, true};//begin == end
}

template<typename T,
         template<typename> typename Iter,
         typename, typename>
Iter<T> func::spec_binary_search(Iter<T> begin, Iter<T> end, T key, last_occurence last_elem) {
    validated_iter<T, Iter> result = func::basic_binary_search<T, Iter>(begin, end, key, last_elem);
    if (!result.valid) {
        return end;
    }

    Iter<T> resulting_iter;
    if (last_elem == last_occurence::right) {
        resulting_iter = std::prev(result.iter);
    }
    else {
        resulting_iter = result.iter;
    }

    if (*resulting_iter == key) {
        return resulting_iter;
    }
    else {
        return end;
    }
}

template<typename T>
std::unordered_map<T, std::size_t> func::elems(const std::vector<T> &v) {
    std::unordered_map<T, std::size_t> map;
    typename std::unordered_map<T, std::size_t>::iterator map_iter;
    for (auto i = v.cbegin(); i != v.cend(); ++i) {
        map_iter = map.find(*i);
        if (map_iter == map.end()) {
            map[*i] = 1;
        }
        else {
            ++map_iter->second;
        }
    }
    return map;
}

template<typename T>
bool func::cmp_vec(std::unordered_map<T, std::size_t> &m, const std::vector<T> &v) {
    for (auto i = v.cbegin(); i != v.cend(); ++i) {
        if (m.find(*i) == m.end()) {
            return false;
        }
        else {
            --m[*i];
        }
    }

    for (auto i = m.cbegin(); i != m.cend(); ++i) {
        if (i->second) {
            return false;
        }
    }

    return true;
}

template<typename T, typename>
bool func::invariants_violated(const std::vector<massive<T>> &v) {
    if (v.size() < 3) {
        return false;//invariant inviolated
    }

    std::size_t X = func::size_of(v, 0);
    std::size_t Y = func::size_of(v, 1);
    std::size_t Z = func::size_of(v, 2);

    return !((Y > X) && (Z > (X + Y)));
}

template<typename T, typename>
std::size_t func::size_of(const std::vector<massive<T>> &v, std::size_t offset) {
    if (offset >= v.size()) {
        std::cerr << "throwing out of range from size_of\n";
        throw std::out_of_range("trying to access element on invalid position");
    }

    return std::next(v.crbegin(), offset)->size();
}

template<bool Left_To_Right,
         typename T,
         typename>
constexpr std::function<bool (T, T)> func::get_cmp_function() {
    if constexpr (Left_To_Right) {
        return  [](T left, T right) {
            return left <= right;
        };
    }
    else {
        return  [](T left, T right) {
            return left >= right;
        };
    }
}

template<bool Left_To_Right,
         typename T,
         template<typename> typename Iter,
         typename,
         typename>
constexpr void func::set_merging_functions(iter_getter<T, Iter> &begin, iter_getter<T, Iter> &end, std::function<bool (T, T)> &cmp) {
    cmp = func::get_cmp_function<Left_To_Right, T>();
    if constexpr (Left_To_Right) {
        end = &std::vector<T>::end;
        begin = &std::vector<T>::begin;
    }
    else {
        end = &std::vector<T>::rend;
        begin = &std::vector<T>::rbegin;
    }
}

void func::advance_offset(std::size_t &offset) {
    ++offset;
    offset *= 2;
    --offset;
}

template<typename T,
         template<typename> typename Iter,
         typename, typename>
void func::insert_range(Iter<T> &insert_pos, Iter<T> &range_begin, Iter<T> range_end) {
    while (range_begin != range_end) {
        *insert_pos = *range_begin;
        ++insert_pos;
        ++range_begin;
    }
}

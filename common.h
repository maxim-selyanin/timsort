#pragma once
#include "pch.h"
#include <QDebug>
//#define SHOW_RUNS
//#define SHOW_MERGES
//#define SHOW_INSERTED_RANGES

#ifndef QDEBUG_H
struct qdeb_subs {
    template<typename T>
    qdeb_subs &operator<<(const T&) {
        return *this;
    }
};
#undef qDebug
using qDebug = qdeb_subs;
#endif

template<typename T>
extern constexpr bool not_bool = !std::is_same_v<T, bool>;

template<typename T>
extern constexpr bool any_char = std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>;

template<typename T>
extern constexpr bool any_int = std::is_integral_v<T> && not_bool<T> && !any_char<T>;

template<typename T>
extern constexpr bool non_bool_int = std::is_integral_v<T> && not_bool<T>;

inline extern constexpr std::size_t default_galloping_treshold = 7;

template<typename T>
using requires_int = std::enable_if_t<any_int<T>>;

template<typename T>
using qiter = typename std::vector<T>::iterator;

template<typename T>
using qreviter = typename std::vector<T>::reverse_iterator;

template<typename T, template<typename> typename Iter>
extern constexpr bool non_const_v_iter = std::is_same_v<Iter<T>, qiter<T>> || std::is_same_v<Iter<T>, qreviter<T>>;

template<typename T, template<typename> typename Iter>
using requires_iter = std::enable_if_t<non_const_v_iter<T, Iter>>;

template<typename T, template<typename> typename Iter>
using iter_getter = Iter<T> (std::vector<T>::*)();

template<typename T, typename = requires_int<T>>
struct massive {
    qiter<T> begin;
    qiter<T> end;

    massive(qiter<T> b, qiter<T> e)
        : begin(b)
        , end(e)
    {}

    massive(const massive &other) = default;
    massive &operator=(const massive &other) = default;

    massive(massive &&other) = default;
    massive &operator=(massive &&other) = default;

    std::size_t size() const {
        return std::distance(begin, end);
    }
};

template<typename T, typename = requires_int<T>>
struct source_massive : massive<T> {
    qiter<T> pos;

    source_massive(std::vector<T> &v)
        : massive<T>(v.begin(), v.end())
        , pos(v.begin())
    {}

    bool ended() const {
        return pos == massive<T>::end;
    }
};

enum class last_occurence {left, right};

enum class current_run {small, big};

enum class order {increasing, decreasing};

class dynamic_counter {
    current_run run = current_run::small;//default value не имеет значения
    std::size_t series = 0;
    std::size_t treshold = default_galloping_treshold;

public:
    dynamic_counter() = default;

    bool operator() (current_run cr) {
        if (series) {
            if (cr == run) {
                ++series;
                if (treshold > 1) {
                    --treshold;
                }
            }
            else {
                series = 1;
                ++treshold;
                run = cr;
            }
        }
        else {
            series = 1;
            run = cr;
        }
        return series >= treshold;//true, если настало время галопа
    }

    void reset() {
        series = 0;
        treshold = default_galloping_treshold;
    }
};

class constant_counter {
    current_run run = current_run::small;
    std::size_t series = 0;

public:
    constant_counter() = default;

    bool operator() (current_run cr) {
        if (series) {
            if (cr == run) {
                ++series;
            }
            else {
                series = 1;
                run = cr;
            }
        }
        else {
            series = 1;
            run = cr;
        }
        return series >= default_galloping_treshold;
    }

    void reset() {
        series = 0;
    }
};

template<typename T,
         template<typename> typename Iter>
struct validated_iter {
    Iter<T> iter;
    bool valid;
};

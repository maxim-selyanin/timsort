#pragma once
#include "functions.h"

namespace sort {
template<typename T, typename = requires_int<T>>
void insertion_sort(qiter<T>, qiter<T>);

template<typename T, typename = std::enable_if_t<non_bool_int<T>>>
massive<T> get_run(source_massive<T>&, std::size_t);

template<typename T, typename = requires_int<T>>
massive<T> merge(qiter<T>, qiter<T>, qiter<T>);

template<typename T,
         template<typename> typename Iter,
         typename = requires_int<T>,
         typename = requires_iter<T, Iter>>
void common_merge(qiter<T>, qiter<T>, qiter<T>);

template<typename T, typename = requires_int<T>>
massive<T> merge_consistent(massive<T>, massive<T>);

template<typename T, typename = requires_int<T>>
void commit_merge(std::vector<massive<T>>&);

template<typename T, typename = requires_int<T>>
bool merge_last_two(std::vector<massive<T>>&);

template<typename T,
         template<typename> typename Iter,
         typename = requires_int<T>,
         typename = requires_iter<T, Iter>>
Iter<T> gallop(Iter<T> begin, Iter<T> end, T source);

template<typename T, typename = requires_int<T>>
long long timsort(std::vector<T>&);
}//namespace sort

template<typename T, typename>
void sort::insertion_sort(qiter<T> begin, qiter<T> end) {
    if (std::distance(begin, end) < 2) {
        return;
    }
    qreviter<T> rend(begin);
    for (qiter<T> i = begin + 1; i < end; ++i) {
        for (qreviter<T> j(std::next(i));//reverse_iterator, указывающий туда же, куда указывает i
             std::next(j) < rend && //j < rend - 1
             j[0] < j[1];
             ++j) {
            func::swap(j[0], j[1]);
        }
    }
}

template<typename T, typename>
massive<T> sort::get_run(source_massive<T> &source, std::size_t minrun) {
    if (std::distance(source.pos, source.end) <= minrun) {
        auto old_pos = source.pos;
        source.pos = source.end;
        sort::insertion_sort<T>(old_pos, source.end);
#ifdef QDEBUG_H
#ifdef SHOW_RUNS
        qDebug() << "run";//del
        func::show<T>(old_pos, source.end);//del
#endif
#endif
        return massive<T>(old_pos, source.end);
    }

    qiter<T> start = source.pos;

    std::function<bool (qiter<T>, qiter<T>)> cmp = [](qiter<T> prev, qiter<T> nxt) {
        return *prev <= *nxt;//by default считаем, что run возрастает
    };
    bool to_reverse = false;

    if (*start > *std::next(start)) {//если run убывает
        cmp = [](qiter<T> prev, qiter<T> nxt) {
            return *prev > *nxt;
        };
        to_reverse = true;
    }

    qiter<T> cur_pos = start + 2;

    while (cur_pos != source.end && cmp(std::prev(cur_pos), cur_pos)) {
        ++cur_pos;
    }

    if (to_reverse) {
        func::reverse<T>(start, cur_pos);
    }

    std::size_t runsize = std::distance(start, cur_pos);
    qiter<T> endrun = cur_pos;
    if (runsize < minrun) {
        endrun += (minrun - runsize);
    }

    source.pos = endrun;
    sort::insertion_sort<T>(start, endrun);
#ifdef QDEBUG_H
#ifdef SHOW_RUNS
    qDebug() << "run";//del
    func::show<T>(start, endrun);//del
#endif
#endif
    return massive<T>(start, endrun);
}

template<typename T, typename>
massive<T> sort::merge(qiter<T> fbegin, qiter<T> fend_lbegin, qiter<T> lend) {
    if (fbegin == fend_lbegin || fend_lbegin == lend) {
        return massive<T>(fbegin, lend);
    }

    //первый элемент последнего массива
    qiter<T> lower_bound = func::basic_binary_search<T, qiter>(fbegin, fend_lbegin, *fend_lbegin, last_occurence::right).iter;

    //последний элемент первого массива, самое левое вхождение
    qiter<T> upper_bound = func::basic_binary_search<T, qiter>(fend_lbegin, lend, *std::prev(fend_lbegin), last_occurence::left).iter;

    if (std::distance(lower_bound, fend_lbegin) <= std::distance(fend_lbegin, upper_bound)) {
        sort::common_merge<T, qiter>(lower_bound, fend_lbegin, upper_bound);
    }
    else {
        sort::common_merge<T, qreviter>(lower_bound, fend_lbegin, upper_bound);
    }

    return massive<T>(fbegin, lend);
}

template<typename T,
         template<typename> typename Iter, typename, typename>
Iter<T> sort::gallop(Iter<T> begin, Iter<T> end, T source) {
    using It = Iter<T>;
    std::size_t offset = 1;
    It prev_iter = begin;
    auto cmp = func::get_cmp_function<std::is_same_v<It, qiter<T>>, T>();//если Iter == iterator, то идём слева направо

    //пока сравнение следующего эл-та с ключом даёт true
    while (std::distance(prev_iter, end) > offset && cmp(*std::next(prev_iter, offset), source)) {
        std::advance(prev_iter, offset);
        func::advance_offset(offset);
    }

    It end_iter;
    if (std::distance(prev_iter, end) >= offset) {//если prev_iter + offset -- валидный итератор
        end_iter = std::next(prev_iter, offset);
    }
    else {
        end_iter = end;
    }

    return  func::basic_binary_search<T, Iter>(prev_iter, end_iter, source, last_occurence::right).iter;
}

template<typename T,
         template<typename> typename Iter, typename, typename>
void sort::common_merge(qiter<T> fbegin, qiter<T> fend_lbegin, qiter<T> lend) {
    if (fbegin == fend_lbegin || fend_lbegin == lend) {
        return;
    }

    using It = Iter<T>;
    constexpr bool left_to_right = std::is_same_v<It, qiter<T>>;

    std::vector<T> tmp;
    if constexpr (left_to_right) {
        tmp.reserve(std::distance(fbegin, fend_lbegin));
        tmp.insert(tmp.end(), fbegin, fend_lbegin);
    }
    else {
        tmp.reserve(std::distance(fend_lbegin, lend));
        tmp.insert(tmp.end(), fend_lbegin, lend);
    }

    iter_getter<T, Iter> begin, end;
    std::function<bool (T, T)> cmp;
    func::set_merging_functions<left_to_right, T, Iter>(begin, end, cmp);

    It tmp_pos((tmp.*begin)());
    const It tmp_end((tmp.*end)());
    It insert_pos((left_to_right) ? fbegin : lend);
    It last_pos(fend_lbegin);
    const It last_end((left_to_right) ? lend : fbegin);

    constant_counter counter;

    while (tmp_pos < tmp_end && last_pos < last_end) {
        if (cmp(*tmp_pos, *last_pos)) {
            *insert_pos = *tmp_pos;
            ++tmp_pos;
            ++insert_pos;
            if (counter(current_run::small) && last_pos != last_end && tmp_pos != tmp_end) {
                func::insert_range<T, Iter>(insert_pos, tmp_pos, sort::gallop<T, Iter>(tmp_pos, tmp_end, *last_pos));
                counter.reset();
            }
        }
        else {
            *insert_pos = *last_pos;
            ++last_pos;
            ++insert_pos;
            if (counter(current_run::big) && tmp_pos != tmp_end && last_pos != last_end) {
                func::insert_range<T, Iter>(insert_pos, last_pos, sort::gallop<T, Iter>(last_pos, last_end, *tmp_pos));
                counter.reset();
            }
        }
    }

    func::insert_range<T, Iter>(insert_pos, tmp_pos, tmp_end);
#ifdef QDEBUG_H
#ifdef SHOW_MERGES
    qDebug() << "merge";//del
    func::show(fbegin, lend);//del
#endif
#endif
}

template<typename T, typename>
massive<T> sort::merge_consistent(massive<T> prev, massive<T> next) {
    if (prev.end != next.begin) {
        std::cerr << "throwing logic error from merge consistent\n";
        throw std::logic_error("merging non consistent massives");
    }

    return sort::merge<T>(prev.begin, prev.end, next.end);
}

template<typename T, typename>
void sort::commit_merge(std::vector<massive<T>> &v) {
    if (v.size() < 3) {
        std::cerr << "throwing logic error from commit merge\n";
        throw std::logic_error("merging in stack that has less than 3 elements");
    }

    std::size_t X = func::size_of(v, 0);
    std::size_t Z = func::size_of(v, 2);

    auto Y_iter = std::next(v.rbegin());//итератор на массив размера Y

    if (X < Z) {
        qDebug() << "calling merge consistent on Y & X";
        *Y_iter = sort::merge_consistent(*Y_iter, *(v.rbegin()));//merge Y and X, store result in Y
        v.pop_back();//cast away X
    }
    else {
        qDebug() << "calling merge consistent on Z & Y";
        auto Z_iter = std::next(v.rbegin(), 2);
        *Z_iter = sort::merge_consistent(*Z_iter, *Y_iter);//merge Z and Y, store result in Z
        *Y_iter = *(v.rbegin());//replace Y with X
        v.pop_back();//cast away copy of X
    }
}

template<typename T, typename>
bool sort::merge_last_two(std::vector<massive<T>> &v) {
    if (v.size() < 2) {
        return false;
    }

    qDebug() << "calling merge consistent on last two";
    *std::next(v.rbegin()) = sort::merge_consistent(*std::next(v.rbegin()), *(v.rbegin()));
    v.pop_back();
    return true;
}

template<typename T, typename>
long long sort::timsort(std::vector<T> &v) {
    std::cout << "got vector with size = " << v.size() << '\n';
    source_massive<T> m(v);
    std::size_t minrun = func::get_minrun(v.size());
    std::cout << "minrun = " << minrun << '\n';
    std::vector<massive<T>> stack;

    auto start = std::chrono::steady_clock::now();
    while (!m.ended()) {
        stack.push_back(sort::get_run(m, minrun));
        while (func::invariants_violated(stack)) {
            sort::commit_merge(stack);
        }
    }

    while (sort::merge_last_two(stack)) {}

    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / v.size();
}

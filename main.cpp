#include "randomizer.h"

int main(int argc, char *argv[]) {
    Q_UNUSED(argc)
    Q_UNUSED(argv)
    /*
     * рандомные вектора можно генерировать следующими функциями:
     * random::random_vector - забивает вектор рандомными элементами
     * random::ordered_vectors_composition - склейка какого то количества упорядоченных как угодно векторов
     * random::any_vectors_composition - склейка какого то количества упорядоченных и рандомных векторов
     */
    random::test_vector<int>(random::any_vectors_composition<int>, 100000, 200000);
    std::cout << "end\n";
    return 0;
}

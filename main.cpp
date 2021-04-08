#include <iostream>
#include <utility>

/*
    Od C++11 zaczela pojawiac sie w jezyku rodzina metod emplace, ktore pozwalaja realizowac
    mechanizm "in-place construction", czyli tworza element np w kontenerze bezposrednio bez
    tworzenia zbednych obiektow posrednich / tymczasowych.

    Metody emplace dzialaja w oparciu o dwa mechanizmy:
    -> perfect forwarding
    -> variadic templates
*/

/*
    Na czym polega problem perfect forwarding?
    Zalozmy ze mamy funkcje f nastepujacej postaci:
    f(T1, T2, ..., Tn)
    Gdzie T1, T2, ..., Tn to parametry generyczne

    Teraz chcemy napisac sobie funkcje ktora "otacza" funkcje f przykladowo nazwijmy ja w1 i argumenty funkcji
    w1 zostana przekazane do funkcji f.

    Teraz pokazemy sobie kilka mozliwosci, jak mozna zrealizowac taki problem.
*/

template<typename T1, typename T2>
void f(const T1& t1, const T2& t2) {
    std::cout << t1 << " " << t2 << std::endl;
}

// ------------------------------------------------------------------------------------------
// Sposob 1
// Ten sposob nie jest dobry.
// Jezeli f pracuje z referencjami to przekazane przez wartosc t1 oraz t2 owszem moga zostac
// zmodyfikowane przez f ale ta modyfikacja bedzie sie tyczyc kopii, ktore przekazano do w1.
// Dlatego jezeli nasza intencja jest modyfikowanie t1 oraz t2 ktore przekazujemy do w1 to nie zadziala
template<typename T1, typename T2>
void w1(T1 t1, T2 t2) {
    f(t1, t2);
}

// ------------------------------------------------------------------------------------------
// Sposob 2
// W takim razie przekazemy do funkcji tym razem w2  parametry przez referencje i nie bedziemy
// dluzej dzialac na kopiach!

// w2(10, "ala");  // blad

// To wywoluje sie prawidlowo
/*
int x = 10;
std::string y = "ala";
w2(x, y);
*/

template<typename T1, typename T2>
void w2(T1& t1, T2& t2) {
    f(t1, t2);
}
// Ale tutaj rowniez pojawiaja sie problemy. Nie bedziemy w stanie przekazywac rvalues
// poniewaz te nie sa w stanie byc przekazane tam gdzie funkcja spodziewa sie referencji

// ------------------------------------------------------------------------------------------
// Sposob 3
// Mozna rozwiazac problem poprzez zastosowanie const
template<typename T1, typename T2>
void w3(const T1& t1, const T2& t2) {
    f(t1, t2);
}
// Ale co jezeli funkcja f bedzie chciala modyfikowac elementy?
// w3(10, "ala");

// ------------------------------------------------------------------------------------------
// Sposob 4
// Kolejne podejscie to zastosowac przeladowanie wszystkich mozliwych wersji funkcji
// jednak duza ilosc argumentow moze nas szybko zniechecic do tego typu rozwiazania

template <typename T1, typename T2>
void w4(T1& t1, T2& t2)
{
    f(t1, t2);
}

template <typename T1, typename T2>
void w4(const T1& t1, T2& t2)
{
    f(t1, t2);
}

template <typename T1, typename T2>
void w4(T1& t1, const T2& t2)
{
    f(t1, t2);
}

template <typename T1, typename T2>
void w4(const T1& t1, const T2& t2)
{
    f(t1, t2);
}

/*
    int x = 10;
    std::string y = "ala";
    w4(x, y);
    w4(x, "ala");
    w4(10, y);
    w4(10, "ala");
*/

// Dodajmy do tego jeszcze koniecznosc obsluzenia argumentow w stylu T1&& t1 zeby byc zgodnym
// ze standardem C++11


// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------

// Na szczescie od C++ 11 mozemy poradzic sobie z ta niedogodnoscia.
// Wprowadzono wtedy dwa mechanizmy
// -> reference collapsing
// -> special type deduction

// -----------------------------------------------------------------------------------------------
// Czym jest reference collapsing
// Mamy funkcje jak ponizej i przesledzmy jej wywolania
template <typename T>
void rc_fun(T t) {
    T& tt = t;
}

/*
    int v = 100;
    rc_fun<int&>(v);
    // T zostanie ustawione jako int&
    // I teraz kiedy przypisujesz do tt to ten ma typ int& &
    // I co zrobi kompilator, jak zinterpetuje taki zapis?

    // A co kompilator zrobi kiedy zapiszemy
    rc_fun<int&&>(10);
    // Wtedy w srodku otrzymamy int&& &

    // Interpretacja tych "zapisow" zajmuje sie mechanizm reference collapsing.
    // Zasada mowi ze zawsze wygrywa &
    // & &  daje nam &
    // && & daje nam &
    // & && daje nam &
    // Jedynie kiedy dojdzie do sytuacji ze otrzymamy && && wtedy wynikiem bedzie &&
*/

// -----------------------------------------------------------------------------------------------
// Mechanizm special type deduction
// W tym mechanizmie T&& pelni inna role niz referencja do rvalue
// Dzieki temu zapisowi interpretacja argumentow zalezec bedzie od tego co przekazemy do funkcji
// Kiedy do funkcji przekazujesz lvalue typu X to T jest dedukowany do X&
// Kiedy do funkcji przekazujesz rvalue typu X to T jest dedukowany do X
/*
     deduction(4);         // 4 to rvalue wiec T to int

     int x = 4;
     deduction(x);         // x to lvalue wiec T do int&
*/
template <typename T>
void deduction(T&& t) {
    // ...
}

// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------
// PERFECT FORWARDING

// Wszystkie powyzsze rozwazania sa wykorzystane w mechanizmie perfect forwarding
// Napiszmy funkcje podobna do tych ktore pokazalismy na samym poczatku
template<typename T>
T&& my_forward(typename std::remove_reference<T>::type& t) noexcept {
    return static_cast<T&&>(t);
}

template <typename T>
T&& my_forward(typename std::remove_reference<T>::type&& t) noexcept {
    return static_cast<T&&>(t);
}

template <typename T1, typename T2>
void w5(T1&& t1, T2&& t2)
{
    f(my_forward<T1>(t1), my_forward<T2>(t2));
}

template <typename T1, typename T2>
void w6(T1&& t1, T2&& t2)
{
    f(std::forward<T1>(t1), std::forward<T2>(t2));
}

// Co nam daje perfect forwarding?
// a. wprowadzenie mechanizmu podobnego do higher order programming
// b. wykorzystywany w funkcjach takich jak std::make_unique czy emplace_back
int main() {

    // ---------------------------------------------------------------------------------
    // Perfect forwarding
    // ---------------------------------------------------------------------------------

    // ---------------------------------------------------------------------------------
    // Wywolanie 1
    // ---------------------------------------------------------------------------------
    int vv = 100;
    std::string ss = "ala";
    w5(vv, ss);

    // Czyli kiedy np mamy pierwszy argument vv to jest to lvalue
    // Dlatego typ T1 zostanie wydedukowany do int& czyli mamy wywolanie w stylu
    // f(my_forward<int&>(vv), ...)
    // czyli dostaniemy wersje my_forward
    /*
    int& && my_forward(int& t) noexcept {
        return static_cast<int& &&>(t)
    }
    */

    // Na podstawie reference collapsing otrzymamy wywolanie
    /*
    int& my_forward(int& t) noexcept {
        return static_cast<int&>(t)
    }
    */

    // Czyli argument zostanie przekazany przez referencje

    // ---------------------------------------------------------------------------------
    // Wywolanie 2
    // ---------------------------------------------------------------------------------
    // Teraz czas na inne wywolanie
    w5(10, "ala");

    // Na przykladzie pierwszego argumentu mamy 10 ktore jest rvalue czyli T1 bedzie
    // wydedukowany do int
    // Dostajemy wywolanie f(my_forward<int>(10), ...) i dostaniemy wersje
    /*
        int&& my_forward(int&& t) noexcept {
            return static_cast<int&&>(t);
        }
    */

    // Okazuje sie ze nie musimy pisac samodzielnie wrappera my_forward poniewaz mamy taki juz
    // w c++ w bibliotece utility i nazywa sie std::forward
    // Pokazalem na przykladzie w6 jak mozemy go wykorzystac
    w6(vv, ss);
    w6(10, "ala");

    return 0;
}

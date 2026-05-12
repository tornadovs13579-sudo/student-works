#include "packed_vector.hpp"
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <numeric>

int main() {
    std::cout << "1. Базовые операции" << std::endl;
    {
        PackedVector<uint8_t, 4> v;
        std::cout << "Пустой вектор: size=" << v.size() 
                  << ", capacity=" << v.capacity() << std::endl;

        v.push_back(5);
        v.push_back(10);
        std::cout << "Добавили 5 и 10: v[0]=" << (int)v[0] 
                  << ", v[1]=" << (int)v[1] << std::endl;

        v[0] = 7;
        std::cout << "Заменили v[0] на 7: v[0]=" << (int)v[0] << std::endl;

        try {
            v.at(100);
        } catch (std::out_of_range& e) {
            std::cout << "v.at(100) выбросил исключение: " << e.what() << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "2. Память" << std::endl;
    {
        PackedVector<uint8_t, 4> v;
        std::cout << "Изначально capacity=" << v.capacity() << std::endl;

        v.reserve(100);
        std::cout << "После reserve(100): capacity=" << v.capacity() << std::endl;

        v.resize(5, 3);
        std::cout << "После resize(5, 3): ";
        for (size_t i = 0; i < v.size(); i++) std::cout << (int)v[i] << " ";
        std::cout << std::endl;

        v.resize(3);
        std::cout << "После resize(3): size=" << v.size() 
                  << ", capacity=" << v.capacity() << std::endl;

        v.shrink_to_fit();
        std::cout << "После shrink_to_fit(): capacity=" << v.capacity() << std::endl;
        std::cout << std::endl;
    }

    std::cout << "3. Специальные методы" << std::endl;
    {
        PackedVector<uint8_t, 4> a(3, 7);
        std::cout << "Вектор a(3, 7): ";
        for (size_t i = 0; i < a.size(); i++) std::cout << (int)a[i] << " ";
        std::cout << std::endl;

        PackedVector<uint8_t, 4> b(a);
        std::cout << "Копия b(a): ";
        for (size_t i = 0; i < b.size(); i++) std::cout << (int)b[i] << " ";
        std::cout << std::endl;

        PackedVector<uint8_t, 4> c(std::move(a));
        std::cout << "Перемещение c(move(a)): c.size=" << c.size() 
                  << ", a.size=" << a.size() << std::endl;

        PackedVector<uint8_t, 4> d;
        d = b;
        std::cout << "Присвоили d = b: d.size=" << d.size() << std::endl;

        PackedVector<uint8_t, 4> e;
        e = std::move(c);
        std::cout << "Присвоили e = move(c): e.size=" << e.size() 
                  << ", c.size=" << c.size() << std::endl;
        std::cout << std::endl;
    }

    std::cout << "4. Вставка и удаление" << std::endl;
    {
        PackedVector<uint8_t, 4> v;
        v.push_back(1);
        v.push_back(3);
        std::cout << "Было: ";
        for (size_t i = 0; i < v.size(); i++) std::cout << (int)v[i] << " ";
        std::cout << std::endl;

        v.insert(v.cbegin() + 1, 2);
        std::cout << "После insert(1, 2): ";
        for (size_t i = 0; i < v.size(); i++) std::cout << (int)v[i] << " ";
        std::cout << std::endl;

        v.insert(v.cbegin(), 0);
        std::cout << "После insert(0, 0): ";
        for (size_t i = 0; i < v.size(); i++) std::cout << (int)v[i] << " ";
        std::cout << std::endl;

        v.erase(v.cbegin() + 1);
        std::cout << "После erase(1):    ";
        for (size_t i = 0; i < v.size(); i++) std::cout << (int)v[i] << " ";
        std::cout << std::endl;
        std::cout << std::endl;
    }

    std::cout << "5. Итераторы" << std::endl;
    {
        PackedVector<uint8_t, 4> v;
        for (uint8_t i = 0; i < 5; i++) v.push_back(i);

        std::cout << "Вектор: ";
        for (size_t i = 0; i < v.size(); i++) std::cout << (int)v[i] << " ";
        std::cout << std::endl;

        std::cout << "Прямой обход:      ";
        for (auto it = v.begin(); it != v.end(); ++it)
            std::cout << (int)*it << " ";
        std::cout << std::endl;

        std::cout << "Range-based for:   ";
        for (auto x : v) std::cout << (int)x << " ";
        std::cout << std::endl;

        std::cout << "Обратный обход:    ";
        for (auto it = v.rbegin(); it != v.rend(); ++it)
            std::cout << (int)*it << " ";
        std::cout << std::endl;

        const auto& cv = v;
        std::cout << "Константный обход: ";
        for (auto it = cv.begin(); it != cv.end(); ++it)
            std::cout << (int)*it << " ";
        std::cout << std::endl;
        std::cout << std::endl;
    }

    std::cout << "6. Алгоритмы STL" << std::endl;
    {
        PackedVector<uint8_t, 4> v;
        v.push_back(5); v.push_back(2); v.push_back(8); v.push_back(1);
        std::cout << "Вектор: ";
        for (auto x : v) std::cout << (int)x << " ";
        std::cout << std::endl;

        auto it = std::find(v.begin(), v.end(), 8);
        std::cout << "find(8): " << (it != v.end() ? "найден" : "не найден") << std::endl;

        int sum = std::accumulate(v.begin(), v.end(), 0);
        std::cout << "accumulate: " << sum << std::endl;

        PackedVector<uint8_t, 4> dst(4);
        std::copy(v.begin(), v.end(), dst.begin());
        std::cout << "copy в dst: ";
        for (auto x : dst) std::cout << (int)x << " ";
        std::cout << std::endl;
        std::cout << std::endl;
    }

    std::cout << "7. Разные типы" << std::endl;
    {
        PackedVector<uint16_t, 5> v;
        for (uint16_t i = 0; i < 10; i++) v.push_back(i % 32);
        std::cout << "uint16_t, 5 бит: size=" << v.size() 
                  << ", v[5]=" << v[5] << ", v[9]=" << v[9] << std::endl;

        PackedVector<uint32_t, 12> w;
        w.push_back(100);
        w.push_back(200);
        w.push_back(4095);
        std::cout << "uint32_t, 12 бит: " << w[0] << ", " << w[1] 
                  << ", " << w[2] << std::endl;
        std::cout << std::endl;
    }

    std::cout << "8. Граничные случаи" << std::endl;
    {
        PackedVector<uint8_t, 4> v;
        v.push_back(255);
        std::cout << "255 в 4 бита = " << (int)v[0] << std::endl;

        v.push_back(16);
        std::cout << "16 в 4 бита = " << (int)v[1] << std::endl;

        PackedVector<uint8_t, 4> v2;
        for (int i = 0; i < 10; i++) v2.push_back(i % 16);
        std::cout << "10 элементов: ";
        for (size_t i = 0; i < v2.size(); i++) std::cout << (int)v2[i] << " ";
        std::cout << std::endl;

        while (!v2.empty()) v2.erase(v2.cbegin());
        std::cout << "После очистки: size=" << v2.size() << std::endl;
    }

    std::cout << std::endl << "9. Упаковка бит" << std::endl;
    {
        PackedVector<uint8_t, 4> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        v.push_back(4);
        std::cout << "4 элемента по 4 бита заняли " << v.capacity() / 2 
                  << " байта вместо 4" << std::endl;
    }
    return 0;
}
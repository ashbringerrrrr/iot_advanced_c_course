/**
 * @file main.c
 * @brief Вычисление площади фигуры, ограниченной тремя кривыми.
 *
 * Программа находит точки пересечения кривых f1, f2, f3 и вычисляет площадь
 * фигуры как сумму двух определённых интегралов с заданной точностью.
 * Поддерживаются опции командной строки для отладки, тестирования и
 * независимого задания точностей ε1 и ε2.
 *
 * Кривые:
 * - f1(x) = 0.6x + 3
 * - f2(x) = (x-2)^3 - 1
 * - f3(x) = 3/x
 *
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define M_PI 3.14159265358979323846

/** 
 ** Глобальные флаги для управления выводом */

/** Флаг печати абсцисс точек пересечения (опция -a) */
int print_abscissas = 0;

/** Флаг печати числа итераций при поиске корней (опция -i) */
int print_iterations = 0;


/** 
/** Пользовательские точности (если заданы через опции)


/** Флаг, указывающий, что точность ε1 задана пользователем */
int eps1_user_defined = 0;
/** Значение пользовательской точности ε1 */
double eps1_user = 0.0;

/** Флаг, указывающий, что точность ε2 задана пользователем */
int eps2_user_defined = 0;
/** Значение пользовательской точности ε2 */
double eps2_user = 0.0;

/* ------------------------------------------------------------------------- */
/** Определения кривых и их производных

/**
 * @brief Первая кривая: f1(x) = 0.6x + 3
 * @param x Аргумент функции
 * @return Значение f1 в точке x
 */
double f1(double x) {
    return 0.6 * x + 3.0;
}

/**
 * @brief Вторая кривая: f2(x) = (x-2)^3 - 1
 * @param x Аргумент функции
 * @return Значение f2 в точке x
 */
double f2(double x) {
    return pow(x - 2.0, 3.0) - 1.0;
}

/**
 * @brief Третья кривая: f3(x) = 3/x
 * @param x Аргумент функции (не должен быть равен 0)
 * @return Значение f3 в точке x
 */
double f3(double x) {
    return 3.0 / x;
}

/**
 * @brief Производная первой кривой: f1'(x) = 0.6
 * @param x Аргумент (не используется)
 * @return 0.6
 */
double df1(double x) {
    (void)x;
    return 0.6;
}

/**
 * @brief Производная второй кривой: f2'(x) = 3(x-2)^2
 * @param x Аргумент
 * @return Значение производной f2 в точке x
 */
double df2(double x) {
    return 3.0 * (x - 2.0) * (x - 2.0);
}

/**
 * @brief Производная третьей кривой: f3'(x) = -3/x^2
 * @param x Аргумент (не должен быть равен 0)
 * @return Значение производной f3 в точке x
 */
double df3(double x) {
    return -3.0 / (x * x);
}


/* ------------------------------------------------------------------------- */
/**  Вспомогательные функции для тестирования */

/**
 * @brief Квадрат числа: x^2
 * @param x Аргумент
 * @return x^2
 */
double square(double x) {
    return x * x;
}

/**
 * @brief Нулевая функция: всегда 0
 * @param x Аргумент (не используется)
 * @return 0
 */
double zero(double x) {
    (void)x;
    return 0.0;
}

/**
 * @brief Синус угла: sin(x)
 * @param x Аргумент в радианах
 * @return sin(x)
 */
double sin_func(double x) {
    return sin(x);
}


/* ------------------------------------------------------------------------- */
/**
 * @brief Подбор интервала со сменой знака.
 *
 * Если на концах исходного интервала [a,b] функция f(x)-g(x) не меняет знак,
 * функция пытается найти внутри меньший подынтервал, где знак меняется,
 * перебирая точки с шагом (b-a)/10. При успехе корректирует a и b.
 *
 * @param f Указатель на первую функцию
 * @param g Указатель на вторую функцию
 * @param a Указатель на левую границу интервала (может быть изменена)
 * @param b Указатель на правую границу интервала (может быть изменена)
 * @return 1 если интервал со сменой знака найден, иначе 0
 */
int adjust_interval(double (*f)(double), double (*g)(double), double *a, double *b) {
    double fa = f(*a) - g(*a);
    double fb = f(*b) - g(*b);
    if (fa * fb <= 0) return 1;

    int max_iter = 100;
    double step = (*b - *a) / 10.0;
    for (int i = 1; i <= max_iter; i++) {
        double x = *a + i * step;
        if (x > *b) break;
        double fx = f(x) - g(x);
        if (fa * fx <= 0) {
            *b = x;
            return 1;
        }
        if (fx * fb <= 0) {
            *a = x;
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
/**
 * @brief Нахождение корня уравнения f(x) = g(x) на отрезке [a,b] методом деления отрезка пополам.
 *
 * Функция ищет приближённое значение x, такое что |f(x)-g(x)| < eps,
 * при условии что на [a,b] функция f(x)-g(x) меняет знак (или может быть подобрана
 * подходящая окрестность с помощью adjust_interval). Возвращает найденный корень.
 *
 * @param f Указатель на первую функцию
 * @param g Указатель на вторую функцию
 * @param a Левая граница начального интервала
 * @param b Правая граница начального интервала
 * @param eps Требуемая точность по аргументу (длина итогового отрезка)
 * @param iter Указатель для сохранения числа выполненных итераций (может быть NULL)
 * @return Приближённое значение корня, или NaN в случае ошибки
 */
double root(double (*f)(double), double (*g)(double), double a, double b, double eps, int *iter) {
    double fa = f(a) - g(a);
    double fb = f(b) - g(b);
    int it = 0;

    if (fa * fb > 0) {
        if (!adjust_interval(f, g, &a, &b)) {
            fprintf(stderr, "Ошибка: не удалось найти интервал со сменой знака.\n");
            if (iter) *iter = -1;
            return NAN;
        }
        fa = f(a) - g(a);
        fb = f(b) - g(b);
    }

    while (b - a > eps) {
        double c = (a + b) / 2.0;
        double fc = f(c) - g(c);
        it++;
        if (fc == 0.0) {
            a = b = c;
            break;
        }
        if (fa * fc <= 0.0) {
            b = c;
            fb = fc;
        } else {
            a = c;
            fa = fc;
        }
    }
    if (iter) *iter = it;
    return (a + b) / 2.0;
}

/* ------------------------------------------------------------------------- */
/**
 * @brief Вычисление определённого интеграла методом Симпсона с автоматическим выбором шага.
 *
 * Интеграл вычисляется с удвоением числа разбиений до тех пор, пока оценка
 * погрешности по правилу Рунге не станет меньше eps.
 *
 * @param fun Подынтегральная функция
 * @param a Нижний предел интегрирования
 * @param b Верхний предел интегрирования
 * @param eps Требуемая точность вычисления интеграла
 * @return Приближённое значение интеграла
 */
double integral(double (*fun)(double), double a, double b, double eps) {
    int n = 2;                     ///< начальное число интервалов (чётное)
    double I_prev = 0.0;
    double I_curr;
    int max_iter = 30;

    for (int iter = 0; iter < max_iter; ++iter) {
        double h = (b - a) / n;
        double sum = fun(a) + fun(b);

        for (int i = 1; i < n; ++i) {
            double x = a + i * h;
            if (i % 2 == 0)
                sum += 2.0 * fun(x);
            else
                sum += 4.0 * fun(x);
        }
        I_curr = h / 3.0 * sum;

        if (iter > 0) {
            /// Правило Рунге: |I_curr - I_prev| / 15 < eps
            if (fabs(I_curr - I_prev) < 15.0 * eps) {
                return I_curr;
            }
        }
        I_prev = I_curr;
        n *= 2;
        if (n > 1000000) break;   ///< защита от бесконечного цикла
    }
    fprintf(stderr, "Метод Симпсона не сошёлся.\n");
    return I_curr;
}

/* ------------------------------------------------------------------------- */
/**  Подынтегральные функции для площади */

/**
 * @brief Разность f1 - f2 (используется на отрезке [xA, xC])
 * @param x Аргумент
 * @return f1(x) - f2(x)
 */
double integrand_f1_minus_f2(double x) {
    return f1(x) - f2(x);
}

/**
 * @brief Разность f1 - f3 (используется на отрезке [xC, xB])
 * @param x Аргумент
 * @return f1(x) - f3(x)
 */
double integrand_f1_minus_f3(double x) {
    return f1(x) - f3(x);
}

/**
 * @brief Тестирование функции root на известных примерах.
 *
 * Проверяются:
 * - корень уравнения x^2 = 2 на [0,2] (ожидается sqrt(2))
 * - корень уравнения sin(x) = 0 на [2,4] (ожидается π)
 * - случай, когда корня нет на [-2,-1] (должен вернуть NaN)
 */
void test_root(void) {
    printf("----- Тестирование root -----\n");
    double eps = 1e-6;
    int iter;
    double r = root(square, zero, 0.0, 2.0, eps, &iter);
    double expected = sqrt(2.0);
    printf("root(x^2-2, [0,2], eps=%g) = %.10f, итераций %d, погрешность %g\n",
           eps, r, iter, fabs(r - expected));

    r = root(sin_func, zero, 2.0, 4.0, eps, &iter);
    expected = M_PI;
    printf("root(sin(x), [2,4], eps=%g) = %.10f, итераций %d, погрешность %g\n",
           eps, r, iter, fabs(r - expected));

    r = root(square, zero, -2.0, -1.0, eps, &iter);
    if (isnan(r))
        printf("root(x^2-2, [-2,-1]) вернул NaN (ожидаемо, корня нет)\n");
    else
        printf("root(x^2-2, [-2,-1]) = %f (неожиданно)\n", r);
}

/**
 * @brief Тестирование функции integral на известных примерах.
 *
 * Проверяются:
 * - интеграл от x^2 на [0,1] (ожидается 1/3)
 * - интеграл от sin(x) на [0,π] (ожидается 2)
 */
void test_integral(void) {
    printf("----- Тестирование integral -----\n");
    double eps = 1e-6;
    double I = integral(square, 0.0, 1.0, eps);
    double expected = 1.0/3.0;
    printf("integral(x^2, [0,1], eps=%g) = %.10f, погрешность %g\n",
           eps, I, fabs(I - expected));

    I = integral(sin_func, 0.0, M_PI, eps);
    expected = 2.0;
    printf("integral(sin(x), [0,pi], eps=%g) = %.10f, погрешность %g\n",
           eps, I, fabs(I - expected));
}

/* ------------------------------------------------------------------------- */
/**
 * @brief Вывод справки по использованию программы.
 */
void print_help(void) {
    printf("Использование: ./area [опции]\n");
    printf("Опции:\n");
    printf("  -a                     печатать абсциссы точек пересечения кривых\n");
    printf("  -i                     печатать число итераций при поиске корней\n");
    printf("  -test                  запустить тестирование функций root и integral\n");
    printf("  -e1 <значение>         задать точность ε1 для поиска корней\n");
    printf("  -e2 <значение>         задать точность ε2 для интегрирования\n");
    printf("  -help                  показать эту справку\n");
    printf("\nЕсли заданы обе точности -e1 и -e2, они используются непосредственно.\n");
    printf("В противном случае программа запрашивает общую точность ε\n");
    printf("и вычисляет ε1 = ε2 = ε/100.\n");
}

/* ------------------------------------------------------------------------- */
/**
 * @brief Главная функция программы.
 *
 * Разбирает аргументы командной строки, устанавливает флаги и пользовательские точности.
 * Если запрошено тестирование (-test), выполняет тесты и завершается.
 * Иначе, если заданы -e1 и -e2, использует их; в противном случае запрашивает ε
 * и вычисляет ε1 = ε2 = ε/100. Затем вычисляет точки пересечения и площадь,
 * при необходимости выводит дополнительную информацию.
 *
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 * @return 0 при успешном завершении, 1 при ошибке
 */
int main(int argc, char *argv[]) {
    int do_test = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'a') {
                print_abscissas = 1;
            }
            else if (argv[i][1] == 'i') {
                print_iterations = 1;
            }
            else if (strcmp(argv[i], "-test") == 0) {
                do_test = 1;
            }
            else if (strcmp(argv[i], "-help") == 0) {
                print_help();
                return 0;
            }
            else if (strcmp(argv[i], "-e1") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "Ошибка: после -e1 необходимо указать значение.\n");
                    return 1;
                }
                eps1_user = atof(argv[++i]);
                eps1_user_defined = 1;
            }
            else if (strcmp(argv[i], "-e2") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "Ошибка: после -e2 необходимо указать значение.\n");
                    return 1;
                }
                eps2_user = atof(argv[++i]);
                eps2_user_defined = 1;
            }
            else {
                fprintf(stderr, "Неизвестная опция: %s\n", argv[i]);
                print_help();
                return 1;
            }
        }
    }

    if (do_test) {
        test_root();
        test_integral();
        return 0;
    }

    double eps1, eps2;

    if (eps1_user_defined && eps2_user_defined) {
        eps1 = eps1_user;
        eps2 = eps2_user;
        printf("Используются заданные точности: ε1 = %g, ε2 = %g\n", eps1, eps2);
    }
    else if (eps1_user_defined || eps2_user_defined) {
        fprintf(stderr, "Ошибка: должны быть заданы обе точности -e1 и -e2, либо ни одной.\n");
        return 1;
    }
    else {
        double eps;
        printf("Введите требуемую точность вычисления площади eps: ");
        if (scanf("%lf", &eps) != 1 || eps <= 0) {
            fprintf(stderr, "Ошибка: необходимо ввести положительное число.\n");
            return 1;
        }
        eps1 = eps / 100.0;
        eps2 = eps / 100.0;
        printf("Автоматически выбраны: ε1 = %g, ε2 = %g\n", eps1, eps2);
    }

    int iter1, iter2, iter3;
    double xA = root(f1, f3, 0.5, 1.5, eps1, &iter1);
    double xB = root(f1, f2, 3.0, 4.0, eps1, &iter2);
    double xC = root(f2, f3, 3.0, 4.0, eps1, &iter3);

    if (isnan(xA) || isnan(xB) || isnan(xC)) {
        fprintf(stderr, "Ошибка: не удалось найти все точки пересечения.\n");
        return 1;
    }

    if (xA > xC) { double t = xA; xA = xC; xC = t; }
    if (xC > xB) { double t = xC; xC = xB; xB = t; }
    if (xA > xC) { double t = xA; xA = xC; xC = t; }

    if (print_abscissas) {
        printf("Абсциссы точек пересечения:\n");
        printf("  xA (f1=f3) = %.10f\n", xA);
        printf("  xC (f2=f3) = %.10f\n", xC);
        printf("  xB (f1=f2) = %.10f\n", xB);
    }

    if (print_iterations) {
        printf("Число итераций при поиске корней:\n");
        printf("  f1=f3: %d\n", iter1);
        printf("  f1=f2: %d\n", iter2);
        printf("  f2=f3: %d\n", iter3);
    }

    double I1 = integral(integrand_f1_minus_f2, xA, xC, eps2);
    double I2 = integral(integrand_f1_minus_f3, xC, xB, eps2);
    double area = I1 + I2;

    printf("Площадь фигуры = %.10f\n", area);

    return 0;
}
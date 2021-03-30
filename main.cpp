#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <cstddef>

struct my_ifstream {
    std::ifstream source;

    my_ifstream(char *other) : source(other) {}

    bool is_open() {
        return source.is_open();
    }

    std::istream &get_stream() {
        return source;
    }

    ~my_ifstream() {
        source.close();
    }
};

struct my_ofstream {
    std::ofstream source;

    my_ofstream(char *other) : source(other) {}

    std::ofstream &get_stream() {
        return source;
    }

    bool is_open() {
        return source.is_open();
    }

    ~my_ofstream() {
        source.close();
    }
};

class big_int {
    std::vector<int32_t> digits;
    bool negate;

    void remove_zero();

    explicit big_int(bool negate);

    explicit big_int();

    explicit big_int(int32_t);

public:
    explicit big_int(std::string s);

    big_int operator-() const;

    friend bool operator==(const big_int &first, const big_int &second);

    friend bool operator!=(const big_int &first, const big_int &second);

    friend bool operator>(const big_int &first, const big_int &second);

    friend bool operator<(const big_int &first, const big_int &second);

    friend bool operator>=(const big_int &first, const big_int &second);

    friend bool operator<=(const big_int &first, const big_int &second);

    friend big_int operator+(big_int first, const big_int &second);

    friend big_int operator-(big_int first, const big_int &second);

    friend big_int operator*(const big_int &first, const big_int &second);

    friend big_int operator/(big_int first, big_int second);

    friend big_int operator%(const big_int &first, const big_int &second);

    friend big_int sqrt(const big_int &number);

    friend big_int short_divide(big_int first, const int &second);

    friend std::ostream &operator<<(std::ostream &out, const big_int &number);

};

static const int32_t base = 1e9;

big_int::big_int(bool negate) : negate(negate), digits({0}) {}

big_int::big_int() : big_int(false) {}

big_int::big_int(int32_t value) : negate(false), digits({static_cast<int32_t>(value % base)}) {}

void big_int::remove_zero() {
    while (digits.size() > 1 && digits.back() == 0) {
        digits.pop_back();
    }

    if (digits.size() == 1 && !digits[0]) {
        negate = false;
    }
}

big_int::big_int(std::string s) {
    if (!s.length()) {
        this->negate = false;
    } else {
        if (s[0] == '-') {
            s = s.substr(1);
            this->negate = true;
        } else {
            this->negate = false;
        }

        for (long long i = s.length(); i > 0; i -= 9) {
            if (i < 9)
                this->digits.push_back(atoi(s.substr(0, i).c_str()));
            else
                this->digits.push_back(atoi(s.substr(i - 9, 9).c_str()));
        }
    }
    remove_zero();;
}

big_int big_int::operator-() const {
    big_int copy(*this);
    copy.negate = !(copy.negate);
    return copy;
}

bool operator!=(const big_int &first, const big_int &second) {
    return !(first == second);
}

bool operator>(const big_int &first, const big_int &second) {
    return !(first <= second);
}

bool operator>=(const big_int &first, const big_int &second) {
    return !(first < second);
}

bool operator<=(const big_int &first, const big_int &second) {
    return (first == second || first < second);
}

big_int operator+(big_int first, const big_int &second) {
    if (first.negate) {
        if (second.negate) {
            return -(-(first) + -(second));
        } else {
            return second - (-first);
        }
    } else if (second.negate) {
        return first - (-second);
    }
    int32_t ost = 0;
    for (std::size_t i = 0; i < std::max(first.digits.size(), second.digits.size()) || ost; i++) {
        if (i == first.digits.size()) {
            first.digits.emplace_back(0);
        }
        first.digits[i] += ost + (i < second.digits.size() ? second.digits[i] : 0);
        ost = first.digits[i] >= base;
        if (ost != 0) {
            first.digits[i] -= base;
        }
    }

    return first;
}

big_int operator-(big_int first, const big_int &second) {
    if (second.negate) return first + (-second);
    else if (first.negate) return -(-first + second);
    else if (first < second) return -(second - first);
    int32_t borrow = 0;
    for (std::size_t i = 0; i < second.digits.size() || borrow != 0; i++) {
        first.digits[i] -= borrow + (i < second.digits.size() ? second.digits[i] : 0);
        if (first.digits[i] < 0) {
            borrow = 1;
        } else {
            borrow = 0;
        }
        if (borrow != 0) {
            first.digits[i] += base;
        }
    }

    first.remove_zero();
    return first;
}

big_int operator*(const big_int &first, const big_int &second) {
    big_int ans;
    ans.digits.resize(first.digits.size() + second.digits.size());
    for (std::size_t i = 0; i < first.digits.size(); i++) {
        int32_t ost = 0;
        for (std::size_t j = 0; j < second.digits.size() || ost != 0; j++) {
            uint64_t cur = ans.digits[i + j] +
                           first.digits[i] * 1LL * (j < second.digits.size() ? second.digits[j] : 0) + ost;
            ans.digits[i + j] = static_cast<int32_t>(cur % base);
            ost = static_cast<int32_t>(cur / base);
        }
    }
    ans.negate = first.negate != second.negate;
    ans.remove_zero();
    return ans;
}

big_int operator/(big_int first, big_int second) {
    big_int zero = big_int();
    if (second == zero) {
        return big_int(true);
    } else {
        big_int l = zero, r = first;
        bool f_neg = first.negate, s_neg = second.negate;
        r.negate = false;
        first.negate = false;
        second.negate = false;
        big_int one = big_int(1);
        if (second == one) {
            return first;
        }
        while (l + one < r) {
            big_int m = short_divide((l + r), 2);
            if (m * second <= first) {
                l = m;
            } else {
                r = m;
            }
        }
        l.negate = (f_neg ^ s_neg);
        return l;
    }
}

big_int short_divide(big_int first, const int &second) {
    int32_t ost = 0;
    for (std::size_t i = first.digits.size(); i > 0; i--) {
        uint64_t cur = first.digits[i - 1] + ost * 1ll * base;
        first.digits[i - 1] = static_cast<int32_t>(cur / second);
        ost = static_cast<int32_t>(cur % second);
    }
    first.remove_zero();
    return first;
}

big_int operator%(const big_int &first, const big_int &second) {
    return (first - (first / second) * second);
}

big_int sqrt(const big_int &number) {
    if (number.negate) {
        big_int ans = big_int();
        ans.negate = true;
        return ans;
    } else {
        big_int zero;
        big_int one = big_int(1);
        if (number == zero || number == one) {
            return number;
        }
        big_int l = zero;
        big_int r = number;

        while (l + one < r) {
            big_int m = short_divide((l + r), 2);
            if (m * m <= number) {
                l = m;
            } else {
                r = m;
            }
        }
        return l;
    }
}

std::ostream &operator<<(std::ostream &out, const big_int &number) {
    if (number.digits.size() == 1 && number.digits.back() == 0 && number.negate) {
        out << "NaN";
    } else {
        if (number.digits.empty()) {
            out << 0;
        } else {
            if (number.negate) {
                out << '-';
            }
            if (number.digits.size() == 1) {
                out << number.digits[number.digits.size() - 1];
            } else {
                out << number.digits[number.digits.size() - 1];
                for (std::size_t i = number.digits.size() - 1; i > 0; i--) {
                    int32_t count = 1e8;
                    int32_t help = number.digits[i - 1];
                    while (count > 0) {
                        out << help / count;
                        help %= count;
                        count /= 10;
                    }
                }
            }
        }
    }
    return out;
}

bool operator==(const big_int &first, const big_int &second) {
    return !(first < second) && !(second < first);
}

bool operator<(const big_int &first, const big_int &second) {
    if (first.negate != second.negate) {
        return second.negate;
    }
    if (first.negate) {
        return (-first < -second);
    }
    if (first.digits.size() != second.digits.size()) {
        return first.digits.size() < second.digits.size();
    }
    for (std::size_t i = first.digits.size(); i > 0; i--) {
        if (first.digits[i - 1] != second.digits[i - 1]) {
            return (first.digits[i - 1] < second.digits[i - 1]);
        }
    }
    return false;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("invalid count of arguments");
        return 1;
    }

    std::string s1, s2, operation;
    my_ifstream fin(argv[1]);
    if (!fin.is_open()) {
        std::cout << "Can't open input file" << std::endl;
        return 2;
    }

    my_ofstream fout(argv[2]);

    if (!fout.is_open()) {
        std::cout << "Can't open output file" << std::endl;
        return 3;
    }

    fin.get_stream() >> s1;
    big_int first_number(s1);

    fin.get_stream() >> operation;
    if (operation == "#") {
        fout.get_stream() << sqrt(first_number);
    } else {
        fin.get_stream() >> s2;
        big_int second_number(s2);
        if (operation == "+") {
            fout.get_stream() << first_number + second_number;
        }
        if (operation == "-") {
            fout.get_stream() << first_number - second_number;
        }

        if (operation == "*") {
            fout.get_stream() << first_number * second_number;
        }

        if (operation == "/") {
            fout.get_stream() << first_number / second_number;
        }
        if (operation == "%") {
            fout.get_stream() << first_number % second_number;
        }
        if (operation == "<=") {
            if (first_number <= second_number) {
                fout.get_stream() << 1;
            } else {
                fout.get_stream() << 0;
            }
        }
        if (operation == "<") {
            if (first_number < second_number) {
                fout.get_stream() << 1;
            } else {
                fout.get_stream() << 0;
            }
        }
        if (operation == "==") {
            if (first_number == second_number) {
                fout.get_stream() << 1;
            } else {
                fout.get_stream() << 0;
            }
        }
        if (operation == ">=") {
            if (first_number >= second_number) {
                fout.get_stream() << 1;
            } else {
                fout.get_stream() << 0;
            }
        }
        if (operation == ">") {
            if (first_number > second_number) {
                fout.get_stream() << 1;
            } else {
                fout.get_stream() << 0;
            }
        }
        if (operation == "!=") {
            if (first_number != second_number) {
                fout.get_stream() << 1;
            } else {
                fout.get_stream() << 0;
            }
        }

    }

    return 0;
}
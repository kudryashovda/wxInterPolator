#include "interpolator.h"

using namespace std;

Interpolator::Interpolator(double first, double last, double step, const std::string& str_data, char delimLines, char delimData) :
    Interpolator(first, last, step,  SplitIntoData(str_data, delimLines, delimData)) {
}

Interpolator::Interpolator(double first, double last, double step, const std::vector<Coord>& input_data) {

    m_output_data.clear();

    constexpr size_t MIN_REQUIRED_POINTS = 2;

    if(input_data.size() < MIN_REQUIRED_POINTS) {
        return;
    }

    vector<Coord> v_sorted(input_data.begin(), input_data.end());

    sort(v_sorted.begin(), v_sorted.end(), [](const Coord &a, const Coord &b) {
        return a.x < b.x;
    });

    first = max(first, v_sorted.front().x);
    last = min(last, v_sorted.back().x);

    size_t steps = 1 + (last - first) / step;

    size_t it = 1; // second element in sorted array

    for(size_t i = 0; i < steps; ++i) {
        double x = first + i * step;

        while(x > v_sorted[it].x) {
            ++it;   // danger! Upper the range check was already done
        }

        double k = 0, b = 0; // slope and intercept

        try {
            k = (v_sorted[it].y - v_sorted[it-1].y) / (v_sorted[it].x - v_sorted[it-1].x);
            b = v_sorted[it-1].y - k * v_sorted[it-1].x;
        } catch(...) {};

        m_output_data.push_back({x, (k*x + b)});
    }
}

std::vector<Coord> Interpolator::getInterpolatedData() {
    return m_output_data;
}

void tokenizer(vector<string>& vs, string str, string delim) {
    size_t pos = 0;
    string token;
    vs.clear();

    if(str.find(delim) == string::npos) return;  // if no delim string - exit

    do {
        pos = str.find_first_of(delim);
        token = str.substr(0,pos);
        vs.push_back(token);
        str = str.substr(pos+1);
    } while(pos != string::npos);
}

vector<string> SplitIntoTerms(const string& text, char delim) {
    vector<string> terms;
    string term;

    for(char c : text) {
        if(c == delim) {
            if(!term.empty()) {
                terms.push_back(term);
                term.clear();
            }
        } else {
            term += c;
        }
    }

    if(!term.empty()) {
        terms.push_back(term);
    }

    return terms;
}

vector<Coord> SplitIntoData(const string& text, char delimLines, char delimData) {
    vector<Coord> dataValues;

    vector<string> lines = SplitIntoTerms(text, delimLines);

    for(const auto& line: lines) {
        vector<string> vs = SplitIntoTerms(line, delimData);

        if(vs.empty()) {
            continue;
        }

        try {
            double x = stof(vs[0]);
            double y = stof(vs[1]);

            dataValues.push_back({x,y});
        } catch(...) {
            continue;
        }
    }

    return dataValues;
}

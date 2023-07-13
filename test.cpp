#include <string>
#include <vector>
#include <utility>
#include <chrono>
#include <iostream>
#include <fstream>

#include <algorithm>
#include <deque>
#include <string_view>

#define VEC_SIZE 4085

using namespace std;
using namespace chrono;

using write_sequence = vector<string>;

using test_pair = pair<uint64_t, string>;
using modify_sequence = vector<test_pair>;
using read_sequence = vector<test_pair>;

ifstream& operator >> (ifstream& _is, test_pair& _value)
{
    _is >> _value.first;
    _is >> _value.second;

    return _is;
}

template <typename S>
S get_sequence(const string& _file_name)
{
    ifstream infile(_file_name);
    S result;

    typename S::value_type item;

    while (infile >> item)
    {
        result.emplace_back(move(item));
    }

    return result;
}


class storage
{
public:
    storage() {
        data_.push_back(deque<const string*>{});
        max_vec_id_ = 0;
    }

    void insert(const string& _str) {
        if (data_[max_vec_id_].size() == VEC_SIZE) {
            data_.push_back(deque<const string*>{});
            ++max_vec_id_;
        }
        binary_insert(0, max_vec_id_, _str);
    }

    void erase(uint64_t _index) {
        int id = _index / VEC_SIZE;
        data_[id].erase(data_[id].begin() + (_index % VEC_SIZE));
        for (int i = max_vec_id_; i > id; --i) {
            const string* temp = *data_[i].begin();
            data_[i].pop_front();
            data_[i - 1].push_back(temp);
        }
    }

    const string& get(uint64_t _index) const {
        int id = _index / VEC_SIZE;
        return *data_[id][_index % VEC_SIZE];
    }
private:
    void binary_insert(int id_start, int id_end, const string& _str) {
        if (id_start == id_end) {
            deque<const string*>::iterator iter = upper_bound(data_[id_end].begin(), data_[id_end].end(), &_str,
                [](const string* a, const string* b) {
                return *a < *b;
            });
            data_[id_end].insert(iter, &_str);
            for (; id_end < max_vec_id_; ++id_end) {
                const string* temp = data_[id_end].back();
                data_[id_end].pop_back();
                data_[id_end + 1].push_front(temp);
            }
        }
        else {
            int temp_id = (id_start + id_end) / 2;

            if (_str > **(data_[temp_id].end() - 1)) {
                binary_insert(temp_id + 1, id_end, _str);
            }
            else if (_str <= **data_[temp_id].begin()) {
                binary_insert(id_start, temp_id, _str);
            }
            else {
                binary_insert(temp_id, temp_id, _str);
            }
        }
    }

    vector<deque<const string*>> data_;
    int max_vec_id_;
};

int main()

{
    setlocale(LC_ALL, "Russian");//delete

    write_sequence write = get_sequence<write_sequence>("write.txt");
    modify_sequence modify = get_sequence<modify_sequence>("modify.txt");
    read_sequence read = get_sequence<read_sequence>("read.txt");

    storage st;
    for (const string& item : write)
    {
        st.insert(item);
    }
    uint64_t progress = 0;
    uint64_t percent = modify.size() / 100;

    time_point<system_clock> time;
    nanoseconds total_time(0);

    modify_sequence::const_iterator mitr = modify.begin();
    read_sequence::const_iterator ritr = read.begin();
    int count = 0;
    for (; mitr != modify.end() && ritr != read.end(); ++mitr, ++ritr)
    {
        time = system_clock::now();
        st.erase(mitr->first);
        st.insert(mitr->second);
        const string& str = st.get(ritr->first);
        total_time += system_clock::now() - time;

        if (ritr->second != str)
        {
            cout << "test failed" << endl;
            return 1;
        }

        if (++progress % (5 * percent) == 0)
        {
            cout << "time: " << duration_cast<milliseconds>(total_time).count()
                << "ms progress: " << progress << " / " << modify.size() << "\n";
        }
        ++count;
    }

    return 0;
}

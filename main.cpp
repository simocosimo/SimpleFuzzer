#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <bitset>

std::vector<std::bitset<8>> get_data(const std::string filename) {
    
    std::ifstream file(filename, std::ios::binary);
    if(file.is_open()) {
        std::vector<std::bitset<8>> data(std::istreambuf_iterator<char>(file), {});

        std::cout << "File '" << filename << "' read correctly" << std::endl;
        file.close();
        return data;
    }

    std::cout << "Couldn't read '" << filename << "' correctly" << std::endl;
    return {};
}

bool create_new(const std::vector<std::bitset<8>> data, const std::string filename) {
    std::ofstream file(filename, std::ios::binary);
    
    if(file.is_open()) {
        std::vector<char> cc;
        for(auto d : data) cc.push_back(static_cast<unsigned char>(d.to_ulong()));
        file.write(static_cast<const char*>(&cc[0]), cc.size());
        file.close();
        std::cout << "Data written to '" << filename << "' correctly" << std::endl;
        return true;
    }

    std::cout << "Couldn't write data to '" << filename << "' correctly" << std::endl;
    return false;
}

template<typename T>
T random(T range_from, T range_to) {
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<T> distr(range_from, range_to);
    return distr(generator);
}

void bit_flip(std::vector<std::bitset<8>> &data) {
    const double ratio_to_flip = 0.01;
    int num_of_flips = (data.size() - 4) * ratio_to_flip;
    std::vector<unsigned long> chosen_indexes {};

    for(int counter = 0; counter < num_of_flips; ++counter) {
        chosen_indexes.push_back(random((unsigned long)4, data.size() - 4));
    }

    std::cout << "Number of chosen indexes: " << chosen_indexes.size() << std::endl;
    std::cout << "Chosen indexes: ";
    for(auto i = chosen_indexes.begin(); i != chosen_indexes.end(); ++i) {
        std::cout << *i << " ";
    }
    std::cout << std::endl;

    for(auto i : chosen_indexes) {
        auto rand_bit = random(0, 7);
        auto prev = data[i];
        data[i].flip(rand_bit);
        std::cout << "data[" << i << "] changed from " << prev << " to " << data[i] << std::endl;
    }

}

int main(int argc, char** argv) {

    const std::string output_file = "modded.jpg";

    if(argc < 2) {
        std::cout << "Usage: " << argv[0] << " <valid_jpg>" << std::endl;
        exit(1);
    }

    const std::string filename = argv[1];
    auto data = get_data(filename);

    // TODO: bettere error handling
    if(data.size() == 0) {
        exit(1);
    }
    
    // -- Bit flipping
    bit_flip(data);

    if(!create_new(data, output_file)) {
        exit(1);
    }

    return 0;
}

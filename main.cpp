#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <bitset>
#include <array>
#include <memory>

std::vector<std::bitset<8>> get_data(const std::string filename) {
    
    std::ifstream file(filename, std::ios::binary);
    if(file.is_open()) {
        std::vector<std::bitset<8>> data(std::istreambuf_iterator<char>(file), {});

        //std::cout << "File '" << filename << "' read correctly" << std::endl;
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
        //std::cout << "Data written to '" << filename << "' correctly" << std::endl;
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
        chosen_indexes.push_back(random(static_cast<unsigned long>(4), data.size() - 4));
    }

    //std::cout << "Number of chosen indexes: " << chosen_indexes.size() << std::endl;
    //std::cout << "Chosen indexes: ";
    // for(auto i = chosen_indexes.begin(); i != chosen_indexes.end(); ++i) {
    //     std::cout << *i << " ";
    // }
    //std::cout << std::endl;

    for(auto i : chosen_indexes) {
        auto rand_bit = random(0, 7);
        auto prev = data[i];
        data[i].flip(rand_bit);
        //std::cout << "data[" << i << "] changed from " << prev << " to " << data[i] << std::endl;
    }

}

void magic(std::vector<std::bitset<8>> &data) {

    // Four byte magics
    const std::vector<std::bitset<4*8>> magics {
        std::bitset<4*8>(0xFF),
        std::bitset<4*8>(0x7F),
        std::bitset<4*8>(0x00),

        std::bitset<4*8>(0xFFFF),
        std::bitset<4*8>(0x0000),
        
        std::bitset<4*8>(0xFFFFFFFF),
        std::bitset<4*8>(0x00000000),
        std::bitset<4*8>(0x80000000),
        std::bitset<4*8>(0x40000000),
        std::bitset<4*8>(0x7FFFFFFF)
    };

    int picked_magic = random(0, 9);
    int chosen_index = random(static_cast<unsigned long>(4), data.size() - 4);
    std::bitset<4*8> bitmask(0xFF);
    //std::cout << "Picked: " << picked_magic << ", " << magics[picked_magic] << std::endl;

    if (picked_magic >= 0 && picked_magic <=2) {
        // One byte values
        data[chosen_index] = std::bitset<8>(magics[picked_magic].to_ulong());
    } else if(picked_magic >= 3 && picked_magic <= 4) {
        // Two bytes values
        std::bitset<8> lower((magics[picked_magic] & bitmask).to_ullong());
        std::bitset<8> higher((magics[picked_magic] >> 8 & bitmask).to_ullong());
        data[chosen_index] = higher;
        data[chosen_index + 1] = lower;

        //std::cout << lower << " " << higher << std::endl;
    } else {
        // Four bytes values
        std::bitset<8> low_lower((magics[picked_magic] & bitmask).to_ullong());
        std::bitset<8> lower((magics[picked_magic] >> 8 & bitmask).to_ullong());
        std::bitset<8> higher((magics[picked_magic] >> 16 & bitmask).to_ullong());
        std::bitset<8> high_higher((magics[picked_magic] >> 24 & bitmask).to_ullong());
        data[chosen_index] = high_higher;
        data[chosen_index + 1] = higher;
        data[chosen_index + 2] = lower;
        data[chosen_index + 3] = low_lower;

        //std::cout << low_lower << " " << lower << " " << higher << " " << high_higher << std::endl; 
    }
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;

    auto pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    
    while (!feof(pipe)) {
        if (fgets(buffer.data(), 128, pipe) != nullptr)
            result += buffer.data();
    }
    
    pclose(pipe);
    return result;
}

void exif(int counter, const std::vector<std::bitset<8>> data) {

    std::string command = "bash -c \"{ ./exif/exif modded.jpg -verbose; } 2>&1\"";
    std::string output = exec(command.c_str());

    if(output.find("Segmentation") != std::string::npos) {
        // Segmentation fault founded
        char buffer[50];
        sprintf(buffer, "crashes/crash.%d.jpg", counter);
        if(!create_new(data, buffer)) {
            exit(1);
        }

        std::cout << "Found a SIGSEGV on iteration #" << counter << std::endl;
    }

    if (counter % 100 == 0) {
        std::cout << "Iteration #" << counter << std::endl;
    }
}

int main(int argc, char** argv) {

    const std::string output_file = "modded.jpg";

    if(argc < 2) {
        std::cout << "Usage: " << argv[0] << " <valid_jpg>" << std::endl;
        exit(1);
    }
    
    const std::string filename = argv[1];

    for(int i = 0; i < 1000; ++i) {
        auto data = get_data(filename);

        // TODO: better error handling
        if(data.size() == 0) {
            exit(1);
        }
        
        int fuzz_mode = random(0, 1);
        if (fuzz_mode) {
            bit_flip(data);
        } else {
            magic(data);
        }

        if(!create_new(data, output_file)) {
            exit(1);
        }

        exif(i, data);
    }

    return 0;
}

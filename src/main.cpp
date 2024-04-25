#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <string>

#include <tlvcpp/tlv_tree.h>
#include <tlvcpp/utilities/hexdump.h>

static std::unordered_map<tlvcpp::tag_t, std::string> tags;

const char *parser(const tlvcpp::tag_t tag)
{
    const auto tag_pair = tags.find(tag);

    if (tag_pair != tags.end())
        return tag_pair->second.c_str();

    static char buffer[16];

    snprintf(buffer, sizeof(buffer), "0x%02x", tag);

    return buffer;
}

void initialize_parser()
{
    std::filesystem::path home_directory;

#ifdef _WIN32
    const char *home_dir = std::getenv("USERPROFILE");
#else
    const char *home_dir = std::getenv("HOME");
#endif

    if (home_dir)
        home_directory = home_dir;

    std::cout << "Home directory: " << home_directory << std::endl;

    tlvcpp::set_tag_parser(parser);
}

void remove_invalid(std::string &string)
{
    auto predicate = [](char c)
    {
        return std::isspace(c) || !std::isxdigit(c);
    };

    string.erase(std::remove_if(string.begin(), string.end(), predicate), string.end());
}

void hex_to_bin(std::string &string)
{
    auto to_nibble = [](char c)
    {
        if (c >= '0' && c <= '9')
            return c - '0';

        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;

        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;

        return 0;
    };

    size_t read_index = 0;
    size_t write_index = 0;
    uint8_t high, low;

    if (string.size() % 2)
    {
        high = 0;
        low = to_nibble(string[read_index++]);

        string[write_index++] = high << 4 | low;
    }

    while (read_index < string.size())
    {
        high = to_nibble(string[read_index++]);
        low = to_nibble(string[read_index++]);

        string[write_index++] = high << 4 | low;
    }

    string.resize(write_index);
}

bool parse(std::string &string)
{
    remove_invalid(string);
    hex_to_bin(string);

    if (string.size() == 0)
        return true;

    tlvcpp::tlv_tree_node root;

    if (root.deserialize(reinterpret_cast<uint8_t *>(string.data()), string.size()))
    {
        root.dump();

        return true;
    }

    return false;
}

int main(int argc, char *argv[])
{
    initialize_parser();

    if (argc > 1)
    {
        for (size_t i = 1; i < argc; i++)
        {
            std::string input{argv[i]};

            if (!parse(input))
                return -1;
        }

        return 0;
    }
    else
    {
        std::string input(std::istreambuf_iterator<char>(std::cin), {});

        if (!parse(input))
            return -1;
    }

    return 0;
}
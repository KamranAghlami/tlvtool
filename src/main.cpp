#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

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
#ifdef _WIN32
    const char *home_dir = std::getenv("USERPROFILE");
#else
    const char *home_dir = std::getenv("HOME");
#endif

    if (!home_dir)
        return;

    auto file_path = std::filesystem::path(home_dir) / ".tlvtool";

    if (!std::filesystem::exists(file_path))
        return;

    std::ifstream file(file_path, std::ios::binary);

    if (!file)
        return;

    std::vector<uint8_t> buffer(std::filesystem::file_size(file_path));

    if (!buffer.size() || !file.read(reinterpret_cast<char *>(buffer.data()), buffer.size()))
        return;

    file.close();

    tlvcpp::tlv_tree_node root;

    if (root.deserialize(buffer.data(), buffer.size()))
    {
        auto emplace = [](const tlvcpp::tlv_tree_node &node)
        {
            tags.emplace(node.data().tag(), std::string(reinterpret_cast<char *>(node.data().value()), node.data().length()));
        };

        if (root.data().tag())
            emplace(root);
        else
            for (const auto &child : root.children())
                emplace(child);
    }

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

bool parse(std::string &string, bool ascii = true)
{
    if (ascii)
    {
        remove_invalid(string);
        hex_to_bin(string);
    }

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

            if (std::filesystem::exists(input))
            {
                std::ifstream file(input, std::ios::binary);

                if (!file)
                    return -1;

                input = std::string(std::istreambuf_iterator<char>(file), {});

                if (!parse(input, false))
                    return -1;
            }
            else if (!parse(input))
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
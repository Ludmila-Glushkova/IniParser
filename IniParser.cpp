#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream> 
#include <set> 
#include <string>
#include <unordered_map>
#include <list>

std::string add_to_str(std::unordered_map<std::string, std::string> const& m) {
    std::ostringstream out;
    for (auto const& mp : m) out << mp.first << " ";
    return out.str();
}

struct Section {
    std::string name;
    std::unordered_map< std::string, std::string > var;
};

std::string& del_comment(std::string& str) {
    size_t comment = str.find(";");
    if (std::string::npos != comment) str = str.substr(0, comment);
    return str;
}

std::string& del_spaces(std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n\v\f");
    if (std::string::npos != start) str = str.substr(start);
    return str;
}

std::string& del_last_spaces(std::string& str) {
    size_t end = str.find_last_not_of(" \t\r\n\v\f");
    if (std::string::npos != end) str = str.substr(0, end + 1);
    return str;
}

class Parser {
public:
    Parser(const std::string& file) {
        parse(file);
    }

    template <class T>
    const T get_value(const std::string& section, const std::string& var) {
        auto res = key_val(section, var);
        return res;
    }
    template <>
    const int get_value(const std::string& section, const std::string& var) {
        int res = std::stoi(key_val(section, var));
        return res;
    }
    template <>
    const double get_value(const std::string& section, const std::string& var) {
        double res = std::stod(key_val(section, var));
        return res;
    }

private:
    std::list<Section> list_sections;
    std::set<std::string> names_sections;

    void section_repet(const std::string section_name, std::string var_name, std::string var_val) {
        Section* section = get_section(section_name);
        section->var[var_name] = var_val;
    }

    Section* get_section(const std::string& section) {
        std::list<Section>::iterator search = std::find_if
        (
            list_sections.begin(), list_sections.end(), [section](const Section& s) {
                return s.name.compare(section) == 0;
            }
        );
        return search != list_sections.end() ? &*search : nullptr;
    }

    void parse(const std::string& file) {
        Section cur_sec;
        std::ifstream fin;
        bool key = false;
        int count_str = 0;
        fin.open(file);
        if (!fin)
            throw std::invalid_argument("Error! " + file + " could not be opened");

        for (std::string str; std::getline(fin, str);) {
            count_str++;
            del_spaces(str);
            del_comment(str);

            if (str[0] == '[') {
                size_t end = str.find_first_of(']');
                if (end != std::string::npos) {
                    if (!cur_sec.name.empty()) {
                        list_sections.push_back(cur_sec);
                        cur_sec.name.clear();
                        cur_sec.var.clear();
                    }
                    cur_sec.name = str.substr(1, end - 1);
                    if (names_sections.count(cur_sec.name)) key = true;
                    else names_sections.insert(cur_sec.name);
                }
                else throw std::invalid_argument("Error in the line " + std::to_string(count_str));
            }
            if (!str.empty()) {
                if (!key == true) {
                    size_t end = str.find_first_of("=");
                    if (end != std::string::npos) {
                        std::string name = str.substr(0, end);
                        std::string vl = str.substr(end + 1);
                        del_spaces(del_last_spaces(name));
                        del_spaces(del_last_spaces(vl));
                        size_t pos = str.find_first_of("=");
                        if (vl != "") cur_sec.var[name] = vl;
                        else cur_sec.var[name] = " ";
                    }
                }
                else {
                    size_t end = str.find_first_of("=");
                    if (end != std::string::npos) {
                        std::string name = str.substr(0, end);
                        std::string vl = str.substr(end + 1);
                        get_section(cur_sec.name);
                        section_repet(cur_sec.name, name, vl);
                    }
                }
            }
        }
        if (!cur_sec.name.empty()) {
            list_sections.push_back(cur_sec);
            cur_sec.name = "";
            cur_sec.var.clear();
        }
    }

    const std::string key_val(const std::string& section_name, const std::string& var_name) {
        Section* section = get_section(section_name);
        if (section != nullptr) {
            const auto it = section->var.find(var_name);
            if (it != section->var.end()) {
                if (it->second != " ") return it->second;
                else {
                    throw std::invalid_argument(var_name + " in " + section_name + " has no value");
                }
            }
            else {
                throw std::invalid_argument(section_name + " has variables: " + add_to_str(section->var));
            }
        }
        throw std::invalid_argument("Section not found");
    }
};

int main() {

    try {
        Parser parser("fin.ini");
        std::string section = "Section1";
        std::string var = "var1";
        auto val = parser.get_value<double>(section, var);
        std::cout << section << ": " << var << " = " << val << '\n';
    }
    catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
    return 0;

}
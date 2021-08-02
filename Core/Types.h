#pragma once

namespace TinySpark::Types {

    enum Types { NUMERIC = 0, STRING = 1 };

    bool to_bool(const std::string& s) { return s != "false"; }

    std::string to_string(Types t) { return t == Types::NUMERIC ? "float" : "const std::string&"; }
}
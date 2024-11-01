#include "ast.hpp"
#include "parser.tab.hh"

void yy::parser::error(const std::string& msg) {
    std::cout << "An error occured: " << msg << std::endl;
}

extern std::vector<definition_ptr> program;

int main() {
    yy::parser parser;
    parser.parse();

    std::cout << "Success. " << program.size() << " definitions.\n";
    for (auto const& definition: program)
        definition->display(0);
}
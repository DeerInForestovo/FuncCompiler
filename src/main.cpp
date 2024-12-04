#include "ast.hpp"
#include "parser.tab.hh"

extern std::vector<definition_ptr> program;
extern int lst_line_id;

bool flag = false;

void yy::parser::error(const std::string& msg) {
    flag = true;
    std::cout << "An error occured at line " << lst_line_id << ": " << msg << std::endl;
}

int main() {
    yy::parser parser;
    parser.parse();
    if (flag) {
        return 0;
    }
    std::cout << program.size() << " definitions.\n";
    for (auto const& definition: program)
        definition->display(0);
}
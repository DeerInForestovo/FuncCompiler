#include "ast.hpp"
#include <iostream>

/* 
 * Display
*/

void display_tabs(int tabs) {
    while (tabs--) std::cout << '\t';
}

void display_structure (const std::string &n) {  // white background + black characters
    std::cout << "\033[30;47m" << n << "\033[0m" << "";
}

void display_function_name (const std::string &n) {  // red characters
    std::cout << "\033[1;31m" << n << "\033[0m";
}

void branch::display(int tabs) const {
    display_tabs(tabs);
    display_structure("BRANCH: ");
    pat->display();
    std::cout << std::endl;

    display_tabs(tabs + 1);
    display_structure("EXPR: ");
    std::cout << std::endl;
    expr->display(tabs + 1);
    std::cout << std::endl;
}

void constructor::display(int tabs) const {
    display_tabs(tabs);
    display_structure("CONSTRUCTOR: ");
    display_function_name(name);
    for (std::string type: types)
        std::cout << ' ' << type;
    std::cout << std::endl;
}

void ast_int::display(int tabs) const {
    display_tabs(tabs);
    std::cout << value;
}

void ast_lid::display(int tabs) const {
    display_tabs(tabs);
    std::cout << id;
}

void ast_uid::display(int tabs) const {
    display_tabs(tabs);
    std::cout << id;
}

void ast_binop::display(int tabs) const {
    display_tabs(tabs);
    display_structure("BINOP");
    std::cout << std::endl;

    display_tabs(tabs + 1);
    display_structure("BINOP-LEFT: ");
    std::cout << std::endl;
    
    left->display(tabs + 1);
    std::cout << std::endl;
    
    display_tabs(tabs + 1);
    display_structure("BINOP-RIGHT: ");
    std::cout << std::endl;

    right->display(tabs + 1);
    std::cout << std::endl;
}

void ast_app::display(int tabs) const {
    display_tabs(tabs);
    left->display(0);
    std::cout << ' ';
    right->display(0);
}

void ast_case::display(int tabs) const {
    display_tabs(tabs);
    display_structure("CASE");
    std::cout << " " << (int)branches.size() << " branches" << std::endl;

    display_tabs(tabs + 1);
    display_structure("OF: ");
    std::cout << std::endl;

    of->display(tabs + 1);
    std::cout << std::endl;
    
    for (const auto& branch: branches)
        branch->display(tabs + 1);
}

void pattern_var::display() const {
    display_structure("PATTERN_VAR: ");
    std::cout << var;
}

void pattern_constr::display() const {
    display_structure("PATTERN_CONSTR: ");
    display_function_name(constr);
    for (const auto& param: params)
        std::cout << ' ' << param;
}

void definition_defn::display(int tabs) const {
    display_tabs(tabs);
    display_structure("DEFN: ");
    display_function_name(name);
    for (const auto& param: params)
        std::cout << ' ' << param;
    std::cout << std::endl;

    body->display(tabs + 1);
    std::cout << std::endl;
}

void definition_data::display(int tabs) const {
    display_tabs(tabs);
    display_structure("DATA: ");
    std::cout << name << std::endl;

    for (const auto& constructor: constructors)
        constructor->display(tabs + 1);
}
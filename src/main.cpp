#include "ast.hpp"
#include "parser.tab.hh"

extern std::vector<definition_ptr> program;
extern int lst_line_id;

bool flag = false;

void yy::parser::error(const std::string& msg) {
    flag = true;
    std::cout << "An error occured at line " << lst_line_id << ": " << msg << std::endl;
}

void typecheck_program(const std::vector<definition_ptr>& prog) {
    type_mgr mgr;
    type_env env;

    type_ptr int_type = type_ptr(new type_base("Int")); 
    type_ptr float_type = type_ptr(new type_base("Float")); 
    type_ptr bool_type = type_ptr(new type_base("Bool")); 
    type_ptr string_type = type_ptr(new type_base("String")); 

    type_ptr binop_type = type_ptr(new type_arr(
                int_type,
                type_ptr(new type_arr(int_type, int_type))));

    env.bind("+", binop_type);
    env.bind("-", binop_type);
    env.bind("*", binop_type);
    env.bind("/", binop_type);

    for(auto& def : prog) {
        def->typecheck_first(mgr, env);
    }

    for(auto& def : prog) {
        def->typecheck_second(mgr, env);
    }
}

int main() {
    yy::parser parser;
    parser.parse();
    typecheck_program(program);
    if (flag) {
        return 0;
    }
    std::cout << program.size() << " definitions.\n";
    // for (auto const& definition: program)
    //     definition->display(0);
}
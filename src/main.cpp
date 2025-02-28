#include "ast.hpp"
#include <iostream>
#include <fstream>
#include "binop.hpp"
#include "definition.hpp"
#include "graph.hpp"
#include "instruction.hpp"
#include "llvm_context.hpp"
#include "parser.hpp"
#include "error.hpp"
#include "type.hpp"
#include "prelude.hpp"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"

extern int lexer_error_cnt;
extern int parser_error_cnt;
extern int yylineno;
int uncovered_parser_error_cnt;
void yy::parser::error(const std::string& msg) {
    std::cerr << "Parser error at line " << yylineno << ": " << msg << std::endl;
    ++uncovered_parser_error_cnt;
}

extern std::map<std::string, definition_data_ptr> defs_data;
extern std::map<std::string, definition_defn_ptr> defs_defn;

void typecheck_program(
        std::map<std::string, definition_data_ptr>& defs_data,
        const std::map<std::string, definition_defn_ptr>& defs_defn,
        type_mgr& mgr, type_env_ptr& env) {
    /*
        Insert types
    */
    // basic types
    type_var* num_type_var = new type_var("Num");
    num_type_var->set_num_type();
    type_ptr num_type = type_ptr(num_type_var);
    type_ptr num_type_app = type_ptr(new type_app(num_type));

    type_ptr int_type = type_ptr(new type_base("Int"));
    type_ptr int_type_app = type_ptr(new type_app(int_type));
    env->bind_type("Int", int_type);

    type_ptr float_type = type_ptr(new type_base("Float"));
    type_ptr float_type_app = type_ptr(new type_app(float_type));
    env->bind_type("Float", float_type);

    type_ptr char_type = type_ptr(new type_base("Char"));
    type_ptr char_type_app = type_ptr(new type_app(char_type));
    env->bind_type("Char", char_type);

    // data types
    if (defs_data.find("Bool") != defs_data.end())
        throw type_error("User self-defined Bool type.");
    constructor_ptr false_constructor = constructor_ptr(new constructor("False", std::vector<parsed_type_ptr>()));
    constructor_ptr true_constructor = constructor_ptr(new constructor("True", std::vector<parsed_type_ptr>()));
    std::vector<constructor_ptr> bool_constructors;
    bool_constructors.push_back(std::move(false_constructor));
    bool_constructors.push_back(std::move(true_constructor));
    definition_data_ptr bool_type = definition_data_ptr(new 
            definition_data("Bool", std::vector<std::string>(), std::move(bool_constructors)));
    defs_data["Bool"] = std::move(bool_type);
    type_data* bool_type_data = new type_data("Bool");
    bool_type_data->constructors["False"] = {0};
    bool_type_data->constructors["True"] = {1};
    type_ptr bool_type_app = type_ptr(new type_app(type_ptr(bool_type_data)));

    if (defs_data.find("List") != defs_data.end())
        throw type_error("User self-defined List type.");

    constructor_ptr nil_constructor = constructor_ptr(new constructor("_Nil", std::vector<parsed_type_ptr>()));

    parsed_type_ptr list_arg_parsed_type = parsed_type_ptr(new parsed_type_var("ListArg"));
    parsed_type_ptr list_arg_cons_parsed_type = parsed_type_ptr(new parsed_type_var("ListArg"));

    std::vector<parsed_type_ptr> param_cons_list;
    param_cons_list.push_back(std::move(list_arg_cons_parsed_type));
    parsed_type_ptr list_parsed_type = parsed_type_ptr(new parsed_type_app("List", std::move(param_cons_list)));

    std::vector<parsed_type_ptr> param_cons_constructor;
    param_cons_constructor.push_back(std::move(list_arg_parsed_type));
    param_cons_constructor.push_back(std::move(list_parsed_type));
    constructor_ptr cons_constructor = constructor_ptr(new constructor("_Cons", std::move(param_cons_constructor)));

    std::vector<constructor_ptr> list_constructors;
    list_constructors.push_back(std::move(nil_constructor));
    list_constructors.push_back(std::move(cons_constructor));
    definition_data_ptr list_type = definition_data_ptr(
            new definition_data(
                    "List", 
                    std::vector<std::string>{"ListArg"}, 
                    std::move(list_constructors)
            )
    );

    defs_data["List"] = std::move(list_type);

    // add empty
    definition_data_ptr empty_type = definition_data_ptr(
            new definition_data("Empty", std::vector<std::string>(), std::vector<constructor_ptr>()));
    empty_type->insert_types(env);
    type_ptr empty_type_app = type_ptr(new type_app(type_ptr(new type_data("Empty"))));

    // add IO
    definition_data_ptr io_type = definition_data_ptr(
            new definition_data("IO", std::vector<std::string>{"IOArg"}, std::vector<constructor_ptr>()));
    io_type->insert_types(env);

    type_ptr io_arg_type = type_ptr(new type_var("IOArg"));
    type_app *io_app = new type_app(type_ptr(env->lookup_type("IO")));
    type_ptr io_type_app = type_ptr(io_app);
    io_app->arguments.push_back(io_arg_type);

    type_scheme_ptr io_scheme_ptr(new type_scheme(io_type_app));
    io_scheme_ptr->forall.emplace_back("IOArg", false);
    env->bind("_IOSimpleCons", io_scheme_ptr);

    type_ptr io_bind_app(new type_arr(io_arg_type, io_type_app));
    type_scheme_ptr io_bind_scheme_ptr(new type_scheme(io_bind_app));
    io_bind_scheme_ptr->forall.emplace_back("IOArg", false);
    env->bind("_IOBindCons", io_bind_scheme_ptr);

    // add array
    definition_data_ptr array_type = definition_data_ptr(
            new definition_data("Array", std::vector<std::string>{"ArrayArg"}, std::vector<constructor_ptr>()));
    array_type->insert_types(env);

    type_ptr array_arg_type = type_ptr(new type_var("ArrayArg"));

    type_app *array_app = new type_app(type_ptr(env->lookup_type("Array")));
    type_ptr array_type_ptr = type_ptr(array_app);
    array_app->arguments.push_back(array_arg_type);

    // insert all data definitions
    for(auto& def_data : defs_data) {
        def_data.second->insert_types(env);
    }
    for(auto& def_data : defs_data) {
        def_data.second->insert_constructors();
    }

    type_ptr list_arg_type = type_ptr(new type_var("ListArg"));
    type_app *list_app = new type_app(type_ptr(env->lookup_type("List")));
    list_app->arguments.emplace_back(list_arg_type);
    type_ptr list_type_app = type_ptr(list_app);

    type_ptr list_bind_app(new type_arr(list_arg_type, list_type_app));
    type_scheme_ptr list_bind_scheme_ptr(new type_scheme(list_bind_app));
    list_bind_scheme_ptr->forall.emplace_back("ListArg", false);

    /*
        Bind op types
    */
    type_ptr num_op_type = type_ptr(new type_arr(num_type_app, type_ptr(
            new type_arr(num_type_app, num_type_app))));
    type_scheme_ptr num_op_type_ptr = type_scheme_ptr(new type_scheme(std::move(num_op_type)));
    num_op_type_ptr->forall.emplace_back("Num", true);
    env->bind("+", num_op_type_ptr);
    env->bind("-", num_op_type_ptr);
    env->bind("*", num_op_type_ptr);
    env->bind("/", num_op_type_ptr);

    type_ptr num_cmp_type = type_ptr(new type_arr(num_type_app, type_ptr(
            new type_arr(num_type_app, bool_type_app))));
    type_scheme_ptr num_cmp_type_ptr = type_scheme_ptr(new type_scheme(std::move(num_cmp_type)));
    num_cmp_type_ptr->forall.emplace_back("Num", true);
    env->bind("<", num_cmp_type_ptr);
    env->bind(">", num_cmp_type_ptr);
    env->bind("<=", num_cmp_type_ptr);
    env->bind(">=", num_cmp_type_ptr);
    env->bind("==", num_cmp_type_ptr);
    env->bind("!=", num_cmp_type_ptr);

    type_ptr int_op_type = type_ptr(new type_arr(int_type_app, type_ptr(
            new type_arr(int_type_app, int_type_app))));
    env->bind("%", int_op_type);
    env->bind("|", int_op_type);
    env->bind("&", int_op_type);
    env->bind("^", int_op_type);
    env->bind("<<", int_op_type);
    env->bind(">>", int_op_type);

    type_ptr bool_op_type = type_ptr(new type_arr(bool_type_app, type_ptr(
            new type_arr(bool_type_app, bool_type_app))));
    env->bind("||", bool_op_type);
    env->bind("&&", bool_op_type);

    type_ptr num_uniop_type = type_ptr(new type_arr(num_type_app, num_type_app));
    type_scheme_ptr num_uniop_type_ptr = type_scheme_ptr(new type_scheme(std::move(num_uniop_type)));
    num_uniop_type_ptr->forall.emplace_back("Num", true);
    env->bind("--", num_uniop_type_ptr);  // This op is negate

    type_ptr int_uniop_type = type_ptr(new type_arr(int_type_app, int_type_app));
    env->bind("~", int_uniop_type);
    
    type_ptr bool_uniop_type = type_ptr(new type_arr(bool_type_app, bool_type_app));
    env->bind("!", bool_uniop_type);

    type_ptr conn_type = type_ptr(new type_arr(list_type_app, type_ptr(
            new type_arr(list_type_app, list_type_app))));
    type_scheme* conn_type_scheme = new type_scheme(std::move(conn_type));
    conn_type_scheme->forall.emplace_back("ListArg", false);
    type_scheme_ptr conn_type_ptr = type_scheme_ptr(conn_type_scheme);
    env->bind("++", conn_type_ptr);

    /*
        Bind prelude functions
    */
    std::set<std::string> prelude_func;

    // read
    type_ptr string_type = mgr.new_type();
    mgr.unify(type_ptr(new type_arr(char_type_app, string_type)), list_bind_scheme_ptr->instantiate(mgr));

    type_ptr read_type = mgr.new_type();
    mgr.unify(type_ptr(new type_arr(string_type, read_type)), io_bind_scheme_ptr->instantiate(mgr));
    env->bind("read", read_type);
    prelude_func.insert("read");

    // print
    type_ptr io_empty_type = mgr.new_type();
    mgr.unify(type_ptr(new type_arr(empty_type_app, io_empty_type)), io_bind_scheme_ptr->instantiate(mgr));
    type_ptr print_type(new type_arr(string_type, io_empty_type));
    env->bind("print", print_type);
    prelude_func.insert("print");

    // charToNum
    type_ptr charToNum_type = type_ptr(new type_arr(char_type_app, num_type_app));
    type_scheme_ptr charToNum_type_ptr = type_scheme_ptr(new type_scheme(std::move(charToNum_type)));
    charToNum_type_ptr->forall.emplace_back("Num", true);
    env->bind("charToNum", charToNum_type_ptr);
    prelude_func.insert("charToNum");

    // numToChar
    type_ptr numToChar_type = type_ptr(new type_arr(num_type_app, char_type_app));
    type_scheme_ptr numToChar_type_ptr = type_scheme_ptr(new type_scheme(std::move(numToChar_type)));
    numToChar_type_ptr->forall.emplace_back("Num", true);
    env->bind("numToChar", numToChar_type_ptr);
    prelude_func.insert("numToChar");

    // floatToNum
    type_ptr floatToNum_type = type_ptr(new type_arr(float_type_app, num_type_app));
    type_scheme_ptr floatToNum_type_ptr = type_scheme_ptr(new type_scheme(std::move(floatToNum_type)));
    floatToNum_type_ptr->forall.emplace_back("Num", true);
    env->bind("floatToNum", floatToNum_type_ptr);
    prelude_func.insert("floatToNum");

    // intToFloat
    type_ptr intToFloat_type = type_ptr(new type_arr(int_type_app, float_type_app));
    type_scheme_ptr intToFloat_type_ptr = type_scheme_ptr(new type_scheme(std::move(intToFloat_type)));
    intToFloat_type_ptr->forall.emplace_back("Num", true);
    env->bind("intToFloat", intToFloat_type_ptr);
    prelude_func.insert("intToFloat");

    // array
    type_app *newarray_list_app = new type_app(type_ptr(env->lookup_type("List")));
    type_ptr newarray_list_type = type_ptr(newarray_list_app);
    newarray_list_app->arguments.push_back(array_arg_type);

    type_ptr newarray_type = type_ptr(new type_arr(newarray_list_type, array_type_ptr));
    type_scheme_ptr newarray_type_ptr = type_scheme_ptr(new type_scheme(std::move(newarray_type)));
    newarray_type_ptr->forall.emplace_back("ArrayArg", false);
    env->bind("array", newarray_type_ptr);
    prelude_func.insert("array");

    // size
    type_ptr size_type = type_ptr(new type_arr(array_type_ptr, int_type_app));
    type_scheme_ptr size_type_ptr = type_scheme_ptr(new type_scheme(std::move(size_type)));
    size_type_ptr->forall.emplace_back("ArrayArg", false);
    env->bind("size", size_type_ptr);
    prelude_func.insert("size");

    // access
    type_ptr access_right = type_ptr(new type_arr(int_type_app, array_arg_type));
    type_ptr access_type = type_ptr(new type_arr(array_type_ptr, access_right));
    type_scheme_ptr access_type_ptr = type_scheme_ptr(new type_scheme(std::move(access_type)));
    access_type_ptr->forall.emplace_back("ArrayArg", false);
    env->bind("access", access_type_ptr);
    prelude_func.insert("access");

    // modify
    type_app *io_array_type_app = new type_app(type_ptr(env->lookup_type("IO")));
    type_ptr io_array_type = type_ptr(io_array_type_app);
    io_array_type_app->arguments.push_back(array_type_ptr);
    type_ptr modify_right1 = type_ptr(new type_arr(array_arg_type, io_array_type));
    type_ptr modify_right2 = type_ptr(new type_arr(int_type_app, modify_right1));
    type_ptr modify_type = type_ptr(new type_arr(array_type_ptr, modify_right2));
    type_scheme_ptr modify_type_ptr = type_scheme_ptr(new type_scheme(std::move(modify_type)));
    modify_type_ptr->forall.emplace_back("ArrayArg", false);
    env->bind("modify", modify_type_ptr);
    prelude_func.insert("modify");

    function_graph dependency_graph;

    for(auto& def_defn : defs_defn) {
        def_defn.second->find_free(mgr, env);
        dependency_graph.add_function(def_defn.second->name);

        for(auto& dependency : def_defn.second->free_variables) {
            if(defs_defn.find(dependency) == defs_defn.end()) {
                if (prelude_func.find(dependency) == prelude_func.end()) {
                    throw type_error("defs_defn cannot find dependency: " + dependency);
                } else {
                    continue;
                }
            }
            dependency_graph.add_edge(def_defn.second->name, dependency);
        }
    }

    std::vector<group_ptr> groups = dependency_graph.compute_order();
    for(auto it = groups.rbegin(); it != groups.rend(); it++) {
        auto& group = *it;
        for(auto& def_defnn_name : group->members) {
            auto& def_defn = defs_defn.find(def_defnn_name)->second;
            def_defn->insert_types(mgr);
        }
        for(auto& def_defnn_name : group->members) {
            auto& def_defn = defs_defn.find(def_defnn_name)->second;
            def_defn->typecheck(mgr);
        }
        for(auto& def_defnn_name : group->members) {
            env->generalize(def_defnn_name, mgr);
        }
    }
}

void compile_program(const std::map<std::string, definition_defn_ptr>& defs_defn) {
    for(auto& def_defn : defs_defn) {
        def_defn.second->compile();
    }
}

void gen_llvm_internal_binop(llvm_context& ctx, binop op) {
    auto new_function = ctx.create_custom_function(binop_action(op), 2);
    std::vector<instruction_ptr> instructions;
    instructions.push_back(instruction_ptr(new instruction_push(1)));
    if (op != CONN) {
        instructions.push_back(instruction_ptr(new instruction_eval()));
    }
    instructions.push_back(instruction_ptr(new instruction_push(1)));
    instructions.push_back(instruction_ptr(new instruction_eval()));
    instructions.push_back(instruction_ptr(new instruction_binop(op)));
    instructions.push_back(instruction_ptr(new instruction_update(2)));
    instructions.push_back(instruction_ptr(new instruction_pop(2)));
    ctx.builder.SetInsertPoint(&new_function->getEntryBlock());
    for(auto& instruction : instructions) {
        instruction->gen_llvm(ctx, new_function);
    }
    ctx.builder.CreateRetVoid();
}

void gen_llvm_internal_uniop(llvm_context& ctx, uniop op) {
    auto new_function = ctx.create_custom_function(uniop_action(op), 1);
    std::vector<instruction_ptr> instructions;
    instructions.push_back(instruction_ptr(new instruction_push(0)));
    instructions.push_back(instruction_ptr(new instruction_eval()));
    instructions.push_back(instruction_ptr(new instruction_uniop(op)));
    instructions.push_back(instruction_ptr(new instruction_update(1)));
    instructions.push_back(instruction_ptr(new instruction_pop(1)));
    ctx.builder.SetInsertPoint(&new_function->getEntryBlock());
    for(auto& instruction : instructions) {
        instruction->gen_llvm(ctx, new_function);
    }
    ctx.builder.CreateRetVoid();
}

void output_llvm(llvm_context& ctx, const std::string& filename) {
    std::string targetTriple = llvm::sys::getDefaultTargetTriple();

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    std::string error;
    const llvm::Target* target =
        llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        std::cerr << error << std::endl;
    } else {
        std::string cpu = "generic";
        std::string features = "";
        llvm::TargetOptions options;
        llvm::TargetMachine* targetMachine =
            target->createTargetMachine(targetTriple, cpu, features,
                    options, llvm::Optional<llvm::Reloc::Model>());

        ctx.module.setDataLayout(targetMachine->createDataLayout());
        ctx.module.setTargetTriple(targetTriple);

        std::error_code ec;
        llvm::raw_fd_ostream file(filename, ec, llvm::sys::fs::F_None);
        if (ec) {
            throw unexpected_error("LLVM error 1.");
        } else {
            llvm::CodeGenFileType type = llvm::CGFT_ObjectFile;
            llvm::legacy::PassManager pm;
            if (targetMachine->addPassesToEmitFile(pm, file, NULL, type)) {
                throw unexpected_error("LLVM error 2.");
            } else {
                pm.run(ctx.module);
                file.close();
            }
        }
    }
}

void gen_llvm(
        const std::map<std::string, definition_data_ptr>& defs_data,
        const std::map<std::string, definition_defn_ptr>& defs_defn) {
    llvm_context ctx;

    gen_llvm_internal_binop(ctx, PLUS);
    gen_llvm_internal_binop(ctx, MINUS);
    gen_llvm_internal_binop(ctx, TIMES);
    gen_llvm_internal_binop(ctx, DIVIDE);
    gen_llvm_internal_binop(ctx, BMOD);
    gen_llvm_internal_binop(ctx, LMOVE);
    gen_llvm_internal_binop(ctx, RMOVE);
    gen_llvm_internal_binop(ctx, BITAND);
    gen_llvm_internal_binop(ctx, BITOR);
    gen_llvm_internal_binop(ctx, XOR);
    gen_llvm_internal_binop(ctx, LT);
    gen_llvm_internal_binop(ctx, GT);
    gen_llvm_internal_binop(ctx, LEQ);
    gen_llvm_internal_binop(ctx, GEQ);
    gen_llvm_internal_binop(ctx, EQ);
    gen_llvm_internal_binop(ctx, NEQ);
    gen_llvm_internal_binop(ctx, AND);
    gen_llvm_internal_binop(ctx, OR);
    
    gen_llvm_internal_uniop(ctx, NEGATE);
    gen_llvm_internal_uniop(ctx, NOT);
    gen_llvm_internal_uniop(ctx, BITNOT);

    for(auto& def_data : defs_data) {
        def_data.second->generate_llvm(ctx);
    }

    gen_llvm_internal_binop(ctx, CONN);

    generate_read_llvm(ctx);
    generate_print_llvm(ctx);

    generate_charToNum_llvm(ctx);
    generate_numToChar_llvm(ctx);
    generate_floatToNum_llvm(ctx);
    generate_intToFloat_llvm(ctx);

    generate_array_llvm(ctx);
    generate_size_llvm(ctx);
    generate_access_llvm(ctx);
    generate_modify_llvm(ctx);

    for(auto& def_defn : defs_defn) {
        def_defn.second->declare_llvm(ctx);
    }

    for(auto& def_defn : defs_defn) {
        def_defn.second->generate_llvm(ctx);
    }

    // ctx.module.print(log_file, nullptr);

    std::error_code EC;
    llvm::raw_fd_ostream file_stream("llvm_log.txt", EC, llvm::sys::fs::OF_None);
    if (!EC) ctx.module.print(file_stream, nullptr);

    output_llvm(ctx, "program.o");
}

int main() {
    yy::parser parser;
    type_mgr mgr;
    type_env_ptr env(new type_env);

    parser.parse();
    if (lexer_error_cnt || parser_error_cnt || uncovered_parser_error_cnt) {
        std::cout << "Parsing failed. (" <<
            lexer_error_cnt << " lexer error(s), " << 
            parser_error_cnt << " covered parser error(s), " <<
            uncovered_parser_error_cnt << " uncovered parser error(s).)" << std::endl;
        return 0;
    }

    std::ofstream log_file("log.txt", std::ios::out | std::ios::trunc);
    if (!log_file.is_open()) {
        std::cout << "Unable to open log.txt." << std::endl;
    }

    log_file << "[AST:]\n";
    for(auto& def_defn : defs_defn) {
        log_file << def_defn.second->name;
        for(auto& param : def_defn.second->params) log_file << " " << param;
        log_file << ":\n";
        def_defn.second->body->print(1, log_file);
    }

    try {
        typecheck_program(defs_data, defs_defn, mgr, env);
        log_file << "\n\n\n[Typecheck result:]\n";
        for(auto& pair : env->names) {
            char fi_letter = pair.first[0];
            log_file << pair.first << ": ";
            pair.second->print(mgr, log_file);
            log_file << "\n";
        }

        compile_program(defs_defn);
        log_file << "\n\n\n[Compile result:]\n";
        for(auto& def_defn : defs_defn) {
            log_file << def_defn.second->name << ":\n";
            for(auto& instruction : def_defn.second->instructions) {
                instruction->print(0, log_file);
            }
            log_file << "\n";
        }

        gen_llvm(defs_data, defs_defn);

        std::cout << "Compiled successfully." << std::endl;
    } catch(unification_error& err) {
        std::cout << std::endl;
        std::cout << "Failed to unify types: " << std::endl;
        std::cout << "  (1) \033[34m";
        err.left->print(mgr, std::cout);
        std::cout << "\033[0m" << std::endl;
        std::cout << "  (2) \033[32m";
        err.right->print(mgr, std::cout);
        std::cout << "\033[0m" << std::endl;
    } catch(type_error& err) {
        std::cout << std::endl;
        std::cout << "Failed to type check program: " << err.description << std::endl;
    } catch(unexpected_error& err) {
        std::cout << std::endl;
        std::cout << "Unexpected Error: " << err.description << std::endl;
    }

    if (log_file.is_open()) {
        log_file.close();
    }
}

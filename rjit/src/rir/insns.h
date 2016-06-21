//                 immediate pop push popi pushi
INSTRUCTION(invalid_, 0, 0, 0, 0, 0); // invalid operation
INSTRUCTION(push_, 1, 0, 1, 0, 0);    // push a constant to the stack
INSTRUCTION(ldfun_, 1, 0, 1, 0, 0);   // load function from env
INSTRUCTION(ldvar_, 1, 0, 1, 0, 0);   // load variable from env
INSTRUCTION(call_, 2, 1, 1, 0, 0);    // call fun
INSTRUCTION(promise_, 1, 0, 1, 0, 0); // create promise
INSTRUCTION(close_, 1, 1, 1, 0, 0);   // create closure
INSTRUCTION(ret_, 0, 1, 0, 0, 0);     // return
INSTRUCTION(force_, 0, 1, 1, 0, 0);   // eval promise
INSTRUCTION(pop_, 0, 1, 0, 0, 0);     // pop tos
INSTRUCTION(pusharg_, 1, 0, 1, 0, 0); // push argument to stack
INSTRUCTION(asast_, 0, 1, 1, 0, 0);   // pop promise, push ast
INSTRUCTION(stvar_, 0, 2, 1, 0, 0);   // ??
// INSTRUCTION(numargi_,   0,    0,    0,   0,   1) //  DELETED
INSTRUCTION(asbool_, 0, 1, 1, 0, 0);  // convert ?? on tos to bool
INSTRUCTION(condjmp_, 1, 1, 0, 0, 0); // jump … if true
INSTRUCTION(condfals_, 1, 1, 0, 0,
            0);                     // jump … if  FIXME -- push on the i stack
INSTRUCTION(jmp_, 1, 0, 0, 0, 0);   // unconditional jump
INSTRUCTION(lti_, 0, 0, 1, 2, 0);   // less than on ints
INSTRUCTION(eqi_, 0, 0, 1, 2, 0);   // equality on ints
INSTRUCTION(pushi_, 1, 0, 0, 0, 1); // push int
INSTRUCTION(dupi_, 0, 0, 0, 1, 2);  // dup int
INSTRUCTION(dup_, 0, 1, 2, 0, 0);   // dup tos
INSTRUCTION(add_, 0, 2, 1, 0, 0);   // +
INSTRUCTION(sub_, 0, 2, 1, 0, 0);   // -
INSTRUCTION(lt_, 0, 2, 1, 0, 0);    // <
INSTRUCTION(isspecial_, 1, 0, 0, 0, 0); // check tos is a special
INSTRUCTION(isfun_, 0, 1, 1, 0, 0);     // check tos is a function
INSTRUCTION(end_, 0, 0, 0, 0, 0);       //

/*
def_instr(invalid_,   0,    0,    0,   0,   0) // invalid operation
def_instr(push_,      1,    0,    1,   0,   0) // push a constant to the stack
def_instr(ldfun_,     1,    0,    1,   0,   0) // load function from env
def_instr(ldvar_,     1,    0,    1,   0,   0) // load variable from env
def_instr(call_,      2,    1,    1,   0,   0) // call fun,
def_instr(promise_,   1,    0,    1,   0,   0) // create promise
def_instr(close_,     1,    2,    1,   0,   0) // create closure
def_instr(ret_,       0,    1,    0,   0,   0) // return
def_instr(force_,     0,    1,    1,   0,   0) // eval promise
def_instr(pop_,       0,    1,    0,   0,   0) // pop tos
def_instr(pusharg_,   1,    0,    1,   0,   0) // push argument to stack
def_instr(asast_,     0,    1,    1,   0,   0) // pop promise, push ast
def_instr(stvar_,     0,    2,    1,   0,   0) //
//def_instr(numargi_,   0,    0,    0,   0,   1) //  DELETED
def_instr(asbool_,    0,    1,    1,   0,   0) // convert ?? on tos to bool
def_instr(condtrue_,  1,    1,    0,   0,   0) // jump … if
def_instr(condfals_,  1,    1,    0,   0,   0) // jump … if  FIXME -- push on
the i stack
def_instr(jmp_,       1,    0,    0,   0,   0) // unconditional jump
def_instr(lti_,       0,    0,    1,   2,   0) // less than on ints
def_instr(eqi_,       0,    0,    1,   2,   0) // equality on ints
def_instr(pushi_,     1,    0,    0,   0,   1) // push int
def_instr(dupi_,      0,    0,    0,   1,   2) // dup int
def_instr(dup_,       0,    1,    2,   0,   0) // dup tos
def_instr(add_,       0,    2,    1,   0,   0) // +
def_instr(sub_,       0,    2,    1,   0,   0) // -
def_instr(lt_,        0,    2,    1,   0,   0) // <
def_instr(isspecial_, 1,    0,    0,   0,   0) //
def_instr(isfun_,     0,    1,    1,   0,   0) // check tos is a function
def_instr(end_,       0,    0,   0,    0,   0)
*/

/*
force_all
jmp_true_
jmp_false_
num_of_
inci_
get_ast_
getfun_
check_function
check_special */
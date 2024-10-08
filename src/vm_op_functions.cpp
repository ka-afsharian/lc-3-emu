#include "lc3vm/vm.hpp"
#include <cstdio>
#include <typeinfo> //for debug
#include <cxxabi.h> //for debug
#include <memory> //for debug
using namespace lc3vm;
using namespace lc3vm::constants;

template<typename T>
std::string demangle(T s) {
  const char* name = typeid(s).name();
    int status = 0;
    std::unique_ptr<char, void(*)(void*)> res(
        __cxxabiv1::__cxa_demangle(name, nullptr, nullptr, &status), std::free);
    return (status == 0) ? res.get() : name;
}

int vm::add(const memory_type& instr){ //memory_type is uint16_t
    memory_type dr = (instr >> 9) & 0x7;
    //source1
    memory_type sr1 = (instr >> 6) & 0x7;
    memory_type imm = (instr >> 5) & 0x1;

    if(imm){
      memory_type imm5 = sign_ext(instr & 0x1F, 5);
      reg_[dr] = reg_[sr1] + imm5;
    }else {
      memory_type sr2 = instr & 0x7;
      reg_[dr] = reg_[sr1] + reg_[sr2];
    }

  return update_flags(dr); //add can only fail inside update_flags
                           //so it will reuturn -3 if update flag fail return 0 if success
}



int vm::AND(const memory_type& instr){
  memory_type dr = (instr >> 9) & 0x7;
  memory_type sr1 = (instr >> 6) & 0x7;
  memory_type imm = (instr >> 5) & 0x1;

  if(static_cast<bool>(imm)){
    memory_type imm5 = sign_ext(instr & 0x1F, 5);
    reg_[dr] = reg_[sr1] & imm5;
  }else {
    memory_type sr2 = instr & 0x7;
    reg_[dr] = reg_[sr1] & reg_[sr2];
  }

  return update_flags(dr);
}



int vm::br(const memory_type& instr){
  memory_type pcoffset9 = instr & 0x1FF;
  memory_type nzp = (instr >> 9) & 0x7;
  if(nzp & reg_[reg::cond]){
  reg_[reg::pc] =  reg_[reg::pc] + sign_ext(pcoffset9,9);
  }
  return 0;
}

int vm::jmp(const memory_type& instr){
  memory_type reg_loc = (instr >> 6) & 0x7;
  reg_[reg::pc] = reg_[reg_loc];
  return 0;
}


int vm::jsr(const memory_type& instr){
  reg_[reg::r7] = reg_[reg::pc];
  if((instr>>11)&0x1){
    reg_[reg::pc] += sign_ext(instr & 0x7FF,11);
  }else{reg_[reg::pc] = reg_[(instr >> 6) & 0x7];
  }
  return 0;
}


int vm::ld(const memory_type& instr){
  memory_type dr = (instr >> 9) & 0x7;
  reg_[dr] = mem_[reg_[reg::pc] + sign_ext(instr & 0x1FF,9)];
  return update_flags(dr);
}

int vm::ldi(const memory_type& instr){
  memory_type dr = (instr >> 9) & 0x7;
  reg_[dr] = mem_[mem_[reg_[reg::pc] + sign_ext(instr & 0x1FF,9)]];
  return update_flags(dr);

}

int vm::ldr(const memory_type& instr){
  memory_type dr = (instr >> 9) & 0x7;
  memory_type baser = (instr >> 6) & 0x7;
  reg_[dr] = mem_[reg_[baser] + sign_ext(instr & 0x3F,6)];
  return update_flags(dr);
}

int vm::lea(const memory_type& instr){
  memory_type dr = (instr >> 9) & 0x7;
  reg_[dr] = reg_[reg::pc] + sign_ext(instr & 0x1FF,9);
  return update_flags(dr);
}

int vm::NOT(const memory_type& instr){
  memory_type dr = (instr >> 9) & 0x7;
  memory_type sr = (instr >> 6) & 0x7;
  reg_[dr] = ~reg_[sr];
  return update_flags(dr);
}
/*
int vm::rti(const memory_type& instr){
  return 0;
}
*/
int vm::st(const memory_type& instr){
  memory_type sr = (instr >> 9) & 0x7;
  memory_type pcoffset9 = sign_ext(instr & 0x1FF,9);
  mem_[reg_[reg::pc]+ pcoffset9] = reg_[sr];
  return 0;
}

int vm::sti(const memory_type& instr){
  memory_type sr = (instr >> 9) & 0x7;
  memory_type pcoffset9 = sign_ext(instr & 0x1FF,9);
  mem_[mem_[reg_[reg::pc]+ pcoffset9]] = reg_[sr];
  return 0;
}


int vm::str(const memory_type& instr){
  memory_type sr = (instr >> 9) & 0x7;
  memory_type baser = (instr >> 6) & 0x7;
  mem_[reg_[baser] + sign_ext(instr & 0x3F,6)] = reg_[sr];
  return 0;

}

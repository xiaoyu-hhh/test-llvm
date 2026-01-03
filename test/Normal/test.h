//
// Created by ygl on 11/6/25.
//

#ifndef LLVM_TEST_H
#define LLVM_TEST_H

class A {
public:
  template <typename T> A(T a){
    return a;
  }
  int A_foo() {
    return 0;
  }
  int A_bar() {
    return 0;
  }
  int A_bar2() {
    return 0;
  }
};

#endif // LLVM_TEST_H

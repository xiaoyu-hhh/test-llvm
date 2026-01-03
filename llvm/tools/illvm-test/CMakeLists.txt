set(LLVM_LINK_COMPONENTS
  MC
  MCParser
  Support
  TargetParser
  ILLVMSupport
  ILLVMFuncVELF
)

add_llvm_tool(illvm-test
  main.cpp

  DEPENDS
  intrinsics_gen
)

target_link_libraries(illvm-test
  PRIVATE
  LLVMILLVMSupport
)
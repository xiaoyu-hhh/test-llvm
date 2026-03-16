#include "iclang/CC1Driver/ShareCC1Driver.h"
#include "iclang/ASTSupport/ASTGlobal.h"
#include "iclang/FuncX/RefSymbolAnalysis.h"
#include "illvm/Support/Diagnostics.h"
#include "illvm/Support/Interval.h"

namespace iclang {

void ShareMasterCC1Driver::run() {
  ILLVM_FCHECK(false, "We haven't implemented ShareMasterCC1Driver yet");
}

void ShareClientCC1Driver::run() {
  ILLVM_FCHECK(false, "We haven't implemented ShareClientCC1Driver yet");
}

void ShareCheckCC1Driver::run() {
  auto &global = Global::getInstance();

  assert(global.getIClangMode() == IClangMode::ShareCheckMode);

  auto &astGlobal = ASTGlobal::getInstance();
  auto &context = astGlobal.getContext();

  auto metaData = global.getMetaData<ShareCheckMetaData>();
  const auto astMetaData = astGlobal.getASTMetaData<ShareCheckASTMetaData>();

  if (!metaData->enableRefedSymbolAnalysisFlag) {
    return;
  }

  if (std::getenv("ShareCollection")) {
    funcx::TempFunc_And_Func_Analysis analysis(context.getSourceManager(),astGlobal);
    analysis.TraverseDecl(context.getTranslationUnitDecl());

    analysis.initMetaData();
    if (std::getenv("ibenchmark_log")) {
      analysis.writeMetaDataToLog();
    }
    else {
      analysis.dumpInfo();
      analysis.dumpMetaData();
    }
    return;
  }

  // All - refed = unRefed.
  // refedFuncDecls: canonical decls.
  // allFuncDecls: all decls.
  funcx::AllFuncDeclVisitor allFuncDeclVisitor(astGlobal);
  allFuncDeclVisitor.TraverseDecl(context.getTranslationUnitDecl());
  // llvm::errs() << "allFuncDecls: " << "\n";
  // llvm::errs() << " " << allFuncDeclVisitor.allFuncDecls.size() << "\n";
  // for (const auto *elem : allFuncDeclVisitor.allFuncDecls) {
  //   if (const auto *Decl = llvm::dyn_cast<clang::FunctionDecl>(elem)) {
  //     llvm::errs() << "FunctionDecl:" << astGlobal.dumpDecl(Decl) << "\n";
  //   }
  //   if (const auto *Decl = llvm::dyn_cast<clang::FunctionTemplateDecl>(elem)) {
  //     llvm::errs() << "FunctionTemplateDecl:" << astGlobal.dumpDecl(Decl) << "\n";
  //   }
  // }
  // llvm::errs() << "allFuncTempDecls: " << "\n";
  // llvm::errs() << " " << allFuncDeclVisitor.allFuncTempDecls.size() << "\n";
  // for (const auto *elem : allFuncDeclVisitor.allFuncTempDecls) {
  //   if (elem) {
  //     llvm::errs() << astGlobal.dumpDecl(elem) << "\n";
  //   }
  // }

  std::unordered_set<const clang::FunctionTemplateDecl *> hasInstantiationFuncTempDecls;
  for (auto *FTD:allFuncDeclVisitor.allFuncTempDecls) {
    bool hasInstantiation = false;
    for (auto *Spec : FTD->specializations()) {
      hasInstantiation = true;
      break;
    }
    if (hasInstantiation) {
      hasInstantiationFuncTempDecls.insert(FTD);
    }
  }



  funcx::AlwaysRefedMangledNamesAnalysis alwaysRefedMangledNamesAnalysis(context.getSourceManager(),astGlobal);
  alwaysRefedMangledNamesAnalysis.TraverseDecl(context.getTranslationUnitDecl());
  auto alwaysRefedFuncDeclsMangledNames = alwaysRefedMangledNamesAnalysis.alwaysRefedFuncDeclsMangledNames;
  // llvm::errs() << "alwaysRefedFuncDeclsMangledNames:" << "\n";
  // for (std::string name : alwaysRefedFuncDeclsMangledNames){
  //   llvm::errs() << "  " << name << "\n";
  // }

  // Init
  funcx::AlwaysRefedAnalysis alwaysRefedAnalysis(context.getSourceManager());
  alwaysRefedAnalysis.NameToFuncDecl = allFuncDeclVisitor.NameToFuncDecl;
  alwaysRefedAnalysis.NameToFuncTempDecl = allFuncDeclVisitor.NameToFuncTempDecl;
  alwaysRefedAnalysis.TraverseDecl(context.getTranslationUnitDecl());


  auto alwaysRefedFuncDecls = alwaysRefedAnalysis.alwaysRefedFuncDecls;
  // 保守处理，所有有MangledName的函数都是永远被引用的
  // for (std::string name : alwaysRefedFuncDeclsMangledNames){
  //   if (name == "" || name == " ") continue;
  //   for (const auto* funcDecl : allFuncDeclVisitor.MangledNameToFuncDecl[name]) {
  //     if (funcDecl) alwaysRefedFuncDecls.insert(funcDecl);
  //   }
  // }
  // 保守处理，有实例化节点的函数模板都视为被引用
  for (const auto *elem : hasInstantiationFuncTempDecls) {
    alwaysRefedFuncDecls.insert(elem->getTemplatedDecl()->getCanonicalDecl());
  }

  // llvm::errs() << "alwaysRefedFuncDecls:" << "\n";
  // for (const auto *elem : alwaysRefedFuncDecls) {
  //   if (elem)
  //     llvm::errs() << "  " << elem->getName().str() << "\n";
  // }

  for (const auto &elem : astMetaData->emitGlobalFuncDefs) {
    alwaysRefedFuncDecls.insert(elem);
  }

  // Propagation.
  funcx::RefSymbolAnalysis refSymbolAnalysis;
  refSymbolAnalysis.funcRefedVisitor.NameToFuncDecl = allFuncDeclVisitor.NameToFuncDecl;
  refSymbolAnalysis.funcRefedVisitor.NameToFuncTempDecl = allFuncDeclVisitor.NameToFuncTempDecl;
  auto refedFuncDecls = refSymbolAnalysis.propagation(alwaysRefedFuncDecls);

  // Handle templates.
  std::unordered_set<const clang::FunctionTemplateDecl *> refedTempDecls;
  for (const clang::FunctionDecl *elem : refedFuncDecls) {
    const auto *priTempDecl = elem->getPrimaryTemplate();
    if (priTempDecl != nullptr) {
      refedTempDecls.insert(priTempDecl->getCanonicalDecl());
    }
    // Note: desTempDecl only work for templated function.
  }

  // extern template 隐式实例化，保守处理
  refedTempDecls.insert(hasInstantiationFuncTempDecls.begin(), hasInstantiationFuncTempDecls.end());
  // UnresolvedTempDecls
  refedTempDecls.insert(alwaysRefedAnalysis.UnresolvedRefedFuncTempDecls.begin(),alwaysRefedAnalysis.UnresolvedRefedFuncTempDecls.end());
  refedTempDecls.insert(refSymbolAnalysis.funcRefedVisitor.UnresolvedRefedFuncTempDecls.begin(),refSymbolAnalysis.funcRefedVisitor.UnresolvedRefedFuncTempDecls.end());

  // name match
  // std::unordered_set<std::string> UnresolvedRefedFuncDeclsNames;
  // UnresolvedRefedFuncDeclsNames.insert(alwaysRefedAnalysis.UnresolvedRefedFuncDeclsNames.begin(),
  //   alwaysRefedAnalysis.UnresolvedRefedFuncDeclsNames.end());
  // UnresolvedRefedFuncDeclsNames.insert(refSymbolAnalysis.funcRefedVisitor.UnresolvedRefedFuncDeclsNames.begin(),
  // refSymbolAnalysis.funcRefedVisitor.UnresolvedRefedFuncDeclsNames.end());
  // for (auto name : UnresolvedRefedFuncDeclsNames) {
  //   if (name == "") continue;
  //   for (const auto *FuncTempDecl : allFuncDeclVisitor.allFuncTempDecls) {
  //     if (name == FuncTempDecl->getTemplatedDecl()->getName().str()) {
  //       refedTempDecls.insert(FuncTempDecl);
  //     }
  //   }
  // }


  for (const auto *elem : refedTempDecls) {
    const auto *templatedFuncDecl = elem->getTemplatedDecl();
    refedFuncDecls.insert(templatedFuncDecl);
  }


  // for (const auto *elem : refedFuncDecls) {
  //   if (auto *FTD = elem->getDescribedFunctionTemplate()) {
  //     refedTempDecls.insert(FTD);
  //   }
  // }

  std::unordered_set<const clang::FunctionDecl *> TempInstPattern;
  for (const auto *elem : refedFuncDecls) {
    if (const auto *Pattern = elem->getInstantiatedFromMemberFunction()) {
        // Pattern 就是实例化后的成员函数elem在类模板中对应原始未实例化的成员函数
        TempInstPattern.insert(Pattern);
    }
  }

  std::unordered_set<const clang::FunctionDecl *> CanonicalDecls;
  for (const auto *elem : refedFuncDecls) {
    if (const auto *CanonicalDecl= elem->getCanonicalDecl()) {
      CanonicalDecls.insert(CanonicalDecl);
    }
  }

  refedFuncDecls.insert(TempInstPattern.begin(), TempInstPattern.end());
  refedFuncDecls.insert(CanonicalDecls.begin(), CanonicalDecls.end());


  std::error_code EC;
  llvm::raw_fd_ostream outFile(
      "/home/ygl/iclang/llvm-project/test/refed.txt",
      EC);

  outFile << "refedFuncDecls:\n";

  for (const auto *elem : refedFuncDecls) {
    outFile << " " << astGlobal.dumpDecl(elem) << "\n";
  }

  std::error_code EC2;
  llvm::raw_fd_ostream outFile2(
    "/home/ygl/iclang/llvm-project/test/refedTemp.txt",
    EC2);

  outFile2 << "refedTempDecls:\n";

  for (const auto *elem : refedTempDecls) {
    outFile2 << " " << astGlobal.dumpDecl(elem) << "\n";
  }

  std::error_code EC3;
  llvm::raw_fd_ostream outFile3(
    "/home/ygl/iclang/llvm-project/test/allFuncTempDecls.txt",
    EC3);

  outFile3 << "allFuncTempDecls:\n";

  for (const auto *elem : allFuncDeclVisitor.allFuncTempDecls) {
    outFile3 << " " << astGlobal.dumpDecl(elem) << "\n";
  }

  std::error_code EC4;
  llvm::raw_fd_ostream outFile4(
  "/home/ygl/iclang/llvm-project/test/allFuncDecls.txt",
  EC4);

  outFile4 << "allFuncDecls:\n";

  for (const auto *elem : allFuncDeclVisitor.allFuncDecls) {
    outFile4 << " " << astGlobal.dumpDecl(elem) << "\n";
  }

  std::error_code EC6;
  llvm::raw_fd_ostream outFile6(
  "/home/ygl/iclang/llvm-project/test/debug.txt",
  EC6);

  // for (const auto* elem :refedFuncDecls) {
  //   outFile6 << astGlobal.dumpDecl(elem) << ":" << astGlobal.dumpDecl(elem->getCanonicalDecl()) << "\n";
  // }
  for (const auto *elem : allFuncDeclVisitor.allFuncDecls) {
    if (const auto* temp = elem->getDescribedFunctionTemplate()) {
      outFile6 << astGlobal.dumpDecl(elem) << ":" << astGlobal.dumpDecl(temp) << "\n";
    }
    else
      outFile6 << astGlobal.dumpDecl(elem) << ":" << "\n";
  }
  outFile6 << "=================\n";

  for (const auto *elem : refedFuncDecls) {
    if (const auto* temp = elem->getDescribedFunctionTemplate()) {
      outFile6 << astGlobal.dumpDecl(elem) << ":" << astGlobal.dumpDecl(temp) << "\n";
    }
    else
      outFile6 << astGlobal.dumpDecl(elem) << ":" << "\n";
  }
  outFile6 << "=================\n";

  for (const auto *elem : refedFuncDecls) {
    if (const auto* temp = elem->getPrimaryTemplate()) {
      outFile6 << astGlobal.dumpDecl(elem) << ":" << astGlobal.dumpDecl(temp) << "\n";
    }
    else
      outFile6 << astGlobal.dumpDecl(elem) << ":" << "\n";
  }




  // unRefed source ranges.
  illvm::IntervalManager intervalManager;

  std::map<std::pair<long long,long long>,std::string> intervalInfo;
  for (const auto *elem : allFuncDeclVisitor.allFuncDecls) {
    if (refedFuncDecls.find(elem->getCanonicalDecl()) == refedFuncDecls.end()) {
      intervalManager.addInterval(
          astGlobal.getDeclSourceInterval(elem).toInterval());
      intervalInfo.insert({astGlobal.getDeclSourceInterval(elem).toInterval(),astGlobal.dumpDecl(elem)});
    }
  }

  for (const auto *elem : allFuncDeclVisitor.allFuncTempDecls) {
    if (refedTempDecls.find(elem->getCanonicalDecl()) == refedTempDecls.end()) {
      intervalManager.addInterval(
          astGlobal.getDeclSourceInterval(elem).toInterval());
      intervalInfo.insert({astGlobal.getDeclSourceInterval(elem).toInterval(),astGlobal.dumpDecl(elem)});
    }
  }

  std::error_code EC5;
  llvm::raw_fd_ostream outFile5(
  "/home/ygl/iclang/llvm-project/test/intervalInfo.txt",
  EC5);

  outFile5 << "intervalInfo:\n";

  for (const auto &it : intervalInfo) {
    outFile5 << it.first.first << " "
             << it.first.second << " : "
             << it.second << "\n";
  }
  metaData->unRefedDeclIntervals =
      intervalManager.merge(intervalManager.getIntervals());
}


} // namespace iclang
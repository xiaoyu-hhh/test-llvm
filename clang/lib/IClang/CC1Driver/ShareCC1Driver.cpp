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
  //   if (elem) {
  //     llvm::errs() << " " << astGlobal.dumpDecl(elem) << "\n";
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

  // llvm::errs() << "allFuncTempDecls: " << "\n";
  // llvm::errs() << " " << allFuncDeclVisitor.allFuncTempDecls.size() << "\n";
  // for (const auto *elem : allFuncDeclVisitor.allFuncTempDecls) {
  //   if (elem) {
  //     llvm::errs() << astGlobal.dumpDecl(elem) << "\n";
  //   }
  // }

  funcx::AlwaysRefedMangledNamesAnalysis alwaysRefedMangledNamesAnalysis(context.getSourceManager(),astGlobal);
  alwaysRefedMangledNamesAnalysis.TraverseDecl(context.getTranslationUnitDecl());
  auto alwaysRefedFuncDeclsMangledNames = alwaysRefedMangledNamesAnalysis.alwaysRefedFuncDeclsMangledNames;
  // llvm::errs() << "alwaysRefedFuncDeclsMangledNames:" << "\n";
  // for (std::string name : alwaysRefedFuncDeclsMangledNames){
  //   llvm::errs() << "  " << name << "\n";
  // }

  // Init
  funcx::AlwaysRefedAnalysis alwaysRefedAnalysis(context.getSourceManager());
  alwaysRefedAnalysis.TraverseDecl(context.getTranslationUnitDecl());


  auto alwaysRefedFuncDecls = alwaysRefedAnalysis.alwaysRefedFuncDecls;
  for (std::string name : alwaysRefedFuncDeclsMangledNames){
    for (const auto* funcDecl : allFuncDeclVisitor.MangledNameToFuncDecl[name]) {
  if (funcDecl)
        alwaysRefedFuncDecls.insert(funcDecl);
    }
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

  // Test
  refedTempDecls.insert(hasInstantiationFuncTempDecls.begin(), hasInstantiationFuncTempDecls.end());

  // name match
  std::unordered_set<std::string> UnresolvedRefedFuncDeclsNames;
  UnresolvedRefedFuncDeclsNames.insert(alwaysRefedAnalysis.UnresolvedRefedFuncDeclsNames.begin(),
    alwaysRefedAnalysis.UnresolvedRefedFuncDeclsNames.end());
  UnresolvedRefedFuncDeclsNames.insert(refSymbolAnalysis.funcRefedVisitor.UnresolvedRefedFuncDeclsNames.begin(),
  refSymbolAnalysis.funcRefedVisitor.UnresolvedRefedFuncDeclsNames.end());
  for (auto name : UnresolvedRefedFuncDeclsNames) {
    for (const auto *FuncTempDecl : allFuncDeclVisitor.allFuncTempDecls) {
      if (name == FuncTempDecl->getName().str()) {
        refedTempDecls.insert(FuncTempDecl);
      }
    }
  }

  for (const auto *elem : refedTempDecls) {
    const auto *templatedFuncDecl = elem->getTemplatedDecl();
    refedFuncDecls.insert(templatedFuncDecl);
  }

  std::unordered_set<const clang::FunctionDecl *> TempInstPattern;
  for (const auto *elem : refedFuncDecls) {
      if (const auto *Pattern =
              elem->getTemplateInstantiationPattern()) {
        // Pattern 就是类模板中未实例化的 foo()
        TempInstPattern.insert(Pattern);
        }
  }
  refedFuncDecls.insert(TempInstPattern.begin(), TempInstPattern.end());

  // llvm::errs() << "refedFuncDecls: " << "\n";
  // for (const auto *elem : refedFuncDecls) {
  //   llvm::errs() << " " << astGlobal.dumpDecl(elem) << "\n";
  // }

  // unRefed source ranges.
  illvm::IntervalManager intervalManager;

  // llvm::errs() << "unRefed: \n";
  for (const auto *elem : allFuncDeclVisitor.allFuncDecls) {
    if (refedFuncDecls.find(elem->getCanonicalDecl()) == refedFuncDecls.end()) {
      // llvm::errs() << " " << astGlobal.dumpDecl(elem->getCanonicalDecl()) << "\n";
      intervalManager.addInterval(
          astGlobal.getDeclSourceInterval(elem).toInterval());
    }
  }

  for (const auto *elem : allFuncDeclVisitor.allFuncTempDecls) {
    if (refedTempDecls.find(elem->getCanonicalDecl()) == refedTempDecls.end()) {
      intervalManager.addInterval(
          astGlobal.getDeclSourceInterval(elem).toInterval());
    }
  }

  metaData->unRefedDeclIntervals =
      intervalManager.merge(intervalManager.getIntervals());
}


} // namespace iclang
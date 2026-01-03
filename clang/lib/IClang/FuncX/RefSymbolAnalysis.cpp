#include "iclang/FuncX/RefSymbolAnalysis.h"

#include <queue>

namespace iclang {
namespace funcx {

bool AllFuncDeclVisitor::TraverseDecl(clang::Decl *decl) {
  if (!decl) {
    return true;
  }

  if (const auto *funcDecl = llvm::dyn_cast<clang::FunctionDecl>(decl)) {
    // Do not use getCanonicalDecl here.
    if (!funcDecl->isImplicit()) {
      allFuncDecls.insert(funcDecl);
    }
  }
  if (const auto *funcTempDecl =
          llvm::dyn_cast<clang::FunctionTemplateDecl>(decl)) {
    if (!funcTempDecl->isImplicit()) {
      allFuncTempDecls.insert(funcTempDecl);
    }
  }

  // Skip local functions.
  if (llvm::dyn_cast<clang::FunctionDecl>(decl) != nullptr ||
      llvm::dyn_cast<clang::VarDecl>(decl) != nullptr ||
      llvm::dyn_cast<clang::FieldDecl>(decl) != nullptr) {
    return true;
  }

  const bool res = RecursiveASTVisitor::TraverseDecl(decl);

  return res;
}

bool AlwaysRefedAnalysis::isSpecialSourceRange(
    const clang::FunctionDecl *funcDecl) const {
  // extern "C".
  if (funcDecl->isExternC()) {
    return true;
  }

  // macro.
  const auto loc = funcDecl->getLocation();
  if (sm.isMacroBodyExpansion(loc) || sm.isMacroArgExpansion(loc)) {
    return true;
  }

  return false;
}

bool AlwaysRefedAnalysis::TraverseDecl(clang::Decl *decl) {
  if (!decl) {
    return true;
  }

  if (const auto *usingShadowDecl =
          llvm::dyn_cast<clang::UsingShadowDecl>(decl)) {
    const auto *targetDecl = usingShadowDecl->getTargetDecl();
    if (const auto *funcDecl =
            llvm::dyn_cast<clang::FunctionDecl>(targetDecl)) {
      alwaysRefedFuncDecls.insert(funcDecl->getCanonicalDecl());
    }
  }

  if (const auto *funcDecl = llvm::dyn_cast<clang::FunctionDecl>(decl)) {
    if (isSpecialSourceRange(funcDecl)) {
      alwaysRefedFuncDecls.insert(funcDecl->getCanonicalDecl());
    }
    return true;
  }

  const bool res = RecursiveASTVisitor::TraverseDecl(decl);

  return res;
}

static std::unordered_set<const clang::FunctionDecl *>
extractFuncDependencies(clang::Stmt *stmt,std::unordered_set<const clang::FunctionDecl *>& Unresolved) {
  std::unordered_set<const clang::FunctionDecl *> res;

  std::vector<const clang::Decl *> targetDecls;
  if (const auto *declRefExpr = llvm::dyn_cast<clang::DeclRefExpr>(stmt)) {
    targetDecls.push_back(declRefExpr->getDecl());
  } else if (const auto *memberExpr = llvm::dyn_cast<clang::MemberExpr>(stmt)) {
    targetDecls.push_back(memberExpr->getMemberDecl());
  } else if (const auto *ctor = llvm::dyn_cast<clang::CXXConstructExpr>(stmt)) {
    targetDecls.push_back(ctor->getConstructor());
  } else if (const auto *usLookupExpr =
                 llvm::dyn_cast<clang::UnresolvedLookupExpr>(stmt)) {
    for (const auto *decl : usLookupExpr->decls()) {
      if (const auto *fd = llvm::dyn_cast<clang::FunctionDecl>(decl)) {
        // FunctionDecl
        Unresolved.insert(fd->getCanonicalDecl());
        targetDecls.push_back(fd->getCanonicalDecl());
      } else if (const auto *ft = llvm::dyn_cast<clang::FunctionTemplateDecl>(decl)) {
        // FunctionTemplateDecl
        Unresolved.insert(ft->getTemplatedDecl()->getCanonicalDecl());
        targetDecls.push_back(ft->getTemplatedDecl()->getCanonicalDecl());
      }
      // targetDecls.push_back(decl);
    }
  } else if (const auto *usMemberExpr =
                 llvm::dyn_cast<clang::UnresolvedMemberExpr>(stmt)) {
    for (const auto *decl : usMemberExpr->decls()) {
      targetDecls.push_back(decl);
    }
  } else if (const auto *newExpr = llvm::dyn_cast<clang::CXXNewExpr>(stmt)) {
    targetDecls.push_back(newExpr->getOperatorNew());
    targetDecls.push_back(newExpr->getOperatorDelete());
  } else if (const auto *deleteExpr =
                 llvm::dyn_cast<clang::CXXDeleteExpr>(stmt)) {
    targetDecls.push_back(deleteExpr->getOperatorDelete());
  } else if (const auto *icie =
                 llvm::dyn_cast<clang::CXXInheritedCtorInitExpr>(stmt)) {
    targetDecls.push_back(icie->getConstructor());
  }

  for (const clang::Decl *targetDecl : targetDecls) {
    if (targetDecl == nullptr) {
      continue;
    }
    const auto *targetFuncDecl =
        llvm::dyn_cast<clang::FunctionDecl>(targetDecl);
    if (targetFuncDecl == nullptr) {
      continue;
    }
    res.insert(targetFuncDecl->getCanonicalDecl());
  }

  return res;
}

bool AlwaysRefedAnalysis::TraverseStmt(clang::Stmt *stmt,
                                       DataRecursionQueue *queue) {
  if (!stmt) {
    return true;
  }
  std::unordered_set<const clang::FunctionDecl *> Unresolved;
  auto dependencies = extractFuncDependencies(stmt,Unresolved);
  alwaysRefedFuncDecls.insert(dependencies.begin(), dependencies.end());
  unresolvedRefedFuncDecls.insert(Unresolved.begin(), Unresolved.end());

  return RecursiveASTVisitor::TraverseStmt(stmt, queue);
}

void FuncRefedVisitor::init() { refedFuncDecls.clear(); }

bool FuncRefedVisitor::TraverseDecl(clang::Decl *decl) {
  if (!decl) {
    return true;
  }
  if (const auto *usingShadowDecl =
          llvm::dyn_cast<clang::UsingShadowDecl>(decl)) {
    const auto *targetDecl = usingShadowDecl->getTargetDecl();
    if (const auto *funcDecl =
            llvm::dyn_cast<clang::FunctionDecl>(targetDecl)) {
      refedFuncDecls.insert(funcDecl->getCanonicalDecl());
    }
  }
  return RecursiveASTVisitor::TraverseDecl(decl);
}

bool FuncRefedVisitor::TraverseStmt(clang::Stmt *stmt, DataRecursionQueue *queue) {
  if (!stmt) {
    return true;
  }
  std::unordered_set<const clang::FunctionDecl *> Unresolved;
  auto dependencies = extractFuncDependencies(stmt,Unresolved);
  unresolvedRefedFuncDecls.insert(Unresolved.begin(), Unresolved.end());
  refedFuncDecls.insert(dependencies.begin(), dependencies.end());

  return RecursiveASTVisitor::TraverseStmt(stmt, queue);
}

std::unordered_set<const clang::FunctionDecl *> RefSymbolAnalysis::propagation(
    const std::unordered_set<const clang::FunctionDecl *>
        &alwaysRefedFuncDecls) {
  std::unordered_set<const clang::FunctionDecl *> refedFuncDecls;

  std::queue<const clang::FunctionDecl *> que;
  for (const auto *decl : alwaysRefedFuncDecls) {
    que.push(decl);
  }

  while (!que.empty()) {
    auto *funcDecl = que.front();
    que.pop();

    if (!refedFuncDecls.emplace(funcDecl->getCanonicalDecl()).second) {
      continue;
    }

    // if (funcDecl->isTemplated()) {
    //   continue;
    // }

    if (funcDecl->getDefinition() != nullptr) {
      funcDecl = funcDecl->getDefinition();
    }
    funcRefedVisitor.init();
    funcRefedVisitor.TraverseDecl(const_cast<clang::FunctionDecl *>(funcDecl));
    for (const auto *refedFuncDecl : funcRefedVisitor.refedFuncDecls) {
      que.push(refedFuncDecl->getCanonicalDecl());
    }
  }

  return refedFuncDecls;
}



bool TempFunc_And_Func_Analysis::TraverseDecl(clang::Decl *decl) {
  if (!decl) {
    return true;
  }

  if (const auto *FTD =
          llvm::dyn_cast<clang::FunctionTemplateDecl>(decl)) {

    if (const auto *FD = FTD->getTemplatedDecl()) {
      recordFunction(FD, true, collection, sm);
    }
    return true;
          }

  if (const auto *FD =
          llvm::dyn_cast<clang::FunctionDecl>(decl)) {

    // 模板实例 or 模板模式，已经由 FunctionTemplateDecl 统计
    if (FD->getDescribedFunctionTemplate())
      return true;

    recordFunction(FD, false, collection, sm);
    return true;
          }


  const bool res = RecursiveASTVisitor::TraverseDecl(decl);

  return res;
}

bool TempFunc_And_Func_Analysis::TraverseStmt(clang::Stmt *stmt,
                                       DataRecursionQueue *queue) {
  if (!stmt) {
    return true;
  }

  return RecursiveASTVisitor::TraverseStmt(stmt, queue);
}

void TempFunc_And_Func_Analysis::recordFunction(const clang::FunctionDecl *FD, bool isTemplate,
                    FunctionCollection &collection,
                    const clang::SourceManager &SM) {
  if (!FD || !FD->getLocation().isValid())
    return;

  clang::SourceLocation loc = FD->getLocation();

  FunctionInfo info;
  info.name = FD->getNameAsString();
  info.isTemplate = isTemplate;

  clang::SourceLocation fileLoc = SM.getFileLoc(loc);
  clang::FileID fid = SM.getFileID(fileLoc);
  auto FER = SM.getFileEntryRefForID(fid);
  if (FER) {
    const clang::FileEntry &FE = FER->getFileEntry();
    llvm::StringRef realPath = FE.tryGetRealPathName();
    if (!realPath.empty()) {
      info.file = realPath.str();
    } else {
      info.file = FER->getName().str();
    }
  }

  info.line = SM.getSpellingLineNumber(loc);


  if (isTemplate)
    collection.templateFunctions.push_back(std::move(info));
  else
    collection.normalFunctions.push_back(std::move(info));
}

void TempFunc_And_Func_Analysis::dumpInfo() {
  llvm::errs() << "===== Function Collection =====\n";

  llvm::errs() << "\n--- Template Functions ---\n";
  for (const auto &info : collection.templateFunctions) {
    llvm::errs() << "Name: " << info.name
                 << ", File: " << info.file
                 << ", Line: " << info.line
                 << "\n";
  }

  llvm::errs() << "\n--- Normal Functions ---\n";
  for (const auto &info : collection.normalFunctions) {
    llvm::errs() << "Name: " << info.name
                 << ", File: " << info.file
                 << ", Line: " << info.line
                 << "\n";
  }

  llvm::errs() << "\n===== Summary =====\n";
  llvm::errs() << "Template functions: "
               << collection.templateFunctions.size() << "\n";
  llvm::errs() << "Normal functions:   "
               << collection.normalFunctions.size() << "\n";
}



} // namespace funcx
} // namespace iclang
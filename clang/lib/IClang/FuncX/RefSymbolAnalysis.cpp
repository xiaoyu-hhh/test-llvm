#include "iclang/FuncX/RefSymbolAnalysis.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
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
      std::string name = funcDecl->getName().str();
      std::string funcDeclMangledName = astGlobal.getMangledName(funcDecl);
      MangledNameToFuncDecl[funcDeclMangledName].push_back(funcDecl);
      NameToFuncDecl[name].push_back(funcDecl);
    }
  }
  if (const auto *funcTempDecl =
          llvm::dyn_cast<clang::FunctionTemplateDecl>(decl)) {
    if (!funcTempDecl->isImplicit()) {
      allFuncTempDecls.insert(funcTempDecl);
      std::string name = funcTempDecl->getTemplatedDecl()->getName().str();
      NameToFuncTempDecl[name].push_back(funcTempDecl);
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


static std::unordered_set<const clang::FunctionDecl *>
extractFuncDependencies(clang::Stmt *stmt, std::unordered_set<std::string>& MangledNames,std::unordered_set<const clang::FunctionTemplateDecl *>& UnresolvedRefedFuncTempDecls,const ASTGlobal &astGlobal,
std::unordered_map<std::string, std::vector<const clang::FunctionDecl *>>& NameToFuncDecl,
std::unordered_map<std::string, std::vector<const clang::FunctionTemplateDecl *>>& NameToFuncTempDecl) {

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
      // const auto *targetFuncDecl =
      //     llvm::dyn_cast<clang::FunctionDecl>(decl);
      // const auto *targetFuncTempDecl =
      //     llvm::dyn_cast<clang::FunctionTemplateDecl>(decl);
      // if (targetFuncDecl != nullptr) {
      //   std::string name = targetFuncDecl->getCanonicalDecl()->getName().str();
      //   UnresolvedNames.insert(name);
      //   static std::set<std::string> Names;
      //   if (Names.find(name) == Names.end()) {
      //     Names.insert(name);
      //     for (const auto* funcDecl : NameToFuncDecl[name]) {
      //       targetDecls.push_back(funcDecl);
      //     }
      //   }
      // }
      // if (targetFuncTempDecl != nullptr) {
      //   std::string name = targetFuncTempDecl->getTemplatedDecl()->getCanonicalDecl()->getName().str();
      //   UnresolvedNames.insert(name);
      //   static std::set<std::string> Names;
      //   if (Names.find(name) == Names.end()) {
      //     Names.insert(name);
      //     for (const auto* funcDecl : NameToFuncDecl[name]) {
      //       targetDecls.push_back(funcDecl);
      //     }
      //   }
      // }
      targetDecls.push_back(decl);
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
    const auto *targetFuncTempDecl =
        llvm::dyn_cast<clang::FunctionTemplateDecl>(targetDecl);
    if (targetFuncDecl != nullptr) {
      res.insert(targetFuncDecl->getCanonicalDecl());
      MangledNames.insert(astGlobal.getMangledName(targetFuncDecl->getCanonicalDecl()));
    }
    if (targetFuncTempDecl != nullptr) {
      res.insert(targetFuncTempDecl->getTemplatedDecl()->getCanonicalDecl());
      MangledNames.insert(astGlobal.getMangledName(targetFuncTempDecl->getTemplatedDecl()->getCanonicalDecl()));
      UnresolvedRefedFuncTempDecls.insert(targetFuncTempDecl->getCanonicalDecl());
    }
  }

  return res;
}


bool AlwaysRefedMangledNamesAnalysis::isSpecialSourceRange(
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

bool AlwaysRefedMangledNamesAnalysis::TraverseDecl(clang::Decl *decl) {
  if (!decl) {
    return true;
  }

  if (const auto *usingShadowDecl =
          llvm::dyn_cast<clang::UsingShadowDecl>(decl)) {
    const auto *targetDecl = usingShadowDecl->getTargetDecl();
    if (const auto *funcDecl =
            llvm::dyn_cast<clang::FunctionDecl>(targetDecl)) {
      alwaysRefedFuncDeclsMangledNames.insert(astGlobal.getMangledName(funcDecl->getCanonicalDecl()));
            }
    if (const auto *funcTempDecl =
            llvm::dyn_cast<clang::FunctionTemplateDecl>(targetDecl)) {
      alwaysRefedFuncDeclsMangledNames.insert(astGlobal.getMangledName(funcTempDecl->getTemplatedDecl()->getCanonicalDecl()));
            }
          }

  if (const auto *funcDecl = llvm::dyn_cast<clang::FunctionDecl>(decl)) {
    if (isSpecialSourceRange(funcDecl)) {
      alwaysRefedFuncDeclsMangledNames.insert(astGlobal.getMangledName(funcDecl->getCanonicalDecl()));
    }
    // not return
  }

  const bool res = RecursiveASTVisitor::TraverseDecl(decl);

  return res;
}


bool AlwaysRefedMangledNamesAnalysis::TraverseStmt(clang::Stmt *stmt,
                                       DataRecursionQueue *queue) {
  if (!stmt) {
    return true;
  }
  std::unordered_set<std::string> AlwaysRefedMangledNames;
  std::unordered_set<const clang::FunctionTemplateDecl *> UnresolvedRefedFuncTempDecls;
  std::unordered_map<std::string, std::vector<const clang::FunctionDecl *>> NameToFuncDecl;
  std::unordered_map<std::string, std::vector<const clang::FunctionTemplateDecl *>> NameToFuncTempDecl;
  auto dependencies = extractFuncDependencies(stmt,AlwaysRefedMangledNames,UnresolvedRefedFuncTempDecls,astGlobal,NameToFuncDecl,NameToFuncTempDecl);
  alwaysRefedFuncDeclsMangledNames.insert(AlwaysRefedMangledNames.begin(), AlwaysRefedMangledNames.end());

  return RecursiveASTVisitor::TraverseStmt(stmt, queue);
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
    if (const auto *funcTempDecl =
            llvm::dyn_cast<clang::FunctionTemplateDecl>(targetDecl)) {
      // std::string name = funcTempDecl->getTemplatedDecl()->getCanonicalDecl()->getName().str();
      // UnresolvedRefedFuncDeclsNames.insert(name);
      UnresolvedRefedFuncTempDecls.insert(funcTempDecl->getCanonicalDecl());
      // for (const auto* elem : NameToFuncDecl[name]) {
      //   alwaysRefedFuncDecls.insert(elem->getCanonicalDecl());
      // }
      alwaysRefedFuncDecls.insert(funcTempDecl->getTemplatedDecl()->getCanonicalDecl());
    }
  }
  // UnresolvedUsingValueDecl
  if (const auto *UnresolvedUsingValueDecl =
        llvm::dyn_cast<clang::UnresolvedUsingValueDecl>(decl)) {
    std::string name = UnresolvedUsingValueDecl->getName().str();
    static std::unordered_set<std::string> SingleName;
    if (SingleName.find(name) == SingleName.end()) {
      SingleName.insert(name);
      for (const auto *Decl : NameToFuncDecl[name]) {
        alwaysRefedFuncDecls.insert(Decl->getCanonicalDecl());
        if (const auto *FuncTempDecl = Decl->getDescribedFunctionTemplate()) {
          UnresolvedRefedFuncTempDecls.insert(FuncTempDecl->getCanonicalDecl());
        }
      }
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

bool AlwaysRefedAnalysis::TraverseStmt(clang::Stmt *stmt,
                                       DataRecursionQueue *queue) {
  if (!stmt) {
    return true;
  }
  std::unordered_set<std::string> AlwaysRefedMangledNames;
  auto &astGlobal = ASTGlobal::getInstance();
  auto dependencies = extractFuncDependencies(stmt,AlwaysRefedMangledNames,UnresolvedRefedFuncTempDecls,astGlobal,
    NameToFuncDecl,NameToFuncTempDecl);
  alwaysRefedFuncDecls.insert(dependencies.begin(), dependencies.end());

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
    if (const auto *funcTempDecl =
            llvm::dyn_cast<clang::FunctionTemplateDecl>(targetDecl)) {
      // std::string name = funcTempDecl->getTemplatedDecl()->getCanonicalDecl()->getName().str();
      // UnresolvedRefedFuncDeclsNames.insert(name);
      UnresolvedRefedFuncTempDecls.insert(funcTempDecl->getCanonicalDecl());
      // for (const auto* elem : NameToFuncDecl[name]) {
      //   refedFuncDecls.insert(elem->getCanonicalDecl());
      // }
      refedFuncDecls.insert(funcTempDecl->getTemplatedDecl()->getCanonicalDecl());
    }
  }

  // UnresolvedUsingValueDecl
  if (const auto *UnresolvedUsingValueDecl =
        llvm::dyn_cast<clang::UnresolvedUsingValueDecl>(decl)) {
    std::string name = UnresolvedUsingValueDecl->getName().str();
    static std::unordered_set<std::string> SingleName;
    if (SingleName.find(name) == SingleName.end()) {
      SingleName.insert(name);
      for (const auto *Decl : NameToFuncDecl[name]) {
        refedFuncDecls.insert(Decl->getCanonicalDecl());
        if (const auto *FuncTempDecl = Decl->getDescribedFunctionTemplate()) {
          UnresolvedRefedFuncTempDecls.insert(FuncTempDecl->getCanonicalDecl());
        }
      }
    }
  }

  return RecursiveASTVisitor::TraverseDecl(decl);
}

bool FuncRefedVisitor::TraverseStmt(clang::Stmt *stmt, DataRecursionQueue *queue) {
  if (!stmt) {
    return true;
  }
  std::unordered_set<std::string> RefedMangledNames;
  auto &astGlobal = ASTGlobal::getInstance();
  auto dependencies = extractFuncDependencies(stmt,RefedMangledNames,UnresolvedRefedFuncTempDecls,astGlobal,
    NameToFuncDecl,NameToFuncTempDecl);
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
void TempFunc_And_Func_Analysis::recordFunction(
    const clang::FunctionDecl *FD,
    bool isTemplate,
    FunctionCollection &collection,
    const clang::SourceManager &SM) {

  if (!FD || !FD->getLocation().isValid())
    return;

  FunctionInfo info;
  info.name = FD->getNameAsString();
  info.isTemplate = isTemplate;

  // ---- 文件名 ----
  clang::SourceLocation loc = FD->getLocation();
  clang::SourceLocation fileLoc = SM.getFileLoc(loc);
  clang::FileID fid = SM.getFileID(fileLoc);
  if (auto FER = SM.getFileEntryRefForID(fid)) {
    llvm::StringRef realPath = FER->getFileEntry().tryGetRealPathName();
    info.file = realPath.empty() ? FER->getName().str()
                                 : realPath.str();
  }
  // ---- 范围 ----
  illvm::SourceInterval si = {};
  if (isTemplate) {
    if (const auto *FTD = FD->getDescribedFunctionTemplate())
    si = astGlobal.getDeclSourceInterval(FTD);
  }
  else {
    si = astGlobal.getDeclSourceInterval(FD);
  }

  // ---- 行数 ----
  info.startLine = si.startLine;
  info.endLine   = si.endLine;
  info.lineCount = info.endLine - info.startLine + 1;

  // ---- 字符数 ----
  info.startOffset = si.startOffset;
  info.endOffset = si.endOffset;
  info.offset = si.endOffset - si.startOffset;

  // ---- 收集 ----
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
                 << ", StartLine: " << info.startLine
                 << ", Lines: " << info.lineCount
                 << ", Chars: " << info.offset
                 << ", File: " << info.file
                 << "\n";
  }

  llvm::errs() << "\n--- Normal Functions ---\n";
  for (const auto &info : collection.normalFunctions) {
    llvm::errs() << "Name: " << info.name
                 << ", StartLine: " << info.startLine
                 << ", Lines: " << info.lineCount
                 << ", Chars: " << info.offset
                 << ", File: " << info.file
                 << "\n";
  }

  llvm::errs() << "\n===== Summary =====\n";
  llvm::errs() << "Template functions: "
               << collection.templateFunctions.size() << "\n";
  llvm::errs() << "Normal functions:   "
               << collection.normalFunctions.size() << "\n";
}

template <typename T>
T mergeAndSumIntervals(std::vector<std::pair<T, T>> &intervals,
                       bool inclusiveEnd) {
  if (intervals.empty())
    return 0;

  // 1. 按 start 排序
  std::sort(intervals.begin(), intervals.end(),
            [](const auto &a, const auto &b) {
              return a.first < b.first;
            });

  T total = 0;
  T curStart = intervals[0].first;
  T curEnd   = intervals[0].second;

  for (size_t i = 1; i < intervals.size(); ++i) {
    T s = intervals[i].first;
    T e = intervals[i].second;

    // 是否重叠 / 相邻
    if (s <= curEnd + (inclusiveEnd ? 1 : 0)) {
      curEnd = std::max(curEnd, e);
    } else {
      // 结算前一个区间
      total += inclusiveEnd
                   ? (curEnd - curStart + 1)
                   : (curEnd - curStart);
      curStart = s;
      curEnd   = e;
    }
  }

  // 最后一个区间
  total += inclusiveEnd
               ? (curEnd - curStart + 1)
               : (curEnd - curStart);

  return total;
}


void TempFunc_And_Func_Analysis::initMetaData() {
  unsigned TFD_num = 0;
  unsigned TFD_line = 0;
  unsigned TFD_offset = 0;
  unsigned FD_num = 0;
  unsigned FD_line = 0;
  unsigned FD_offset = 0;
  std::string filename = "";
  unsigned filesize = 0;
  unsigned fileline = 0;

  TFD_num = collection.templateFunctions.size();
  FD_num = collection.normalFunctions.size();
  if (TFD_num) {
    filename = collection.templateFunctions[0].file;
  }
  else if (FD_num) {
    filename = collection.normalFunctions[0].file;
  }
  using Interval = std::pair<unsigned, unsigned>;
  std::vector<Interval> TFD_Line_Interval;
  std::vector<Interval> TFD_Offset_Interval;
  std::vector<Interval> FD_Line_Interval;
  std::vector<Interval> FD_Offset_Interval;
  for (auto elem:collection.templateFunctions) {
    TFD_Line_Interval.emplace_back(elem.startLine, elem.endLine);
    TFD_Offset_Interval.emplace_back(elem.startOffset, elem.endOffset);
  }
  for (auto elem:collection.normalFunctions) {
    FD_Line_Interval.emplace_back(elem.startLine, elem.endLine);
    FD_Offset_Interval.emplace_back(elem.startOffset, elem.endOffset);
  }
  // 区间合并

  // TFD_line
  // TFD_offset
  // FD_line
  // FD_offset
  // ---- 模板函数 ----
  TFD_line = mergeAndSumIntervals<unsigned>(
    TFD_Line_Interval,
    /*inclusiveEnd=*/true);

  TFD_offset = mergeAndSumIntervals<unsigned>(
    TFD_Offset_Interval,
    /*inclusiveEnd=*/false);

  // ---- 普通函数 ----
  FD_line = mergeAndSumIntervals<unsigned>(
    FD_Line_Interval,
    /*inclusiveEnd=*/true);

  FD_offset = mergeAndSumIntervals<unsigned>(
    FD_Offset_Interval,
    /*inclusiveEnd=*/false);

  if (!filename.empty()) {
    clang::FileID fid = sm.getMainFileID();
    if (const clang::FileEntry *FE = sm.getFileEntryForID(fid)) {
      filesize = FE->getSize();
    }

    // clang::SourceLocation startLoc = sm.getLocForStartOfFile(fid);
    clang::SourceLocation endLoc = sm.getLocForEndOfFile(fid);
    if (endLoc.isValid())
      endLoc = endLoc.getLocWithOffset(-1);

    fileline = sm.getSpellingLineNumber(endLoc);
  }

  md.TFD_num = TFD_num;
  md.TFD_line = TFD_line;
  md.TFD_offset = TFD_offset;
  md.FD_num = FD_num;
  md.FD_line = FD_line;
  md.FD_offset = FD_offset;
  md.filename = filename;
  md.filesize = filesize;
  md.fileline = fileline;
}

static std::string extractProjectName(const std::string &path) {
  llvm::SmallVector<llvm::StringRef, 20> components;
  llvm::StringRef(path).split(components, '/', -1, false);

  // path = /root/ibenchmark/文件名/XXX/XXX
  // components = ["root", "ibenchmark", "文件名", "XXX", "XXX"]

  if (components.size() >= 3)
    return components[2].str();  // 文件名

  return "unknown";
}

void TempFunc_And_Func_Analysis::writeMetaDataToLog() {
  std::string projectName = extractProjectName(md.filename);
  std::string logPath = "/root/ibenchmark/logfile_" + projectName;

  std::error_code EC;
  llvm::raw_fd_ostream logFile(
      logPath,
      EC,
      llvm::sys::fs::OF_Append | llvm::sys::fs::OF_Text);

  if (EC) {
    llvm::errs() << "Failed to open log file: " << logPath
                 << " : " << EC.message() << "\n";
    return;
  }

  logFile
      << md.fileline << " "
      << md.filesize << " "
      << md.TFD_num << " "
      << md.TFD_line << " "
      << md.TFD_offset << " "
      << md.FD_num << " "
      << md.FD_line << " "
      << md.FD_offset << "\n";
      // << md.filename << "\n";

  logFile.flush();
}


void TempFunc_And_Func_Analysis::dumpMetaData() {
  llvm::errs()  << "======== MetaData ========\n";
  llvm::errs()  << "File Name            : " << md.filename << "\n";
  llvm::errs()  << "File Line            : " << md.fileline << "\n";
  llvm::errs()  << "File Size            : " << md.filesize << "\n";

  llvm::errs()  << "\n[Template Functions]\n";
  llvm::errs()  << "  Count              : " << md.TFD_num << "\n";
  llvm::errs()  << "  Total Lines        : " << md.TFD_line << "\n";
  llvm::errs()  << "  Total Offset       : " << md.TFD_offset << "\n";

  llvm::errs()  << "\n[Normal Functions]\n";
  llvm::errs()  << "  Count              : " << md.FD_num << "\n";
  llvm::errs()  << "  Total Lines        : " << md.FD_line << "\n";
  llvm::errs()  << "  Total Offset       : " << md.FD_offset << "\n";

  llvm::errs()  << "===========================\n";
}

} // namespace funcx
} // namespace iclang
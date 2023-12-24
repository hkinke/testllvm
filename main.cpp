#include "KaleidoscopeJIT.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

#include <memory>
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>

using namespace llvm;
using namespace llvm::orc;

std::unique_ptr<KaleidoscopeJIT> TheJIT;

std::unique_ptr<LLVMContext> TheContext;
std::unique_ptr<Module> TheModule;
std::unique_ptr<IRBuilder<>> Builder;


int main(int argc,const char * argv[])
{
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
    
    TheJIT=std::move(*KaleidoscopeJIT::Create());
    
    TheContext=std::make_unique<LLVMContext>();
    TheModule=std::make_unique<Module>("JIT module",*TheContext);
    Builder=std::make_unique<IRBuilder<>>(*TheContext);
    
    
    TheModule->setDataLayout(TheJIT->getDataLayout());
    
    Type * bool_type=Type::getInt1Ty(*TheContext);
    Type * int_type=Type::getInt32Ty(*TheContext);
    Type * int_ptr_type=PointerType::get(int_type,0);
    Type * int64_type=Type::getInt64Ty(*TheContext);
    Type * double_type=Type::getDoubleTy(*TheContext);
    Type * void_type=Type::getVoidTy(*TheContext);
    Type * void_ptr_type=PointerType::get(void_type,0);
    Type * void_ptr_ptr_type=PointerType::get(void_ptr_type,0);
    
    std::vector<Type *> malloc_arg_types(1,int64_type);
    
    FunctionType * malloc_type=FunctionType::get(void_ptr_type,malloc_arg_types,false);
    
    Function * malloc_declare=Function::Create(malloc_type,Function::ExternalLinkage,"malloc",TheModule.get());
    
    std::vector<Type *> myalloc_arg_types(1,int64_type);
    std::vector<std::string> myalloc_arg_name={"size"};
    
    FunctionType * myalloc_type=FunctionType::get(void_ptr_type,myalloc_arg_types,false);
    
    Function * myalloc_define=Function::Create(myalloc_type,Function::ExternalLinkage,"myalloc",TheModule.get());
    
    int i=0;
    for(auto & arg:myalloc_define->args())
    {
        arg.setName(myalloc_arg_name[i++]);
    }
    
    BasicBlock * BB=BasicBlock::Create(*TheContext,"entry",myalloc_define);
    
    Builder->SetInsertPoint(BB);
    
    std::vector<Value *> arg_malloc={myalloc_define->getArg(0)};
    Value * m=Builder->CreateCall(malloc_declare,arg_malloc);
    
    Builder->CreateRet(m);
    
    verifyFunction(*myalloc_define);
    
    
    auto RT = TheJIT->getMainJITDylib().createResourceTracker();
    
    auto TSM=ThreadSafeModule(std::move(TheModule),std::move(TheContext));
    Error err=TheJIT->addModule(std::move(TSM),RT);
    if(err)
    {
        errs() << err;
        return 1;
    }
    
    void * (*myalloc)(size_t);
    auto myallocSymbol=TheJIT->lookup("myalloc");
    if(myallocSymbol)
    {
        myalloc=myallocSymbol->getAddress().toPtr<void * (*)(size_t)>();
    }
    else
    {
        errs() << myallocSymbol.takeError();
        return 1;
    }
    
    int * p=static_cast<int *>(myalloc(4*sizeof(int)));
    
    p[0]=1;
    p[1]=1;
    p[2]=2;
    p[3]=3;
    
    std::cout << p[0] << " " << p[1] << " " << p[2] << " " << p[3] << "\n";
    
    free(p);
    
    return 0;
}
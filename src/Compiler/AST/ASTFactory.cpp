/*
 * ASTFactory.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ASTFactory.h"
#include "Helper.h"
#include "Exception.h"


namespace Xsc
{

namespace ASTFactory
{


Expr* FindSingleExpr(Expr* ast, const AST::Types searchedExprType)
{
    while (ast)
    {
        if (ast->Type() == searchedExprType)
        {
            /* Searched expression found */
            return ast;
        }
        else if (ast->Type() == AST::Types::BracketExpr)
        {
            /* Continue search in inner bracket expression */
            ast = static_cast<BracketExpr*>(ast)->expr.get();
        }
        else
        {
            /* Other expressions are not allowed for this search => stop searching */
            break;
        }
    }
    return nullptr;
}

FunctionCallPtr FindSingleFunctionCall(Expr* ast)
{
    if (auto expr = FindSingleExpr(ast, AST::Types::FunctionCallExpr))
        return static_cast<FunctionCallExpr*>(expr)->call;
    return nullptr;
}

VarIdentPtr FindSingleVarIdent(Expr* ast)
{
    if (auto expr = FindSingleExpr(ast, AST::Types::VarAccessExpr))
        return static_cast<VarAccessExpr*>(expr)->varIdent;
    return nullptr;
}

FunctionCallExprPtr MakeIntrinsicCallExpr(
    const Intrinsic intrinsic, const std::string& ident, const TypeDenoterPtr& typeDenoter, const std::vector<ExprPtr>& arguments)
{
    auto ast = MakeShared<FunctionCallExpr>(SourcePosition::ignore);

    auto funcCall = MakeShared<FunctionCall>(SourcePosition::ignore);

    funcCall->varIdent          = MakeShared<VarIdent>(SourcePosition::ignore);
    funcCall->varIdent->ident   = ident;
    funcCall->typeDenoter       = typeDenoter;
    funcCall->arguments         = arguments;
    funcCall->intrinsic         = intrinsic;

    ast->call = funcCall;

    return ast;
}

ListExprPtr MakeSeparatedSinCosFunctionCalls(FunctionCall& funcCall)
{
    if (funcCall.arguments.size() == 3)
    {
        /*
        Convert "sincos(x, a, b)" expression to "a = sin(x), b = cos(x)"
        see https://msdn.microsoft.com/en-us/library/windows/desktop/bb509652(v=vs.85).aspx
        */
        auto listExpr = MakeShared<ListExpr>(funcCall.area);

        auto arg0 = funcCall.arguments[0];
        auto arg1 = funcCall.arguments[1];
        auto arg2 = funcCall.arguments[2];

        if (auto varOutSin = ASTFactory::FindSingleVarIdent(arg1.get()))
        {
            auto sinExpr = MakeShared<VarAccessExpr>(funcCall.area);
            sinExpr->varIdent   = varOutSin;
            sinExpr->assignOp   = AssignOp::Set;
            sinExpr->assignExpr = MakeIntrinsicCallExpr(Intrinsic::Sin, "sin", arg1->GetTypeDenoter(), { arg0 });
            listExpr->firstExpr = sinExpr;
        }
        else
            RuntimeErr("single variable identifier expected in intrinsic 'sincos'", arg1.get());

        if (auto varOutCos = ASTFactory::FindSingleVarIdent(arg2.get()))
        {
            auto cosExpr = MakeShared<VarAccessExpr>(funcCall.area);
            cosExpr->varIdent   = varOutCos;
            cosExpr->assignOp   = AssignOp::Set;
            cosExpr->assignExpr = MakeIntrinsicCallExpr(Intrinsic::Cos, "cos", arg2->GetTypeDenoter(), { arg0 });
            listExpr->nextExpr = cosExpr;
        }
        else
            RuntimeErr("single variable identifier expected in intrinsic 'sincos'", arg2.get());

        return listExpr;
    }
    else
        RuntimeErr("invalid number of arguments in intrinsic", &funcCall);
}


} // /namespace ASTFactory

} // /namespace Xsc



// ================================================================================
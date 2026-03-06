#pragma once
#include <Core/QualifiedTableName.h>
#include <Parsers/IAST_fwd.h>
#include <Interpreters/InDepthNodeVisitor.h>


namespace DB
{

class ASTFunction;
class ASTFunctionWithKeyValueArguments;
class ASTStorage;

using TableNamesSet = std::unordered_set<QualifiedTableName>;

/// Returns a list of "hard" dependencies of a table, i.e. tables without which a dependent table will fail to load.
/// For example, a dictionary depends on its source table; a table with dictGet() in column defaults depends on that dictionary.
/// Hard dependencies are a subset of referential dependencies (which include everything mentioned in the CREATE query).
/// Because a table cannot function without its hard dependencies, they are checked by default
/// when trying to DROP or RENAME a table (see the `check_table_dependencies` setting).
/// Does not validate AST, works a best-effort way.
TableNamesSet getHardDependenciesFromCreateQuery(ContextPtr global_context, const QualifiedTableName & table, const ASTPtr & ast, bool can_throw = false);


class DDLMatcherBase
{
public:
    static bool needChildVisit(const ASTPtr & node, const ASTPtr & child);
    static ssize_t getPositionOfTableNameArgumentToVisit(const ASTFunction & function);
    static ssize_t getPositionOfTableNameArgumentToEvaluate(const ASTFunction & function);
};

/// Visits ASTCreateQuery and extracts the names of all tables which are "hard" dependencies of a specified table.
/// Hard dependencies are a subset of referential dependencies: only structural dependencies like
/// dictGet() in column defaults, dictionary sources, TTL expressions, and specific engine arguments.
/// A table will fail to load if any of its hard dependencies is missing.
class DDLHardDependencyVisitor : public DDLMatcherBase
{
public:
    struct Data
    {
        String default_database;
        TableNamesSet dependencies;
        ContextPtr global_context;
        ASTPtr create_query;
        QualifiedTableName table_name;
        bool can_throw;
    };

    using Visitor = ConstInDepthNodeVisitor<DDLHardDependencyVisitor, true>;

    static void visit(const ASTPtr & ast, Data & data);

private:
    static void visit(const ASTFunction & function, Data & data);
    static void visit(const ASTFunctionWithKeyValueArguments & dict_source, Data & data);
    static void visit(const ASTStorage & storage, Data & data);

    static void extractTableNameFromArgument(const ASTFunction & function, Data & data, size_t arg_idx);
};

}

#include <memory>
#include <Databases/IDatabase.h>
#include <Storages/IStorage.h>
#include <Parsers/ASTCreateQuery.h>
#include <Common/quoteString.h>
#include <Interpreters/DatabaseCatalog.h>
#include <Common/NamePrompter.h>


namespace DB
{

namespace ErrorCodes
{
    extern const int UNKNOWN_TABLE;
    extern const int CANNOT_BACKUP_TABLE;
    extern const int CANNOT_RESTORE_TABLE;
}

StoragePtr IDatabase::getTable(const String & name, ContextPtr context) const
{
    if (auto storage = tryGetTable(name, context))
        return storage;
    TableNameHints hints(this->shared_from_this(), context);
    std::vector<String> names = hints.getHints(name);
    if (names.empty())
        throw Exception(ErrorCodes::UNKNOWN_TABLE, "Table {}.{} does not exist", backQuoteIfNeed(getDatabaseName()), backQuoteIfNeed(name));
    else
        throw Exception(ErrorCodes::UNKNOWN_TABLE, "Table {}.{} does not exist. Maybe you meant {}?", backQuoteIfNeed(getDatabaseName()), backQuoteIfNeed(name), backQuoteIfNeed(names[0]));
}

std::vector<std::pair<ASTPtr, StoragePtr>> IDatabase::getTablesForBackup(const FilterByNameFunction &, const ContextPtr &) const
{
    /// Cannot backup any table because IDatabase doesn't own any tables.
    throw Exception(ErrorCodes::CANNOT_BACKUP_TABLE,
                    "Database engine {} does not support backups, cannot backup tables in database {}",
                    getEngineName(), backQuoteIfNeed(getDatabaseName()));
}

void IDatabase::createTableRestoredFromBackup(const ASTPtr & create_table_query, ContextMutablePtr, std::shared_ptr<IRestoreCoordination>, UInt64)
{
    /// Cannot restore any table because IDatabase doesn't own any tables.
    throw Exception(ErrorCodes::CANNOT_RESTORE_TABLE,
                    "Database engine {} does not support restoring tables, cannot restore table {}.{}",
                    getEngineName(), backQuoteIfNeed(getDatabaseName()),
                    backQuoteIfNeed(create_table_query->as<const ASTCreateQuery &>().getTable()));
}

/// Add a table to the database, but do not add it to the metadata. The database may not support this method.
///
/// Note: ATTACH TABLE statement actually uses createTable method.
void IDatabase::attachTable(ContextPtr context, const String & name, const StoragePtr & table, const String & relative_table_path) /// NOLINT
{
    std::lock_guard lock(mutex);
    attachTableUnlocked(context, name, table, relative_table_path);
}

void IDatabase::registerLazyTable(ContextPtr, const String & table_name, LazyTableCreator table_creator, const String & relative_table_path) /// NOLINT
{
    std::lock_guard lock(mutex);
    registerLazyTableUnlocked(table_name, std::move(table_creator), relative_table_path);
}


}


# 代码

## 目录结构
```
./src/
├── Access 
├── AggregateFunctions
├── Analyzer
├── Backups
├── Bridge
├── BridgeHelper
├── CMakeLists.txt
├── Client
├── Columns
├── Common
├── Compression
├── Coordination
├── Core // 
├── Daemon
├── DataTypes
├── Databases
├── Dictionaries
├── Disks
├── Formats
├── Functions
├── IO
├── Interpreters // 
├── Loggers //
├── NOTICE
├── Parsers //将SQL解析成parse tree
├── Planner //
├── Processors //
├── QueryPipeline
├── Server // 
├── Storages // 
├── TableFunctions
```


## 列存
Clickhouse是面向OLAP的列存储数据库系统，数据的存储和读写都是批量处理的。
Block Stream就是一个个 Block 组成的数据。
Block Stream分为两种，
- 负责数据写入的实现 IBlockOutputStream接口，
- 负责数据读取的实现 IBlockInputStream接口，

IStorage 是Clickhouse存储引擎的接口，我们直接看最关键的 MergeTree引擎的实现。

MergeTreeBlockOutputStream一次写入一个Block,然后会唤醒后台任务将一个个小的Block合并。这应该就是MergeTree命名的由来了。由此我们可知，Clickhouse应尽可能的批量写入数据而不是一条一条的写。

## 高性能的秘密是
Clickhouse高性能的秘密是vectorized query execution 和 runtime code generation，即向量化SIMD的运用和JIT。

若是开启了USE_EMBEDDED_COMPILER， compileFunctions函数会将复杂的表达式即时编译成机器码执行, Clickhouse会缓存编译结果，由此提高性能。


## server启动
```
programs/main.cpp
 programs/server/Server.cpp::mainEntryClickHouseServer
  Server::run()
   //主程序会根据指令分发到 dbms/programs 目录下的程序中处理
   Application::run()
    Application::initialize

Server::main
 Server::createServers
  buildProtocolStackFromConfig
   TCPServerConnectionFactory
    TCPHandlerFactory

TCPHandlerFactory::createConnection
 TCPHandler
```

## HTTP入口
```
HTTPHandler::processQuery
Interpreters/executeQuery.h/executeQuery
executeQueryImpl
 parseKQLQuery //Dialect::kusto
 parseQuery //Dialect::prql
  tryParseQuery
   parser.parse

```


## server处理网络请求
请求->parser:
```
TCPHandler::runImpl()
 //处理client发送的sql
 executeQuery.cpp::executeQuery
  executeQueryImpl
   Parsers/parseQuery.cpp::parseQuery
    parseQueryAndMovePosition
     tryParseQuery
      IParserBase::parse
       parseImpl
```



## MergeTree入口
读操作:
```
MergeTreeDataSelectExecutor.cpp

```

写操作:
```
MergeTreeDataWriter.cpp

```


## 解析 SQL、构建 Pipeline
- 核心处理逻辑在DB::executeQueryImpl()方法
- 解析 SQL 拿到 AST
- 不同的 AST，使用相应的 Interpreter 去构建 Pipeline

## 执行 Pipeline
- 会根据 pipeline 是 Push/Pull 模式分别处理不同的 SQL 语句
- PullingAsyncPipelineExecutor::pull(Block & block)方法会通过线程池，异步执行 Pipeline

## 源码
- QueryPlan::buildQueryPipeline()方法会根据 QueryPlan 构建 QueryPipeline
- QueryPlan::buildQueryPipeline()  通过栈实现了 DFS 算法


## MVCC
TODO




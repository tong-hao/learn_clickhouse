
# 入口

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



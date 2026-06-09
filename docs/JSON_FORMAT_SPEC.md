# 编译器 GUI 接口 — JSON 数据格式规范

本文档定义了编译器前端（GUI）与编译器后端之间交换数据的四种 JSON 文件。

## 通用约定

- 所有文件使用 **UTF-8 编码**
- 字符串内双引号用 `\"` 转义
- 文件名约定：源文件 `test.src` → `test_tokens.json` / `test_ast.json` / `test_semantic.json` / `test_ir.json`（放在同目录）

---

## 1. tokens.json — 词法分析输出

### 字段

| 字段 | 类型 | 说明 |
|------|------|------|
| `source` | string | 源文件名 |
| `timestamp` | string | ISO 8601 格式时间（如 `2026-06-10T14:30:00`） |
| `tokens` | array\<Token\> | Token 数组，按源码顺序排列 |
| `errors` | array\<string\> | 词法错误列表（空数组表示无错误） |

### Token 结构

| 字段 | 类型 | 说明 |
|------|------|------|
| `type` | int | 种别编码：1=关键字 2=标识符 3=整数 4=运算符 5=分隔符 |
| `value` | string | 词法单元的原始字符串 |
| `line` | int | 所在行号（1 起始） |

### 示例

```json
{
  "source": "test.src",
  "timestamp": "2026-06-10T14:30:00",
  "tokens": [
    {"type": 1, "value": "if",    "line": 1},
    {"type": 5, "value": "(",     "line": 1},
    {"type": 2, "value": "x",     "line": 1},
    {"type": 4, "value": ">",     "line": 1},
    {"type": 3, "value": "0",     "line": 1},
    {"type": 5, "value": ")",     "line": 1},
    {"type": 5, "value": "{",     "line": 2},
    {"type": 2, "value": "y",     "line": 2},
    {"type": 4, "value": "=",     "line": 2},
    {"type": 3, "value": "1",     "line": 2},
    {"type": 5, "value": ";",     "line": 2},
    {"type": 5, "value": "}",     "line": 3}
  ],
  "errors": []
}
```

---

## 2. ast.json — 语法分析输出

### 字段

| 字段 | 类型 | 说明 |
|------|------|------|
| `source` | string | 源文件名 |
| `timestamp` | string | ISO 8601 时间 |
| `ast` | ASTNode \| null | 语法树根节点；语法错误时为 null |
| `errors` | array\<string\> | 语法错误列表（空数组表示无错误） |

### ASTNode 结构

每个节点为一个 JSON 对象，包含 `type` 字段标识节点种类：

| type 值 | 含义 | 额外字段 |
|---------|------|---------|
| `NODE_IF` | if-else 语句 | `cond`、`thenBranch`、`elseBranch`（子 ASTNode） |
| `NODE_COND` | 条件表达式 E→id1 op id2 | `op`（"<"/">"/"="）、`left`、`right`（子 ASTNode） |
| `NODE_ASSIGN` | 赋值语句 P→id op NUM | `op`（"="）、`left`、`right`（子 ASTNode） |
| `NODE_ID` | 标识符（叶子） | `value`（标识符名）、`line`（行号） |
| `NODE_NUM` | 整数常量（叶子） | `value`（数值）、`line`（行号） |

### 示例

```json
{
  "source": "test.src",
  "timestamp": "2026-06-10T14:30:01",
  "ast": {
    "type": "NODE_IF",
    "cond": {
      "type": "NODE_COND",
      "op": ">",
      "left":  { "type": "NODE_ID",  "value": "x", "line": 1 },
      "right": { "type": "NODE_NUM", "value": "0", "line": 1 }
    },
    "thenBranch": {
      "type": "NODE_ASSIGN",
      "op": "=",
      "left":  { "type": "NODE_ID",  "value": "y", "line": 2 },
      "right": { "type": "NODE_NUM", "value": "1", "line": 2 }
    },
    "elseBranch": null
  },
  "errors": []
}
```

---

## 3. semantic.json — 语义分析输出

### 字段

| 字段 | 类型 | 说明 |
|------|------|------|
| `source` | string | 源文件名 |
| `timestamp` | string | ISO 8601 时间 |
| `symtab` | array\<Symbol\> | 符号表，所有已声明标识符 |
| `errors` | array\<string\> | 语义错误列表（空数组表示无错误） |

### Symbol 结构

| 字段 | 类型 | 说明 |
|------|------|------|
| `name` | string | 标识符名 |
| `type` | string | 类型（如 `"int"`） |
| `scope` | int | 作用域层级（当前文法只有一层，值为 1） |
| `line` | int | 声明行号 |

### 示例

```json
{
  "source": "test.src",
  "timestamp": "2026-06-10T14:30:02",
  "symtab": [
    {"name": "x", "type": "int", "scope": 1, "line": 1},
    {"name": "y", "type": "int", "scope": 1, "line": 2}
  ],
  "errors": ["第3行: 变量 'z' 未声明"]
}
```

---

## 4. ir.json — 中间代码（四元式）输出

### 字段

| 字段 | 类型 | 说明 |
|------|------|------|
| `source` | string | 源文件名 |
| `timestamp` | string | ISO 8601 时间 |
| `quads` | array\<Quad\> | 四元式数组 |
| `errors` | array\<string\> | 中间代码生成错误列表（空数组表示无错误） |

### Quad 结构

| 字段 | 类型 | 说明 |
|------|------|------|
| `op` | string | 操作符：`j>` / `j<` / `j=`（条件跳转）、`j`（无条件跳转）、`=`（赋值） |
| `arg1` | string | 第一操作数；无操作数时填 `"-"` |
| `arg2` | string | 第二操作数；无操作数时填 `"-"` |
| `result` | string | 结果变量名或跳转目标标签编号 |

### 示例

```json
{
  "source": "test.src",
  "timestamp": "2026-06-10T14:30:03",
  "quads": [
    {"op": "j>",  "arg1": "x", "arg2": "0", "result": "3"},
    {"op": "j",   "arg1": "-", "arg2": "-", "result": "5"},
    {"op": "=",   "arg1": "1", "arg2": "-", "result": "y"}
  ],
  "errors": []
}
```

---

## 修订历史

| 版本 | 日期 | 说明 |
|------|------|------|
| 1.1 | 2026-06-10 | 四个 JSON 文件统一增加 `errors` 字段 |

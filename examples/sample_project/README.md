# sample_project

cclint の動作確認用サンプルプロジェクト。ビルドもできる。

## 使い方

```bash
# cclint をビルド後:
cd examples/sample_project

make build   # 通常ビルド (lint なし)
make lint    # cclint でラップしてビルド (lint + compile)
make clean   # 成果物削除
```

## ファイル構成

| File | Violations |
|------|-----------|
| `src/bad_naming.hpp` | class名非PascalCase, enum名非PascalCase, private member trailing `_` 欠落 |
| `src/bad_safety.hpp` | 基底クラスdestructor非virtual, constructor非explicit, destructor内throw |
| `src/bad_restrictions.hpp` | global `using namespace`, class内 `malloc`/`free` |
| `src/bad_structure.hpp` | 1ファイルに複数クラス |
| `src/good_example.hpp` | 違反なし (全ルール適合) |
| `src/main.cpp` | エントリポイント |

`bad_*` は意図的にルール違反を含む。`good_example.hpp` は警告ゼロを確認する。

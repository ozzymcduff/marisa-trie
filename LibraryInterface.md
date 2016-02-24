# はじめに #

libmarisa には，C++ 用のインタフェースと C 言語用のインタフェースがあります．すべての機能は C++ により実装されていて，それらに対するラッパとして C 言語用のインタフェースが提供されているという構成になっています．

  * Version: 0.1.5




---


# エラー処理 #

C++ 用のインタフェースでは，エラーを検出したとき，例外（`marisa::Exception`）を投げるようになっています．ただし，I/O において `std::iostream` が `std::ios_base::failure` 以外を投げた場合，およびにコールバック関数が `std::bad_alloc` もしくは `std::length_error` 以外の例外を投げた場合，それらの例外はライブラリ内部でキャッチされないので，呼び出し側で対処する必要があります．

一方，C 言語用のインタフェースでは，例外をライブラリの内部でキャッチし，エラー番号（`marisa_status`）を返すようになっています．呼び出し側に例外が届くようなことはありません．あるとすれば，それはライブラリのバグです．


---


## 例外 ##

例外クラスである `marisa::Exception` には，`std::exception` と同じように関数 `what()` を持たせているほか，ファイル名，行番号，エラー番号を持たせるようにしています．

```
class Exception {
 public:
  Exception(const char *filename, int line, Status status);
  Exception(const Exception &ex);

  Exception &operator=(const Exception &rhs);

  // 例外を投げたファイルの名前（__FILE__）を返します．
  const char *filename() const;

  // 例外を投げた行の番号（__LINE__）を返します．
  int line() const;

  // エラー番号を返します．
  Status status() const;

  // エラー番号に対応する文字列を返します．
  // marisa_strerror() を呼び出すだけです．
  const char *what() const;
};
```


---


## エラー番号 ##

エラー番号の一覧は，列挙体 `marisa_status`（`marisa::Status` と同じ）のメンバとして定義されています．

```
typedef enum marisa_status_ {
  // C 言語用のインタフェース専用であり，問題がないときに返されます．
  MARISA_OK               = 0,

  // C 言語用のインタフェース専用であり，不正なハンドルを指定したときに
  // 返されます．
  MARISA_HANDLE_ERROR     = 1,

  // オブジェクトの状態と指定した動作の組み合わせが適切でないことを示す
  // エラーです．例えば，構築前の marisa::Trie オブジェクトで検索しようと
  // した場合に発生します．
  MARISA_STATE_ERROR      = 2,

  // 引数が正しくないときに起きるエラーです．NULL ポインタや範囲外の値を
  // 引数として渡したときに発生することがあります．
  MARISA_PARAM_ERROR      = 3,

  // 基本的には，入力されたキー集合が大きすぎるとき，あるいは検索結果が
  // 大きくなりすぎたときに起きるエラーです．`std::length_error' の代わり
  // として使っています．
  MARISA_SIZE_ERROR       = 4,

  // メモリの確保に失敗したことを示すエラーです．
  MARISA_MEMORY_ERROR     = 5,

  // 入出力に失敗したことを示すエラーです．
  MARISA_IO_ERROR         = 6,

  // 想定していないエラーです．ライブラリのバグである可能性があります．
  MARISA_UNEXPECTED_ERROR = 7
} marisa_status;
```

エラーの番号から名前を取得できるように，`marisa_strerror()` という関数を用意しています．引数が `MARISA_OK` であれば "MARISA\_OK" を返すという程度の味気ない関数ですが，多少は役に立つと思います．

```
const char *marisa_strerror(marisa_status status);
```


---


# C++ 用のインタフェース #

libmarisa を利用するときは，`marisa.h` をインクルードし，`marisa::Trie` というクラスを用いることになります．また，キーの ID として使用する型（`unsigned __int32` もしくは `uint32_t`）には，`marisa_uint32` および `marisa::UInt32` という別名が与えられています．以下，`marisa::Trie` の解説になっています．対応するヘッダは `marisa/trie.h` です．


---


## コンストラクタ ##

```
Trie();
```

`marisa::Trie` のコンストラクタはメンバを初期化するだけです．例外を投げることはありません．


---


## 構築 ##

### `build()` ###

```
void build(const char * const *keys, std::size_t num_keys,
    const std::size_t *key_lengths = NULL,
    const double *key_weights = NULL,
    UInt32 *key_ids = NULL, int flags = 0);
```

新しい辞書を構築します．構築の途中でメモリの確保に失敗した場合，もしくは辞書のサイズが上限（4GiB）を超えることが判明した場合，オブジェクトには変更を加えず，例外を投げるようになっています．

呼び出しに必要な引数は，キーの配列を指定する第 1 引数の `keys` と，キーの数を指定する第 2 引数の `num_keys` だけです．他の引数を省略した場合，デフォルトの設定にしたがって辞書を構築します．`build()` の内部でキーを整列するようになっているので，あらかじめ整列しておく必要はありません．

第 3 引数の `key_lengths` は，各キーの長さを指定するための引数です．省略した場合，あるいは `NULL` を渡した場合，各キーは `0` を終端とする文字列として扱われます．`0` をキーの一部として使用するには，`key_lengths` の指定が必要です．

第 4 引数の `key_weights` は，各キーの重みを指定するための引数です．省略した場合，あるいは `NULL` を渡した場合，すべてのキーに `1.0` の重みが与えられます．キーが重複しているときは重みが加算されることに注意してください．キーの重みは，ノードの配置順序を重み順（`MARISA_WEIGHT_ORDER`）にしたときにのみ利用される値です．例えば，各キーの出現頻度を指定することにより，通過しやすいノードを前方に配置し，検索時間を短縮することができます．

第 5 引数の `key_ids` は，各キーに割り当てられた ID を受け取るための引数です．有効なポインタを指定すれば，`keys[i]` の ID が `key_ids[i]` に格納されます．キー ID が不要なときは，引数を省略するか，`NULL` を指定してください．

第 6 引数の `flags` には，構築する辞書の設定を指定するようになっています．省略した場合，あるいは `0` を渡した場合，デフォルトの設定が使用されます．指定できる値は下表のとおりです．例えば，2 段のパトリシアトライにバイナリモードの TAIL，ノードの配置はラベル順という組み合わせであれば，`2 | MARISA_PATRICIA_TRIE | MARISA_BINARY_TAIL | MARISA_LABEL_ORDER` を指定します．省略された箇所はデフォルトの設定で埋めるようになっているので，変更したい箇所を指定するだけでも大丈夫です．

| **項目** | **値** | **概要** |
|:-------|:------|:-------|
| トライの数  | `0 - 255` | 辞書を構成するトライの数を指定します．（デフォルト：3） |
| トライの種類 | `MARISA_PATRICIA_TRIE` | パトリシアトライを使用します．（デフォルト） |
|        | `MARISA_PREFIX_TRIE` | プレフィックストライを使用します． |
| TAIL の種類 | `MARISA_WITHOUT_TAIL` | TAIL を使用しません． |
|        | `MARISA_BINARY_TAIL` | バイナリモードの TAIL を使用します． |
|        | `MARISA_TEXT_TAIL` | テキストモードの TAIL を使用します．（デフォルト） |
| ノードの配置順序 | `MARISA_LABEL_ORDER` | ノードの配置順序をラベル順にします． |
|        | `MARISA_WEIGHT_ORDER` | ノードの配置順序を重み順にします．（デフォルト） |

トライの数については，大きくするほど辞書が小さくなるものの，検索時間は悪化するという傾向があります．デフォルトの `3` という値は，単語や短いフレーズの集合から辞書を構築するのに適した設定になっています．見出し語・品詞・読みをつなげて登録する場合など，キーが複雑になるときは，`10` 程度の大きな値を指定することにより，辞書のサイズをさらに小さくできるかもしれません．適切な設定については，付属のツールである `marisa-benchmark` を使って調べることができます．

トライの種類にはパトリシアトライ（`MARISA_PATRICIA_TRIE`）とプレフィックストライ（`MARISA_PREFIX_TRIE`）の 2 種類が用意されていますが，基本的にパトリシアトライの方が優秀です．あえてプレフィックストライを選択する理由はほとんどありません．

TAIL の種類は 3 種類ありますが，多くの場合，テキストモード（`MARISA_TEXT_TAIL`）が最も優秀です．`0` をキーの一部として含むような辞書に有効な設定としてバイナリモード（`MARISA_BINARY_TAIL`）を用意していますが，テキストモードが無効なときは自動的にバイナリモードに切り替わるので，常にテキストモードを指定しておけば問題ありません．TAIL の利用は，辞書のサイズを小さくできるだけでなく，検索時間の短縮にもつながるため，`MARISA_WITHOUT_TAIL` を選択する理由はほとんどありません．

ノードの配置順序は，キーの重みをノードの配置に反映するための設定です．`MARISA_WEIGHT_ORDER` を指定することにより，通過しやすいノードを前方に配置し，検索時間を短縮することができます．各キーの出現頻度や出現割合を重みとして渡すことにより，検索時間をさらに短縮できる可能性があります．一方，`MARISA_LABEL_ORDER` については，検索結果としてキーをラベル順に受け取りたい場合などに使ってください．

```
void build(const std::vector<std::string> &keys,
    std::vector<UInt32> *key_ids = NULL, int flags = 0);
void build(const std::vector<std::pair<std::string, double> > &keys,
    std::vector<UInt32> *key_ids = NULL, int flags = 0);
```

キーと重みの指定およびキー ID の受け取りに STL コンテナを使用するという違いがあるだけです．キーの重みや辞書の設定については，上記の解説を参照してください．


---


## 入出力 ##

### `mmap()`, `map()` ###

```
bool mmap(Mapper *mapper, const char *filename,
    long offset = 0, int whence = SEEK_SET);
bool map(const void *ptr, std::size_t size);
bool map(Mapper &mapper);
```

Memory mapped I/O によりアドレス空間に割り当てられたデータやメモリ上に展開されたデータから辞書を復元します．データの変更やメモリの解放は致命的なエラーにつながるので注意してください．

`mmap()` はファイルのハンドルや Memory mapped I/O により得たアドレスを `mapper` に関連付けるようになっています．そのため，呼び出し側では，復元された辞書だけでなく，`mapper` も維持する必要があるので注意してください．

`offset` と `whence` は，`std::fseek()` の第 2 引数および第 3 引数と同じ役割を果たします．

### `load()`, `read()` ###

```
bool load(const char *filename, long offset = 0, int whence = SEEK_SET);
bool fread(::FILE *file);
bool read(int fd);
bool read(std::istream &stream);
bool read(Reader &reader);
```

辞書をファイルなどから入力します．入力元の指定には，ファイル名だけでなく，ファイルディスクリプタや `std::FILE *`, `std::ostream &` も利用できます．

`offset` と `whence` は，ファイルを開いた後で呼び出す `std::fseek()` の第 2 引数と第 3 引数になります．

### `save()`, `write()` ###

```
bool save(const char *filename, bool trunc_flag = true,
    long offset = 0, int whence = SEEK_SET) const;
bool fwrite(::FILE *file) const;
bool write(int fd) const;
bool write(std::ostream &stream) const;
bool write(Writer &writer) const;
```

辞書をファイルなどに出力します．出力先の指定には，ファイル名だけでなく，ファイルディスクリプタや `std::FILE *`, `std::istream &` も利用できます．

`trunc_flag` はファイルの切り詰めをおこなうかどうかを指定するフラグです．`true` にであれば切り詰めをおこない，`false` であれば古い内容を残して上書きします．

`offset` と `whence` は，ファイルを開いた後で呼び出す `std::fseek()` の第 2 引数と第 3 引数になります．


---


## 検索 ##

### `operator[]()` ###

```
std::string operator[](UInt32 key_id) const;
```

ID からキーを復元します．正体は `restore()` です．呼び出しのたびに `std::string` のオブジェクトを作成するので，オーバーヘッドが大きくなります．繰り返し呼び出す場合，第 2 引数を受け取る `restore()` の利用を検討してください．

```
UInt32 operator[](const char *str) const;
UInt32 operator[](const std::string &str) const;
```

キーから ID を求めます．正体は `lookup()` です．

### `restore()` ###

```
std::string restore(UInt32 key_id) const;
void restore(UInt32 key_id, std::string *key) const;
```

ID からキーを復元します．範囲外の ID を指定したときは例外を投げるようになっています．`std::string` のオブジェクトを返す前者の方が使いやすいものの，オーバーヘッドは大きくなります．パフォーマンスを重視するときは後者の利用を検討してください．

```
std::size_t restore(UInt32 key_id, char *key_buf,
    std::size_t key_buf_size) const;
```

ID から復元したキーを `key_buf` で指定した領域に格納し，キーの長さを返します．`key_buf_size` が十分に大きければ，キーを書き込んだ後，終端記号として `0` を加えるようになっています．`key_buf_size` が不足していた場合，呼び出し後に `key_buf` に書き込まれている内容は未定義ですが，返り値はキーの長さになることが保証されます．必要に応じて領域を拡張し，再び呼び出すことを検討してください．

### `lookup()` ###

```
UInt32 lookup(const char *str) const;
UInt32 lookup(const char *ptr, std::size_t length) const;
UInt32 lookup(const std::string &str) const;
```

指定された文字列がキーとして辞書に登録されているかどうかを確認して，登録されていれば当該キーの ID を返し，登録されていなければ `MARISA_NOT_FOUND`（`notfound()` の返り値と同じ）を返します．

### `find()` ###

```
std::size_t find(const char *str,
    UInt32 *key_ids, std::size_t *key_lengths,
    std::size_t max_num_results) const;
std::size_t find(const char *ptr, std::size_t length,
    UInt32 *key_ids, std::size_t *key_lengths,
    std::size_t max_num_results) const;
std::size_t find(const std::string &str,
    UInt32 *key_ids, std::size_t *key_lengths,
    std::size_t max_num_results) const;
```

指定された文字列の前方部分列と一致するキーを検索します．一致するキーを見つけたときは，`key_ids` と `key_lengths` が `NULL` でなければ，それぞれにキーの ID と長さを格納します．検索において見つけたキーの数が `max_num_results` に到達したときは，その時点で検索を中止し，`max_num_results` が返り値になります．`max_num_results` に到達しなかったときは，見つけたキーの数が返り値になります．

```
std::size_t find(const char *str,
    std::vector<UInt32> *key_ids = NULL,
    std::vector<std::size_t> *key_lengths = NULL,
    std::size_t max_num_results = MARISA_MAX_NUM_KEYS) const;
std::size_t find(const char *ptr, std::size_t length,
    std::vector<UInt32> *key_ids = NULL,
    std::vector<std::size_t> *key_lengths = NULL,
    std::size_t max_num_results = MARISA_MAX_NUM_KEYS) const;
std::size_t find(const std::string &str,
    std::vector<UInt32> *key_ids = NULL,
    std::vector<std::size_t> *key_lengths = NULL,
    std::size_t max_num_results = MARISA_MAX_NUM_KEYS) const;
```

キーの ID と長さを `key_ids` と `key_lengths` の末尾に追加するという違いがあるだけです．`max_num_results` や返り値については，上記の解説を参照してください．

### `find_first()`, `find_last()` ###

```
UInt32 find_first(const char *str,
    std::size_t *key_length = NULL) const;
UInt32 find_first(const char *ptr, std::size_t length,
    std::size_t *key_length = NULL) const;
UInt32 find_first(const std::string &str,
    std::size_t *key_length = NULL) const;
```

指定された文字列の前方部分列と一致するキーを検索して，最初に見つかった（一番短い）キーの ID を返します．`key_length` が `NULL` でなければ，見つかったキーの長さが格納されるようになっています．見つからなければ，返り値は `MARISA_NOT_FOUND` になり，`key_length` は変更されません．

```
UInt32 find_last(const char *str,
    std::size_t *key_length = NULL) const;
UInt32 find_last(const char *ptr, std::size_t length,
    std::size_t *key_length = NULL) const;
UInt32 find_last(const std::string &str,
    std::size_t *key_length = NULL) const;
```

指定された文字列の前方部分列と一致するキーを検索して，最後に見つかった（一番長い）キーの ID を返します．`key_length` が `NULL` でなければ，見つかったキーの長さが格納されるようになっています．見つからなければ，返り値は `MARISA_NOT_FOUND` になり，`key_length` は変更されません．

### `find_callback()` ###

```
// bool callback(UInt32 key_id, std::size_t key_length);
template <typename T>
std::size_t find_callback(const char *str, T callback) const;
template <typename T>
std::size_t find_callback(const char *ptr, std::size_t length,
    T callback) const;
template <typename T>
std::size_t find_callback(const std::string &str, T callback) const;
```

指定された文字列の前方部分列と一致するキーを検索して，キーが見つかるたびにコールバック関数（`callback`）を呼び出します．コールバック関数の第 1 引数にはキーの ID，第 2 引数にはキーの長さが渡されます．また，コールバック関数の返り値が `0` あるいは `false` であれば，検索を中断します．`find_callback()` の返り値はコールバック関数を呼び出した回数です．

### `predict()` ###

```
std::size_t predict(const char *str,
    UInt32 *key_ids, std::string *keys, std::size_t max_num_results) const;
std::size_t predict(const char *ptr, std::size_t length,
    UInt32 *key_ids, std::string *keys, std::size_t max_num_results) const;
std::size_t predict(const std::string &str,
    UInt32 *key_ids, std::string *keys, std::size_t max_num_results) const;
```

指定された文字列で始まるキーを検索します．一致するキーを見つけたときは，`key_ids` と `keys` が `NULL` でなければ，それぞれにキー ID と復元したキーを格納します．検索において見つけたキーの数が `max_num_results` に到達したときは，その時点で検索を中止し，`max_num_results` が返り値になります．`max_num_results` に到達しなかったときは，見つけたキーの数が返り値になります．

実際には，`keys` が `NULL` であれば `predict_breadth_first()` を呼び出し，そうでなければ `predict_depth_first()` を呼び出すようになっています．`keys` 次第で検索結果の順序が変化するので注意してください．

`keys` によって動作を切り替える理由は，キー ID のみを必要とする状況では `predict_breadth_first()` の方が高速になり，キー本体を必要とする状況では `predict_depth_first()` の方が高速になるためです．

```
std::size_t predict(const char *str,
    std::vector<UInt32> *key_ids = NULL,
    std::vector<std::string> *keys = NULL,
    std::size_t max_num_results = MARISA_MAX_NUM_KEYS) const;
std::size_t predict(const char *ptr, std::size_t length,
    std::vector<UInt32> *key_ids = NULL,
    std::vector<std::string> *keys = NULL,
    std::size_t max_num_results = MARISA_MAX_NUM_KEYS) const;
std::size_t predict(const std::string &str,
    std::vector<UInt32> *key_ids = NULL,
    std::vector<std::string> *keys = NULL,
    std::size_t max_num_results = MARISA_MAX_NUM_KEYS) const;
```

キー ID と復元しキーを `key_ids` と `keys` の末尾に追加するという違いがあるだけです．`max_num_results` や返り値については，上記の解説を参照してください．

### `predict_breadth_first()` ###

深さ優先な ID が欲しいときやキーを復元したいときは，後述の `predict()` を利用してください．

```
std::size_t predict_breadth_first(const char *str,
    UInt32 *key_ids, std::string *keys, std::size_t max_num_results) const;
std::size_t predict_breadth_first(const char *ptr, std::size_t length,
    UInt32 *key_ids, std::string *keys, std::size_t max_num_results) const;
std::size_t predict_breadth_first(const std::string &str,
    UInt32 *key_ids, std::string *keys, std::size_t max_num_results) const;

std::size_t predict_breadth_first(const char *str,
    std::vector<UInt32> *key_ids = NULL,
    std::vector<std::string> *keys = NULL,
    std::size_t max_num_results = MARISA_MAX_NUM_KEYS) const;
std::size_t predict_breadth_first(const char *ptr, std::size_t length,
    std::vector<UInt32> *key_ids = NULL,
    std::vector<std::string> *keys = NULL,
    std::size_t max_num_results = MARISA_MAX_NUM_KEYS) const;
std::size_t predict_breadth_first(const std::string &str,
    std::vector<UInt32> *key_ids = NULL,
    std::vector<std::string> *keys = NULL,
    std::size_t max_num_results = MARISA_MAX_NUM_KEYS) const;
```

指定された文字列で始まるキーを検索します．引数および返り値の役割については，`predict()` の解説を参照してください．

指定された文字列にしたがって探索した後，トライを幅優先に探索することで，該当するキーの ID を列挙します．幅優先に探索するため，キー ID が昇順に列挙されるという特徴があります．

`keys` が `NULL` であれば `predict_depth_first()` より高速に動作しますが，そうでない場合，キー ID からキーを復元するため，効率が著しく低下します．キーを復元しなければならない状況では，`predict_depth_first()` の利用を検討してください．

### `predict_depth_first()` ###

```
std::size_t predict_depth_first(const char *str,
    UInt32 *key_ids, std::string *keys, std::size_t max_num_results) const;
std::size_t predict_depth_first(const char *ptr, std::size_t length,
    UInt32 *key_ids, std::string *keys, std::size_t max_num_results) const;
std::size_t predict_depth_first(const std::string &str,
    UInt32 *key_ids, std::string *keys, std::size_t max_num_results) const;

std::size_t predict_depth_first(const char *str,
    std::vector<UInt32> *key_ids = NULL,
    std::vector<std::string> *keys = NULL,
    std::size_t max_num_results = MARISA_MAX_NUM_KEYS) const;
std::size_t predict_depth_first(const char *ptr, std::size_t length,
    std::vector<UInt32> *key_ids = NULL,
    std::vector<std::string> *keys = NULL,
    std::size_t max_num_results = MARISA_MAX_NUM_KEYS) const;
std::size_t predict_depth_first(const std::string &str,
    std::vector<UInt32> *key_ids = NULL,
    std::vector<std::string> *keys = NULL,
    std::size_t max_num_results = MARISA_MAX_NUM_KEYS) const;
```

指定された文字列で始まるキーを検索します．引数および返り値の役割については，`predict()` の解説を参照してください．

指定された文字列にしたがって探索した後，トライを深さ優先に探索することで，該当するキーの ID を列挙します．トライの探索と同時にキーを復元できるため，`keys` が `NULL` でないときは `predict_breadth_first()` より高速に動作します．

### `predict_callback()` ###

```
// bool callback(UInt32 key_id, const std::string &key);
template <typename T>
UInt32 predict_callback(const char *str, T callback) const;
template <typename T>
UInt32 predict_callback(const char *ptr, std::size_t length,
    T callback) const;
template <typename T>
UInt32 predict_callback(const std::string &str, T callback) const;
```

指定された文字列で始まるキーを検索して，該当するキーが見つかるたびにコールバック関数（`callback`）を呼び出します．コールバック関数の第 1 引数にはキーの ID，第 2 引数には復元されたキーが渡されます．また，コールバック関数の返り値が `0` あるいは `false` であれば，検索を中断します．`predict_callback()` の返り値はコールバック関数を呼び出した回数です．


---


## その他 ##

### `empty()` ###

```
bool empty() const;
```

辞書が空っぽかどうかを返します．登録されているキーの数が `0` であっても，構築済みの辞書であれば，返り値は `false` になります．`empty()` の返り値が `true` であれば，検索は不可能な状態です．

### `num_tries()`, `num_keys()`, `num_nodes()`, `size()` ###

```
std::size_t num_tries() const;
std::size_t num_keys() const;
std::size_t num_nodes() const;
std::size_t size() const;
```

上から順に，辞書を構成するトライの数，辞書に登録されているキーの数，辞書を構成するノードの数，出力時のサイズ（バイト単位）を返します．

### `clear()`, `swap()` ###

```
void clear();
```

辞書の内容を破棄し，初期状態に戻します．実際には，新しいオブジェクトを作成して，そのオブジェクトと現在のオブジェクトの内容を交換するようになっています．

```
void swap(Trie *rhs);
```

オブジェクトの中身を交換します．`rhs` が不正なアドレスを指していない限り，例外は発生しません．

### `notfound()`, `mismatch()` ###

```
static UInt32 notfound();
static std::size_t mismatch();

#define MARISA_UINT32_MAX ((marisa_uint32)-1)
#define MARISA_NOT_FOUND  MARISA_UINT32_MAX
#define MARISA_MISMATCH   MARISA_UINT32_MAX
```

`notfound()` と `mismatch()` は，それぞれ `MARISA_NOT_FOUND` と `MARISA_MISMATCH` を返します．notfound() と `MARISA_NOT_FOUND` のどちらを使うのかは気分次第で決めてください．

`MARISA_NOT_FOUND` は条件に該当するキーが見つからなかったことを示す定数です．一方，`MARISA_MISMATCH` はライブラリの内部で使用されている定数であり，現在のところ，外部で使う機会はありません．


---


# C 言語用のインタフェース #

C 言語用のインタフェースを利用するときは，`marisa.h` をインクルードします．gcc では C++ 用のライブラリをリンクするオプション（`-lstdc++`）が必要になるので注意してください．

辞書に対応する構造体は `marisa_trie` ですが，不完全な型になっているので，`marisa_trie` 型の変数を定義することはできません．`marisa_trie *` をハンドルとして，辞書の構築・検索や入出力などをおこなうようになっています．また，ほとんどの関数は，エラー番号（`marisa_status`）を返します．`MARISA_OK` 以外は関数の失敗を意味するので注意してください．


---


## 初期化・終了処理 ##

```
marisa_status marisa_init(marisa_trie **h);
```

辞書のオブジェクトを作成し，新しいオブジェクトへのポインタを `*h` に格納します．呼び出しの時点で `*h == NULL` でなければエラーになるので注意してください．また，作成したオブジェクトについては，`marisa_end()` によって破棄する必要があります．

`marisa_trie` は不完全な型なので，`marisa_trie` 型の変数を定義することはできません．辞書の構築・検索や入出力など，すべての操作は `marisa_init()` により作成したオブジェクトを用いるようになっています．

```
marisa_status marisa_end(marisa_trie *h);
```

`marisa_init()` によって作成されたオブジェクトを破棄します．


---


## 構築 ##

```
marisa_status marisa_build(marisa_trie *h, const char * const *keys,
    size_t num_keys, const size_t *key_lengths, const double *key_weights,
    marisa_uint32 *key_ids, int flags);
```

`marisa::Trie::build()` と対応しています．例外の代わりに `marisa_status` を返すという違いがあるだけで，引数の扱いや動作は同じです．


---


## 入出力 ##

```
marisa_status marisa_mmap(marisa_trie *h, const char *filename,
    long offset, int whence);
marisa_status marisa_map(marisa_trie *h, const void *ptr, size_t size);

marisa_status marisa_load(marisa_trie *h, const char *filename,
    long offset, int whence);
marisa_status marisa_fread(marisa_trie *h, FILE *file);
marisa_status marisa_read(marisa_trie *h, int fd);

marisa_status marisa_save(const marisa_trie *h, const char *filename,
    int trunc_flag, long offset, int whence);
marisa_status marisa_fwrite(const marisa_trie *h, FILE *file);
marisa_status marisa_write(const marisa_trie *h, int fd);
```

`marisa::Trie` のメンバ関数とそれぞれ対応しています．ただし，`std::istream &`, `std::ostream &` を受け取る関数はありません．


---


## 検索 ##

```
marisa_status marisa_restore(const marisa_trie *h, marisa_uint32 key_id,
    char *key_buf, size_t key_buf_size, size_t *key_length);
```

`marisa::Trie::restore()` と対応しています．ただし，復元されたキーの長さを `*key_length` に格納するという違いがあります．

```
marisa_status marisa_lookup(const marisa_trie *h,
    const char *ptr, size_t length, marisa_uint32 *key_id);
```

`marisa::Trie::lookup()` と対応しています．ただし，見つけたキーの ID を `*key_id` に格納するという違いがあります．

```
marisa_status marisa_find(const marisa_trie *h,
    const char *ptr, size_t length,
    marisa_uint32 *key_ids, size_t *key_lengths,
    size_t max_num_results, size_t *num_results);
```

`marisa::Trie::find()` と対応しています．ただし，見つけたキーの数を `*num_results` に格納するという違いがあります．

```
marisa_status marisa_find_first(const marisa_trie *h,
    const char *ptr, size_t length,
    marisa_uint32 *key_id, size_t *key_length);

marisa_status marisa_find_last(const marisa_trie *h,
    const char *ptr, size_t length,
    marisa_uint32 *key_id, size_t *key_length);
```

`marisa::Trie::find_first()`, `marisa::Trie::find_last()` と対応しています．ただし，見つけたキーの ID を `*key_id` に格納するという違いがあります．

```
marisa_status marisa_find_callback(const marisa_trie *h,
    const char *ptr, size_t length,
    int (*callback)(void *, marisa_uint32, size_t),
    void *first_arg_to_callback);
```

`marisa::Trie::find_callback()` と対応しています．ただし，コールバック関数に任意のポインタ（`first_arg_to_callback`）を渡せるようになっています．また，コールバック関数の呼び出し回数を返さないという違いがあります．

```
marisa_status marisa_predict(const marisa_trie *h,
    const char *ptr, size_t length, marisa_uint32 *key_ids,
    size_t max_num_results, size_t *num_results);

marisa_status marisa_predict_breadth_first(const marisa_trie *h,
    const char *ptr, size_t length, marisa_uint32 *key_ids,
    size_t max_num_results, size_t *num_results);

marisa_status marisa_predict_depth_first(const marisa_trie *h,
    const char *ptr, size_t length, marisa_uint32 *key_ids,
    size_t max_num_results, size_t *num_results);
```

`marisa::Trie::predict()` と対応しています．ただし，見つけたキーの数を `*num_results` に格納するという違いがあります．

```
marisa_status marisa_predict_callback(const marisa_trie *h,
    const char *ptr, size_t length,
    int (*callback)(void *, marisa_uint32, const char *, size_t),
    void *first_arg_to_callback);
```

`marisa::Trie::predict_callback()` と対応しています．ただし，コールバック関数に任意のポインタ（`first_arg_to_callback`）を渡せるようになり，文字列の表現方法が `const char *` と `size_t` の組に変化しています．また，コールバック関数の呼び出し回数を返さないという違いがあります．


---


## その他 ##

```
size_t marisa_get_num_tries(const marisa_trie *h);
size_t marisa_get_num_keys(const marisa_trie *h);
size_t marisa_get_num_nodes(const marisa_trie *h);
size_t marisa_get_total_size(const marisa_trie *h);
```

上から順に，辞書を構成するトライの数，辞書に登録されているキーの数，辞書を構成するノードの数，出力時のサイズ（バイト単位）を返します．ただし，`h == NULL` のときは `0` を返すようになっています．

```
marisa_status marisa_clear(marisa_trie *h);
```

辞書の内容を破棄し，初期状態に戻します．オブジェクトは有効なので，`marisa_end()` による終了処理が必要です．
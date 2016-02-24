# 使い方 #

marisa-trie の基本的な使い方に関するドキュメントです．

  * version: 0.1.5



## 互換性 ##

marisa-trie 0.1.0 には下位互換性があるため，0.0.1 で構築した辞書は 0.1.0 でも使用できます．0.1.0 で構築した辞書については，TAIL がバイナリモードでなければ，0.0.1 でも使用できます．また，marisa-trie 0.1.1 の辞書は 0.1.0 と同じなので，そのまま利用することができます．

marisa-trie 0.1.2 では辞書の書式が少し変更されています．互換性はないので，0.0.1 および 0.1.0, 0.1.1 で構築した辞書を使うことはできません．辞書の再構築が必要になります．また，marisa-trie 0.1.3, 0.1.4, 0.1.5 の辞書は 0.1.2 と同じなので，そのまま利用することができます．


---


## ビルド・インストール ##

### `gcc` ###

`g++` および `make` がインストールされている環境であれば，`configure` と `make` の組み合わせでビルド・インストールできます．

```
$ wget http://marisa-trie.googlecode.com/files/marisa-0.1.5.tar.gz
...
$ tar zxf marisa-0.1.5.tar.gz
$ cd marisa-0.1.5/
$ ./configure
...
$ make
...
$ make check
...
$ sudo make install
...
```

### Visual Studio ###

Visual Studio 2008 のソリューション（`.sln`）およびプロジェクト（`.vcproj`）が `vs2008/` 以下にあります．Visual Studio 2008 以降であれば，`vs2008/vs2008.sln` を開くだけでビルドできるようになっています．

Visual Studio 2008 以前の環境では，新しくプロジェクトを作る必要があります．プロジェクトを作成すれば問題なくビルドできると思いますが，試していないので断言はできません．


---


## ツール ##

marisa-trie に含まれるツールは，`marisa-build`, `marisa-lookup`, `marisa-find`, `marisa-predict`, `marisa-benchmark` の 5 つであり，以下のように動作します．

```
# 辞書を構築するツールです．
$ gzip -cd jawiki-20110129-all-titles-in-ns0.gz | marisa-build > jawiki.dic
#keys: 1174430
#tries: 3
#nodes: 2316597
size: 6806033

# 完全一致検索をおこなうツールです．
$ marisa-lookup jawiki.dic
あ
2608	あ

# 入力されたクエリの前半に含まれるキーを検索するツールです．
$ marisa-find jawiki.dic 
トライ
3 found
2490	ト	トライ
134510	トラ	トライ
510443	トライ	トライ

# 入力されたクエリで始まるキーを検索するツールです．
$ marisa-predict -n 3 jawiki.dic
あ
2405 found
2608	あ	あ
137194	あい	あ
875067	あいち健康の森	あ

# 辞書のサイズや構築・検索にかかる時間を計測するツールです．
$ gzip -cd jawiki-20110129-all-titles-in-ns0.gz | marisa-benchmark -n 3
#tries: 1 - 3
trie: patricia
tail: text
order: weight
predict: only IDs
#keys: 1174430
total length: 24438317
------+---------+---------+-------+-------+-------+-------+-------+-------
#tries    #nodes      size   build restore  lookup    find predict predict
                                                           breadth   depth
                   [bytes]   [K/s]   [K/s]   [K/s]   [K/s]   [K/s]   [K/s]
------+---------+---------+-------+-------+-------+-------+-------+-------
     1   1602151  10074222  305.05 1129.26 1003.79  910.41  869.95  804.40
     2   2136398   7355508  278.30  628.04  599.20  551.38  546.25  510.62
     3   2316597   6806033  266.92  556.60  531.42  497.64  487.32  458.76
------+---------+---------+-------+-------+-------+-------+-------+-------
```

各ツールに関するもう少し詳しい説明が ListOfTools にあります．marisa-benchmark をいくつかのキー集合に対して実行した結果が BenchmarkResults にあります．


---


## ライブラリ ##

marisa-trie のライブラリである libmarisa を用いるサンプルコードです．このサンプルでは，4 種類のキーを登録した辞書を構築し，構築された辞書からの検索をおこなっています．

```
// sample.cc
#include <iostream>
#include <string>
#include <vector>

// libmarisa の利用に必要なヘッダです．
#include <marisa.h>

int main() {
  // キー集合を整列しておく必要はなく，重複があっても特に問題ありません．
  // ただし，重複はキーの重みとして扱われるので，重複しているキーに対応する
  // ノードは前方に配置されやすくなります．
  std::vector<std::string> keys;
  keys.push_back("apple");
  keys.push_back("android");
  keys.push_back("market");
  keys.push_back("app");
  keys.push_back("android");

  // 辞書のクラスは marisa::Trie であり，build() という関数を使って辞書を
  // 構築するようになっています．第 1 引数はキー集合を渡すための引数，
  // 第 2 引数はキー ID を受け取るための引数です．キー ID が不要であれば，
  // 第 2 引数は省略することができます．
  marisa::Trie trie;
  std::vector<marisa::UInt32> key_ids;
  trie.build(keys, &key_ids);

  // lookup() はキー ID からキーを復元する関数であり，restore() はキーから
  // キー ID を得る関数です．
  std::cout << "lookup() and restore():\n";
  for (std::size_t i = 0; i < keys.size(); ++i) {
    std::cout << '(' << keys[i] << ", " << key_ids[i] << ") <-> ("
        << trie.lookup(keys[i]) << ", " << trie.restore(key_ids[i]) << ")\n";
  }
  std::cout << std::flush;

  // find() は第 1 引数として渡された文字列の前方部分列に一致するキーを検索する
  // 関数です．返り値は見つかったキーの数であり，各キーの ID は第 2 引数として
  // 渡したベクタの末尾に追加されます．
  std::cout << "find(\"apple\"):\n";
  std::vector<marisa::UInt32> found_key_ids;
  marisa::UInt32 num_keys = trie.find("apple", &found_key_ids);
  std::cout << num_keys << " keys found -";
  for (marisa::UInt32 i = 0; i < num_keys; ++i) {
    std::cout << " (" << found_key_ids[i] << ", "
        << trie.restore(found_key_ids[i]) << ')';
  }
  std::cout << std::endl;

  // predict() は第 1 引数として渡された文字列で始まるキーを検索する関数です．
  // 返り値は見つかったキーの数であり，各キーの ID および文字列は，それぞれ
  // 第 2 引数と第 3 引数に渡したベクタの末尾に追加されます．
  std::cout << "predict(\"a\"):\n";
  std::vector<std::string> found_keys;
  found_key_ids.clear();
  num_keys = trie.predict("a", &found_key_ids, &found_keys);
  std::cout << num_keys << " keys predicted - ";
  for (marisa::UInt32 i = 0; i < num_keys; ++i) {
    std::cout << " (" << found_key_ids[i] << ", "
        << trie.restore(found_key_ids[i]) << ')';
  }
  std::cout << std::endl;

  return 0;
}
```

ビルドするときは，libmarisa をリンクする必要があります．C 言語用のインタフェースを使う場合は `-lstdc++` も必要になるのでご注意ください．

```
$ g++ sample.cc -lmarisa

$ ./a.out
lookup() and restore():
(apple, 3) <-> (3, apple)
(android, 1) <-> (1, android)
(market, 0) <-> (0, market)
(app, 2) <-> (2, app)
(android, 1) <-> (1, android)
find("apple"):
2 keys found - (2, app) (3, apple)
predict("a"):
3 keys predicted -  (1, android) (2, app) (3, apple)
```

ライブラリのインタフェースに関する詳しい説明は LibraryInterface にあります．
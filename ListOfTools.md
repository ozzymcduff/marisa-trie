# ツール #

marisa-trie には 5 つのコマンドラインツールが含まれています．ツールの機能は，辞書の構築，辞書からの検索，およびに辞書のベンチマークとなっています．各ツールにはヘルプを表示するオプション（`-h`）があるので，有効に利用してください．

  * version: 0.1.5




---


## `marisa-build` ##

与えられたキー集合から辞書を構築するツールです．libmarisa の `marisa::Trie::build()` と対応します．

オプション以外の引数があれば，それらを入力ファイルの名前として扱います．入力ファイルの指定がなければ，標準入力からキー集合を読み込むようになっています．また，複数の入力ファイルが指定された場合，それらを連結して 1 つの辞書を構築します．入力するのは改行区切りのキー集合であり，水平タブ（`'\t'`）が含まれる行については，最後の水平タブ以降をキーの重みとして扱うようになっています．

構築した辞書は，特に指定がなければ標準出力へと書き出されます．出力先のファイルを指定したいときは，リダイレクトを利用するか，`-o, --output` オプションを利用してください．

```
$ marisa-build -h
Usage: marisa-build [OPTION]... [FILE]...

Options:
  -n, --num-tries=[N]  limits the number of tries to N (default: 3)
  -P, --patricia-trie  build patricia tries (default)
  -p, --prefix-trie    build prefix tries
  -T, --text-tail      build a dictionary with text TAIL (default)
  -b, --binary-tail    build a dictionary with binary TAIL
  -t, --without-tail   build a dictionary without TAIL
  -w, --weight-order   arranges siblings in weight order (default)
  -l, --label-order    arranges siblings in label order
  -o, --output=[FILE]  write tries to FILE (default: stdout)
  -h, --help           print this help
```


---


## `marisa-lookup` ##

完全一致検索をおこなうツールです．libmarisa の `marisa::Trie::lookup()` と対応します．

オプション以外に，辞書のファイル名を引数として受け取るようになっています．辞書の指定がない場合，もしくは複数の辞書が指定されたときはエラーとして扱うので注意してください．

```
$ marisa-lookup -h
Usage: marisa-lookup [OPTION]... DIC

Options:
  -m, --mmap-dictionary  use memory-mapped I/O to load a dictionary (default)
  -r, --read-dictionary  read an entire dictionary into memory
  -h, --help             print this help
```


---


## `marisa-find` ##

与えられた文字列の前半部分に一致するキーを検索するツールです．libmarisa の `marisa::Trie::find()` と対応します．

オプション以外に，辞書のファイル名を引数として受け取るようになっています．辞書の指定がない場合，もしくは複数の辞書が指定されたときはエラーとして扱うので注意してください．

```
$ marisa-find -h
Usage: marisa-find [OPTION]... DIC

Options:
  -n, --max-num-results=[N]  limits the number of results to N (default: 10)
                             0: no limit
  -a, --find-all         find all prefix keys (default)
  -f, --find-first       find a shortest prefix key
  -l, --find-last        find a longest prefix key
  -m, --mmap-dictionary  use memory-mapped I/O to load a dictionary (default)
  -r, --read-dictionary  read an entire dictionary into memory
  -h, --help             print this help
```


---


## `marisa-predict` ##

与えられた文字列で始まるキーを求めるツールです．libmarisa の `marisa::Trie::predict()` と対応します．

オプション以外に，辞書のファイル名を引数として受け取るようになっています．辞書の指定がない場合，もしくは複数の辞書が指定されたときはエラーとして扱うので注意してください．

```
$ marisa-predict -h
Usage: marisa-predict [OPTION]... DIC

Options:
  -n, --max-num-results=[N]  limits the number of outputs to N (default: 10)
                             0: no limit
  -d, --depth-first      predict keys in depth first order(default)
  -b, --breadth-first    predict keys in breadth first order
  -m, --mmap-dictionary  use memory-mapped I/O to load a dictionary (default)
  -r, --read-dictionary  read an entire dictionary into memory
  -h, --help             print this help
```


---


## `marisa-benchmark` ##

与えられたキー集合について，トライの数を変化させつつ，辞書のサイズや構築時間・検索時間を計測するツールです．どのような設定が適切かを調べるのに使えます．

オプション以外の引数があれば，それらを入力ファイルの名前として扱います．入力ファイルの指定がなければ，標準入力からキー集合を読み込むようになっています．また，複数の入力ファイルが指定された場合，それらを連結してベンチマークをおこないます．入力するのは改行区切りのキー集合であり，水平タブ（`'\t'`）が含まれる行については，最後の水平タブ以降をキーの重みとして扱うようになっています．

```
$ marisa-benchmark -h
Usage: marisa-benchmark [OPTION]... [FILE]...

Options:
  -N, --min-num-tries=[N]  limits the number of tries to N (default: 1)
  -n, --max-num-tries=[N]  limits the number of tries to N (default: 10)
  -P, --patricia-trie  build patricia tries (default)
  -p, --prefix-trie    build prefix tries
  -T, --text-tail      build a dictionary with text TAIL (default)
  -b, --binary-tail    build a dictionary with binary TAIL
  -t, --without-tail   build a dictionary without TAIL
  -w, --weight-order   arrange siblings in weight order (default)
  -l, --label-order    arrange siblings in label order
  -I, --predict-ids    get key IDs in predictive searches (default)
  -i, --predict-strs   restore key strings in predictive searches
  -S, --print-speed    print speed [1000 keys/s] (default)
  -s, --print-time     print time [us/key]
  -h, --help           print this help
```
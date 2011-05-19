use marisa;

$keyset = new marisa::Keyset;
$keyset->push_back("apple");
$keyset->push_back("orange");

$trie = new marisa::Trie;
$trie->build($keyset);

$agent = new marisa::Agent;

$agent->set_query("apple");
print "apple: ", $trie->lookup($agent), "\n";

$agent->set_query("banana");
print "banana: ", $trie->lookup($agent), "\n";

$agent->set_query("orange");
print "orange: ", $trie->lookup($agent), "\n";

$agent->set_query("");
while ($trie->predictive_search($agent)) {
  print "predictive_search: ", $agent->key()->str(), "\n";
}

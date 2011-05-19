import marisa

keyset = marisa.Keyset()
keyset.push_back("apple");
keyset.push_back("orange");

trie = marisa.Trie()
trie.build(keyset)

agent = marisa.Agent()

agent.set_query("apple")
print("apple: %s" % trie.lookup(agent))

agent.set_query("banana")
print("banana: %s" % trie.lookup(agent))

agent.set_query("orange")
print("orange: %s" % trie.lookup(agent))

agent.set_query("")
while trie.predictive_search(agent):
  print("predictive_search: %s" % agent.key().str())

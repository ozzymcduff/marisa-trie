require "marisa"

keyset = Marisa::Keyset.new
keyset.push_back("apple")
keyset.push_back("orange")

trie = Marisa::Trie.new
trie.build(keyset)

agent = Marisa::Agent.new

agent.set_query("apple")
print("apple: ", trie.lookup(agent), "\n")

agent.set_query("banana")
print("banana: ", trie.lookup(agent), "\n")

agent.set_query("orange")
print("orange: ", trie.lookup(agent), "\n")

agent.set_query("")
while trie.predictive_search(agent)
  print("predictive_search: ", agent.key().str(), "\n")
end

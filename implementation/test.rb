require 'graphviz'

def test
    g = GraphViz.new :DFA, :type => :digraph
    g[:rankdir] = "LR"
    g.node[:fixedsize] = true
    n1 = g.add_node "n1", :shape => :circle
    n2 = g.add_node "n2", :shape => :circle
    n3 = g.add_node "n3", :shape => :circle

    g.add_edge(n1, n2).label = "a"
    g.add_edge(n2, n3).label = "b"
    g.add_edge(n3, n1).label = "c"

    #file = Tempfile.new('test.png')
    file = "test.dot"
    g.output :dot => file
end
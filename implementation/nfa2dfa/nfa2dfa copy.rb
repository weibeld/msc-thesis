require 'yaml'
require 'nokogiri'
require 'set'
require 'graphviz' # Requires installation of gem ruby-graphviz and of Graphviz

class DFA
    attr_accessor :states, :alphabet, :transitions, :start_state, :accept_states    

    def initialize(file=nil)
        if file == nil then return end
        yaml = YAML::load_file(file)
        @states = yaml['states']
        @alphabet = yaml['alphabet']
        @transitions = format_transitions(yaml['transitions'])
        @start_state = yaml['start_state']
        @accept_states = yaml['accept_states']
    end

    def accept?(string)
        current_state = @start_state
        string.each_char { |symbol| current_state = @transitions[current_state][symbol] }
        @accept_states.include? current_state
    end

    def to_yaml(filename='dfa.yaml')
        h = Hash.new
        h['states'] = @states
        h['alphabet'] = @alphabet
        h['start_state'] = @start_state
        h['accept_states'] = @accept_states
        h['transitions'] = @transitions
        File.open(filename, 'w') { |file| file.write h.to_yaml }
    end

    def visualize(filename="dfa.pdf")
        visualize_automaton self, filename
    end
end

class NFA
    public

    attr_accessor :states, :alphabet, :transitions, :start_state, :accept_states

    def initialize(file)
        yaml = YAML::load_file(file)
        @states = yaml['states']
        @alphabet = yaml['alphabet']
        @transitions = format_transitions(yaml['transitions'])
        @start_state = yaml['start_state']
        @accept_states = yaml['accept_states']
    end          

    def accept?(word)
        current_states = Set[@start_state]
        word.each_char do |symbol|
            new_states = Set.new
            current_states.each do |state|
                if @transitions[state][symbol] != nil
                    new_states.merge @transitions[state][symbol]
                end
            end
            current_states = new_states
        end
        not (@accept_states & current_states).empty?
    end

    def determinize
        dfa_states = [Set[@start_state]]
        dfa_transitions = Hash.new
        dfa_states.each do |current_state|
            transitions_of_current_state = Hash.new
            @alphabet.each do |symbol|
                target_state = SortedSet.new
                current_state.each do |component|
                    target_state.merge transition_function(component, symbol)
                end
                if target_state.empty? then next end
                transitions_of_current_state[symbol] = target_state.stringify
                dfa_states << target_state unless dfa_states.include? target_state
            end
            dfa_transitions[current_state.stringify] = transitions_of_current_state
        end
        dfa_accept_states = []
        dfa_states.each do |s|
            if not (s & @accept_states).empty? then dfa_accept_states << s.stringify end
        end
        
        # Create DFA object
        dfa = DFA.new
        dfa.states = dfa_states.map { |set| set.stringify }
        dfa.alphabet = @alphabet
        dfa.transitions = dfa_transitions
        dfa.start_state = Set[@start_state].stringify
        dfa.accept_states = dfa_accept_states
        dfa
    end

    def visualize(filename="nfa.pdf")
        visualize_automaton self, filename
    end

    def transition_function(state, symbol)
        s = Set.new
        if @transitions[state]
            @transitions[state].each do |successor, symbols|
                if symbols.include? symbol then s.add successor end
            end
        end
        s
    end
end

class Set
    def stringify 
        s = "{"
        self.each { |e| s << "#{e}," }
        s = s.chomp(",") << "}"
    end
end

# Transform
#   ["p -> q: a,b",
#    "p -> r: a",
#    "q -> r: b"]
# into
#   {"p" => {"q" => Set{"a","b"}, "r" => Set{"a"}},
#    "q" => {"r" => Set{"b"}}}
def format_transitions(array)
    transitions = {}
    array.each do |edge|
        states = edge.split(":")[0].split("->")
        from = states[0].strip
        to = states[1].strip
        symbols = edge.split(":")[1].split(",").each { |sym| sym.strip! }.to_set
        if transitions[from]
            if transitions[from][to]
                transitions[from][to].merge symbols
            else
                transitions[from][to] = symbols
            end
        else
            transitions[from] = {to => symbols}
        end
    end
    transitions
end

def visualize_automaton(automaton, filename)
    g = GraphViz.new :DFA, :type => :digraph
    g[:rankdir] = "LR"
    g.node[:fixedsize] = true
    h = {}
    automaton.states.each do |state|
        if @accept_states.include? state
            h[state] = g.add_node state, :shape => :doublecircle
        else
            h[state] = g.add_node state, :shape => :circle
        end
    end
    g.add_edge(g.add_node("start", :shape => :plaintext), h[automaton.start_state])
    automaton.transitions.each do |a, transitions|
        hh = {}
        transitions.each do |symbol, b|
            if not b.respond_to? :each then b = Array[b] end
            b.each do |b|
            if hh[b] == nil
                (hh[b] = g.add_edge(h[a], h[b])).label = symbol
            else
                hh[b].label = hh[b][:label].to_s.tr('"','') << ",#{symbol}"
            end
            end
        end
    end
    g.output :pdf => filename
end

def yaml_to_goal(yaml_file, goal_file)
    yaml = YAML::load_file(yaml_file)
    builder = Nokogiri::XML::Builder.new do |xml|
        xml.Structure(:'label-on' => "Transition", :type => "FiniteStateAutomaton") {
            xml.Alphabet(:type => "Classical") {
                yaml['alphabet'].each do |symbol|
                    xml.Symbol symbol.to_s
                end
            }
            xml.StateSet {
                yaml['states'].each_with_index do |state, index|
                    xml.State(:sid => index)
                end
            }
            xml.InitiaStateSet {
                xml.StateID yaml['states'].index(yaml['start_state'])
            }
            xml.TransitionSet(:complete => "false") {
                x
            }
        }
    end
    File.open(goal_file, 'w') { |file| file.write builder.to_xml }
    puts builder.to_xml
end

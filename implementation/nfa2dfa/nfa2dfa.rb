require 'yaml'
require 'set'
require 'graphviz' # Requires installation of gem ruby-graphviz and of Graphviz

class DFA
    attr_accessor :states, :alphabet, :transitions, :start_state, :accept_states    

    def initialize(file=nil)
        if file == nil then return end
        yaml = YAML::load_file(file)
        @states = yaml['states']
        @alphabet = yaml['alphabet']
        @transitions = yaml['transitions']
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
        @transitions = yaml['transitions']
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

    private

    def transition_function(state, symbol)
        if @transitions[state] == nil then return Set.new
        elsif @transitions[state][symbol] == nil then return Set.new
        else return @transitions[state][symbol].to_set end
    end
end

class Set
    def stringify 
        s = "{"
        self.each { |e| s << "#{e}," }
        s = s.chomp(",") << "}"
    end
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
            #if not bs.respond_to? :each then bs = Array[bs] end
            #s.each do |b|
            if hh[b] == nil
                (hh[b] = g.add_edge(h[a], h[b])).label = symbol
            else
                hh[b].label = hh[b][:label].to_s.tr('"','') << ",#{symbol}"
            #end
            end
        end
    end
    g.output :pdf => filename
end

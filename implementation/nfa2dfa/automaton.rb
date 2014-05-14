require 'yaml'
require 'nokogiri'
require 'set'
require 'graphviz' # Requires installation of gem ruby-graphviz and of Graphviz

class Automaton
  attr_accessor :states, :alphabet, :transitions, :start_state, :accept_states

  # Creates an automaton either from an initilization hash or a YAML string.
  def initialize(arg)
    if arg.is_a?(Hash)
      @states = arg['states']
      @alphabet = arg['alphabet']
      @transitions = arg['transitions']
      @start_state = arg['start_state']
      @accept_states = arg['accept_states']
    elsif arg.is_a?(String)
      yaml = YAML::load(arg)
      @states = yaml['states']
      @alphabet = yaml['alphabet']
      @transitions = machinize_transitions(yaml['transitions'])
      @start_state = yaml['start_state']
      @accept_states = yaml['accept_states']
    end        
  end

  # Returns YAML string of automaton.
  def to_yaml
    h = Hash.new
    h['states'] = @states
    h['alphabet'] = @alphabet
    h['start_state'] = @start_state
    h['accept_states'] = @accept_states
    h['transitions'] = humanize_transitions(@transitions)
    h.to_yaml
  end

  # Returns true if automaton is determinstic and false if automaton is
  # non-deterministic.
  def is_deterministic
    @states.each do |state|
      @alphabet.each do |symbol|
        if transition_function(state, symbol).size > 1 then return false end
      end
    end
    return true
  end

  
  def accept?(word)
    tmp_file = to_goal_file
    # goal input returns either:
    #   (true, s0 s2 s4 { s1 })
    #   (false, null)
    #   The inpute sequence is invalid.
    result = `goal #{tmp_file} input #{word}`
    if result.include?("true") then return true
    elsif result.include?("false") then return false
    else return false
    end 
    `rm #{tmp_file}`
  end

  
  # Generates and returns a random automaton.
  def self.generate_random
  end
  
  

private

  # Returns a new automaton object from a GOAL file string.
  def self.create_new_from_goal(string)
    
  end

  # Creates a temporary GOAL file of the automaton and returns the filename.
  def to_goal_file
  end


  # Returns the set of new states when the automaton is in state <state> and
  # reads symbol <symbol>. The returned set may contain 0, 1, or more states.
  def transition_function(state, symbol)
    set = Set.new
    if !@transitions[state] return set
    @transitions[state].each do |successor, symbols|
      if symbols.include? symbol then set << successor end
    end
    set
  end

  # Transform
  #   {"p" => {"q" => Set{"a","b"}, "r" => Set{"a"}},
  #    "q" => {"r" => Set{"b"}}}
  # into
  #   ["p -> q: a,b",
  #    "p -> r: a",
  #    "q -> r: b"]
  def humanize_transitions(hash)
    transitions = []
    hash.each do |from, tos|
      tos.each do |to, symbol_set|
        string = "#{from} -> #{to}: "
        symbol_set.each do |symbol|
          string << "#{symbol},"
        end
        transitions << string.chomp(',')
      end
    end
    transitions
  end

  # Transform
  #   ["p -> q: a,b",
  #    "p -> r: a",
  #    "q -> r: b"]
  # into
  #   {"p" => {"q" => Set{"a","b"}, "r" => Set{"a"}},
  #    "q" => {"r" => Set{"b"}}}
  def machinize_transitions(array)
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
end

class DFA
    

    def accept?(string)
        current_state = @start_state
        string.each_char { |symbol| current_state = @transitions[current_state][symbol] }
        @accept_states.include? current_state
    end

    

    def visualize(filename="dfa.svg")
        visualize_automaton self, filename
    end
end

class NFA
    public

    attr_accessor :states, :alphabet, :transitions, :start_state, :accept_states         

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

    def visualize(filename="nfa.svg")
        visualize_automaton self, filename
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
    g.output :svg => filename
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
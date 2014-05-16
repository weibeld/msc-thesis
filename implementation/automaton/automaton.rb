require 'yaml'
require 'nokogiri'
require 'set'
require 'graphviz' # Requires installation of gem ruby-graphviz and of Graphviz
require 'tempfile'

class Automaton

  # Creates an automaton either from an initilization hash or a YAML string.
  def initialize(arg)
    if arg.is_a?(Hash)
      @states = arg['states']
      @alphabet = arg['alphabet']
      @start_state = arg['start_state']
      @accept_states = arg['accept_states']
      @transitions = arg['transitions']
    elsif arg.is_a?(String)
      #yaml = YAML::load(arg)
      yaml = YAML::load_file(arg) # For testing
      @states = yaml['states']
      @alphabet = yaml['alphabet']
      @start_state = yaml['start_state']
      if @start_state.is_a?(Array) then @start_state = @start_state[0] end
      @accept_states = yaml['accept_states']
      @transitions = machinize_transitions(yaml['transitions'])
    end        
  end

  # Returns the set of new states when the automaton is in state <state> and
  # reads symbol <symbol>. The returned set may contain 0, 1, or more states.
  def transition_function(state, symbol)
    set = SortedSet.new
    if !@transitions[state] then return set end
    @transitions[state].each do |successor, symbols|
      if symbols.include? symbol then set << successor end
    end
    set
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
    file_name = to_goal_file
    # goal input returns either:
    #   (true, s0 s2 s4 { s1 })
    #   (false, null)
    #   The inpute sequence is invalid.
    result = `goal #{file_name} input #{word}`
    if result.include?("true") then return true
    elsif result.include?("false") then return false
    else return "invalid"
    end 
    `rm #{file_name}`
  end

  def is_accepting(state)
    @accept_states.include? state
  end

  def index(state)
    return @states.index(state)
  end

  # Returns YAML string of automaton.
  def to_yaml
    h = Hash.new
    h['states'] = @states
    h['alphabet'] = @alphabet
    h['start_state'] = [@start_state]
    h['accept_states'] = @accept_states
    h['transitions'] = humanize_transitions(@transitions)
    h.to_yaml
  end

  # Transform
  #   ["p -> q: a,b", "p -> r: a", "q -> r: b"]
  # into
  #   {"p" => {"q" => Set{"a","b"}, "r" => Set{"a"}}, "q" => {"r" => Set{"b"}}}
  def machinize_transitions(array)
    transitions = {}
    array.each do |edge|
      states = edge.split(":")[0].split("->")
      from = states[0].strip
      to = states[1].strip
      symbols = edge.split(":")[1].split(",").each { |sym| sym.strip! }.to_set
      transitions = self.class.add_transition(transitions, from, to, symbols)
    end
    transitions
  end

  # Transform
  #   {"p" => {"q" => Set{"a","b"}, "r" => Set{"a"}}, "q" => {"r" => Set{"b"}}}
  # into
  #   ["p -> q: a,b", "p -> r: a", "q -> r: b"]
  def humanize_transitions(hash)
    transitions = []
    hash.each do |from, tos|
      tos.each do |to, symbol_set|
        string = "#{from} -> #{to}: "
        symbol_set.each do |symbol| string << "#{symbol}," end
        transitions << string.chomp(',')
      end
    end
    transitions
  end

  def self.add_transition(hash, from, to, sym)
    if !sym.is_a?(Set) then sym = SortedSet.new([sym]) end
    if !hash[from] then hash[from] = {to => sym}
    elsif !hash[from][to] then hash[from][to] = sym
    elsif hash[from][to] then hash[from][to].merge sym end
    hash
  end

  # Generates and returns a random automaton.
  def self.generate_random
    create_new_from_goal(`goal generate -A classical`)
  end

  # Returns a new automaton object from a GOAL file string.
  def self.create_new_from_goal(string)
    document = Nokogiri::XML(string)

    states = []
    document.xpath("//StateSet/State").each do |state|
      states << state.get_attribute("sid")
    end

    alphabet = []
    document.xpath("//Alphabet/Symbol").each do |symbol|
      alphabet << symbol.content
    end

    start_state = document.xpath("//InitialStateSet/StateID").first.content

    accept_states = []
    document.xpath("//Acc/StateID").each do |state|
      accept_states << state.content
    end

    transitions = {}
    document.xpath("//TransitionSet/Transition").each do |transition|
      from = transition.xpath("./From").first.content
      to = transition.xpath("./To").first.content
      symbol = transition.xpath("./Label").first.content
      transitions = add_transition(transitions, from, to, symbol)
    end

    self.new({"states"=>states, "alphabet"=>alphabet,
              "start_state"=>start_state, "accept_states"=>accept_states,
              "transitions"=>transitions})
  end
  
  # Translates the automaton to the GOAL XML format (GFF), writes it to a
  # temporary file, and returns the filename.
  def to_goal_file
    builder = Nokogiri::XML::Builder.new do |xml|
      xml.Structure(:'label-on' => "Transition", :type => "FiniteStateAutomaton") {
        xml.Alphabet(:type => "Classical") {
          @alphabet.each do |symbol| xml.Symbol symbol.to_s end
        }
        xml.StateSet {
          @states.each do |state| xml.State(:sid => index(state)) end
        }
        xml.InitialStateSet {
          xml.StateID index(@start_state)
        }
        xml.TransitionSet(:complete => "false") {
          tid = 0
          @transitions.each do |from, tos|
            tos.each do |to, symbols|
              symbols.each do |symbol|
                xml.Transition(:tid => tid) {
                  xml.From index(from)
                  xml.To index(to)
                  xml.Label symbol
                }
                tid = tid + 1
              end
            end
          end
        }
        xml.Acc(:type => "Buchi") {
          @accept_states.each do |state| xml.StateID index(state) end
        }
      }
    end
    file = File.open(`mktemp`, "w")   # Default location for mktemp is /tmp
    file.write(builder.to_xml)
    file.close
    file.path
  end
  
  def draw(filename)
    g = GraphViz.new :Automaton, :type => :digraph
    g[:rankdir] = "LR"
    g.node[:fixedsize] = true

    nodes = {}
    @states.each do |state|
      nodes[state] = g.add_node(state)
      if is_accepting(state) then nodes[state].shape = :doublecircle
      else nodes[state].shape = :circle end
    end

    g.add_edge(g.add_node("", :shape => :plaintext), nodes[@start_state])

    @transitions.each do |from, tos|
      tos.each do |to, symbols|
        g.add_edge(nodes[from], nodes[to]).label = self.class.format_set(symbols, false)
      end
    end
    g.output(:pdf => filename)
  end

  def subset_construction
    out_start_state = Set[@start_state]
    out_states = [out_start_state]
    out_accept_states = []
    if is_accepting(@start_state) then out_accept_states << out_start_state end
    out_transitions = {}

    out_states.each do |state|
      @alphabet.each do |symbol|
        successor = SortedSet.new
        state.each do |element|
            successor.merge(transition_function(element, symbol))
        end
        if successor.empty? then next end
        self.class.add_transition(out_transitions, self.class.format_set(state),
          self.class.format_set(successor), symbol)
        if !out_states.include? successor then out_states << successor end
        if !(successor & @accept_states).empty? then out_accept_states << successor end
      end
    end

    out_states.map! { |set| self.class.format_set(set) }
    out_accept_states.map! { |set| self.class.format_set(set) }
    out_start_state = self.class.format_set(out_start_state)

    self.class.new({"states"=>out_states, "alphabet"=>@alphabet,
      "start_state"=>out_start_state, "accept_states"=>out_accept_states,
      "transitions"=>out_transitions})
  end
  
  def self.format_set(set, braces=true)
    if braces then s = "{" else s = "" end
    set.each do |elt| s << "#{elt}," end
    s = s.chomp(",")
    if braces then s << "}" end
    s
  end

end

require 'yaml'
require 'set'

class NFA
    attr_accessor :transitions
    def initialize(file)
        yaml = YAML::load_file(file)
        @states = yaml['states']
        @alphabet = yaml['alphabet']
        @transitions = yaml['transitions']
        #@transitions.each do |k0, v0|
        #    v0.each { |k1, v1| @transitions[k0][k1] = v1.to_set }
        #end
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

    def transition_function(state, symbol)
        if @transitions[state] == nil then return Set.new
        elsif @transitions[state][symbol] == nil then return Set.new
        else return @transitions[state][symbol].to_set end
    end

    def determinize
        dfa_states = [Set[@start_state]]
        dfa_transitions = Hash.new
        dfa_states.each do |current_state|
            transitions_of_current_state = Hash.new
            @alphabet.each do |symbol|
                target_state = Set.new
                current_state.each do |component|
                    target_state.merge transition_function(component, symbol)
                end
                #print "Target state: {"
                #target_state.each { |elt| print elt }
                #puts "}"
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
end


class Set
    def stringify 
        s = "{"
        self.each { |e| s << "#{e}," }
        s = s.chomp(",") << "}"
    end
end
require 'yaml'

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

    def to_yaml
        h = Hash.new
        h['states'] = @states.to_a
        h['alphabet'] = @alphabet.to_a
        h['start_state'] = @start_state.class == Set ? @start_state.print : @start_state
        h['accept_states'] = 
        h['transitionyamlize_transitions
        s << (@start_state.class == Set ? @start_state.print : @start_state).to_yaml
        s << @accept_states.to_a
    end
end
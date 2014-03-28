require 'yaml'
require 'forwardable'

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
end

class State
    include Enumerable
    extend Forwardable

    def initialize(state=nil)
        if state == nil then @state = Set.new
        else @state = state end
    end

    def_delegators :@state, :each, :merge

    def to_s
        if state.class == String then state
        elsif state.class == Set
            s = "{"
            self.each { |e| s << "#{e}," }
            s = s.chomp(",") << "}"
        else
        end
    end

end
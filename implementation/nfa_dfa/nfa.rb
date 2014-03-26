require 'yaml'
require 'set'

class NFA
    def initialize(file)
        yaml = YAML::load_file(file)
        @states = yaml['states']
        @alphabet = yaml['alphabet']
        @transitions = yaml['transitions']
        @start_state = yaml['start_state']
        @accept_states = yaml['accept_states']
    end

    def accept?(string)
        current_states = Set[@start_state]
        string.each_char do |symbol|
            new_states = []
            current_states.each do |state|
                successors = @transitions[state][symbol]
                new_states << successors  unless successors == nil
            end
            current_states = new_states.flatten.to_set
        end
        not (@accept_states & current_states).empty?
    end
end